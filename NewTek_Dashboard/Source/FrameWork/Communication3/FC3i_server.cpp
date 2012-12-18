#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
server::server( const wchar_t name[] )
	:	m_queue( name ), m_event( name ), 
		m_p_name( NULL ), m_ref( 1 )
{	// Keep a copy of the name
	const size_t len = ::wcslen( name ) + 1;
	m_p_name = new wchar_t [ len ];
	::memcpy( m_p_name, name, len * sizeof( wchar_t ) );
}

// Destructor
server::~server( void )
{	// Free the name
	if ( m_p_name )
		delete [] m_p_name;
}

// Reference counting
const long server::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long server::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

// This will return the current reference count. Obviously there are thread safety
// issues relating to it's use.
const long server::refcount( void ) const
{	// If the reference count has
	return m_ref;
}

// Get the name
const wchar_t *server::name( void ) const
{	// Return the name
	return m_p_name;
}

// This will add a message to the message_queue
bool server::send_message( const DWORD block_id, const DWORD addr )
{	// Put the message on the queue
	if ( !m_queue.push( block_id, addr ) ) return false;

	// Trigger the event
	m_event.set();

	// Success
	return true;
}

// Wait for a message (0 means time-out). If the time-out is 
// zero we just "ping" the message_queue.
bool server::get_message( const DWORD time_out, DWORD &block_id, DWORD &addr )
{	// Wait for the event if wanted
	if ( time_out ) 
		m_event.wait( time_out );

	// Now get the message from the queue
	return m_queue.pop( block_id, addr );
}

// This will lock the write queue, this is used when flushing a queue
const DWORD server::lock_write( void )
{	// Lock the queue
	return m_queue.lock_write();
}

void server::unlock_write( const DWORD lock_write_return )
{	// Unlock the queue
	m_queue.unlock_write( lock_write_return );
}

bool server::abort_get_message( void )
{	// Trigger the event
	m_event.set();

	// Success
	return true;
}

// Get the current instantenous queue depth
const DWORD server::queue_depth( void ) const
{	return m_queue.queue_depth();
}

// Error ?
const bool server::error( void ) const
{	return m_queue.error() || m_event.error();
}

// Get and set the heart-beat.
const __int64 server::heart_beat( void ) const
{	return m_queue.heart_beat();
}

void server::update_heart_beat( void )
{	m_queue.update_heart_beat();
}

// Reset the heard beat
void server::reset_heart_beat( void )
{	m_queue.reset_heart_beat();
}