#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::audio_video;

// Constructor
receive::receive( const wchar_t name[], client *p_dst, const bool flush_queue )
	:	m_p_destination( p_dst ),
		m_p_vid_msg( NULL ), m_p_aud_msg( NULL ),
		m_sync_av( false )
{	// Debugging ;)
	assert( m_p_destination );	

	// Start the server
	const bool _check = FrameWork::Communication3::implementation::receive::start( name, this, flush_queue );
	assert( _check );

	if ( config::debug_receive_creation )
		FrameWork::Communication3::debug::debug_output( FrameWork::Communication3::config::debug_category, L"Audio/Video server created, %s", name );
}

// Destructor
receive::~receive( void )
{	// Stop the server
	const bool _check = FrameWork::Communication3::implementation::receive::stop();
	assert( _check );
}

// The client implementation
void receive::deliver( const DWORD block_id, const DWORD addr )
{	// Try to create an audio message
	FrameWork::Communication3::audio::message	*p_aud_msg = new FrameWork::Communication3::audio::message( block_id, addr );

#define RELEASE( a ) { a->release(); a=NULL; }

	// If it was not audio, it was video
	FrameWork::Communication3::video::message	*p_vid_msg = NULL;
	if ( p_aud_msg->error() )
	{	p_vid_msg = new FrameWork::Communication3::video::message( block_id, addr );
		p_vid_msg->simulate_send();
		RELEASE( p_aud_msg );

		// If it was not video, it is an error
		if ( p_vid_msg->error() )
		{	RELEASE( p_vid_msg );
			
			// Nothing worth doing
			return;
		}
	}

	if ( m_sync_av )
	{	// Deliver any old frames
		if ( m_p_vid_msg ) { m_p_destination->deliver_video_audio( m_p_vid_msg, NULL ); RELEASE( m_p_vid_msg ); }
		if ( m_p_aud_msg ) { m_p_destination->deliver_video_audio( NULL, m_p_aud_msg ); RELEASE( m_p_aud_msg ); }

		// Deliver this frame
		if ( p_vid_msg ) { m_p_destination->deliver_video_audio( p_vid_msg, NULL ); RELEASE( p_vid_msg); }
		if ( p_aud_msg ) { m_p_destination->deliver_video_audio( NULL, p_aud_msg ); RELEASE( p_aud_msg); }

		// Finished
		return;
	}

	// Update the buffers that we have internally
	if ( p_vid_msg ) 
	{	// Send old messages
		if ( m_p_vid_msg )
		{	m_p_destination->deliver_video_audio( m_p_vid_msg, NULL );
			RELEASE( m_p_vid_msg );
		}

		// Store
		m_p_vid_msg = p_vid_msg;		
	}
	else // if ( p_aud_msg ) 
	{	// Send old messages
		if ( m_p_aud_msg )
		{	m_p_destination->deliver_video_audio( NULL, m_p_aud_msg );
			RELEASE( m_p_aud_msg );
		}

		// Store
		m_p_aud_msg = p_aud_msg;		
	}

	// if we have a stored audio and video message
	if ( ( m_p_vid_msg ) && ( m_p_aud_msg ) )
	{	// If they both are on the same time-stamp, feliver them together
		if ( m_p_vid_msg->time_stamp() == m_p_aud_msg->time_stamp() )
		{	m_p_destination->deliver_video_audio( m_p_vid_msg, m_p_aud_msg );
			RELEASE( m_p_vid_msg );
			RELEASE( m_p_aud_msg );
		}
		// Deliver the older one first
		else if ( m_p_vid_msg->time_stamp() < m_p_aud_msg->time_stamp() )
		{	m_p_destination->deliver_video_audio( m_p_vid_msg, NULL );
			RELEASE( m_p_vid_msg );
		}
		else // if ( m_p_vid_msg->time_stamp() > m_p_aud_msg->time_stamp() )
		{	m_p_destination->deliver_video_audio( NULL, m_p_aud_msg );
			RELEASE( m_p_aud_msg );
		}
	}

#undef RELEASE
}

void receive::set_av_sync( const bool flag )
{	m_sync_av = flag;
}

bool receive::get_av_sync( void ) const
{	return m_sync_av;
}