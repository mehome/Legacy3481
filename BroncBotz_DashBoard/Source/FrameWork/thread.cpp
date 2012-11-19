#include "stdafx.h"
#include "FrameWork.h"

__declspec(thread) 
void*	g_p_current_thread = NULL;

using namespace FrameWork;

thread* thread::get_thread( void )
{	assert( g_p_current_thread );
	return (thread*)g_p_current_thread;
}

// Constructor
thread::thread( const DWORD stack_size )
	:	m_h_thread( NULL ), 
		m_h_thread_should_exit( ::CreateEvent( NULL, TRUE, FALSE, NULL ) ),
		m_h_thread_started( ::CreateEvent( NULL, FALSE, FALSE, NULL ) )
{	// Start the thread
	m_h_thread = ::CreateThread( NULL, stack_size, threadproc, (void*)this, 0, &m_thread_id );

	// Wait for the thread to have started
	::WaitForSingleObject( m_h_thread_started, INFINITE );

	// No need keeping a handle that we will not use nuw
	::CloseHandle( m_h_thread_started );
	m_h_thread_started = NULL;
}

// Destructor
thread::~thread( void )
{	// Signal the thread to exit
	::SetEvent( m_h_thread_should_exit );

	// Ask it to exit
	::WaitForSingleObject( m_h_thread, INFINITE );
	::CloseHandle( m_h_thread_should_exit );
	::CloseHandle( m_h_thread );
}

// The thread proc
DWORD WINAPI thread::threadproc( void* lpParameter )
{	// Checking
	thread *p_this = (thread*)lpParameter;

	// Set the current thread
	g_p_current_thread = lpParameter;
	::SetEvent( p_this->m_h_thread_started );

	// Debugging
	assert( p_this->m_thread_id == ::GetCurrentThreadId() );
	
	// Remain alertable	
	while( p_this->wait( INFINITE ) != e_wait_result_exit );

	// Finished
	return 0;
}

// This will allow you to sleep, allowing other asynchronous function calls to 
// proceed. Note that this should be treated with slight care, but is still very useful.
//bool thread::sleep( const DWORD time_out )
//{	// Wait for the desired time
//	return ( ::WaitForSingleObjectEx( p_this->m_h_thread_should_exit, time_out, TRUE ) == WAIT_OBJECT_0 );
//}
thread::e_wait_result thread::wait( const DWORD time_out, HANDLE event0, HANDLE event1, HANDLE event2, HANDLE event3 )
{	
	// Store the wait objects
	HANDLE	wait_objs[ 5 ] = { m_h_thread_should_exit, event0, event1, event2, event3 };

	// Get the number of wait objects
	int		no_wait_objs;
		 if ( event3 )	no_wait_objs = 5;
	else if ( event2 )	no_wait_objs = 4;
	else if ( event1 )	no_wait_objs = 3;
	else if ( event0 )	no_wait_objs = 2;
	else				no_wait_objs = 1;

	// Now wait on the multiple objects
	DWORD test = ::WaitForMultipleObjectsEx( no_wait_objs, wait_objs, FALSE, time_out, TRUE );
	switch( test )
	{	case WAIT_OBJECT_0 + 0:		return e_wait_result_exit;
		case WAIT_OBJECT_0 + 1:		return e_wait_result_triggered_0;
		case WAIT_OBJECT_0 + 2:		return e_wait_result_triggered_1;
		case WAIT_OBJECT_0 + 3:		return e_wait_result_triggered_2;
		case WAIT_OBJECT_0 + 4:		return e_wait_result_triggered_3;
		case WAIT_TIMEOUT:			return e_wait_result_timeout;
		case WAIT_IO_COMPLETION:	return e_wait_result_fcncall;
		default:					assert( false );
									return e_wait_result_timeout;
	}
}

void thread::priority_idle( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_IDLE );
}

void thread::priority_lowest( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_LOWEST );
}

void thread::priority_below_normal( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_BELOW_NORMAL );
}

void thread::priority_normal( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_NORMAL );
}

void thread::priority_above_normal( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_ABOVE_NORMAL );
}

void thread::priority_highest( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_HIGHEST );
}

void thread::priority_time_critical( void )
{	::SetThreadPriority( m_h_thread, THREAD_PRIORITY_TIME_CRITICAL );
}