#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
message_event::message_event( const wchar_t name[] )
{	// Create the memory map
	const size_t len_pre  = ::wcslen( FrameWork::Communication3::config::name_message_queue_event );
	const size_t len_name = ::wcslen( name );
	wchar_t *p_evt_name   = (wchar_t*)_alloca( ( len_pre + len_name + 1 ) * sizeof( wchar_t ) );
	::wcscpy( p_evt_name, FrameWork::Communication3::config::name_message_queue_event );
	::wcscat( p_evt_name, name );

	// Setup the event and check it worked
	event::setup( p_evt_name );
	assert( !event::error() );
}

// Destructor
message_event::~message_event( void )
{
}