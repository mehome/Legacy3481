#pragma once

struct FRAMEWORKCOMMUNICATION3_API client_cache
{			// Get the client cache
			static client_cache& get_cache( void );

protected:	// Constructor
			client_cache( void );

			// Destructor
			~client_cache( void );

private:	// Send a message to a client
			const bool send( const wchar_t *p_client_name, const int port_no, const wchar_t* p_dst_name, const void* p_data, const DWORD data_size );
			
			// The lock on the cache
			read_write_lock			m_clients_lock;
	
			// The list of all clients
			std::vector< client* >	m_p_clients;

			// A friend
			friend FrameWork::Communication3::implementation::message;
};