#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
memory_allocator::memory_allocator( void* p_mem, const DWORD size )
	:	m_p_data( (data_segment*)p_mem ), m_size( size )
{	// Initialize
	assert( size >= sizeof(data_segment) + 8*4 );
}

// Allocator
void* memory_allocator::malloc( const DWORD size )
{	// Call the allocator
	return m_p_data->malloc( size );
}

void memory_allocator::free( void *p_ptr )
{	// Call the free block
	if ( p_ptr ) m_p_data->free( p_ptr );
}

// Get the locking element
volatile LONG* memory_allocator::lock( void )
{	return (volatile LONG*)&m_p_data->m_data[ 0 ];
}

void memory_allocator::init( void )
{	// Only initialize once
	m_p_data->init( m_size );
}

#ifdef _DEBUG

void memory_allocator::data_segment::debug_check( const DWORD curr_hdr )
{	// Check the sizes
	if ( !curr_hdr ) return;
	assert( m_data[ curr_hdr + 2 ] == m_data[ curr_hdr + m_data[ curr_hdr + 2 ] - 1 ] );
}

void memory_allocator::check_all_free( void )
{	// Check all the blocks are free
	m_p_data->check_all_free();
}

void memory_allocator::data_segment::check_all_free( void )
{	const DWORD tot_sz   = m_data_size - header_size;
	const DWORD pool_idx = free_pool_idx( tot_sz );
	for( int i=0; i<no_free_pools; i++ )
	{	if ( i == pool_idx ) assert( m_free_pool[ i ] == header_size );
		else assert( !m_free_pool[ i ] );
	}
	assert( m_data[ header_size     ] == 0 );
	assert( m_data[ header_size + 1 ] == m_free_pool + pool_idx - m_data );
	assert( m_data[ header_size + 2 ] == tot_sz );
	assert( m_data[ header_size + tot_sz - 1 ] == tot_sz );
}

#endif _DEBUG

// Initialize the list
void memory_allocator::data_segment::init( const DWORD size )
{	// If we are already initialized, skip it all
	if ( !m_data_size ) 
	{	// The total size
		const DWORD sz4 = size / sizeof( DWORD );
		m_data_size = sz4;

		// Add in a single entry for the entire free block
		add_free( header_size, sz4 - header_size );
	}
}

// Add an item to the free list
void memory_allocator::data_segment::add_free( const DWORD curr_hdr, const DWORD sz4 )
{	// This is the pool we going into
	DWORD prev_hdr = (DWORD)( m_free_pool + free_pool_idx( sz4 ) - m_data );

	// Set the size of this block
	m_data[ curr_hdr + 2 ] =								// curr->hdr->sz
	m_data[ curr_hdr + sz4 - 1 ] = sz4;						// curr->ftr->sz

	while( true )
	{	// Get the next value
		const DWORD next_hdr = m_data[ prev_hdr ];			// prev->m_next

		// Debugging
		debug_check( next_hdr );

		// If there is no next item, we are finished
		if ( !next_hdr ) break;

		// We insert this item into the list such that the shortes items are first
		if ( m_data[ next_hdr + 2 ] >= sz4 )
		{	// Insert myself here
			m_data[ prev_hdr + 0 ] = curr_hdr;				// prev->hdr->next
			m_data[ next_hdr + 1 ] = curr_hdr;				// next->hdr->prev
			m_data[ curr_hdr + 0 ] = next_hdr;				// curr->hdr->next
			m_data[ curr_hdr + 1 ] = prev_hdr;				// curr->hdr->prev

			// Finished
			return;
		}

		// Move onto the next item
		prev_hdr = next_hdr;
	}

	// Place it at the end of the list
	m_data[ prev_hdr + 0 ] = curr_hdr;						// prev->hdr->next
	m_data[ curr_hdr + 0 ] = 0;								// curr->hdr->next
	m_data[ curr_hdr + 1 ] = prev_hdr;						// curr->hdr->prev
}

DWORD memory_allocator::data_segment::free_pool_idx( const DWORD sz4 )
{	// Just work out which pool they belong to
	for( DWORD i=0; i<no_free_pools-1; i++ )
	if ( sz4 < ( (DWORD)1 << ( i + 2 ) ) ) return i;
	return no_free_pools - 1;
}

// Perform a memory allocation
void* memory_allocator::data_segment::malloc( const DWORD size )
{	// We compute the real size of the block
	const DWORD sz4 = ( /* header */ 4 + /* data */( size + 3 ) / 4 + /* footer */1 + /* round to 4 DWORDs */ 3 ) & ( ~3 );

	// We now need to cycle across the pools of possible memory it could come from
	for( DWORD pool_idx = (DWORD)( free_pool_idx( sz4 ) + m_free_pool - m_data ); pool_idx < header_size; pool_idx++ )
	{	// Get the item on the list
		DWORD prev_hdr = pool_idx;
		
		// We are going to cycle through the list until we find a big enough item
		while( true )
		{	// Get the current header
			DWORD curr_hdr = m_data[ prev_hdr ];	

			// No items found
			if ( !curr_hdr ) break;

			// Debugging
			debug_check( curr_hdr );
			
			// Is this item large enough
			if ( m_data[ curr_hdr + 2 ] >= sz4 )
			{	// Remove the free block
				remove_free( curr_hdr );

				// Should we split this block
				if ( m_data[ curr_hdr + 2 ] > sz4 + maximum_wasteage )
				{	// Insert it as a free item
					add_free( curr_hdr + sz4, m_data[ curr_hdr + 2 ] - sz4 );

					// Resize this block
					m_data[ curr_hdr + 2 ] =					// curr->hdr->size
					m_data[ curr_hdr + sz4 - 1 ] = sz4;			// curr->ftr->size
				}

				// Return the pointer
				return &m_data[ curr_hdr + 4 ];
			}

			// Get the next item on the list
			prev_hdr = curr_hdr;
		}
	}

	// Unable to allocate
	return NULL;
}

// Remove an item from the free list
void memory_allocator::data_segment::remove_free( const DWORD curr_hdr )
{	// Debugging
	debug_check( curr_hdr );
	
	// Get the next and previous indices
	DWORD prev_hdr = m_data[ curr_hdr + 1 ];			// curr->hdr->prev
	DWORD next_hdr = m_data[ curr_hdr + 0 ];			// curr->hdr->next
		
	// Remove it from the previous item
	m_data[ prev_hdr + 0 ] = next_hdr;					// prev->hdr->next
		
	// Remove if from the next item if we are not on the end of the list
	if ( next_hdr )	m_data[ next_hdr + 1 ] = prev_hdr;	// next->hdr->prev

	// Mark it as allocated (pedantic)
	m_data[ curr_hdr + 0 ] = 0;							// curr->hdr->next
	m_data[ curr_hdr + 1 ] = 0;							// curr->hdr->prev
}

// Free a block
void  memory_allocator::data_segment::free( void* p_ptr )
{	// Get the block index
	DWORD curr_hdr = (DWORD)( (DWORD*)p_ptr - m_data - 4 );
	DWORD sz4 = m_data[ curr_hdr + 2 ];

	// Debugging
	debug_check( curr_hdr );

	// We now look at whether the next block is free
	DWORD next_hdr = curr_hdr + sz4;
	if ( ( next_hdr < m_data_size ) && ( m_data[ next_hdr + 1 ] ) ) // next->hdr->prev
	{	// Remove it from the allocated list
		remove_free( next_hdr );

		// Add it to the current block
		sz4 += m_data[ next_hdr + 2 ];
	}

	// Now look at whether the previous block is free
	if ( curr_hdr > header_size )
	{	// Track backwards
		DWORD prev_hdr = curr_hdr - m_data[ curr_hdr - 1 ];			// prev->ftr->size

		// Check it is a free block
		if ( m_data[ prev_hdr + 1 ] )								// prev->hdr->prev
		{	// Remove it from the allocated list
			remove_free( prev_hdr );

			// Add it to the current block
			sz4 += m_data[ prev_hdr + 2 ];
			curr_hdr = prev_hdr;
		}
	}

	// Add this as a new free block
	add_free( curr_hdr, sz4 );
}