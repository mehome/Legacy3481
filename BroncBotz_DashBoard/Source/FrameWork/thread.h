#pragma once

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