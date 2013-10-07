#pragma once

struct memory_cache
{			// Get the global cache
			static memory_cache& get_cache( void );

protected:	// Constructor
			memory_cache( void );

			// Destructor
			~memory_cache( void );

private:	// This will allocate a message of a particular size
			bool new_message( message* p_dst_msg, const DWORD size );

			// This will access an existing message
			bool ref_message( message* p_dst_msg, const DWORD block_id, const DWORD addr );

			// Read and write locks
			critical_section	m_cache_lock;

			// We maintain a list of all cache blocks
			// This is a HUGE ammount of memory, but by not allowing the list
			// to be dynamic we can make the code far more efficient internally.
			static const int	max_no_blocks = 1024;
			memory_block*		m_blocks[ max_no_blocks ];

			// A friend
			friend message;
};