#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

#ifdef	FC3_VERSION_3_5

// For internal use only
void debug_output( const wchar_t *catagory , const wchar_t *p_format, ... );

// Keep track of how many objects there are
DWORD memory_block::g_debug_no_objects = 0;

// Constructor
memory_block::memory_block( const DWORD block_id, const bool must_exist )
	:	m_ref( 1 ), m_block_id( block_id ),
		m_p_allocator( NULL ), m_lock_handle( NULL )
{	// Name the file
	wchar_t	map_name[ 128 ];
 	::swprintf( map_name, L"%s%08X", FrameWork::Communication3::config::name_memory_map, block_id );

	// We need a named event to make cross process locking of the memory pool more efficient
	wchar_t	map_name_event[ 128 ];
	::swprintf( map_name_event, L"%s%08X", FrameWork::Communication3::config::name_memory_map_event, block_id );
	m_lock_handle = ::CreateEventW( NULL, FALSE, FALSE, map_name_event );
	assert( m_lock_handle );
	if ( !m_lock_handle ) return;	// Error

	// Create the new nammed file
	if ( mapped_file::setup( map_name, FrameWork::Communication3::config::memory_block_size, true, must_exist ) )
	{	// Setup the memory allocator
		m_p_allocator = new memory_allocator( ptr(), size() );

		// Lock the allocator
		lock( true );

		// Initialize it if it is not
		m_p_allocator->init();

		// Unlock the allocator
		unlock();

		// Debugging
		if ( FrameWork::Communication3::config::debug_object_creation )
		{	::_InterlockedIncrement( (LONG*)&g_debug_no_objects );
			debug_output( FrameWork::Communication3::config::debug_category, L"New pool (%d) : %s (mem=%dMb)\n", g_debug_no_objects, map_name,
						  g_debug_no_objects * FrameWork::Communication3::config::memory_block_size / (1024*1024) );
		}
	}
}

memory_block::~memory_block( void )
{	// Debugging
	if ( ( FrameWork::Communication3::config::debug_object_creation ) && ( m_p_allocator ) )
	{	// Debugging
		debug_output( FrameWork::Communication3::config::debug_category, L"Del pool (%d) (mem=%dMb)\n", g_debug_no_objects, 
						g_debug_no_objects * FrameWork::Communication3::config::memory_block_size / (1024*1024) );
		::_InterlockedDecrement( (LONG*)&g_debug_no_objects );
	}

	// Destroy the allocator
	if ( m_p_allocator ) 
		delete m_p_allocator;
	m_p_allocator = NULL;

	// CLose the handle
	if ( m_lock_handle )
		::CloseHandle( m_lock_handle );
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

// Release the block
const long memory_block::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

// This will allocate memory from the block, NULL if the allocation is not possible
// NULL means error. This is an offset into the memory block, this can then be used to
// get an actual pointer from the mapped_file class.
BYTE* memory_block::malloc( const DWORD size, const bool blocking )
{	// Quite important to check for errors
	assert( m_p_allocator );
	if ( !m_p_allocator ) return NULL;

	// We lock the pool
	if ( !lock( blocking ) ) return NULL;
	
	// Allocate the memory
	BYTE *p_ret = (BYTE*)m_p_allocator->malloc( size );

	// We unlock the pool
	unlock();

	// Return the result
	return p_ret;
}

// This is a way to signal the block that memory has been released. If you fail to do this
// The memory does end up being released when all handles are discared, but it is less ewfficient.
void memory_block::free( const BYTE* p_mem )
{	// There really must be an allocator
	assert( m_p_allocator );

	// We lock the pool
	lock( true );

	// As long as it is not a NULL pointer, we allocate
	if ( p_mem ) m_p_allocator->free( (void*)p_mem );

	// We unlock the pool
	unlock();
}

// lock the pool
const bool memory_block::lock( const bool blocking )
{	// There REALLY must be an allocator if you are here
	assert( m_p_allocator );

	// Try to get a lock (note that this is all windows critical sections are
	// although they have the capability to be re-entrant on the same thread.)
	while( true )
	{	// Get the locked value
		LONG lock = ::InterlockedExchange( m_p_allocator->lock(), 1 );

		// If we where not locked, we are good to go
		if ( !lock ) return true;

		// If this is a non locking call, then we need to return with a failure.
		if ( !blocking ) return false;

		// Wait for the ability to lock. I soft wait here.
		::WaitForSingleObject( m_lock_handle, 500 );
	}
}

void memory_block::unlock( void )
{	// There REALLY must be an allocator if you are here
	assert( m_p_allocator );

	// Unlock the list (no need for this to be atomic
	*( m_p_allocator->lock() ) = 0;

	// Signal anyone waiting
	::SetEvent( m_lock_handle );
}

#endif	FC3_VERSION_3_5