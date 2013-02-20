#include "StdAfx.h"
#include "FrameWork.Communication3.h"

namespace FC3i  = FrameWork::Communication3::implementation;
namespace FC3ir = FrameWork::Communication3::implementation::remote;

using namespace FC3ir;

// Constructor
server::server( const int port_no )
	:	m_port_no( port_no ), 
		m_listening( false ),
		m_error( false )
{	// Create the thread
	m_hThread = ::CreateThread( NULL, 8192, g_thread_proc, (void*)this, 0, NULL );
	assert( m_hThread );	

	// Wait until we are listening
	while( ( !m_listening ) && ( !m_error ) ) ::Sleep( 10 );
}

// Destructor
server::~server( void )
{	// Close the socket
	m_server_socket.closesocket();
	
	// Wait for the thread to exit
	::WaitForSingleObject( m_hThread, INFINITE );
	::CloseHandle( m_hThread );

	// Close all connections
	for( int i=0; i<(int)m_connections_list.size(); i++ )
		// Destroy the connection
		delete m_connections_list[ i ];
}

const bool server::error( void ) const
{	return m_error;
}

// The thread callback
DWORD server::g_thread_proc( void* p_data )
{	return ( (server*)p_data )->thread_proc();
}

DWORD server::thread_proc( void )
{	// The master server thread
	set_thread_name( "FC3remote[master]" );

	// Create a socket description
	sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons( m_port_no );	

	// Bind the socket
	if ( !m_server_socket.bind( (sockaddr*)&sin, sizeof( sin ) ) ) goto error;

	// Listen for a connection
	if ( !m_server_socket.listen( SOMAXCONN ) ) goto error;
	m_listening = true;

	// Wait for connections
	SOCKET client_socket;
	int	   size_sockaddr_in = sizeof( sin );
	while( ( client_socket = m_server_socket.accept( (sockaddr*)&sin, &size_sockaddr_in ) ) != INVALID_SOCKET )
	{	// Set the oprtions on the client socket
		int nOne_ = 1;
		setsockopt( client_socket, IPPROTO_TCP, TCP_NODELAY, (char*) &nOne_, sizeof(nOne_) );

		// Simply shutting down will leave the socket in a TIME_WAIT state (as seen in netstat /a).
		// We have received everything that is to be sent, we don't want the socket to anymore.
		LINGER linger_;
		linger_.l_linger = 0;
		linger_.l_onoff = 1;
		setsockopt( client_socket, SOL_SOCKET, SO_LINGER, (const char *) &linger_, sizeof(linger_) );
		
		// Lock the list of connections
		FC3::implementation::auto_lock	lock( m_connections_lock );
		
		// Create a new connection
		m_connections_list.push_back( new connection( this, client_socket ) );

		// Clean up any bad connections that are left over.
		for( int i=0; i<(int)m_connections_list.size(); )
		if ( m_connections_list[ i ]->error() )
		{	// Destroy the connection
			delete m_connections_list[ i ];

			// Remove it from the list
			m_connections_list[ i ] = m_connections_list.back();
			m_connections_list.pop_back();
		}
		else i++;

		// Start listening again
		m_server_socket.listen( SOMAXCONN );
	}

	// Error
error:
	m_error = true;

	// Finished
	return 0;
}

// Constructor
server::connection::connection( server* p_parent, SOCKET client_socket )
	:	m_client_socket( client_socket ),
		m_p_parent( p_parent ),
		m_error( false )
{	// Create the thread. 
	// Note that we do not need to keep the handle around. This looks crazy, but it is not.
	m_hThread = ::CreateThread( NULL, 8192, g_thread_proc, (void*)this, 0, NULL );
	assert( m_hThread );

	// We want to receive messages quickly. We are not time-critical but quite high.
	::SetThreadPriority( m_hThread, THREAD_PRIORITY_HIGHEST );
}

// Destructor
server::connection::~connection( void )
{	// Close the socket
	m_client_socket.closesocket();
	
	// Wait for the thread to exit
	::WaitForSingleObject( m_hThread, INFINITE );
}

// Has there been an error
const bool server::connection::error( void ) const
{	return m_error;
}

// Listen for data
DWORD WINAPI server::connection::g_thread_proc( void* p_data )
{	return ( (connection*)p_data )->thread_proc();
}

DWORD server::connection::thread_proc( void )
{	// The master server thread
	set_thread_name( "FC3remote[connection]" );
	
	// Wait for data
	while( true )
	{	// Get the header data
		tcpip_message_header header;
		if ( !m_client_socket.recv( (char*)&header, sizeof(header), 0 ) ) break;

		// Version support
		if ( header.m_version_no != 1 ) { assert( false ); continue; }

		// This must be a send message
		if ( header.m_message_type != FC3ir::tcpip_message_header::message_type_send ) { assert( false ); continue; }
		
		// Get the destination name
		wchar_t *p_dst_name = (wchar_t*)_alloca( header.m_destination_size + 2 );
		if ( !m_client_socket.recv( (char*)p_dst_name, header.m_destination_size, 0 ) ) break;

		// Handle names that are not correctly null terminated.
		// DHOMAS bug :(
		p_dst_name[ header.m_destination_size/2 ] = 0;

		// Get the message
		const DWORD message_data_size = header.m_message_size - server::remote_message::header_size;
		server::remote_message	msg( message_data_size + 2 );
		char *p_data = msg.error() ? (char*)::malloc( header.m_message_size + 2 ) : (char*)msg.ptr_raw();

		// Read the message
		if ( !m_client_socket.recv( p_data, header.m_message_size, 0 ) ) break;

		// Terminate the string if it was not, at the expense of messages being 2 bytes larger than they
		// need to be. This would fix any XML sent to us.
		// DHOMAS bug :(
		p_data[ header.m_message_size   ] = 0;
		p_data[ header.m_message_size+1 ] = 0;
		
		// Free if it was an error
		if ( msg.error() )  { assert( false ); ::free( p_data ); }

		// Mark this message as coming from the network
		msg.set_from_network( message_data_size );

		// Send this message on
		msg.send( p_dst_name );
	}

	// There has been an error
	m_error = true;

	// Finished
	return 0;
}

// Constructor
server::remote_message::remote_message( const DWORD size ) 
	: FC3i::message( size )
{	// This message is not remote
}