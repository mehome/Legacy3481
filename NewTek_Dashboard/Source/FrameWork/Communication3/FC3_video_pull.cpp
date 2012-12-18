#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::video;

// Constructor
pull::pull( const wchar_t name[], const bool flush_queue )
{	// Start the server
	const bool _check = start( name, NULL, flush_queue );
	assert( _check );	

	// Flush the queue
	if ( FC3::config::debug_receive_creation )
		FrameWork::Communication3::debug::debug_output( FrameWork::Communication3::config::debug_category, L"Video server created, %s", name );	
}

// Destructor
pull::~pull( void )
{	// Stop the server
	const bool _check = stop();
	assert( _check );
}

// The client implementation
const FrameWork::Communication3::video::message* pull::operator() ( const DWORD timeout )
{	// Get a message
	DWORD block_id, addr;
	if ( !__super::pull( timeout, block_id, addr ) ) return false;

	// Create a message
	FrameWork::Communication3::video::message	*p_new_msg = new FrameWork::Communication3::video::message( block_id, addr );

	// Check for errors
	if ( p_new_msg->error() ) { p_new_msg->release(); p_new_msg = NULL; }
	assert( p_new_msg );
	
	// Finished
	return p_new_msg;
}