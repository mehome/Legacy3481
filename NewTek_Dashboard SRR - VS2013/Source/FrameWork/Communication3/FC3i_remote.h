#pragma once

// HACK for including FC3
#ifndef	_WINSOCK2API_
typedef UINT_PTR        SOCKET;
#endif	_WINSOCK2API_

// The definition of the header
struct tcpip_message_header
{	// The current header version
	static const DWORD	current_version = 1;

	// The message type (might vary later.)
	static const DWORD	message_type_send = 0;	// A FC3 message of any type, not a control message
	
	// The current TCP-IP transmission version
	DWORD	m_version_no;

	// The message type
	DWORD	m_message_type;

	// message_type_send : The destination name size. This is in bytes.
	DWORD	m_destination_size;

	// message_type_send : This is the total message size. This is in bytes.
	DWORD	m_message_size;
};

struct socket
{			// Constructor
			socket( void );
			socket( SOCKET socket_to_use );

			// Destructor
			~socket( void );

			// Has there neem an error
			const bool error( void ) const;

			// Force close the socket now
			void closesocket( void );						

			// Winsock functions
			const bool connect( const struct sockaddr* name, const int namelen ) const;
			const bool bind(const struct sockaddr* name, const int namelen ) const;
			const bool listen( const int backlog ) const;
			const SOCKET accept( struct sockaddr* addr, int* addrlen ) const;

			// Reliable versions
			const bool send( const char* buf, const int len, const int flags ) const;
			const bool recv( char* buf, const int len, const int flags ) const;

			// Get a hostname
			struct hostent* gethostbyname( const char* p_name ) const;

private:	// Initialization required
			void wsa_startup( void );
			
			// The socket
			SOCKET	m_socket;
};