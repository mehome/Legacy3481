#pragma once

struct server_cache
{			// Get the cache
			static server_cache& get_cache( void );

protected:	// Constructor
			server_cache( void );

			// Destructor
			~server_cache( void );

public:		// This will recover a particular server pointer
			server*	get_server( const wchar_t name[] );

			// Clean up the list
			void clean( void );
			
			// Read and write locks
			read_write_lock	m_cache_lock;
			
			// The current time-stamp
			DWORD m_time_stamp;
	
			// We maintain a last accessed 
			struct server_desc
			{	DWORD m_time_stamp;
				server* m_p_server;
				bool operator== ( const wchar_t name[] ) const { assert(name); return ::wcscmp( m_p_server->name(), name ) == 0; }
			};

			typedef std::vector< server_desc >::iterator iterator;
			std::vector< server_desc > m_servers;

			// Friends
			friend message;
			friend const DWORD FrameWork::Communication3::utilities::queue_depth( const wchar_t server_name[] );
};