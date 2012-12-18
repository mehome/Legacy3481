#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3;
using namespace FC3::utilities;


// Get the current time-value		
const __int64 master_clock::clock( void* p_data ) const
{	// Because this is a slightly larger data structure, reading of it is not 
	// atomic.
	auto_lock	lock( m_queue_lock );

	// Copy the data
	if ( p_data )
	{	// There is per-frame data
		if ( m_clock.m_p_data )	::memcpy( p_data, m_clock.m_p_data, m_data_size );

		// Just put nothing there
		else					::memset( p_data, 0, m_data_size );		
	}

	// Return the clock value
	return m_clock.m_time_stamp;
}

// These are just utility functions that add frames with no data
void master_clock::operator += ( const FrameWork::Communication3::video::message &msg )
{	add( msg );
}

void master_clock::operator += ( const FrameWork::Communication3::audio::message &msg )
{	add( msg );
}

// This will query the performance counter in 10ns intervals
const __int64 master_clock::get_clock( void ) const
{	// Get the current time
	LARGE_INTEGER current_time;
	::QueryPerformanceCounter( &current_time );	

	// Return the result
	return current_time.QuadPart;
}

// Flush all buffers on the queue
void master_clock::flush_queues( void )
{	// Free any remaining frames
	while( !m_video_frames.empty() )
	{	// Get the frame
		video_desc frame_desc = m_video_frames.front();
		m_video_frames.pop_front();

		// Release the frame data
		release_data( frame_desc.m_p_data );
		if ( frame_desc.m_p_frame ) 
			frame_desc.m_p_frame->release();
	}

	while( !m_audio_frames.empty() )
	{	// Get the frame
		audio_desc frame_desc = m_audio_frames.front();
		m_audio_frames.pop_front();

		// Release the frame data
		release_data( frame_desc.m_p_data );
		if ( frame_desc.m_p_frame ) 
			frame_desc.m_p_frame->release();
	}
}

void master_clock::flush_play_queue( void )
{	// Free any remaining frames
	while( ( !m_video_frames.empty() ) && ( m_video_frames.front().m_time >= 0 ) )
	{	// Get the frame
		video_desc frame_desc = m_video_frames.front();
		m_video_frames.pop_front();

		// Release the frame data
		release_data( frame_desc.m_p_data );
		if ( frame_desc.m_p_frame ) 
			frame_desc.m_p_frame->release();
	}

	while( ( !m_audio_frames.empty() ) && ( m_audio_frames.front().m_time >= 0 ) )
	{	// Get the frame
		audio_desc frame_desc = m_audio_frames.front();
		m_audio_frames.pop_front();

		// Release the frame data
		release_data( frame_desc.m_p_data );
		if ( frame_desc.m_p_frame ) 
			frame_desc.m_p_frame->release();
	}
}

void master_clock::flush_seek_queue( void )
{	// Free any remaining frames
	while( ( !m_video_frames.empty() ) && ( m_video_frames.front().m_time < 0 ) )
	{	// Get the frame
		video_desc frame_desc = m_video_frames.front();
		m_video_frames.pop_front();

		// Release the frame data
		release_data( frame_desc.m_p_data );
		if ( frame_desc.m_p_frame ) 
			frame_desc.m_p_frame->release();
	}

	while( ( !m_audio_frames.empty() ) && ( m_audio_frames.front().m_time < 0 ) )
	{	// Get the frame
		audio_desc frame_desc = m_audio_frames.front();
		m_audio_frames.pop_front();

		// Release the frame data
		release_data( frame_desc.m_p_data );
		if ( frame_desc.m_p_frame ) 
			frame_desc.m_p_frame->release();
	}
}

// Send the front video frame
const bool master_clock::send_video( void )
{	// Get the frame
	video_desc frame_desc = m_video_frames.front();
	m_video_frames.pop_front();
	
	// Send the frame
	if ( frame_desc.m_p_frame ) 
		FC3::utilities::safe_send_message( *frame_desc.m_p_frame, m_p_video_dst, 1 );

	// Keep the clock if it is newer than what we had
	if ( frame_desc.m_used_with_clock )
	{	// Release old data, then store the clock
		release_data( m_clock.m_p_data );
		m_clock.m_time_stamp  = frame_desc.m_time_stamp;
		m_clock.m_stream_time = frame_desc.m_time;
		m_clock.m_p_data      = frame_desc.m_p_data;
		m_clock.m_is_video    = true;
	}
	else
	{	// Just keep what we had and release this data
		release_data( frame_desc.m_p_data );
	}

	// Release the frame
	if ( frame_desc.m_p_frame ) 
		frame_desc.m_p_frame->release();

	// Should we trigger available buffers
	return ( m_video_frames.size() == m_queue_depth_video-1 );
}

const bool master_clock::send_audio( void )
{	// Get the frame
	audio_desc frame_desc = m_audio_frames.front();
	m_audio_frames.pop_front();
	
	// Send the frame
	if ( frame_desc.m_p_frame ) 
		FC3::utilities::safe_send_message( *frame_desc.m_p_frame, m_p_audio_dst, 1 );	

	// Keep the clock if it is newer than what we had
	if ( frame_desc.m_used_with_clock )
	{	// Release old data, then store the clock
		release_data( m_clock.m_p_data );
		m_clock.m_time_stamp  = frame_desc.m_time_stamp;
		m_clock.m_stream_time = frame_desc.m_time;
		m_clock.m_p_data      = frame_desc.m_p_data;
		m_clock.m_is_video    = false;
	}
	else
	{	// Just keep what we had and release this data
		release_data( frame_desc.m_p_data );
	}

	// Release the frame
	if ( frame_desc.m_p_frame ) 
		frame_desc.m_p_frame->release();

	// Should we trigger available buffers
	return ( m_audio_frames.size() == m_queue_depth_audio-1 );
}

const HANDLE master_clock::clock_event( void ) const
{	// Just return the handle for the clock
	return m_clock_updated;
}

// Get and release data items
void*	master_clock::get_data( const void* p_src )
{	// If no data is being used, do nothing
	if ( !p_src ) return NULL;
	if ( !m_data_size ) return NULL;

	// get a new block
	void* p_ret;
	if ( m_data_list.empty() )
	{	// Allocate a new block
		p_ret = ::malloc( m_data_size );
	}
	else
	{	// User an existing one
		p_ret = m_data_list.back();
		m_data_list.pop_back();
	}

	// Copy the data
	::memcpy( p_ret, p_src, m_data_size );

	// Return the results
	return p_ret;
}

void	master_clock::release_data( void* p_src )
{	// Add it to the free list
	if ( p_src ) m_data_list.push_back( p_src );
}

// Updat the reference time
void master_clock::update_reference_time( void )
{	// Does the reference time need updating ?
	if ( !m_stream_reference_time )
	{	// Get the clock
		const __int64 clock_time = get_clock();

		// Thrsholds on playback size
		const int video_threshold = std::max( 2, m_queue_depth_video/2 );
		const int audio_threshold = std::max( 2, m_queue_depth_audio/2 );
		
		// We start playback when one of the queues is full.
		if ( (int)m_video_frames.size() >= video_threshold ) 
		{	// If it looks like there might be audio, make sure that some ammount is buffered as well
			if ( ( !m_audio_frames.size() ) || ( (int)m_audio_frames.size() >= video_threshold/2 ) )
				// We are now playing, so the current stream time is the best we can do
				m_stream_reference_time = clock_time;
		}
		else if ( (int)m_audio_frames.size() >= audio_threshold )
		{	// If it looks like there might be video, make sure that some ammount is buffered as well
			if ( ( !m_video_frames.size() ) || ( (int)m_video_frames.size() >= audio_threshold/2 ) )
				// We are now playing, so the current stream time is the best we can do
				m_stream_reference_time = clock_time;
		}

		// Have we timed out
		if ( ( clock_time - m_stream_playback_started_at > m_time_out ) &&
			 ( m_audio_frames.size() || m_video_frames.size() ) )
		{	// We are now playing, so the current stream time is the best we can do
			m_stream_reference_time = clock_time;
		}

		// Offset first frame by a small ammount to avoid any potential TBC misses in audio or video
		if ( m_stream_reference_time )
			// Make it so that we are already slightly behind to cause first pair of frames to be 
			// be delivered more quickly than usual.			
			m_stream_reference_time -= ( m_freq * start_delay ) / 1000;
	}
	else
	{	// The clock is running
		// If the queues are empty, then we have dropped frames
		if ( ( m_audio_frames.empty() ) && ( m_video_frames.empty() ) )
		{	// We stop the playback clock to allow the buffers to fill up again
			m_stream_reference_time = 0;

			// Avoid A + V going out of sync
			// Note that when playing a long play list there can be a case where
			// audio has not played for quite some time (i.e. seconds!) thie means
			// that the using the minimum of the audio and video time can schedule
			// video frames way into the future. which results in the video hanging.
			// It is ultimately probably better to cause audio to potentially be slightly
			// out of sync on long packets and then simply cause a resync at the end 
			// of the clip than the alternatices.
			//const __int64 min_time = std::min( m_stream_time_video, m_stream_time_audio );
			//m_stream_time_video -= min_time;
			//m_stream_time_audio -= min_time;
			m_stream_time_video = 0;
			m_stream_time_audio = 0;

			// Store when playback got reset so that we do not just start again immediately
			m_stream_playback_started_at = get_clock();
		}
	}
}

const bool master_clock::dice_audio_buffers( const FrameWork::Communication3::audio::message &msg, const void* p_data, const bool used_with_clock )
{	// Get the no samples and sample-rate
	const int no_samples  = msg.no_samples();
	const int sample_rate = msg.sample_rate();
	const int no_channels = msg.no_channels();

	// Get the length in samples of the desired buffer
	const int limit_no_samples = ( abs( sample_rate ) * audio_buffer_len ) / 1000;

	// If the buffer is already the right size, we just do not do anything
	if ( no_samples <= limit_no_samples ) return false;

	// We want the number of samples per buffer to be close to constant. This rounds up !
	int buffer_no_samples = no_samples / ( ( no_samples + limit_no_samples - 1 ) / limit_no_samples );

	// Let some sanity prevail. Have a multiple of 16 samples in each buffer if possible.
	buffer_no_samples = ( buffer_no_samples + 15 ) & ( ~15 );

	// Just check my own logic
	assert( buffer_no_samples <= limit_no_samples );

	// Wait for space on the list
	const bool time_out = ( ::WaitForSingleObject( m_event_audio_available, m_add_is_blocking ? 3000 : 1 ) != WAIT_OBJECT_0 );	

	// Lock the queue
	m_queue_lock.lock();

	// Now generate the output buffers
	for( int i=0; ; )
	{	// Get the no samples actually needed
		const int no_samples_to_copy = std::min( i + buffer_no_samples, no_samples ) - i;
		if ( no_samples_to_copy <= 0 ) break;

		// Get the samples to copy
		const int sample_start = ( sample_rate < 0 ) ? ( no_samples - i - no_samples_to_copy ) : i;

		// Allocate a new message
		FrameWork::Communication3::audio::message* p_msg = new FrameWork::Communication3::audio::message( no_samples_to_copy, no_channels, 0 );
		
		// Store buffer properties
		p_msg->sample_rate() = sample_rate;		

		// Copy the correct number of samples		
		FAUD::buffer_f32 dst; dst.reference_in_bytes( (float*)&msg.audio()( 0, sample_start ), no_channels, no_samples_to_copy, msg.audio().stride_in_bytes() );
		p_msg->audio() = dst;		

		// Add it by calling myself
		add_raw( *p_msg, i ? NULL : p_data, i ? false : used_with_clock );
		p_msg->release();

		// Increment the sample position
		i += no_samples_to_copy;
	}

	// Lock the queue
	m_queue_lock.unlock();

	// We used diced buffers
	return true;
}