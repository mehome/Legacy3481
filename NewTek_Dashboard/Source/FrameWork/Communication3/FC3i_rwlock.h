#pragma once

struct FRAMEWORKCOMMUNICATION3_API read_write_lock
{			// Constructor
			read_write_lock( void );

			// Destructor
			~read_write_lock( void );

			// Deliberately not implemented
			explicit read_write_lock( const read_write_lock& from ) { assert( false ); }
			void operator= ( const read_write_lock& from )		{ assert( false ); }

			// Lock this object for reading
			__forceinline void read_lock( void )				{ fcn_AcquireSRWLockShared( &m_lock ); }

			// Try to lock this object for reading.
			__forceinline const bool try_read_lock( void )		{ return fcn_TryAcquireSRWLockShared( &m_lock ) ? true : false; }

			// Unlock this object for reading
			__forceinline void read_unlock( void )				{ fcn_ReleaseSRWLockShared( &m_lock ); }

			// Lock this object for writing
			__forceinline void write_lock( void )				{ const DWORD thread = ::GetCurrentThreadId();  if ( m_thread != thread ) { fcn_AcquireSRWLockExclusive( &m_lock );	assert( m_thread == 0 ); m_thread = thread; } else m_recursion++; }

			// Try to lock this object for writing
			__forceinline const bool try_write_lock( void )		{ const DWORD thread = ::GetCurrentThreadId(); if ( m_thread != thread ) { if ( !fcn_TryAcquireSRWLockExclusive( &m_lock ) ) return false; assert( m_thread == 0 ); m_thread = thread; } else m_recursion++; return true; }

			// Unlock this object for writing
			__forceinline void write_unlock( void )				{ assert( ::GetCurrentThreadId() == m_thread ); if ( !m_recursion ) { m_thread = 0; fcn_ReleaseSRWLockExclusive( &m_lock ); } else m_recursion--; }
		
private:	// The lock variable
			SRWLOCK	m_lock;

			// The thread and recursion count
			DWORD	m_thread;
			DWORD	m_recursion;

			// Global functions
			static BOOLEAN (WINAPI *fcn_TryAcquireSRWLockShared) ( PSRWLOCK SRWLock );
			static BOOLEAN (WINAPI *fcn_TryAcquireSRWLockExclusive) ( PSRWLOCK SRWLock );
			static VOID (WINAPI *fcn_AcquireSRWLockShared) ( PSRWLOCK SRWLock );
			static VOID (WINAPI *fcn_ReleaseSRWLockShared) ( PSRWLOCK SRWLock );
			static VOID (WINAPI *fcn_AcquireSRWLockExclusive) ( PSRWLOCK SRWLock );
			static VOID (WINAPI *fcn_ReleaseSRWLockExclusive) ( PSRWLOCK SRWLock );
			static VOID (WINAPI *fcn_InitializeSRWLock) ( PSRWLOCK SRWLock );
};

struct FRAMEWORKCOMMUNICATION3_API read_auto_lock
{				// Constructor
				__forceinline read_auto_lock( read_write_lock& lock )	: m_p_lock( &lock ) { m_p_lock->read_lock(); }
				__forceinline read_auto_lock( read_write_lock* p_lock )	: m_p_lock( p_lock ) { m_p_lock->read_lock(); }

				// Destructor
				__forceinline ~read_auto_lock( void ) { m_p_lock->read_unlock(); }

private:		// The lock
				read_write_lock *m_p_lock;
};

struct FRAMEWORKCOMMUNICATION3_API write_auto_lock
{				// Constructor
				__forceinline write_auto_lock( read_write_lock& lock )	 : m_p_lock( &lock ) { m_p_lock->write_lock(); }
				__forceinline write_auto_lock( read_write_lock* p_lock ) : m_p_lock( p_lock ) { m_p_lock->write_lock(); }

				// Destructor
				__forceinline ~write_auto_lock( void ) { m_p_lock->write_unlock(); }

private:		// The lock
				read_write_lock *m_p_lock;
};

struct FRAMEWORKCOMMUNICATION3_API write_auto_unlock
{				// Constructor
				__forceinline write_auto_unlock( read_write_lock& lock )   : m_p_lock( &lock )  { m_p_lock->write_unlock(); }
				__forceinline write_auto_unlock( read_write_lock* p_lock ) : m_p_lock( p_lock ) { m_p_lock->write_unlock(); }

				// Destructor
				__forceinline ~write_auto_unlock( void ) { m_p_lock->write_lock(); }

private:		// The lock
				read_write_lock *m_p_lock;
};