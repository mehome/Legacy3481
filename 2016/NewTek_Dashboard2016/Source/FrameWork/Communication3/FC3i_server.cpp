#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3i;

// Constructor
server::server( const wchar_t name[] )
	:	m_queue( name ), m_p_name( NULL ), 
		m_ref( 1 ), m_hServerAlive( NULL )
{	// Keep a copy of the name
	const size_t len = ::wcslen( name ) + 1;
	m_p_name = new wchar_t [ len ];
	::memcpy( m_p_name, name, len * sizeof( wchar_t ) );

	// Get the named server event so that we can watch for lifetime
	std::wstring l_server_alive_name( FC3::config::name_server_alive );
	l_server_alive_name += name;
	m_hServerAlive = ::CreateEventW( NULL, TRUE, FALSE, l_server_alive_name.c_str() );
}

// Destructor
server::~server( void )
{	// Close the server alive message
	::CloseHandle( m_hServerAlive );
	
	// Free the name
	if ( m_p_name )
		delete [] m_p_name;
}

// This will wait and determine whether the server is alive
const bool server::is_server_alive( const DWORD time_out )
{	// Look to the event to see if the server is alive
	return ( ::WaitForSingleObject( m_hServerAlive, time_out ) == WAIT_OBJECT_0 );
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
const bool server::send_message( const DWORD block_id, const DWORD addr, const DWORD time_out )
{	// Put the message on the queue
	if ( !m_queue.push( block_id, addr, time_out ) ) return false;

	// Success
	return true;
}

// Would a send succeed
const bool server::would_send_message_succeed( const DWORD time_out )
{	return m_queue.would_push_succeed( time_out );
}

// Wait for a message (0 means time-out). If the time-out is 
// zero we just "ping" the message_queue.
const bool server::get_message( DWORD &block_id, DWORD &addr, const DWORD time_out )
{	// Now get the message from the queue
	return m_queue.pop( block_id, addr, time_out );
}

// This will lock the write queue, this is used when flushing a queue
const DWORD server::lock_write( void )
{	// Lock the writer
	return m_queue.lock_write();
}

void server::unlock_write( const DWORD prev_value )
{	// Unlock the writer
	m_queue.unlock_write( prev_value );
}

// Get the current instantenous queue depth
const DWORD server::queue_depth( void ) const
{	return m_queue.queue_depth();
}

// Error ?
const bool server::error( void ) const
{	return m_queue.error();
}