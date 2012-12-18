#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

DWORD trigger_event::g_debug_no_objects = 0;

//for internal use only
void debug_output( const wchar_t *catagory , const wchar_t *p_format, ... );

// Constructor
trigger_event::trigger_event( const DWORD trigger_id )
	:	m_ref( 1 ), 
		m_trigger_id( trigger_id )
{	// Create the trigger name
	const size_t len_pre  = ::wcslen( FrameWork::Communication3::config::name_trigger );
	const size_t len_name = 8;
	wchar_t *p_tgr_name   = (wchar_t*)_alloca( ( len_pre + len_name + 1 ) * sizeof( wchar_t ) );
	::swprintf( p_tgr_name, L"%s%08X", FrameWork::Communication3::config::name_trigger, trigger_id );

	// Setup using the protected methods of event
	event::setup( p_tgr_name );

	// Debugging
	if ( FrameWork::Communication3::config::debug_object_creation )
	{	debug_output( FrameWork::Communication3::config::debug_category, L"New trigger (%d) : %s\n", g_debug_no_objects, p_tgr_name );
		::_InterlockedIncrement( (LONG*)&g_debug_no_objects );
	}
}

// Destructor
trigger_event::~trigger_event( void )
{	// Debugging
	if ( FrameWork::Communication3::config::debug_object_creation )
	{	::_InterlockedDecrement( (LONG*)&g_debug_no_objects );
		debug_output( FrameWork::Communication3::config::debug_category, L"Del trigger (%d)\n", g_debug_no_objects );
	}
}

// This will return the current reference count. Obviously there are thread safety
// issues relating to it's use.
const long trigger_event::refcount( void ) const
{	// If the reference count has
	return m_ref;
}

// Reference counting
const long trigger_event::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long trigger_event::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

// Get the trigger ID
const DWORD trigger_event::trigger_id( void ) const
{	return m_trigger_id;
}

// Get the handle
HANDLE trigger_event::get_handle( void ) const
{	return event::get_handle();
}