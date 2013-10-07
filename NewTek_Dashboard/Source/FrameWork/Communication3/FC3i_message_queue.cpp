#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3i;

DWORD message_queue::g_debug_no_objects = 0;

//for internal use only
void debug_output( const wchar_t *p_category , const wchar_t *p_format, ... )
{	
	// Get the arguments
	va_list args;
	va_start( args , p_format );

	const size_t OutputLength = _vscwprintf( p_format, args ) + 1; // _vscprintf doesn't count terminating '\0'
	// Allocate on the stack (assuming that strings are not really long)
	wchar_t *pOutput = (wchar_t*)_alloca( sizeof(wchar_t)*OutputLength );
	// Create the string
	#pragma warning( push )
	#pragma warning( disable : 4996 ) // Depreciated, but it should not be !
	_vsnwprintf( pOutput , OutputLength , p_format , args );
	#pragma warning( pop )

	// Debug output
	OutputDebugStringW( L"[" );
	OutputDebugStringW( p_category );
	OutputDebugStringW( L"] " );
	OutputDebugStringW( pOutput );
}



// Constructor
message_queue::message_queue( const wchar_t name[] ) 
	: m_p_header( NULL ), 
	  m_p_msg_message_queue( NULL ),
	  m_h_queue_empty( NULL ),
	  m_h_queue_full( NULL )
{	// This one is important
	assert( ::wcslen( name ) );
	
	// Create the handle names
	std::wstring	map_name	= config::name_memory_map;
	std::wstring	empty_name	= config::name_message_queue_event1;
	std::wstring	full_name	= config::name_message_queue_event2;

	map_name   += name;
	empty_name += name;
	full_name  += name;

	mapped_file::setup( map_name.c_str(), header_size + message_queue_size, true, false );
	assert( !mapped_file::error() );

	// We now setup the pointers
	m_p_header	  = (header*)mapped_file::ptr();
	m_p_msg_message_queue = (LONGLONG*)mapped_file::ptr( header_size );

	// Create a name
	m_h_queue_empty = ::CreateEventW( NULL, FALSE, FALSE, empty_name.c_str() );
	m_h_queue_full  = ::CreateEventW( NULL, FALSE, FALSE, full_name.c_str() );

	// Debugging
	if ( FC3::config::debug_object_creation )
	{	::_InterlockedIncrement( (LONG*)&g_debug_no_objects );
		debug_output( FC3::config::debug_category, L"New queue (%d) : %s\n", g_debug_no_objects, map_name.c_str() );
	}
}

// Destructor
message_queue::~message_queue( void )
{	// Debugging
	if ( FC3::config::debug_object_creation )
	{	debug_output( FC3::config::debug_category, L"Del queue (%d)\n", g_debug_no_objects );
		::_InterlockedDecrement( (LONG*)&g_debug_no_objects );
	}

	// Free up the semaphores
	if ( m_h_queue_empty ) 
		::CloseHandle( m_h_queue_empty );

	if ( m_h_queue_full ) 
		::CloseHandle( m_h_queue_full );
}

// Error
const bool message_queue::error( void ) const
{	// Has there been an error
	return mapped_file::error();
}

// Would a send succeed
const bool message_queue::would_push_succeed( const DWORD time_out )
{	// A local helper
	spinhelp spin;

	// Aquire the writer lock	
	// If we could not acquire it. We spin until we can. Note that the spin helper avoids
	// thread priority inversion problems by allowing lower priority threads to gain access
	// every 16 passes through the lock.
acquire_lock:
	LONG write_posn;
	while( ( write_posn = ::_InterlockedExchange( &m_p_header->m_write_posn, -1 ) ) == -1 ) spin++;

	// Is this slot empty for writing too ?
	if ( _InterlockedCompareExchange64( &m_p_msg_message_queue[ write_posn ], 0LL, 0LL ) != 0 )
	{	// Release the lock by returning the lock to the old value.
		::_InterlockedExchange( &m_p_header->m_write_posn, write_posn );

		// We now wait on there being a spare slot to see if we can try to get access again
		// Kernel transition here unfortunately.
		if ( ::WaitForSingleObject( m_h_queue_empty, time_out ) == WAIT_TIMEOUT ) return false;

		// Try to acquire the lock again
		goto acquire_lock;
	}

	// Increment the write position and unlock
	::_InterlockedExchange( &m_p_header->m_write_posn, write_posn );

	// Finished
	return true;
}

// This will add a message to the message_queue
const bool message_queue::push( const DWORD block_id, const DWORD addr, const DWORD time_out )
{	// A local helper
	spinhelp spin;
	
	// We first break the message down into a LONGLONG
	const LONGLONG this_msg = ( ( (LONGLONG)block_id ) << 32 ) | ( (LONGLONG)addr );

	// Aquire the writer lock	
	// If we could not acquire it. We spin until we can. Note that the spin helper avoids
	// thread priority inversion problems by allowing lower priority threads to gain access
	// every 16 passes through the lock.
acquire_lock:
	LONG write_posn;
	while( ( write_posn = ::_InterlockedExchange( &m_p_header->m_write_posn, -1 ) ) == -1 ) spin++;

	// Is this slot empty for writing too ?
	if ( _InterlockedCompareExchange64( &m_p_msg_message_queue[ write_posn ], this_msg, 0 ) != 0 )
	{	// Release the lock by returning the lock to the old value.
		::_InterlockedExchange( &m_p_header->m_write_posn, write_posn );

		// We now wait on there being a spare slot to see if we can try to get access again
		// Kernel transition here unfortunately. No need for a kernel transition with no time-out
		// we already determined that it will not work.
		if ( ( !time_out ) || ( ::WaitForSingleObject( m_h_queue_empty, time_out ) == WAIT_TIMEOUT ) ) return false;

		// Try to acquire the lock again
		goto acquire_lock;
	}

	// Increment the write position and unlock
	::_InterlockedExchange( &m_p_header->m_write_posn, ( write_posn + 1 ) & ( FC3::config::message_queue_length - 1 ) );

	// Increase the queue depth
	::_InterlockedIncrement( (LONG*)&m_p_header->m_queue_depth );

	// If there happen to be any consumer threads that need waking up because there is data, we do it.
	::SetEvent( m_h_queue_full );	
	
	// Finished
	return true;
}

const bool message_queue::pop( DWORD &block_id, DWORD &addr, const DWORD time_out )
{	// A local helper
	spinhelp spin;

	// Aquire the read position lock. We normally do not need to support concurrent access
	// here, but we do it anyway for good measure. The chances of a lock are pretty much
	// non existant in any of the current implementation.
acquire_lock:
	LONG read_posn;
	while( ( read_posn = ::_InterlockedExchange( &m_p_header->m_read_posn, -1 ) ) == -1 ) spin++;

	// We take the item off the queue
	const LONGLONG new_msg = _InterlockedExchange64( &m_p_msg_message_queue[ read_posn ], 0LL );

	// If there was no new message to take, then we 
	if ( new_msg == 0LL )
	{	// We unlock the read queue
		::_InterlockedExchange( &m_p_header->m_read_posn, read_posn );

		// We need to wait until it looks like there is a new item to consider.
		// There is no need for a kernel transition if the timeout is zero. We already know that there
		// is nothing to get.
		if ( ( !time_out ) || ( ::WaitForSingleObject( m_h_queue_full, time_out ) == WAIT_TIMEOUT ) ) return false;

		// Try to re-aquire the lock. Hopefully his happens first go
		goto acquire_lock;
	}

	// Increment the read position and unlock
	::_InterlockedExchange( &m_p_header->m_read_posn, ( read_posn + 1 ) & ( FC3::config::message_queue_length - 1 ) );

	// Decrease the queue depth
	::_InterlockedDecrement( (LONG*)&m_p_header->m_queue_depth );

	// Trigger anyone attempting to write into the queue. It is worth a go since we have now just emptied a slot up.
	::SetEvent( m_h_queue_empty );	

	// The results. We write these at the end when any possible rights back to main memory have occured; this way
	// the time that is crucial to avoid a potential future restart problem in the case of a crash is minimized.
	if ( new_msg == -1 )
	{	// This acts like a message failure, but we got worken up !
		return false;
	}
	else
	{	// Return the message values
		block_id = (DWORD)( new_msg >> 32 );	
		addr     = (DWORD)( new_msg );

		// Success
		return true;
	}
}

// Get the current instantenous queue depth
const DWORD message_queue::queue_depth( void ) const
{	// Get the current queue depth
	LONG ret = m_p_header->m_queue_depth;

	// Because we do not lock adding and releasing threads it is slightly possible that the queue
	// values can sometimes return out of range values. We guard against that to avoid higher
	// level code doing unexpected things
		 if ( ret < 0 ) ret = 0;
	else if ( ret > FC3::config::message_queue_length ) ret = FC3::config::message_queue_length;

	// Return the result. It will now safely case without problems
	return (DWORD)ret;
}

// This will lock the write queue, this is used when flushing a queue
const DWORD message_queue::lock_write( void )
{	// A local helper
	spinhelp spin;

	// Aquire the writer lock	
	// If we could not acquire it. We spin until we can. Note that the spin helper avoids
	// thread priority inversion problems by allowing lower priority threads to gain access
	// every 16 passes through the lock.
	LONG write_posn;
	while( ( write_posn = ::_InterlockedExchange( &m_p_header->m_write_posn, -1 ) ) == -1 ) spin++;

	// Return the position
	return write_posn;
}

void message_queue::unlock_write( const DWORD old_write_posn )
{	// Unlock again
	const LONG should_be_locked = ::_InterlockedExchange( &m_p_header->m_write_posn, old_write_posn );
	assert( should_be_locked == -1 );
}