#include "StdAfx.h"
#include "FrameWork.Communication3.h"

namespace FC3i  = FC3i;
namespace FC3ir = FC3i::remote;

using namespace FC3ir;

static volatile LONG wsa_startup_init = 0;
void socket::wsa_startup( void )
{	// Lock the startup value
	LONG locked = ::InterlockedExchange( &wsa_startup_init, -1 );

	// Wait until we get the lock
	while( locked == -1 )
	{	::Sleep( 10 );
		locked = ::InterlockedExchange( &wsa_startup_init, -1 );
	}
	
	// We need to start Winsock	
	if ( !locked )
	{	WSAData wsaData_;
		::WSAStartup( MAKEWORD( 2, 2 ), &wsaData_ );
	}

	// Unlock
	wsa_startup_init = 1;
}

// Constructor
socket::socket( void )
{	// Ensure that winsock is started
	wsa_startup();

	// Open a socket
	m_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
		
	// Set the socket options
	if ( m_socket != INVALID_SOCKET )
	{	// Set this as having no delay
		static const int one = 1;
		::setsockopt( m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one) );
	}
}

// Constructor
socket::socket( SOCKET socket_to_use )
	:	// Open the socket
		m_socket( socket_to_use )
{	// Set the socket options
	if ( m_socket != INVALID_SOCKET )
	{	// Set this as having no delay
		static const int one = 1;
		::setsockopt( m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one) );
	}
}


// Force close the socket now
void socket::closesocket( void )
{	if ( m_socket != INVALID_SOCKET )
	{	// Set the linger options
		const LINGER linger = { 1, 0 };
		::setsockopt( m_socket, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger) );

		// Close the socket
		::shutdown( m_socket, SD_BOTH );
		::closesocket( m_socket );

		// Not set
		m_socket = INVALID_SOCKET;
	}
}

// Winsock functions
const bool socket::connect( const struct sockaddr* name, const int namelen ) const
{	// Connect
	if ( m_socket == INVALID_SOCKET ) return false;
	return ( ::connect( m_socket, name, namelen ) != SOCKET_ERROR );
}

const bool socket::bind(const struct sockaddr* name, const int namelen ) const
{	// Bind
	if ( m_socket == INVALID_SOCKET ) return false;
	return ( ::bind( m_socket, name, namelen ) != SOCKET_ERROR );
}

const bool socket::listen( const int backlog ) const
{	// Listen
	if ( m_socket == INVALID_SOCKET ) return false;
	return ( ::listen( m_socket, backlog ) != SOCKET_ERROR );
}

const SOCKET socket::accept( struct sockaddr* addr, int* addrlen ) const
{	if ( m_socket == INVALID_SOCKET ) return INVALID_SOCKET;
	return ::accept( m_socket, addr, addrlen );
}

// Get a hostname
struct hostent* socket::gethostbyname( const char* p_name ) const
{	// Return the host-name
	return ::gethostbyname( p_name );
}

// Reliable sending
const bool socket::send( const char* buf, const int len, const int flags ) const
{	// Error ?
	if ( m_socket == INVALID_SOCKET ) return false;
	
	// Cycle until the buffer has been sent
	for( int N = len; N; )
	{	// Send this many bytes and check for an error
		const int size_sent = ::send( m_socket, buf, N, flags );
		if ( size_sent <= 0 ) return false;

		// Add this on
		buf += size_sent;
		N   -= size_sent;
	}

	// We sent everything
	return true;
}

const bool socket::recv( char* buf, const int len, const int flags ) const
{	// Error ?
	if ( m_socket == INVALID_SOCKET ) return false;
	
	// Cycle until the buffer has been sent
	for( int N = len; N; )
	{	// Send this many bytes and check for an error
		const int size_sent = ::recv( m_socket, buf, N, flags );
		if ( size_sent <= 0 ) return false;

		// Add this on
		buf += size_sent;
		N   -= size_sent;
	}

	// We sent everything
	return true;
}


// Destructor
socket::~socket( void )
{	// Close the socket
	closesocket();
}

// Has there neem an error
const bool socket::error( void ) const
{	return ( m_socket == INVALID_SOCKET );
}