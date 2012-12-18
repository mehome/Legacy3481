#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

DWORD message_queue::g_debug_no_objects = 0;

#ifdef _M_IX86 

__forceinline __int64 __cdecl c_InterlockedCompareExchange64( __int64 volatile *Destination, __int64 Exchange, __int64 Comparand )
{	__int64 Result_;

	__asm
	{	mov  edx, DWORD PTR [Comparand+4]
		mov  eax, DWORD PTR [Comparand+0]
		mov  ecx, DWORD PTR [Exchange+4]
		mov  ebx, DWORD PTR [Exchange+0]
		mov  edi, Destination

		lock cmpxchg8b qword ptr [edi]

		mov  DWORD PTR [Result_+0], eax
		mov  DWORD PTR [Result_+4], edx
	}

	return Result_;
} 

__forceinline __int64 c_InterlockedExchange64( __int64 volatile *Target, __int64 Value )
{
    __int64 Old;

    do {
        Old = *Target;
    } while (c_InterlockedCompareExchange64(Target, Value, Old) != Old);

    return Old;
}

#define _InterlockedCompareExchange64		c_InterlockedCompareExchange64
#define _InterlockedExchange64				c_InterlockedExchange64

#endif

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
	  m_p_msg_message_queue( NULL )
{	// Create the memory map
	const size_t len_pre  = ::wcslen( FrameWork::Communication3::config::name_message_queue_map );
	const size_t len_name = ::wcslen( name );
	wchar_t *p_map_name   = (wchar_t*)_alloca( ( len_pre + len_name + 1 ) * sizeof( wchar_t ) );
	::wcscpy( p_map_name, FrameWork::Communication3::config::name_message_queue_map );
	::wcscat( p_map_name, name );
	mapped_file::setup( p_map_name, header_size + message_queue_size, true, false );
	assert( !mapped_file::error() );

	// We now setup the pointers
	m_p_header	  = (header*)mapped_file::ptr();
	m_p_msg_message_queue = (LONGLONG*)mapped_file::ptr( header_size );

	// Debugging
	if ( FrameWork::Communication3::config::debug_object_creation )
	{	::_InterlockedIncrement( (LONG*)&g_debug_no_objects );
		debug_output( FrameWork::Communication3::config::debug_category, L"New queue (%d) : %s\n", g_debug_no_objects, p_map_name );
	}
}

// Destructor
message_queue::~message_queue( void )
{	// Debugging
	if ( FrameWork::Communication3::config::debug_object_creation )
	{	debug_output( FrameWork::Communication3::config::debug_category, L"Del queue (%d)\n", g_debug_no_objects );
		::_InterlockedDecrement( (LONG*)&g_debug_no_objects );
	}
}

// Is this message queue running
void message_queue::update_heart_beat( void )
{	// Set the value
	::QueryPerformanceCounter( (LARGE_INTEGER*)&m_p_header->m_running );
}

void message_queue::reset_heart_beat( void )
{	m_p_header->m_running = 0;
}

const __int64 message_queue::heart_beat( void ) const
{	// Mark the running state
	return (__int64)m_p_header->m_running;
}

// Error
const bool message_queue::error( void ) const
{	return mapped_file::error();
}	


// This will lock the write queue, this is used when flushing a queue
const DWORD message_queue::lock_write( void )
{	// Spin lock helper
	spinhelp	spin_help;

	// Try to lock the queue
	while( true )
	{	// We need to try to lock the write position
		DWORD write_posn = (DWORD)::_InterlockedExchange( (LONG*)&m_p_header->m_write_posn, -1 );

		// If we could not lock the write position, try again.
		if ( write_posn != -1 ) return write_posn;

		// Spin
		spin_help++;
	}
}

void message_queue::unlock_write( const DWORD lock_write_return )
{	// Simply reset the value
	m_p_header->m_write_posn = lock_write_return;
}

// This will add a message to the message_queue
bool message_queue::push( const DWORD block_id, const DWORD addr )
{	// We first break the message down into a LONGLONG
	const LONGLONG this_msg = ( ( (LONGLONG)block_id ) << 32 ) | ( (LONGLONG)addr );

	// This must be valid
	assert( this_msg );

	// Spin lock helper
	spinhelp	spin_help;

	// This is quite complex. We try to write into the current message_queue
	// position. The code below is significantly more difficult to get correct
	// that it actually looks like
	while( true )
	{	// We need to try to lock the write position
		DWORD write_posn = (DWORD)::_InterlockedExchange( (LONG*)&m_p_header->m_write_posn, -1 );

		// If we could not lock the write position, try again.
		if ( write_posn == -1 ) 
		{	spin_help++;
			continue;
		}

		// If the destination value is 0, then we can put this entry into it
		if ( ::_InterlockedCompareExchange64( &m_p_msg_message_queue[ write_posn ], this_msg, 0LL ) == 0LL )
		{	// Store the write position
			m_p_header->m_write_posn = ( write_posn + 1 ) & ( FrameWork::Communication3::config::message_queue_length - 1 );
			
			// We where able to write the value
			// One more message added. We do not care about message_queue depth timing to much. We do have to do this before
			// the event is set because otherwise there is a small chance that the receiving thread might see a negative
			// message_queue depth.
			::_InterlockedIncrement( (LONG*)&m_p_header->m_queue_depth );	

			// We where successful
			return true;
		}
		else
		{	// We where not able to write the value,
			// So we restore the write position
			m_p_header->m_write_posn = write_posn;

			// And return
			return false;
		}
	}
}

bool message_queue::pop( DWORD &block_id, DWORD &addr )
{	// Get the read position
	const long read_posn = ( m_p_header->m_read_posn ) & ( FrameWork::Communication3::config::message_queue_length - 1 );

	// We assume that only one thread will be reading from the message_queue at once since there is only
	// one server. This allows us to read from the message_queue with regular memory operations. We do not
	// make this assumption on writing to the message_queue
	const LONGLONG new_msg = ::_InterlockedExchange64( &m_p_msg_message_queue[ read_posn ], 0LL );

	// if there was no message, just return
	if ( !new_msg ) return false;

	// Increment thte read position. See comment above on why this need not be interlocked
	m_p_header->m_read_posn++;

	// Reduce the message_queue depth, since this is ahared with senders it must be interlocked. I do this right after
	// we have decided to increment the position. 
	::_InterlockedDecrement( (LONG*)&m_p_header->m_queue_depth );

	// The results. We write these at the end when any possible rights back to main memory have occured; this way
	// the time that is crucial to avoid a potential future restart problem in the case of a crash is minimized.
	block_id = (DWORD)( new_msg >> 32 );	
	addr     = (DWORD)( new_msg );

	// Return the message
	return true;
}

// Get the current instantenous queue depth
const DWORD message_queue::queue_depth( void ) const
{	return m_p_header->m_queue_depth;
}