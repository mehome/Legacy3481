#pragma once

#pragma once

// A basic critical section class
struct FRAMEWORKCOMMUNICATION3_API critical_section : private read_write_lock
{				// Lock this critical section
				__forceinline void lock( void )				{ read_write_lock::write_lock(); }

				// Unlock this critical section
				__forceinline void unlock( void )			{ read_write_lock::write_unlock(); }

				// Try locking this critical section and return true if success
				__forceinline const bool try_lock( void )	{ return read_write_lock::try_write_lock(); }
};

// A stack based lock for the critical section
struct FRAMEWORKCOMMUNICATION3_API auto_lock
{				// Constructor
				__forceinline auto_lock( critical_section& lock )	: m_p_lock( &lock ) { m_p_lock->lock(); }
				__forceinline auto_lock( critical_section* p_lock ) : m_p_lock( p_lock ) { m_p_lock->lock(); }

				// Destructor
				__forceinline ~auto_lock( void ) { m_p_lock->unlock(); }

private:		// The lock
				critical_section *m_p_lock;
};

struct FRAMEWORKCOMMUNICATION3_API auto_unlock
{				// Constructor
				__forceinline auto_unlock( critical_section& lock )   : m_p_lock( &lock )  { m_p_lock->unlock(); }
				__forceinline auto_unlock( critical_section* p_lock ) : m_p_lock( p_lock ) { m_p_lock->unlock(); }

				// Destructor
				__forceinline ~auto_unlock( void ) { m_p_lock->lock(); }

private:		// The lock
				critical_section *m_p_lock;
};