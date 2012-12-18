#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

#ifdef	FC3_VERSION_3_5

// Get the global cache
struct global_memory_cache : public memory_cache {};
static global_memory_cache g_global_memory_cache;

memory_cache& memory_cache::get_cache( void )
{	return g_global_memory_cache;
}


// Constructor
memory_cache::memory_cache( void )
{	// No blocks yet allocated
	::memset( m_blocks, 0, sizeof( m_blocks ) );

	// Lock the cache
	auto_lock lock( m_cache_lock );

	// There will of course always be one item the moment that we try to send a frame.
	m_blocks[ 0 ] = new memory_block( 0 );
}

// Destructor
memory_cache::~memory_cache( void )
{	// Lock the cache
	auto_lock lock( m_cache_lock );

	// Destroy all items
	for( int i=0; i<max_no_blocks; i++ )
	{	// Destroy the block
		if ( m_blocks[ i ] ) 
		{	m_blocks[ i ]->release();
			m_blocks[ i ] = (memory_block*)NULL;
		}
	}
}

// This will allocate a message of a particular size
bool memory_cache::new_message( message* p_dst_msg, const DWORD size )
{	// Compute the real size of the allocated block
	const DWORD real_size = message::allocation_size( size );
	
	// This might go through a couple of times
	for( int i=0; i<config::memory_cache_alloc_retries; i++ )
	{	// We cycle through the list twice, the first time we are non blocking, the second time we do need to
		// block. This reduces thread contention hopefully.
		for( int use_blocking = 0; use_blocking < 2; use_blocking++ )
		{	// Start at the first block
			for( DWORD block_id = 0; ( ( m_blocks[ block_id ] ) && ( block_id < max_no_blocks ) ); block_id++ )
			{	// Try to allocate with this block
				assert( m_blocks[ block_id ] );
				BYTE *p_ptr = m_blocks[ block_id ]->malloc( real_size, ( use_blocking == 1 ) ? true : false );

				// Success ?
				if ( p_ptr )
				{	// Add a reference to the block
					m_blocks[ block_id ]->addref();

					// Assign the message
					return p_dst_msg->setup( m_blocks[ block_id ], p_ptr, size );
				}
			}
		}

		// We now need to create a new block
		auto_lock lock( m_cache_lock );

		// Find the first entry that is empty and create a new one
		for( DWORD block_id = 0; block_id < max_no_blocks; block_id++ )
		{	// Allocate a new block
			bool created = false;
			if ( !m_blocks[ block_id ] )
			{	// A new block
				m_blocks[ block_id ] = new memory_block( block_id );
				created = true;

				// Handle error case
				if ( m_blocks[ block_id ]->error() )
				{	// This is bad
					assert( false );
					created = false;	// ug
					
					// Release the item
					m_blocks[ block_id ]->release();

					// Not allocated
					m_blocks[ block_id ] = NULL;
				}
			}

			// This can only not be true if above actually failed for some reasons (e.g. out of memory)
			if ( m_blocks[ block_id ] )
			{	// Since we allocatea a block. We know that we should be able to allocate using it
				BYTE *p_ptr = m_blocks[ block_id ]->malloc( real_size, true );

				// Success ?
				if ( p_ptr )
				{	// Add a reference to the block
					m_blocks[ block_id ]->addref();

					// Assign the message
					return p_dst_msg->setup( m_blocks[ block_id ], p_ptr, size );
				}
			}

		} // for( DWORD block_id = 0; block_id < max_no_blocks; block_id++ )
	}

	// Failure
	return false;
}

// This will access an existing message
bool memory_cache::ref_message( message* p_dst_msg, const DWORD block_id, const DWORD addr )
{	// Check the input block is even legal
	assert( block_id < max_no_blocks );

	// This only ever actually goes through twice, but the code is more concise and less error
	// prone to handle it this way.
	while( true )
	{	// We can check _without a lock_ if the pointer is not NULL (!)
		if ( m_blocks[ block_id ] )
		{	// We can now reference and return
			m_blocks[ block_id ]->addref();

			// Assign the message
			return p_dst_msg->setup( m_blocks[ block_id ], m_blocks[ block_id ]->ptr( addr ) );
		}

		// We now need to lock and potentially create the block
		auto_lock lock( m_cache_lock );

		// Create it if it does not exist
		if ( !m_blocks[ block_id ] ) 
		{	m_blocks[ block_id ] = new memory_block( block_id, true );

			// Handle error case
			if ( m_blocks[ block_id ]->error() )
			{	// Release the item
				m_blocks[ block_id ]->release();

				// Not allocated
				m_blocks[ block_id ] = NULL;

				// Error
				return false;
			}
		}

		// The next time through will succeed !
	}
}

#endif	FC3_VERSION_3_5