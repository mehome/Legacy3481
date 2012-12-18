#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
event::event( const wchar_t name[] ) 
	: m_handle( NULL )
{	// Setup the name
	setup( name );
}

// The constructor to set up the item specifically
event::event( void )
	: m_handle( NULL )
{
}

void event::setup( const wchar_t name[] )
{	// Check
	assert( !m_handle );
	m_handle = ::CreateEventW( NULL, FALSE, FALSE, name );
}

// Destructor
event::~event( void )
{	// Close the handle
	if ( m_handle ) 
		::CloseHandle( m_handle );
}

// Set and reset the event
void event::set( const bool flag )
{	// Set or reset the flag
	if ( flag )	
			::SetEvent( m_handle );
	else	::ResetEvent( m_handle );
}

// Wait for the event
const bool event::wait( const DWORD timeout ) const
{	// Wait for the object to succeed.
	assert( m_handle );
	return ( ::WaitForSingleObject( m_handle, timeout ) == WAIT_OBJECT_0 );
}

// Was there an error
const bool event::error( void ) const
{	return ( m_handle == NULL );
}

// Get the handle
HANDLE event::get_handle( void ) const
{	return m_handle;
}