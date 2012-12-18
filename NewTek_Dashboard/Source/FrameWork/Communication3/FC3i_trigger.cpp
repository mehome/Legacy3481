#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
trigger::trigger( void ) : m_p_event( NULL ), m_ref( 1 )
{	// Try to get the trigger
	trigger_cache::get_cache().new_trigger( this );
}

// Constructor
trigger::trigger( const DWORD trigger_id ) : m_p_event( NULL ), m_ref( 1 )
{	// Try to get the trigger
	trigger_cache::get_cache().ref_trigger( this, trigger_id );
}

trigger::~trigger( void )
{	// Release any previous event
	if ( m_p_event ) m_p_event->release();

	// Set this to NULL
	m_p_event = NULL;
}

// Setup the item
bool trigger::setup( trigger_event *p_event )
{	// Release any previous event
	if ( m_p_event ) m_p_event->release();

	// Set this event
	m_p_event = p_event;

	// Success
	return true;
}

// Wait for the event
const bool trigger::wait( const DWORD timeout ) const
{	return m_p_event->wait( timeout );
}

// Set and reset the event
void trigger::set( const bool flag )
{	m_p_event->set( flag );
}

// This will return the current reference count. Obviously there are thread safety
// issues relating to it's use.
const long trigger::refcount( void ) const
{	// If the reference count has
	return m_ref;
}

// Reference counting
const long trigger::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long trigger::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

// Get the trigger ID
const DWORD trigger::trigger_id( void ) const
{	// Call the event
	return m_p_event ? m_p_event->trigger_id() : 0;
}

// Get the handle
HANDLE trigger::get_handle( void ) const
{	return m_p_event ? m_p_event->get_handle() : NULL;
}