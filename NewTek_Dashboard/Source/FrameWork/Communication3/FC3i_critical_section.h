#pragma once

// A basic critical section class
struct FRAMEWORKCOMMUNICATION3_API critical_section
{				// Constructor
				critical_section( void );

				// Destructor
				~critical_section( void );

				// Lock this critical section
				void lock( void );

				// Unlock this critical section
				void unlock( void );

				// Try locking this critical section and return true if success
				bool try_lock( void );

	private:	// Internal data
				CRITICAL_SECTION	m_CS;
#ifdef _DEBUG
				volatile LONG m_c_locks;
#endif
};

// A stack based lock for the critical section
struct FRAMEWORKCOMMUNICATION3_API auto_lock
{				// Constructor
				auto_lock( critical_section *pCritSec );
				auto_lock( critical_section &rCritSec );

				// Destructor
				~auto_lock( void );

	private:	// Internal data
				critical_section		*m_p_critsec;
};

// A stack based lock for the critical section
struct FRAMEWORKCOMMUNICATION3_API auto_unlock
{				// Constructor
				auto_unlock( critical_section *pCritSec );
				auto_unlock( critical_section &rCritSec );

				// Destructor
				~auto_unlock( void );

	private:	// Internal data
				critical_section		*m_p_critsec;
};