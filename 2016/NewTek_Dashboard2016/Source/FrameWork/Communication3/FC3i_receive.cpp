#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3i;

receive::receive( void )
	: m_p_server( NULL ), m_p_client( NULL ), m_should_exit( false ),
	  m_h_thread( NULL ), m_thread_id( 0 )
{	
}

receive::~receive( void )
{	// In case. We must have stopped manually to avoid vtable problems.
	assert( !m_p_server );
	stop();
}

const bool receive::start( const wchar_t name[], client *p_client, const bool flush_queue, const DWORD stack_size )
{	// We cannot be started
	stop();

	// Create the server
	m_p_server = server_cache::get_cache().get_server( name );
	if ( !m_p_server ) return false;	

	// Set the vlient
	m_p_client = p_client;

	// We aer not yet ready to exit
	m_should_exit = false;

	// Ensure that we start up with an empty queue (or really close)
	if ( flush_queue )
		this->flush_queue();

	// Pull servers do not have their own thread.
	if ( m_p_client )
	{	// Start the thread
		m_h_thread = ::CreateThread( NULL, stack_size, threadproc, (void*)this, 0, &m_thread_id );
		assert( m_h_thread );
		if ( !m_h_thread ) { stop(); return false; }
	}

	// Success
	return true;
}

const bool receive::stop( void )
{	// If we are running
	if ( m_p_server )
	{	// Wait for the thread to have exited
		if ( m_h_thread ) 
		{	// Wait for the thread to exit
			while( true )
			{	// Signal the event. We send a specially ignored message
				m_should_exit = true;
				m_p_server->send_message( (DWORD)-1, (DWORD)-1, 250 );

				// Try seeing if we are done
				if ( ::WaitForSingleObject( m_h_thread, 100 ) != WAIT_TIMEOUT ) 
					break;
			}

			// Close the thread
			::CloseHandle( m_h_thread );
			m_h_thread = NULL;
		}

		// Release the server
		m_p_server->release();
		m_p_server = NULL;

		// Reset to original state
		m_should_exit = false;
	}

	// Finished
	return true;
}

void receive::priority_idle( void )
{	assert( m_h_thread );
	if ( m_h_thread )
		::SetThreadPriority( m_h_thread, THREAD_PRIORITY_IDLE );
}

void receive::priority_lowest( void )
{	assert( m_h_thread );
	if ( m_h_thread )
		::SetThreadPriority( m_h_thread, THREAD_PRIORITY_LOWEST );
}

void receive::priority_below_normal( void )
{	assert( m_h_thread );
	if ( m_h_thread )
		::SetThreadPriority( m_h_thread, THREAD_PRIORITY_BELOW_NORMAL );
}

void receive::priority_normal( void )
{	assert( m_h_thread );
	if ( m_h_thread )
		::SetThreadPriority( m_h_thread, THREAD_PRIORITY_NORMAL );
}

void receive::priority_above_normal( void )
{	assert( m_h_thread );
	if ( m_h_thread )
		::SetThreadPriority( m_h_thread, THREAD_PRIORITY_ABOVE_NORMAL );
}

void receive::priority_highest( void )
{	assert( m_h_thread );
	if ( m_h_thread )
		::SetThreadPriority( m_h_thread, THREAD_PRIORITY_HIGHEST );
}

void receive::priority_time_critical( void )
{	assert( m_h_thread );
	if ( m_h_thread )::SetThreadPriority( m_h_thread, THREAD_PRIORITY_TIME_CRITICAL );
}

// Get the queue depth
const DWORD receive::queue_depth( void ) const
{	assert( m_p_server );
	return m_p_server ? m_p_server->queue_depth() : 0;
}

// This will flush the server queue
void receive::flush_queue( void )
{	// We first need to lock the queue
	const DWORD old_val = m_p_server->lock_write();

	// While the queue is not empty
	DWORD	block_id, addr;
	while( m_p_server->get_message( block_id, addr, 0 ) )
		struct null_message : public message { null_message( const DWORD block_id, const DWORD addr ) : message( block_id, addr ) {} }
			// We just instantiate a message on the stack so that it is freed
			a_message( block_id, addr );

	// Unlock the queue
	m_p_server->unlock_write( old_val );
}

// This is only used by pull servers and allows you to poll for frames with a time-out
const bool receive::pull( DWORD& block_id, DWORD& addr, const DWORD time_out )
{	// Pull servers are not running if there is a thread.
	if ( m_h_thread ) return false;

	// There must be a server
	if ( !m_p_server ) return false;

	// Wait for an item, or time-out
	if ( !m_p_server->get_message( block_id, addr, time_out ) )
	{	// Error
		block_id = (DWORD)-1;
		addr     = (DWORD)-1;
		return false;
	}

	// Success
	return true;
}

void receive::disable_callback( void )
{	// This will ensure that no more messages are going to be delivered
	// because the thread is no longer running.
	assert( ::GetCurrentThreadId() == m_thread_id );
	m_should_exit = true;
}

// The thread proc
DWORD WINAPI receive::threadproc( void* lpParameter )
{	// Checking
	receive *p_this = (receive*)lpParameter;

	// Debugging
	FC3::debug::debug_output( FC3::config::debug_category, L"Server thread started, %s", p_this->name() );		

	// A thread name
	wchar_t thread_name_w[ 1024 ];
	char	thread_name_a[ 1024 ];
	::swprintf( thread_name_w, L"%s [FC3]", p_this->name() );
	::wcstombs( thread_name_a, thread_name_w, sizeof( thread_name_a )-1 );
	set_thread_name( p_this->m_thread_id, thread_name_a );

	// Lets signal that we are runnng
	std::wstring l_server_name( FC3::config::name_server_alive );
	l_server_name += p_this->name();

	// Create the event. Note that we do not create it enabled because this is not guranrateed
	// to trigger another thread that might already be waiting on it.
	// Trigger anyone that might be waiting.
	HANDLE hThreadRunning = ::CreateEventW( NULL, TRUE, FALSE, l_server_name.c_str() );
	assert( hThreadRunning );
	::SetEvent( hThreadRunning );

	// The current sleep time
	static const DWORD default_wait_time = 1000;

	// While we are not being asked to exit
	while( !p_this->m_should_exit )
	{	// Wait for a message to be delivered
		DWORD	block_id, addr;
		if ( p_this->m_p_server->get_message( block_id, addr, default_wait_time ) )
			// We deliver the message
			p_this->m_p_client->deliver( block_id, addr );
	}

	// Debugging
	FC3::debug::debug_output( FC3::config::debug_category, L"Server thread stopped, %s", p_this->name() );

	// Mark ourselves as no longer running and close the event
	::ResetEvent( hThreadRunning );
	::CloseHandle( hThreadRunning );

	// Finished
	return 0;
}

// Get the server name
const wchar_t* receive::name( void ) const
{	// Just call my fcn
	assert( m_p_server );
	return m_p_server ? m_p_server->name() : NULL;
}