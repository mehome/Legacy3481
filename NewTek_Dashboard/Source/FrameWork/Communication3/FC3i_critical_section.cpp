#include "stdafx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
critical_section::critical_section( void )
{	::InitializeCriticalSection( &m_CS );
#ifdef _DEBUG
	m_c_locks = 0L;
#endif
}

// Destructor
critical_section::~critical_section( void )
{	::DeleteCriticalSection( &m_CS );
}

// Lock this critical section
void critical_section::lock( void )
{	::EnterCriticalSection( &m_CS );
#ifdef _DEBUG
	::InterlockedIncrement( &m_c_locks );
#endif
}

// Unlock this critical section
void critical_section::unlock( void )
{	
#ifdef _DEBUG
	LONG cLocks = ::InterlockedDecrement( &m_c_locks );
	assert( cLocks>=0 );
#endif
	::LeaveCriticalSection( &m_CS );
}

// Try locking this critical section and return true if success
bool critical_section::try_lock( void )
{	//return ( ::TryEnterCriticalSection( &m_CS ) ) ? true : false;
	if ( !::TryEnterCriticalSection( &m_CS ) ) return false;
#ifdef _DEBUG
	InterlockedIncrement(&m_c_locks);
#endif
	return true;
}



// Constructor
auto_lock::auto_lock( critical_section *pCritSec ) : m_p_critsec( pCritSec )
{	assert( m_p_critsec );
	m_p_critsec->lock();
}

auto_lock::auto_lock( critical_section &rCritSec ) : m_p_critsec( &rCritSec )
{	assert( m_p_critsec );
	m_p_critsec->lock();
}

// Destructor
auto_lock::~auto_lock( void )
{	m_p_critsec->unlock();
}


// Constructor
auto_unlock::auto_unlock( critical_section *pCritSec ) : m_p_critsec( pCritSec )
{	assert( m_p_critsec );
	m_p_critsec->unlock();
}

auto_unlock::auto_unlock( critical_section &rCritSec ) : m_p_critsec( &rCritSec )
{	assert( m_p_critsec );
	m_p_critsec->unlock();
}

// Destructor
auto_unlock::~auto_unlock( void )
{	m_p_critsec->lock();
}
