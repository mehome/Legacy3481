#pragma once

struct wait_item_init;

struct thread_proc_interface
{			virtual void fcn( void ) = 0;
};

struct wait_item
{
			static const int	maximum_block_size = 128;

			// This must be the first item on the list.
			__declspec( align( MEMORY_ALLOCATION_ALIGNMENT ) )
			SLIST_ENTRY		m_list_entry;

			// The stack data that is used
			char			m_data[ maximum_block_size ];

			// A cached wait handle
			HANDLE			m_wait_handle;						

			// The thread handle to run upon
			HANDLE			m_run_thread;

			// The flags to run with
			work_flags		m_flags;

			// The thread_proc to use
			LPTHREAD_START_ROUTINE	m_p_thread_proc;

			// The reference count value
			__declspec( align( 4 ) )
			mutable volatile long m_ref, m_complete;

			// Launch this work
			void launch( void );

			// This is reference counted
			void release( void ) const;

			// Fast lock-free allocation
			void* operator new    ( const size_t size );

private:	// Cached re-cycling
			void  operator delete ( void* p_data );
			
			// Perform any work that is needed
			void perform_work( void );

			// Internal methods
			static DWORD WINAPI thread_proc( LPVOID lpParameter );
			
			// Truly private items
			static PSLIST_HEADER	m_p_free_items;
			friend wait_item_init;
};