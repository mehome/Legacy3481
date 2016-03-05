#pragma once

namespace Threads
{

/*	An example of how to use the templated thread class :

		struct MyTest
		{	void operator() ( const int T )
			{	OutputDebugString("Hello World !\n");
				Sleep(100);
			}
		} _MyTest;

		FrameWork::Threads::thread<MyTest, const int >	Thread1( _MyTest, 1 );
		Thread1.low_priority();

		FrameWork::Threads::thread<MyTest, const int >	Thread2( _MyTest, 2 );
		Thread2.high_priority();


	auto_loop specifies whether this thread will call your function repeatedly (true) or just once (false).

	this is the logic

	if ( auto_loop )	while(!beingdeleted) { yourclass->operator() ( T ) };
	else				yourclass->operator() ( T );
*/

//struct FRAMEWORK_THREADS_API thread_memtrack {};

template< typename threadcall_type, typename threadcall_param_type = const void*, const bool auto_loop = true >
struct thread
{			// Constructor
			thread( threadcall_type *pCaller );
			thread( threadcall_type &rCaller );

			thread( threadcall_type *pCaller, threadcall_param_type data );
			thread( threadcall_type &rCaller, threadcall_param_type data );

			// Destructor
			~thread( void );			

			// Set the priority
			void realtime_priority( void );
			void high_priority( void );
			void medium_priority( void );
			void low_priority( void );
			void idle_priority( void );

			// Get the Thread ID
			const DWORD thread_id( void ) const;

			// By default when you destroy a thread it will wait forever for the thread to have
			// finished. If you want it instead to wait for a given ammount of time (short?)
			// before it would just terminate the thread to avoid a lock-up on exit, you can specify
			// this value here.
			void set_time_out( const DWORD time = INFINITE );

			// In order to trap when errors on exit occur, since they are in the destructor
			// You need to set a boolean that will be triggered
			void set_exit_error( bool* p_err );

			// Set thread name
			void set_thread_name( const char *p_thread_name );

private:	// Deliberately private so that it cannot be copied.
			thread( const thread &CopyFrom ) { assert(false); }
	
			// Start the thread
			bool Start( const int StackSize = 128*1024 );
			bool Stop( void );
	
			// The thread callback
			static DWORD thread_proc( void *p_ptr );

			// The method to call.
			threadcall_type			*m_pDestination;

			// A parameter to use
			threadcall_param_type	m_param;
	
			// The thread handle
			HANDLE	m_Handle;

			// The thread ID
			DWORD	m_ID;

			// The destructor time_out
			DWORD	m_time_out;

			// The exit error indicator
			bool*	m_p_error;

			// The thread should exit now
			bool	m_thread_should_exit;
};

#include "thread-inc.h"
}

namespace Work
{
struct thread
{			// Constructor
	thread( const DWORD stack_size = 16*1024 );	// Warning small default stack size

	// Destructor
	~thread( void );

	// Cast the thread
	__forceinline operator HANDLE ( void ) { return m_h_thread; }

	// Is this the thread that we are currently running on (this is a very fast call)
	__forceinline bool is_current_thread( void ) { return ( ::GetCurrentThreadId() == m_thread_id ); }

	// Set the thread priority
	void priority_idle( void );
	void priority_lowest( void );
	void priority_below_normal( void );
	void priority_normal( void );
	void priority_above_normal( void );
	void priority_highest( void );
	void priority_time_critical( void );

	// This allows you to wait on an event and also allow other functions to run
	enum e_wait_result
	{	e_wait_result_exit,			// The thread is being exited
	e_wait_result_timeout,		// We timed out waiting.
	e_wait_result_fcncall,		// A function call was made which made the wait complete
	e_wait_result_triggered_0,	// Event 0 was trigered
	e_wait_result_triggered_1,	// Event 1 was trigered
	e_wait_result_triggered_2,	// Event 2 was trigered
	e_wait_result_triggered_3,	// Event 3 was trigered				
	};

	e_wait_result	wait( const DWORD time_out = INFINITE, HANDLE event0 = NULL, HANDLE event1 = NULL, HANDLE event2 = NULL, HANDLE event3 = NULL );

	// This will recover the thread pointer that you are currently on, or NULL otherwise.
	static thread* get_thread( void );

private:	// The thread handle
	HANDLE	m_h_thread;
	HANDLE	m_h_thread_should_exit;
	HANDLE	m_h_thread_started;

	// The thread ID for very quick determination whether this is the thread being used.
	DWORD	m_thread_id;

	// The thread proc
	static DWORD WINAPI threadproc( void* lpParameter );

}; // struct thread
}