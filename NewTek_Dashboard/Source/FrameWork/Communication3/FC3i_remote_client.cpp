#include "StdAfx.h"
#include "FrameWork.Communication3.h"

namespace FC3i  = FC3i;
namespace FC3ir = FC3i::remote;

using namespace FC3ir;

// Constructor
client::client( const wchar_t* p_server, const int port_no )
	:	m_p_server_name( new wchar_t [ ::wcslen( p_server ) + 1 ] ),
		m_p_socket( NULL ), m_port_no( port_no )
{	// Store the name
	::wcscpy( m_p_server_name, p_server );

	// Note that we do not open the socket here. We do it when the message is sent
	// so that there is no locking connecting to sources that are not available.
}

// Destructor
client::~client( void )
{	// Close the socket, then destroy iy
	if ( m_p_socket )
	{	m_p_socket->closesocket();
		delete m_p_socket;
	}

	// Free the memory
	delete [] m_p_server_name;
}

// Get the client name
const wchar_t* client::server_name( void ) const
{	// Get the server name
	return m_p_server_name;
}

// Get the port number
const int client::port_no( void ) const
{	return m_port_no;
}

// Send a message of a given type
const bool client::send( const wchar_t* p_dst_name, const void* p_data, const DWORD data_size )
{	// We cannot have two threads in the same process using the same socket at the same time
	FC3i::auto_lock	lock( m_lock );

	// Allow one reconnection attempt
	for( int i=0; i<2; i++ )
	{	// Open a socket if required
		if ( !m_p_socket )
		{	// Create a new socket.
			m_p_socket = new socket;
			
			// Get the host name
			const size_t no_chars = ::wcslen( m_p_server_name )*2 + 1;
			char *p_serverA = new char [ no_chars ];
			::wcstombs( p_serverA, m_p_server_name, no_chars );
			hostent* p_host = m_p_socket->gethostbyname( p_serverA );	

			// Try to connect to the server
			sockaddr_in sin = { 0 };
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = p_host ? ( ( (in_addr*)(p_host->h_addr) )->s_addr ) :  ::inet_addr( p_serverA );
			sin.sin_port = ::htons( m_port_no );

			// Delete the string
			delete [] p_serverA;		

			// Connect to the port
			if ( !m_p_socket->connect( (sockaddr*)&sin, sizeof(sin) ) ) goto error;
		}
		
		// Check for errors
		if ( m_p_socket->error() ) goto error;

		// The destination name size
		const DWORD dst_size  = (DWORD)( ::wcslen( p_dst_name ) + 1 ) * sizeof( wchar_t );

		// Build the header packet.
		const FC3ir::tcpip_message_header hdr = { FC3ir::tcpip_message_header::current_version, 
												  FC3ir::tcpip_message_header::message_type_send,
												  dst_size, data_size };

		// Send the header
		if ( !m_p_socket->send( (char*)&hdr, sizeof(hdr), 0 ) ) goto error;

		// Send the destination
		if ( !m_p_socket->send( (char*)p_dst_name, dst_size, 0 ) ) goto error;

		// Send the data
		if ( !m_p_socket->send( (char*)p_data, data_size, 0 ) ) goto error;

		// Success
		return true;

	error:
		if ( m_p_socket )
		{	// Close the socket
			m_p_socket->closesocket();
			delete m_p_socket;

			// No longer exists
			m_p_socket = NULL;
		}
	} // for( int i=0; i<2; i++ )

	// Error 
	return false;
}