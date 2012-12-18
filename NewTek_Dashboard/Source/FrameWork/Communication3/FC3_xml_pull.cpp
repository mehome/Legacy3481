#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::xml;

// Constructor
pull::pull( const wchar_t name[], const bool flush_queue )
{	// Start the server
	const bool _check = start( name, NULL, flush_queue );
	assert( _check );	

	// Flush the queue
	if ( config::debug_receive_creation )
		FrameWork::Communication3::debug::debug_output( FrameWork::Communication3::config::debug_category, L"XML server created, %s", name );	

	// If this gets triggered we have a bug in the code.
#ifdef	_DEBUG
	if ( queue_depth() == FC3::config::message_queue_length )
		switch( ::MessageBoxW( NULL, L"The XML message queue is full.\n"
									 L"This most likely means that the module will\n"
									 L"be in a state that is different from what the\n"
									 L"sender expects. This is an important issue that\n"
									 L"needs to be fixed on the sending side; it is not.\n"
									 L"a problem caused by the pullr.\n\n"
									 L"Click OK to debug.\n"
									 L"Click CANCEL to continue.\n\n"
									 L"This message is NOT displayed in release mode.",
									 name, MB_OKCANCEL ) )
		{	case IDOK:		::DebugBreak(); break;
			case IDCANCEL:	break;
		}
#endif	_DEBUG
}

// Destructor
pull::~pull( void )
{	// Stop the server
	const bool _check = stop();
	assert( _check );
}

// The client implementation
const FrameWork::Communication3::xml::message* pull::operator() ( const DWORD timeout )
{	// Get a message
	DWORD block_id, addr;
	if ( !__super::pull( timeout, block_id, addr ) ) return false;

	// Create a message
	FrameWork::Communication3::xml::message	*p_new_msg = new FrameWork::Communication3::xml::message( block_id, addr );

	// Check for errors
	if ( p_new_msg->error() ) { p_new_msg->release(); p_new_msg = NULL; }
	assert( p_new_msg );
	
	// Finished
	return p_new_msg;
}