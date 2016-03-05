#pragma once

struct slot_cache
{			// Get the cache
			static slot_cache& get_cache( void );

			// This will recover a particular slot pointer
			message_slot* get_slot( const wchar_t name[] );

protected:	// Constructor
			slot_cache( void );

			// Destructor
			~slot_cache( void );

			// Clean up the list
			void clean( void );
			
			// Read and write locks
			read_write_lock	m_cache_lock;
			
			// The current time-stamp
			DWORD m_time_stamp;
	
			// We maintain a last accessed 
			struct slot_desc
			{	DWORD m_time_stamp;
				message_slot* m_p_slot;
				bool operator== ( const wchar_t name[] ) const { assert(name); return ::wcscmp( m_p_slot->name(), name ) == 0; }
			};

			typedef std::vector< slot_desc >::iterator iterator;
			std::vector< slot_desc > m_slots;

			// Friends
			friend message;
			friend const DWORD FC3::utilities::queue_depth( const wchar_t slot_name[] );
};