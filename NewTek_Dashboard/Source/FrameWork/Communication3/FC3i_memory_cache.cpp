#include "StdAfx.h"
#include "FrameWork.Communication3.h"

// Use the namespace
using namespace FrameWork::Communication3::implementation;

#ifndef	FC3_VERSION_3_5

// The shared data segment
#pragma bss_seg(".fc3")

// This is the current, cross process block that is currently being used to	
// allocate from. When we run out of blocks, this is incremented.
DWORD memory_cache::g_next_block_id;

#pragma bss_seg()

// Get the global cache
struct global_memory_cache : public memory_cache {} g_global_memory_cache;
memory_cache& memory_cache::get_cache( void )
{	return g_global_memory_cache;
}

// Constructor
memory_cache::memory_cache( void )
	: m_time_stamp( (DWORD)-1 )
{
}

// Destructor
memory_cache::~memory_cache( void )
{	// Lock the message_queue
	write_auto_lock	lock( m_cache_lock );

	// We cycle over the blocks and release them
	for( int i=0; i<(int)m_cached_blocks.size(); i++ )
		m_cached_blocks[ i ].m_p_block->release();
}

// Try to allocate a block with the current set of allocators
bool memory_cache::try_new( message* p_dst_msg, const DWORD size )
{	// Compute the real size of the allocated block
	const DWORD real_size = message::allocation_size( size );

	// Cycle over all items in the list (backwards) and try to allocate the memory.
	for( int i = (int)m_cached_blocks.size() - 1; i >= 0; i-- )
	{	// Try to allocate a member
		memory_block* p_alloc = m_cached_blocks[ i ].m_p_block;

		// Check this is a valid blovk
		if ( p_alloc->error() ) continue;

		// Get the block, as long as there was not an error.
		BYTE *p_ptr = p_alloc->malloc( real_size );

		// If we succeeded in allocating it
		if ( p_ptr )
		{	// Add a reference to the allocator
			p_alloc->addref();

			// Time-stamp the item
			m_cached_blocks[ i ].m_time_stamp = m_time_stamp;

			// Setup the message
			return p_dst_msg->setup( p_alloc, p_ptr, size );
		}
	}

	// Error
	return false;
}

// This will allocate a message of a particular size
bool memory_cache::new_message( message* p_dst_msg, const DWORD size )
{	{	// Read lock
		read_auto_lock	lock( m_cache_lock );

		// Try to allocate using what we have
		if ( try_new( p_dst_msg, size ) ) return true;
	}
		// Write lock
		write_auto_lock	lock( m_cache_lock );

	// Only try a small number of times before failing
	for ( DWORD i=0; i<config::memory_cache_alloc_retries; i++ )
	{	// Try allocating using the current set
		if ( try_new( p_dst_msg, size ) ) return true;

		// Clean up the list to make space for new items
		clean();

		// If it was not found, we need a new block ID
		if ( std::reverse_find( m_cached_blocks.begin(), m_cached_blocks.end(), g_next_block_id ) != m_cached_blocks.end() ) 
			g_next_block_id++;

		// Allocate teh new block
		memory_block *p_new_block = new memory_block( g_next_block_id );

		// If it was not in error, then we increment the time-stamp
		if ( !p_new_block->error() ) m_time_stamp++;

		// We are going to add in a new allocator
		block_desc	new_block = { m_time_stamp, p_new_block };
		m_cached_blocks.push_back( new_block );		
	}

	// We could not allocate at all
	return p_dst_msg->setup( NULL, NULL );
}

// Try to reference a message
bool memory_cache::try_ref( message* p_dst_msg, const DWORD block_id, const DWORD addr, bool &ret )
{	// Look for the item
	iterator i = std::reverse_find( m_cached_blocks.begin(), m_cached_blocks.end(), block_id );
	
	// Check the ID and that the block was not in error.
	if ( i != m_cached_blocks.end() )
	{	// Handle errors
		if ( i->m_p_block->error() )
		{	// Return a NULL reference
			ret = p_dst_msg->setup( NULL, NULL );
		}
		else
		{	// Add a reference to the tallocator
			i->m_p_block->addref();

			// We time-stamp this item
			i->m_time_stamp = m_time_stamp;

			// Setup the message
			ret = p_dst_msg->setup( i->m_p_block, i->m_p_block->ptr( addr ) );
		}

		// We successfully setup the message, even if it failed
		return true;
	}

	// Not found
	return false;
}

// This will access an existing message
bool memory_cache::ref_message( message* p_dst_msg, const DWORD block_id, const DWORD addr )
{	{	// Read lock
		read_auto_lock	lock( m_cache_lock );

		// Try to allocate using what we have
		bool ret; if ( try_ref( p_dst_msg, block_id, addr, ret ) ) return ret;
	}
		// Write lock
		write_auto_lock	lock( m_cache_lock );

		// Try allocating using the current set. it is very rare that this happens.
		bool ret; if ( try_ref( p_dst_msg, block_id, addr, ret ) ) return ret;

		// Clean up the list to make space for new items
		clean();

		// We are going to add in a new allocator
		memory_block *p_alloc = new memory_block( block_id, true );		

		// Add it to the list
		block_desc	new_block = { ++m_time_stamp, p_alloc };
		m_cached_blocks.push_back( new_block );

		// If it was in error, we cannot return the frame
		if ( p_alloc->error() )
				// Bad frame
				return p_dst_msg->setup( NULL, NULL );

		// Add another reference count
		p_alloc->addref();

		// No need to search through the list
		return p_dst_msg->setup( p_alloc, p_alloc->ptr( addr ) );
}

// This will clean up the cache of items that are no longer accessed and are outdated.
void memory_cache::clean( void )
{	// We cycle over all the items
	int dst = 0;
	for( int i=0; i<(int)m_cached_blocks.size(); i++ )
	{	// If this item has only one reference count and is old enough
		if ( ( m_cached_blocks[ i ].m_p_block->refcount() == 1 ) &&
			 ( m_cached_blocks[ i ].m_p_block->time_stamp_is_old() ) &&
			 ( (long)( m_time_stamp - m_cached_blocks[ i ].m_time_stamp ) >= (long)FrameWork::Communication3::config::memory_cache_history ) )
					// Remove the item
					m_cached_blocks[ i ].m_p_block->release();
		else		// Keep it
					m_cached_blocks[ dst++ ] = m_cached_blocks[ i ];
	}

	// Shorten the list
	m_cached_blocks.resize( dst );
}


#endif	FC3_VERSION_3_5