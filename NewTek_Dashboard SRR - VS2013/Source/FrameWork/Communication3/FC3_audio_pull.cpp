#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::audio;

// Constructor
pull::pull( const wchar_t name[], const bool flush_queue )
{	// Start the server
	const bool _check = start( name, NULL, flush_queue );
	assert( _check );	

	// Flush the queue
	if ( config::debug_receive_creation )
		FC3::debug::debug_output( FC3::config::debug_category, L"Audio server created, %s", name );	
}

// Destructor
pull::~pull( void )
{	// Stop the server
	const bool _check = stop();
	assert( _check );
}

// The client implementation
const FC3::audio::message* pull::operator() ( const DWORD time_out )
{	// Get a message
	DWORD block_id, addr;
	if ( !__super::pull( block_id, addr, time_out ) ) return false;

	// Create a message
	FC3::audio::message	*p_new_msg = new FC3::audio::message( block_id, addr );

	// Check for errors
	if ( p_new_msg->error() ) { p_new_msg->release(); p_new_msg = NULL; }
	assert( p_new_msg );
	
	// Finished
	return p_new_msg;
}