#pragma once

// A basic critical section class
struct  critical_section
{				// Constructor
	critical_section(void);

	// Destructor
	~critical_section(void);

	// Lock this critical section
	void lock(void);

	// Unlock this critical section
	void unlock(void);

	// Try locking this critical section and return true if success
	bool try_lock(void);

private:	// Internal data
	CRITICAL_SECTION	m_CS;
	//#ifdef _DEBUG
					// This is not used, but at leasy it keeps the structures the same size between
					// debug and release.
	volatile LONG m_c_locks;
	//#endif
};

// A stack based lock for the critical section
struct  auto_lock
{				// Constructor
	auto_lock(critical_section *pCritSec);
	auto_lock(critical_section &rCritSec);
	//auto_lock(mutex *pMutex, const DWORD time_out = INFINITE);
	//auto_lock(mutex &rMutex, const DWORD time_out = INFINITE);

	// Did we lock. This might not be true if a mutex timed out
	const bool is_locked(void) const;

	// Destructor
	~auto_lock(void);

private:	// Internal data
	critical_section	*m_pCritSec;
	//mutex				*m_pMutex;
};

// A stack based lock for the critical section
struct  auto_unlock
{				// Constructor
	auto_unlock(critical_section *pCritSec);
	auto_unlock(critical_section &rCritSec);
	//auto_unlock(mutex *pMutex);
	//auto_unlock(mutex &rMutex);

	// Destructor
	~auto_unlock(void);

private:	// Internal data
	critical_section	*m_pCritSec;
	//mutex				*m_pMutex;
};