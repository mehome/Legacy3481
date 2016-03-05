#pragma once

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
thread<threadcall_type,threadcall_param_type,auto_loop>::thread( threadcall_type *pCaller )
	:	m_pDestination( pCaller ) ,
		m_Handle( NULL ) , m_thread_should_exit( false ),
		m_time_out( INFINITE ),
		m_p_error( NULL )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
thread<threadcall_type,threadcall_param_type,auto_loop>::thread( threadcall_type &rCaller )
	:	m_pDestination( &rCaller ) ,
		m_Handle( NULL ) , m_thread_should_exit( false ),
		m_time_out( INFINITE ),
		m_p_error( NULL )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
thread<threadcall_type,threadcall_param_type,auto_loop>::thread( threadcall_type *pCaller, threadcall_param_type data )
	:	m_pDestination( pCaller ) ,
		m_Handle( NULL ) , 
		m_thread_should_exit( false ), 
		m_param( data ),
		m_time_out( INFINITE ),
		m_p_error( NULL )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
thread<threadcall_type,threadcall_param_type,auto_loop>::thread( threadcall_type &rCaller, threadcall_param_type data )
	:	m_pDestination( &rCaller ) ,
		m_Handle( NULL ) , 
		m_thread_should_exit( false ),
		m_param( data ),
		m_time_out( INFINITE ),
		m_p_error( NULL )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

// Destructor
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
thread<threadcall_type,threadcall_param_type,auto_loop>::~thread( void )
{	// Stop the thread
	// Start the thread
	const bool _Stop = Stop();
	assert(_Stop);
}

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
bool thread<threadcall_type,threadcall_param_type,auto_loop>::Start( const int StackSize )
{	// We cannot be running
	if (m_Handle) return false;
	if (!m_pDestination) return false;

	// Check someone is not setting crazy values
	assert( StackSize >= 1024 );

	// Start the thread
	m_thread_should_exit = false;
	m_Handle = ::CreateThread( NULL, StackSize, (LPTHREAD_START_ROUTINE)thread_proc, (void*)this, NULL, &m_ID );
	assert(m_Handle);
	if (!m_Handle) return false;

	// Success
	return true;
}

// Get the Thread ID
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
const DWORD thread<threadcall_type,threadcall_param_type,auto_loop>::thread_id( void ) const
{	return m_ID;
}

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
bool thread<threadcall_type,threadcall_param_type,auto_loop>::Stop( void )
{	// We must be running
	if (!m_Handle) return false;

	// Signal an exit
	m_thread_should_exit = true;

	// No error yet
	if ( m_p_error ) *m_p_error = false;

	// Wait for the thread to finish
#ifdef	_DEBUG
try_again:
#endif	_DEBUG

	if ( ::WaitForSingleObject( m_Handle , m_time_out ) == WAIT_TIMEOUT )
	{	// Signal the thread as having been terminated
		if ( m_p_error ) *m_p_error = true;
		
		// If this gets triggered we have a bug in the code.
#ifdef	_DEBUG
		switch( ::MessageBoxW( NULL,	L"A thread being used by the application\n"
										L"has taken to long to exit and so is about\n"
										L"to be terminated to avoid locking-up\n"
										L"the application.\n\n"
										L"Click ABORT to debug.\n"
										L"Click RETRY to wait for a bit longer.\n"
										L"Click IGNORE to terminate the thread.\n\n"
										L"This message is NOT displayed in release mode.",
										L"Thread exit has timed out.",
										MB_ABORTRETRYIGNORE ) )
		{	case IDRETRY:	goto try_again;
			case IDABORT:	::DebugBreak(); break;
			case IDIGNORE:	break;
		}
#endif	_DEBUG

		// Free thread memory
		CONTEXT c_ = {0};
		c_.ContextFlags = CONTEXT_FULL;
		::GetThreadContext( m_Handle, &c_ );
		MEMORY_BASIC_INFORMATION Info_ = {0};

#ifdef _M_X64
		::VirtualQuery( (PVOID) c_.Rsp, &Info_, sizeof(Info_) );
#else
		::VirtualQuery( (PVOID) c_.Esp, &Info_, sizeof(Info_) );
#endif
		// Terminate the thread
		::TerminateThread( m_Handle, 0 );
  
		// Free the memory
		::VirtualFree( Info_.AllocationBase, 0, MEM_RELEASE ); 		
	}

	// The thread has finished
	CloseHandle( m_Handle );
	m_Handle = NULL;

	// Success
	return true;
}

// The thread callback
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
DWORD thread<threadcall_type,threadcall_param_type,auto_loop>::thread_proc( void *p_ptr )
{	// Get the pointer to myself
	thread *p_this = (thread*)p_ptr;
	assert( p_this );
	assert( p_this->m_pDestination );

	// We always run at least once
	if ( auto_loop )
	{	// Cycle forever
		do (*p_this->m_pDestination)( p_this->m_param );
		while( !p_this->m_thread_should_exit );
	}
	else
	{	// Just call once
		(*p_this->m_pDestination)( p_this->m_param );
	}

	return 0;
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::realtime_priority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_TIME_CRITICAL );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::high_priority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_HIGHEST );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::medium_priority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_NORMAL );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::low_priority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_LOWEST );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::idle_priority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_IDLE );
}

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::set_time_out( const DWORD time )
{	m_time_out = time;
}

#define MS_VC_EXCEPTION 0x406D1388

// Set thread name
template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::set_thread_name( const char *p_thread_name )
{	
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{	DWORD dwType;		// Must be 0x1000.
	LPCSTR szName;		// Pointer to name (in user addr space).
	DWORD dwThreadID;	// Thread ID (-1=caller thread).
	DWORD dwFlags;		// Reserved for future use, must be zero.

}	THREADNAME_INFO;
#pragma pack(pop)

	// Set the information
	THREADNAME_INFO info = { 0x1000, p_thread_name, m_ID, 0 };

	// Raise the exception
	__try { ::RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info ); }
   __except( EXCEPTION_EXECUTE_HANDLER ) {}

 }

template< typename threadcall_type, typename threadcall_param_type, const bool auto_loop >
void thread<threadcall_type,threadcall_param_type,auto_loop>::set_exit_error( bool* p_err )
{	m_p_error = p_err;
}