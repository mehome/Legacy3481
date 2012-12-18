#pragma once

// A read-write lock class.
//
// In general this will allow many reads at once, but only a single write.
// A read and write cannot occur at the same time. Although this is a
// very efficient implementation, bear in mind that in most cases it is
// still slightly slower than a single critical section. In general, the
// performance is still good enough that it is likely to be recommended.
//
// This implementation supports recursion, but not elevation. This means
// that you can perform multiple reads and multiple writes on the same
// thread. What you cannot do however is perform a read, then write without
// first doing a read unlock.
//
// Unlike most implementations, this also supports try_lock functions to
// attempt to recover a lock quickly.
//
// Compared to the Intel TBB, CodeGuru, .Net implementaionts this should
// be significantly faster and does not spin-lock for waiting threads.
//
struct FRAMEWORKCOMMUNICATION3_API read_write_lock
{            // Constructor
           read_write_lock( void );

           // Destructor
           ~read_write_lock( void );

           // Lock this object for reading
           void read_lock( void );

		   // Try to lock this object for reading.
           bool try_read_lock( void );

		   // Unlock this object for reading
           void read_unlock( void );

           // Lock this object for writing
           void write_lock( void );

		   // Try to lock this object for writing
           bool try_write_lock( void );

		   // Unlock this object for writing
           void write_unlock( void );

private:    // The write lock critical section
           CRITICAL_SECTION    m_write_lock;

           // An event to trigger waiting writes
           HANDLE m_write_trigger;

           // The number of reading threads
           int m_no_reads;

           // The number of outstanding write requests
           int m_no_pending_writes;
}; 

struct read_auto_lock
{				// Constructor
				read_auto_lock( read_write_lock& lock );
				read_auto_lock( read_write_lock* p_lock );

				// Destructor
				~read_auto_lock( void );

private:		// The lock
				read_write_lock *m_p_lock;
};

struct write_auto_lock
{				// Constructor
				write_auto_lock( read_write_lock& lock );
				write_auto_lock( read_write_lock* p_lock );

				// Destructor
				~write_auto_lock( void );

private:		// The lock
				read_write_lock *m_p_lock;
};