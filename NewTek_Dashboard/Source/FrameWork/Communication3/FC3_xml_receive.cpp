#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::xml;

// Constructor
receive::receive( const wchar_t name[], client *p_dst, const bool flush_queue )
	:	m_p_destination( p_dst )
{	// Debugging ;)
	assert( m_p_destination );	

	// Start the server
	const bool _check = FC3i::receive::start( name, this, flush_queue );
	assert( _check );	

	// Flush the queue
	if ( config::debug_receive_creation )
		FC3::debug::debug_output( FC3::config::debug_category, L"XML server created, %s", name );	

	// If this gets triggered we have a bug in the code.
#ifdef	_DEBUG
	const DWORD this_queue_depth = queue_depth();
	if ( ( this_queue_depth == FC3::config::message_queue_length ) && ( !flush_queue ) ) 
		switch( ::MessageBoxW( NULL, L"The XML message queue is full.\n"
									 L"This most likely means that the module will\n"
									 L"be in a state that is different from what the\n"
									 L"sender expects. This is an important issue that\n"
									 L"needs to be fixed on the sending side; it is not.\n"
									 L"a problem caused by the receiver.\n\n"
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
receive::~receive( void )
{	// Stop the server
	const bool _check = FC3i::receive::stop();
	assert( _check );
}

// The client implementation
void receive::deliver( const DWORD block_id, const DWORD addr )
{	// Create a message
	FC3::xml::message	*p_new_msg = new FC3::xml::message( block_id, addr );
	assert( p_new_msg );
	
	// We check the type
	if ( !p_new_msg->error() )
			// Deliver the message
			m_p_destination->deliver_xml( p_new_msg );

	// Error in debug mode
	else assert( false );
	
	// Release. This might dispose of the item
	p_new_msg->release();
}