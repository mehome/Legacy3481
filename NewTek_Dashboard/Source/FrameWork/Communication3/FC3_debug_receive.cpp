#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::debug;

// Constructor
receive::receive( const wchar_t name[], client *p_dst, const bool flush_queue )
	:	m_p_destination( p_dst )
{	// Debugging ;)
	assert( m_p_destination );

	// Start the server
	const bool _check = FrameWork::Communication3::implementation::receive::start( name, this, flush_queue );
	assert( _check );

	if ( FrameWork::Communication3::config::debug_receive_creation )
		FrameWork::Communication3::debug::debug_output( FrameWork::Communication3::config::debug_category, L"Debug server created, %s", name );
}

// Destructor
receive::~receive( void )
{	// Stop the server
	const bool _check = FrameWork::Communication3::implementation::receive::stop();
	assert( _check );
}

// The client implementation
void receive::deliver( const DWORD block_id, const DWORD addr )
{	// Create a message
	FrameWork::Communication3::debug::message	*p_new_msg = new FrameWork::Communication3::debug::message( block_id, addr );
	
	// We check the type
	if ( !p_new_msg->error() )
			// Deliver the message
			m_p_destination->deliver_debug( p_new_msg );

	// Error in debug mode
	else assert( false );
	
	// Release. This might dispost of the item
	p_new_msg->release();
}