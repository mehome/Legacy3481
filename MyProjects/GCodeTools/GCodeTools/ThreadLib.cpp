#include "pch.h"
#include "ThreadLib.h"

// Constructor
critical_section::critical_section(void)
{
	::InitializeCriticalSection(&m_CS);
#ifdef _DEBUG
	m_c_locks = 0L;
#endif
}

// Destructor
critical_section::~critical_section(void)
{
	::DeleteCriticalSection(&m_CS);
#ifdef _DEBUG
	assert(m_c_locks == 0);
#endif
}

// Lock this critical section
void critical_section::lock(void)
{
	::EnterCriticalSection(&m_CS);
#ifdef _DEBUG
	InterlockedIncrement(&m_c_locks);
#endif
}

// Unlock this critical section
void critical_section::unlock(void)
{
#ifdef _DEBUG
	LONG cLocks = InterlockedDecrement(&m_c_locks);
	assert(cLocks >= 0);
#endif
	::LeaveCriticalSection(&m_CS);
}

// Try locking this critical section and return true if success
bool critical_section::try_lock(void)
{	//return ( ::TryEnterCriticalSection( &m_CS ) ) ? true : false;
	if (!::TryEnterCriticalSection(&m_CS)) return false;
#ifdef _DEBUG
	InterlockedIncrement(&m_c_locks);
#endif
	return true;
}



// Constructor
auto_lock::auto_lock(critical_section *pCritSec) : m_pCritSec(pCritSec)//, m_pMutex(NULL)
{
	assert(m_pCritSec);
	m_pCritSec->lock();
}

auto_lock::auto_lock(critical_section &rCritSec) : m_pCritSec(&rCritSec)//, m_pMutex(NULL)
{
	assert(m_pCritSec);
	m_pCritSec->lock();
}

//auto_lock::auto_lock(mutex *pMutex, const DWORD time_out) : m_pCritSec(NULL), m_pMutex(pMutex)
//{
//	assert(m_pMutex);
//	if (!m_pMutex->lock(time_out)) m_pMutex = NULL;
//}
//
//auto_lock::auto_lock(mutex &rMutex, const DWORD time_out) : m_pCritSec(NULL), m_pMutex(&rMutex)
//{
//	assert(m_pMutex);
//	if (!m_pMutex->lock(time_out))
//		m_pMutex = NULL;
//}

// Did we lock. This might not be true if a mutex timed out
const bool auto_lock::is_locked(void) const
{
	//return (m_pCritSec || m_pMutex);
	return (m_pCritSec);
}

// Destructor
auto_lock::~auto_lock(void)
{
	if (m_pCritSec)	m_pCritSec->unlock();
	//else if (m_pMutex)	m_pMutex->unlock();
}


// Constructor
auto_unlock::auto_unlock(critical_section *pCritSec) : m_pCritSec(pCritSec)//, m_pMutex(NULL)
{
	assert(m_pCritSec);
	m_pCritSec->unlock();
}

auto_unlock::auto_unlock(critical_section &rCritSec) : m_pCritSec(&rCritSec)//, m_pMutex(NULL)
{
	assert(m_pCritSec);
	m_pCritSec->unlock();
}

//auto_unlock::auto_unlock(mutex *pMutex) : m_pCritSec(NULL), m_pMutex(pMutex)
//{
//	assert(m_pMutex);
//	m_pCritSec->unlock();
//}
//
//auto_unlock::auto_unlock(mutex &rMutex) : m_pCritSec(NULL), m_pMutex(&rMutex)
//{
//	assert(m_pMutex);
//	m_pCritSec->unlock();
//}

// Destructor
auto_unlock::~auto_unlock(void)
{
	if (m_pCritSec) m_pCritSec->lock();
	//else if (m_pMutex) m_pMutex->lock();
}



// Constructor
event::event(const bool auto_reset)
	: m_event_handle(::CreateEvent(nullptr, auto_reset ? FALSE : TRUE, FALSE, nullptr))
{
	assert(m_event_handle);
}

// Destructor
event::~event(void)
{	// Kill the event
	assert(m_event_handle);
	::CloseHandle(m_event_handle);
}

bool event::wait(const DWORD Time) const
{
	assert(m_event_handle);
	return (::WaitForSingleObject(m_event_handle, Time) == WAIT_OBJECT_0) ? true : false;
}

// Set (and reset) the object
void event::set(const bool Flag /* false for reset */)
{
	assert(m_event_handle);
	if (Flag)
		::SetEvent(m_event_handle);
	else	::ResetEvent(m_event_handle);
}

// Reset the object. Same as calling Set(false)
void event::reset(void)
{
	set(false);
}

event::operator HANDLE (void) const
{
	return m_event_handle;
}

// Assign
void event::operator= (const bool value)
{
	set(value);
}

event::operator bool(void) const
{
	return wait(0);
}
