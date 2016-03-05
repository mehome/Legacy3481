#include "StdAfx.h"
#include "FrameWork.h"

using namespace FrameWork;

// Constructor
event::event( const bool AutoReset )
: m_event_handle( ::CreateEvent( NULL , AutoReset ? FALSE : TRUE , FALSE , NULL ) )
{	assert(m_event_handle);
}

// Destructor
event::~event( void )
{	// Kill the event
	assert(m_event_handle);
	::CloseHandle( m_event_handle );
}

bool event::wait( const DWORD Time ) const
{	assert(m_event_handle);
	return ( ::WaitForSingleObject( m_event_handle , Time ) == WAIT_OBJECT_0 ) ? true : false;
}

// Set (and reset) the object
void event::set( const bool Flag /* false for reset */)
{	assert( m_event_handle );
	if (Flag)	::SetEvent( m_event_handle );
	else		::ResetEvent( m_event_handle );
}

// Reset the object. Same as calling Set(false)
void event::reset( void )
{	set( false );
}

event::operator HANDLE (void) const
{	return m_event_handle;
}

// Assign
void event::operator= ( const bool value )
{	set( value );
}

event::operator bool ( void ) const
{
	return wait( 0 );
}


  /*******************************************************************************************************/
 /*												event_array												*/
/*******************************************************************************************************/

// Add an event to the list
void event_array::push_back( const event &new_event )
{	
	// Add this
	m_events.push_back( (HANDLE)new_event );
}

// Clear the list
void event_array::clear( void )
{
	// Clear
	m_events.clear();
}

// Get the number of events
size_t event_array::size( void ) const
{
	// Return the size
	return m_events.size();
}

int event_array::wait( const bool WaitAll, const DWORD Time )
{	// Perform the wait
	const DWORD result = ::WaitForMultipleObjects( (DWORD)size(), (HANDLE*)&m_events[0], WaitAll, Time );
	
	if (result==WAIT_FAILED)
	{
		DWORD error=GetLastError();
		//This shouldn't be failing
		assert(false);
	}

	if ((result>=WAIT_OBJECT_0)&&(result<WAIT_OBJECT_0+size()))
		return result-WAIT_OBJECT_0;
	else
		return-1;
}