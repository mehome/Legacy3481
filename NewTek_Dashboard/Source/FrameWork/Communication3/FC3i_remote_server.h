#pragma once

struct FRAMEWORKCOMMUNICATION3_API server
{			// Constructor
			server( const int port_no = FrameWork::Communication3::config::remote_port_number );

			// Destructor
			~server( void );

			// Was there an error instantiating the server
			const bool error( void ) const;

private:	// We can have many sources connected to us.
			struct FRAMEWORKCOMMUNICATION3_API connection
			{	// Constructor
				connection( server* p_parent, SOCKET client_socket );

				// Destructor
				~connection( void );

				// Has there been an error
				const bool error( void ) const;

private:		// Listen for data
				static DWORD WINAPI g_thread_proc( void* p_data );
				DWORD thread_proc( void );

				// The parent
				server* m_p_parent;

				// The thread that we use
				HANDLE	m_hThread;

				// Has there been an error
				bool	m_error;

				// The socket
				socket	m_client_socket;
			};

			// Listen for connections
			static DWORD WINAPI g_thread_proc( void* p_data );
			DWORD	thread_proc( void );

			// The server socket
			socket	m_server_socket;

			// The thread handle we use for listening
			HANDLE	m_hThread;

			// The port no
			const int m_port_no;

			// Am I listening yet
			bool m_listening;

			// Is there an error
			bool m_error;

			// The list of connections
			critical_section			m_connections_lock;
			std::vector< connection* >	m_connections_list;

			// We need to implement our own kind of message since we are just
			// transferring these on without even understanding what they are.
			struct remote_message : public FrameWork::Communication3::implementation::message
			{		// Constructor
					remote_message( const DWORD size );

					// This has to access me
					friend connection;
			};

			// A friend
			friend connection;
};