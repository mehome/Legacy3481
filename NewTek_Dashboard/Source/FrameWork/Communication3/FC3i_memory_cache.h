#pragma once

#ifdef	FC3_VERSION_3_5

#include "FC3i_memory_cache_35.h"

#else	FC3_VERSION_3_5

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

			// Try to allocate a block with the current set of allocators
			bool try_new( message* p_dst_msg, const DWORD size );

			// Try to reference a message
			bool try_ref( message* p_dst_msg, const DWORD block_id, const DWORD addr, bool &ret );			

			// This will clean up the cache of items that are no longer accessed and are
			// outdated.
			void clean( void );

			// Read and write locks
			read_write_lock	m_cache_lock;

			// The current time-stamp
			DWORD m_time_stamp;

			// A global
			static DWORD g_next_block_id;

			// We maintain a list of all of the memory blocks that we currently have.
			// Because it is rare that we have a lot of blocks open at any one time
			// it is best just to search the list, it would be slower to actually 
			// need to used a sorted list with binary search.
			struct block_desc
			{	DWORD m_time_stamp;
				memory_block* m_p_block;
				bool operator== ( const DWORD id ) const { return m_p_block->block_id() == id; }
			};

			typedef std::vector< block_desc >::iterator	iterator;
			std::vector< block_desc > m_cached_blocks;

			// A friend
			friend message;
};

#endif	FC3_VERSION_3_5