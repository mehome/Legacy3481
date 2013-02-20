#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Because we have out of date SDKs
BOOLEAN (WINAPI *read_write_lock::fcn_TryAcquireSRWLockShared) ( PSRWLOCK SRWLock ) = NULL;
BOOLEAN (WINAPI *read_write_lock::fcn_TryAcquireSRWLockExclusive) ( PSRWLOCK SRWLock ) = NULL;
VOID (WINAPI *read_write_lock::fcn_AcquireSRWLockShared) ( PSRWLOCK SRWLock ) = NULL;
VOID (WINAPI *read_write_lock::fcn_ReleaseSRWLockShared) ( PSRWLOCK SRWLock ) = NULL;
VOID (WINAPI *read_write_lock::fcn_AcquireSRWLockExclusive) ( PSRWLOCK SRWLock ) = NULL;
VOID (WINAPI *read_write_lock::fcn_ReleaseSRWLockExclusive) ( PSRWLOCK SRWLock ) = NULL;
VOID (WINAPI *read_write_lock::fcn_InitializeSRWLock) ( PSRWLOCK SRWLock ) = NULL;

// Constructor
read_write_lock::read_write_lock( void )
	:	m_thread( 0 ), m_recursion( 0 )
{	// Avoid having problems with different Windows SDK versions
	if ( !fcn_InitializeSRWLock )
	{	// Load the Kernel Library
		HMODULE hKernelDLL = ::LoadLibraryW( L"Kernel32.dll" );

		// Get the names
		*((FARPROC*)&fcn_TryAcquireSRWLockShared)    = ::GetProcAddress( hKernelDLL, "TryAcquireSRWLockShared" );		
		*((FARPROC*)&fcn_AcquireSRWLockShared)       = ::GetProcAddress( hKernelDLL, "AcquireSRWLockShared" );
		*((FARPROC*)&fcn_ReleaseSRWLockShared)       = ::GetProcAddress( hKernelDLL, "ReleaseSRWLockShared" );
		*((FARPROC*)&fcn_TryAcquireSRWLockExclusive) = ::GetProcAddress( hKernelDLL, "TryAcquireSRWLockExclusive" );		
		*((FARPROC*)&fcn_AcquireSRWLockExclusive)    = ::GetProcAddress( hKernelDLL, "AcquireSRWLockExclusive" );
		*((FARPROC*)&fcn_ReleaseSRWLockExclusive)    = ::GetProcAddress( hKernelDLL, "ReleaseSRWLockExclusive" );
		*((FARPROC*)&fcn_InitializeSRWLock)          = ::GetProcAddress( hKernelDLL, "InitializeSRWLock" );

		// If this asserts, get a new OS
		assert( fcn_TryAcquireSRWLockShared && 
				fcn_AcquireSRWLockShared && 
				fcn_ReleaseSRWLockShared && 
				fcn_TryAcquireSRWLockExclusive &&
				fcn_AcquireSRWLockExclusive && 
				fcn_ReleaseSRWLockExclusive && 
				fcn_InitializeSRWLock );
	}
	
	// Initialize the lock
	fcn_InitializeSRWLock( &m_lock );
}

// Destructor
read_write_lock::~read_write_lock( void )
{	assert( !m_thread );
	assert( !m_recursion );
}