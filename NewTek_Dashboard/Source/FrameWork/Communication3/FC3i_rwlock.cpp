#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
read_write_lock::read_write_lock( void )
{
   // Setup the locks
   ::InitializeCriticalSection( &m_write_lock );
   m_no_reads = 0;
   m_no_pending_writes = 0;
   m_write_trigger = ::CreateEvent( NULL, false, false, NULL );
}

// Destructor
read_write_lock::~read_write_lock( void )
{
   // Clean things up
   ::DeleteCriticalSection( &m_write_lock );
   ::CloseHandle( m_write_trigger );
   assert( !m_no_reads );
}

// Read lock and unlock
void read_write_lock::read_lock( void )
{
   // We need to ensure that we are not writing
   ::EnterCriticalSection( &m_write_lock );

   // We increment the reading cound
   m_no_reads++;

   // We release the write lock
   ::LeaveCriticalSection( &m_write_lock );
}

bool read_write_lock::try_read_lock( void )
{
   // We need to ensure that we are not writing
   if ( !::TryEnterCriticalSection( &m_write_lock ) ) return false;

   // We increment the reading cound
   m_no_reads++;

   // We release the write lock
   ::LeaveCriticalSection( &m_write_lock );

   // Success
   return true;
}

void read_write_lock::read_unlock( void )
{    // We need to ensure that we are not writing
   ::EnterCriticalSection( &m_write_lock );

   // We decrement the reading cound
   assert( m_no_reads > 0 );
   m_no_reads--;

   // Trigger any pending write operations
   const bool trigger_needed = ( ( !m_no_reads ) && ( m_no_pending_writes ) );

   // We release the write lock
   ::LeaveCriticalSection( &m_write_lock );

   // We do this after we release the event to ensure that the wait for single
   // object in the write_lock does not immediately sleep on the enter critical section.
   if ( trigger_needed )
		::SetEvent( m_write_trigger );
}

// Write lock and unlock
void read_write_lock::write_lock( void )
{    // Enter the write section
   ::EnterCriticalSection( &m_write_lock );

   // If there are on-going reads
   while( m_no_reads )
   {    // There is now a pending write operation
       m_no_pending_writes++;

       // Unlock, allowing the read to unlock
       ::LeaveCriticalSection( &m_write_lock );

       // Wait without using CPU time
       ::WaitForSingleObject( m_write_trigger, INFINITE );

       // Re-aquire the lock
       ::EnterCriticalSection( &m_write_lock );

       // One less pending write
       m_no_pending_writes--;
   }

   // We leave the write lock enabled.
}

bool read_write_lock::try_write_lock( void )
{
   // Enter the write section
   if ( !::TryEnterCriticalSection( &m_write_lock ) ) return false;

   // Check there are no pending reads
   if ( m_no_reads )
   {    // Leave the write section
       ::LeaveCriticalSection( &m_write_lock );

       // Error
       return false;
   }

   // Succes, we got the lock
   return true;
}

void read_write_lock::write_unlock( void )
{    // Leave the write section
   ::LeaveCriticalSection( &m_write_lock );
}

// Constructor
read_auto_lock::read_auto_lock( read_write_lock& lock )
	: m_p_lock( &lock )
{
	m_p_lock->read_lock();
}

read_auto_lock::read_auto_lock( read_write_lock* p_lock )
	: m_p_lock( p_lock )
{
	m_p_lock->read_lock();
}

// Destructor
read_auto_lock::~read_auto_lock( void )
{
	m_p_lock->read_unlock();
}

// Constructor
write_auto_lock::write_auto_lock( read_write_lock& lock )
	: m_p_lock( &lock )
{
	m_p_lock->write_lock();
}

write_auto_lock::write_auto_lock( read_write_lock* p_lock )
	: m_p_lock( p_lock )
{
	m_p_lock->write_lock();
}

// Destructor
write_auto_lock::~write_auto_lock( void )
{
	m_p_lock->write_unlock();
}