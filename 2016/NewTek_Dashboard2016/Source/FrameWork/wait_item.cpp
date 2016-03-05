#include "stdafx.h"
#include "FrameWork.h"

using namespace FrameWork;

PSLIST_HEADER wait_item::m_p_free_items = NULL;

namespace FrameWork {

struct wait_item_init
{	wait_item_init( void )
	{	// Setup the list
		wait_item::m_p_free_items = (PSLIST_HEADER)::_aligned_malloc( sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT );
		::InitializeSListHead( wait_item::m_p_free_items );
	}

	~wait_item_init( void )
	{	// Free all items
		while( true )
		{	wait_item* p_wait_item = (wait_item*)::InterlockedPopEntrySList( wait_item::m_p_free_items );
			if (!p_wait_item) break;
			::CloseHandle( p_wait_item->m_wait_handle );
			::_aligned_free( p_wait_item );
		}

		// Free everything
		::_aligned_free( wait_item::m_p_free_items );
	}

}	g_wait_item_init;

} // namespace FrameWork { 


void* wait_item::operator new ( const size_t size )
{	// Try getting an item
	wait_item* p_wait_item = (wait_item*)::InterlockedPopEntrySList( wait_item::m_p_free_items );
	if ( !p_wait_item )
	{	p_wait_item = (wait_item*)::_aligned_malloc( sizeof(wait_item), MEMORY_ALLOCATION_ALIGNMENT );
		p_wait_item->m_wait_handle = ::CreateEvent( NULL, TRUE, FALSE, NULL );
	}
	else
	{	// We need to ensure that we are not triggered
		::ResetEvent( p_wait_item->m_wait_handle );
	}

	// There are always 3 initial handles
	// There are always two initial outstanding references to the object
	p_wait_item->m_ref = 2;

	// This is not currently complete
	p_wait_item->m_complete = 0;

	// Return the value
	return p_wait_item;
}

void wait_item::operator delete ( void* p_data )
{	// Delete the item
	wait_item* p_item = (wait_item*)p_data;
	::InterlockedPushEntrySList( wait_item::m_p_free_items, &p_item->m_list_entry );
}

// This is reference counted
void wait_item::release( void ) const
{	// This is quite simple
	if ( !::InterlockedDecrement( &m_ref ) ) 
		// Return myself to the cache
		delete this;
}

// Internal methods
DWORD WINAPI wait_item::thread_proc( LPVOID lpParameter )
{	// Call the routine.
	wait_item	*p_this = (wait_item*)lpParameter;
	const DWORD ret = p_this->m_p_thread_proc( (void*)p_this );

	// We avoid triggering the event if noone would ever wait on it
	// anyway.
	if ( !( p_this->m_flags & do_not_wait ) ) 
	{	// Because of the way that Windows schedules threads, choosing the next runable
		// thread. Since we have just finished some work , we often want to hand off control
		// to the person that might actually receive it and not worry about running the next
		// APC (etc... )
		::SetEvent( p_this->m_wait_handle );

		// Hand off the rest of my thread quantum
		::Sleep( 0 );
	}

	// Decrement the reference count before we do the work.
	// This would mean that any other threads submitting dependant
	// work items would always perform them directly.
	// This will never delete the item since the working task
	// has two outstanding locks.
	p_this->m_complete = 1;

	// Release the pointer
	// This might delete the item since we have two outstanding tasks
	p_this->release();

	// Finished
	return ret;
}

// Launch this work
void wait_item::launch( void )
{	if ( m_run_thread )
			// Launch on a particular thread
			::QueueUserAPC( (PAPCFUNC)thread_proc, m_run_thread, (ULONG_PTR)this );
	else	// Launch on the global thread pool
			::QueueUserWorkItem( (LPTHREAD_START_ROUTINE)thread_proc, (void*)this, ( m_flags & long_function ) ? WT_EXECUTELONGFUNCTION : WT_EXECUTEDEFAULT );
}