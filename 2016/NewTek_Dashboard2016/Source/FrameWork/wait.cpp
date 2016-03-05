#include "stdafx.h"
#include "FrameWork.h"

using namespace FrameWork;

// Wait with a time-out
bool wait::operator() ( const DWORD time_out )
{	// If we have already waited, there is nothing to do.
	if ( !m_p_wait_item ) return true;

	// If this is an item that would not wait, then do nothing
	if ( m_p_wait_item->m_flags & do_not_wait )
	{	// Return it and success
		m_p_wait_item->release();
		m_p_wait_item = NULL;
		return true;
	}
	// We can avoid needing to actually us an OS call to wait if we can
	// read the status as complete in a read-only fashion.
	else if ( m_p_wait_item->m_complete )
	{	// Release the item
		// We need to reset the event since we will not ever wait on it, making it never be 
		// triggered.
		::ResetEvent( m_p_wait_item->m_wait_handle );
		m_p_wait_item->release();
		m_p_wait_item = NULL;
		return true;
	}
	// Perform a wait
	else if ( ::WaitForSingleObject( m_p_wait_item->m_wait_handle, time_out ) == WAIT_OBJECT_0 )
	{	// Return it and success
		m_p_wait_item->release();
		m_p_wait_item = NULL;
		return true;
	}
	else
	{	// Timeout
		return false;
	}
}