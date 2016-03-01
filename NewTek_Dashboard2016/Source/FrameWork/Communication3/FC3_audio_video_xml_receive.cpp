#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::audio_video_xml;

// Constructor
receive::receive( const wchar_t name[], client *p_dst, const bool flush_queue )
	:	m_p_destination( p_dst )
{	// Start the server
	const bool _check = FC3i::receive::start( name, this, flush_queue );
	assert( _check );

	if ( config::debug_receive_creation )
		FC3::debug::debug_output( FC3::config::debug_category, L"Audio/Video/xml server created, %s", name );
}

// Destructor
receive::~receive( void )
{	// Stop the server
	const bool _check = FC3i::receive::stop();
	assert( _check );
}

// The client implementation
void receive::deliver( const DWORD block_id, const DWORD addr )
{	// Try to create an audio message
	FC3::audio::message	*p_aud_msg = new FC3::audio::message( block_id, addr );	

	// Handle audio
	if ( !p_aud_msg->error() )
	{	// Deliver the message
		m_p_destination->deliver_audio( p_aud_msg );

		// Finished
		p_aud_msg->release();
		return;
	}	

	// Try to create an video message
	FC3::video::message	*p_vid_msg = new FC3::video::message( block_id, addr );	

	// The reason for this is that when a message is sent the reference count on the header
	// is incremented, and when the message is released it is then decremented. Because we
	// got the wrong message type we have to avoid the message being recycled by the release.
	p_vid_msg->simulate_send();
	p_aud_msg->release();

	// Handle video
	if ( !p_vid_msg->error() )
	{	// Deliver the message
		m_p_destination->deliver_video( p_vid_msg );

		// Finished
		p_vid_msg->release();
		return;
	}

	// Try to create an video message
	FC3::xml::message		*p_xml_msg = new FC3::xml::message( block_id, addr );	

	// See comments above on this
	p_xml_msg->simulate_send();
	p_vid_msg->release();

	// Handle video
	if ( !p_xml_msg->error() )
	{	// Deliver the message
		m_p_destination->deliver_xml( p_xml_msg );

		// Finished
		p_xml_msg->release();
		return;
	}

	// Try to create an video message
	FC3::raw::message		*p_raw_msg = new FC3::raw::message( block_id, addr );	

	// Unknown message type
	p_raw_msg->simulate_send();
	p_xml_msg->release();

	// Handle it
	if ( !p_raw_msg->error() )
	{	// Deliver the message
		m_p_destination->deliver_raw( p_raw_msg );

		// Finished
		p_raw_msg->release();
		return;
	}

	// Releast the raw message
	p_raw_msg->release();

	// Error in debug mode
	assert( false );
}