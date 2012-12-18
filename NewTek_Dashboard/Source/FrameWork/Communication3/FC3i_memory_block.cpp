#include "StdAfx.h"
#include "FrameWork.Communication3.h"

#ifndef	FC3_VERSION_3_5

using namespace FrameWork::Communication3::implementation;

DWORD memory_block::g_debug_no_objects = 0;

//for internal use only
void debug_output( const wchar_t *catagory , const wchar_t *p_format, ... );

// Constructor
memory_block::memory_block( const DWORD block_id, const bool must_exist )
	:	m_ref( 1 ), m_p_header( NULL ), m_block_id( block_id )
{	// Name the file
	wchar_t	map_name[ 128 ];
 	::swprintf( map_name, L"%s%08X", FrameWork::Communication3::config::name_memory_map, block_id );

	// Create the new nammed file
	if ( mapped_file::setup( map_name, header_size + FrameWork::Communication3::config::memory_block_size, true, must_exist ) )
	{	// Get the header location
		m_p_header = (header*)ptr();

		// Debugging
		if ( FrameWork::Communication3::config::debug_object_creation )
		{	::_InterlockedIncrement( (LONG*)&g_debug_no_objects );
			debug_output( FrameWork::Communication3::config::debug_category, L"New pool (%d) : %s (mem=%dMb)\n", g_debug_no_objects, map_name,
						  g_debug_no_objects*(header_size + FrameWork::Communication3::config::memory_block_size)/(1024*1024) );
		}
	}
}

memory_block::~memory_block( void )
{	// Debugging
	if ( ( FrameWork::Communication3::config::debug_object_creation ) && ( m_p_header ) )
	{	debug_output( FrameWork::Communication3::config::debug_category, L"Del pool (%d) (mem=%dMb)\n", g_debug_no_objects, 
						g_debug_no_objects*(header_size + FrameWork::Communication3::config::memory_block_size)/(1024*1024) );
		::_InterlockedDecrement( (LONG*)&g_debug_no_objects );
	}
}

// Get the ID
const DWORD memory_block::block_id( void ) const
{	return m_block_id;
}

// This will return the current reference count. Obviously there are thread safety
// issues relating to it's use.
const long memory_block::refcount( void ) const
{	// If the reference count has
	return m_ref;
}

// Reference counting
const long memory_block::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

// This will time-stamp the current block
void memory_block::time_stamp( void ) const
{	assert( m_p_header );
	m_p_header->m_time_stamp = ::GetTickCount();
}

// Are we old enough that this item could reasonably be removed
bool memory_block::time_stamp_is_old( void ) const
{	return ( !m_p_header ) || ( (long)( ::GetTickCount() - m_p_header->m_time_stamp ) > (long)config::memory_cache_time );
}

const long memory_block::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

// This will try to recover an item from the linked list of free blocks
BYTE* memory_block::recycle( const DWORD block_sz )
{	// Spin lock helper
	spinhelp	spin_help;
	
	// Lock the item on the block list
	while( true )
	{	// Try to lock the list
		const DWORD prev_item = (DWORD)::_InterlockedExchange( (LONG*)&m_p_header->m_free_mem[ block_sz ], -1 );

		switch( prev_item )
		{	case (DWORD)-1:	// The list was already locked, so we need to spin.
							spin_help++; break;

			case 0:			// There was no previous item, so we unlock the list and return
							m_p_header->m_free_mem[ block_sz ] = 0;
							return NULL;

			default:	{	// We get the pointer of the next block
							alloc_header *p_mem_hdr = (alloc_header*)ptr( prev_item );

							// We set the linked list back to the new value (this unlocks the list)
							m_p_header->m_free_mem[ block_sz ] = p_mem_hdr->m_next_free;

							// And return the pointer
							return alloc_header_size + (BYTE*)p_mem_hdr;
						}
		}

	} // while( true )
}

// Add a reference to a block
void memory_block::free( const BYTE* p_mem )
{	// No point doing any work if we are not recycling
	if ( !FrameWork::Communication3::config::memory_block_recycling ) return;

	// Spin lock helper
	spinhelp	spin_help;

	// Get the allocation header
	alloc_header *p_mem_hdr = (alloc_header*)( p_mem - alloc_header_size );
	const DWORD block_sz = p_mem_hdr->m_size_idx;
	const DWORD block_addr = addr( (BYTE*)p_mem_hdr );

	while( true )
	{	// We first need to lock the linked list
		const DWORD prev_item = (DWORD)::_InterlockedExchange( (LONG*)&m_p_header->m_free_mem[ block_sz ], -1 );

		// Wait until we succees
		if ( prev_item == -1 ) 
		{	spin_help++; 
			continue; 
		}

		// Now we setup the header
		p_mem_hdr->m_next_free = prev_item;

		// And store the 
		m_p_header->m_free_mem[ block_sz ] = block_addr;

		// Finished
		return;
	}
}

// This will allocate memory from the block, NULL if the allocation is not possible
BYTE* memory_block::malloc( const DWORD size )
{	// Make come more readable below
	#define	try_alloc( a ) { BYTE *p_ret = (a); if ( p_ret ) return p_ret; }

	// We need to add a small header to every memory allocation. We round the allocation
	// size up using the table at the end of this file.
	std::pair< DWORD, DWORD >	alloc_info = alloc_size( size + alloc_header_size );

	// We try to recover a block of the exact required size
	if ( FrameWork::Communication3::config::memory_block_recycling ) try_alloc( recycle( alloc_info.first ) );

	// Spin lock helper
	spinhelp	spin_help;

	// Now try to perform a stack based allocation
	DWORD original_value;
	while( true )
	{	// Get the original value
		original_value = m_p_header->m_stack_position;
	
		// Perform the operation
		const DWORD new_value = original_value + alloc_info.second;

		// Check that the block is not full
		if ( new_value > FrameWork::Communication3::config::memory_block_size ) 
		{	// We failed to get a block of exact correct size, so we try to recycle a block
			// up to two additional steps away from the current one, which allows for just
			// slightly less than a 2x wasteage of memory. 
			if ( FrameWork::Communication3::config::memory_block_recycling ) 
			{	if ( alloc_info.first < 63 ) try_alloc( recycle( alloc_info.first + 1 ) );	// 23% wasteage
				if ( alloc_info.first < 62 ) try_alloc( recycle( alloc_info.first + 2 ) );	// 53% wasteage
			}

			// No reasonabel ability to allocate
			return NULL;
		}

		// Write it interlocked
		if ( ::_InterlockedCompareExchange( (LONG*)&m_p_header->m_stack_position, new_value, original_value ) == original_value ) 
			break;

		// Spin lock safely
		spin_help++;
	}

	// Get the pointer, by offsetting back by the ammount
	BYTE* p_mem = ptr( header_size + (DWORD)original_value );

	// Get the allocation header and setup it's values
	alloc_header *p_header = (alloc_header*)p_mem;
	p_header->m_size_idx = alloc_info.first;

	// Return the pointer
	return p_mem + alloc_header_size;
}

// These are generated as a power series that fits the correct range.
const DWORD memory_block::g_block_sizes[ 64 ] = {	64, 80, 112, 144, 192, 240, 304, 384, 480, 608, 768, 960, 1200, 1488, 1856, 2304,
													2864, 3552, 4400, 5456, 6768, 8384, 10384, 12864, 15936, 19744, 24448, 30272, 
													37488, 46416, 57472, 71152, 88096, 109072, 135040, 167184, 206976, 256240, 317232, 
													392736, 486208, 601936, 745200, 922560, 1142144, 1413984, 1750512, 2167136, 2682928, 
													3321472, 
													/* 4111984 */ ( 1920*1080*2 + memory_block::alloc_header_size + 511 ) & (~511), 
													5090640, 6302224, 7802160, 9659088, 11957952, 14803952, 18327296, 
													22689200, 28089232, 34774480, 43050816, 53296912, FrameWork::Communication3::config::memory_max_alloc_size };

// This uses the current block list to generate a size and index for the size number
const std::pair< DWORD, DWORD > memory_block::alloc_size( const DWORD sz )
{	// No need for block sizing.
	if ( !FrameWork::Communication3::config::memory_block_recycling ) return std::pair< DWORD, DWORD >( 0, sz );
	
	// Check the maximum size
	const int no_blocks = sizeof( g_block_sizes ) / sizeof( g_block_sizes[ 0 ] );
	assert( sz < g_block_sizes[ no_blocks - 1 ] );
	
	// Return the results
	const DWORD *p_block = std::lower_bound( g_block_sizes, g_block_sizes + no_blocks, sz );
	return std::pair< DWORD, DWORD >( (DWORD)( p_block - g_block_sizes ), *p_block );
}

#endif	FC3_VERSION_3_5