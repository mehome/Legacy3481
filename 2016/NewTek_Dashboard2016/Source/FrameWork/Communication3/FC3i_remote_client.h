#pragma once

struct FRAMEWORKCOMMUNICATION3_API client
{			// Constructor
			client( const wchar_t* p_server, const int port_no );

			// Destructor
			~client( void );

			// Send a message of a given type
			const bool send( const wchar_t* p_dst_name, const void* p_data, const DWORD data_size );

			// Get the client name
			const wchar_t* server_name( void ) const;

			// Get the port number
			const int port_no( void ) const;

private:	// The socket
			socket* m_p_socket;

			// The server name
			wchar_t* m_p_server_name;

			// The port number to use
			int m_port_no;

			// We need critical sections around sending
			critical_section m_lock;
};