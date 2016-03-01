#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3;
using namespace FC3::utilities;

// Constructor
master_clock::master_clock( // The audio and video FC3 server destinations to send frames too.
							const wchar_t *p_video_dst, 
							const wchar_t *p_audio_dst, 
							// With each frame you can provide a block of data that can be used to 
							// identify the last frames displayed. This is the size of that data
							const int per_frame_data_size,
							// The internal queue depth to use. Once you have delivered this many frames
							// that have not yet "come due" (i.e. sent to destination, or the current
							// distance ahead of output) the calls to add new frames will block until
							// the queue depths are correct.
							const int video_queue_depth, 
							const int audio_queue_depth, 
							master_clock_idle_interface *p_idle )
	:	// Allocate the name
		m_p_video_dst( p_video_dst ? ( new wchar_t [ ::wcslen( p_video_dst ) + 1 ] ) : NULL ),
		m_p_audio_dst( p_audio_dst ? ( new wchar_t [ ::wcslen( p_audio_dst ) + 1 ] ) : NULL ),

		// Store the data size
		m_data_size( per_frame_data_size ),

		// Store the queue depths
		m_queue_depth_video( video_queue_depth ),
		m_queue_depth_audio( audio_queue_depth ),

		// The events
		m_event_video_available( ::CreateEvent( NULL, TRUE, TRUE, NULL ) ),
		m_event_audio_available( ::CreateEvent( NULL, TRUE, TRUE, NULL ) ),
		m_frames_available( ::CreateEvent( NULL, TRUE, FALSE, NULL ) ),
		m_clock_updated( ::CreateEvent( NULL, FALSE, FALSE, NULL ) ),

		// The clock thread is still running
		m_clock_thread_must_exit( false ),

		// No reference clock time yet
		m_stream_reference_time( 0 ),
		m_stream_time_video( 0 ),
		m_stream_time_audio( 0 ),
		m_stream_playback_started_at( 0 ),

		// The current initial state is set to paused
		m_state( state_paused ),

		// The add call is blocking by default
		m_add_is_blocking( true ),

		// Store the idle interface
		m_last_send_time( ::GetTickCount() ),
		m_p_idle( p_idle ),
		m_idle_sent( true )

{	// The default clock
	m_clock.m_p_data = NULL;
	m_clock.m_stream_time = 0LL;
	m_clock.m_time_stamp = 0LL;
	m_clock.m_is_video = false;
	
	// Store the names
	if ( p_video_dst ) ::wcscpy( m_p_video_dst, p_video_dst );
	if ( p_audio_dst ) ::wcscpy( m_p_audio_dst, p_audio_dst );

	// Get the clock speed
	LARGE_INTEGER freq;
	::QueryPerformanceFrequency( &freq );
	m_freq = freq.QuadPart;

	// Get the timeout
	m_time_out = ( start_time_out * m_freq + 500 ) / 1000;

	// Start the clocking thread
	DWORD thread_id;
	m_clock_thread = ::CreateThread( NULL, /* 32kB */32*1024, threadproc, (void*)this, 0, &thread_id );
	assert( m_clock_thread );
	::SetThreadPriority( m_clock_thread, THREAD_PRIORITY_TIME_CRITICAL );
}

// Destructor
master_clock::~master_clock( void )
{	// Exit the clock thread	
	m_clock_thread_must_exit = true;
	::SetEvent( m_frames_available );
	::WaitForSingleObject( m_clock_thread, INFINITE );
	m_clock_thread = NULL;

	// Go idle
	if ( m_p_idle )
	{	FC3i::auto_lock	idle_lock( m_idle_lock );					
		if ( !m_idle_sent )
		{	m_p_idle->clock_idle();
			m_idle_sent = true;
		}
	}
	
	// Free the names
	if ( m_p_video_dst ) delete [] m_p_video_dst;
	if ( m_p_audio_dst ) delete [] m_p_audio_dst;

	// Free the queues
	flush_queues();

	// Close the events
	::CloseHandle( m_event_video_available );
	::CloseHandle( m_event_audio_available );
	::CloseHandle( m_frames_available );
	::CloseHandle( m_clock_updated );

	// Free memory
	while( !m_data_list.empty() )
	{	::free( m_data_list.back() );
		m_data_list.pop_back();
	}

	// Free anything left on the clock
	if ( m_clock.m_p_data ) 
		::free( m_clock.m_p_data );
}

// Call this when you want to reset the media time. This means that any frames with
// a time-stamp of "0" would play immediately. You can specify an offset to move the
// time at which reset is called. For instance, if you want frames with time-stamp
// "0" to play 100ms in the future, specify an offset of 100ms
void master_clock::play( void )
{	// Lock the queue
	m_queue_lock.lock();	
	
	// Changing from a paused state, to a playing one
	if ( m_state == state_paused )
	{	// We are currently in playing state. We reset the 
		m_state = state_playing;

		// Discard any frames left over from seeking
		flush_seek_queue();

		// Get the time
		const __int64 time = get_clock();

		// If the stream was running before it was paused then we
		// offset it to the current time.
		if ( m_stream_reference_time < 0 )
		{	m_stream_reference_time += time;
			assert( m_stream_reference_time > 0 );
		}

		// Store when playback started
		m_stream_playback_started_at = time;

		// Perform a reset ?
		if ( queues_empty() )
		{	m_stream_reference_time = 0;
			m_stream_time_video = 0;
			m_stream_time_audio = 0;
		}

		// Make sure that these events are set correctly
		if ( (int)m_video_frames.size() >= m_queue_depth_video )
				::ResetEvent( m_event_video_available );
		else	::SetEvent( m_event_video_available );

		if ( (int)m_audio_frames.size() >= m_queue_depth_audio )
				::ResetEvent( m_event_audio_available );
		else	::SetEvent( m_event_audio_available );

		// Unlock the queue
		m_queue_lock.unlock();
	}
	else
	{	// Unlock the queue
		m_queue_lock.unlock();
	}
}

void master_clock::pause( void )
{	// Lock the queue
	m_queue_lock.lock();

	// If we where previous playing
	if ( m_state == state_playing )
	{	// We are now in paused state
		m_state = state_paused;

		// If the stream was playing, we store the ammount of time that 
		// has passed since it was started. This must be less than zero
		if ( m_stream_reference_time )
		{	m_stream_reference_time -= get_clock();
			assert( m_stream_reference_time < 0 );
		}

		// Unlock the queue
		m_queue_lock.unlock();

		// There are always buffers free to go
		::SetEvent( m_event_video_available );
		::SetEvent( m_event_audio_available );
	}	
	else
	{	// Unlock the queue
		m_queue_lock.unlock();
	}
}

void master_clock::insert_sync_point( void )
{	// Lock the queue
	m_queue_lock.lock();

	if ( ( m_stream_time_audio < m_stream_time_video ) && 
		 // If the audio gap is quite big, allow the audio mixer to handle the delta. Basically if there is more than one frame delta
		 // we are best off leaving it up to the audio mixer. This occurs when going from clips with audio to those without.
		 ( ( m_stream_time_video - m_stream_time_audio ) < ( m_freq / 20 ) ) )
	{	// We insert silence to bring them back into sync
		int sample_rate = 96000;
		int no_channels = 2;
		for( int i=(int)m_audio_frames.size()-1; i>=0; i-- )
		if ( m_audio_frames[ i ].m_p_frame ) 
		{	sample_rate = abs( m_audio_frames[ i ].m_p_frame->sample_rate() );
			no_channels = m_audio_frames[ i ].m_p_frame->no_channels();
			break;
		}

		// Now compute how many samples this is
		const int no_samples = (int)( ( ( m_stream_time_video - m_stream_time_audio ) * sample_rate ) / m_freq );

		// Got to be something worth doing
		if ( no_samples > 64 )
		{	// Allocate a new sample
			FC3::audio::message* p_msg = new FC3::audio::message( no_samples, no_channels );
			p_msg->sample_rate() = sample_rate;
			p_msg->audio() = 0.0f;

			// Add this sample
			add_raw( *p_msg, NULL, false );
			p_msg->release();
		}
		else
		{	// Just change the stream times
			m_stream_time_video = 
			m_stream_time_audio = std::max( m_stream_time_video, m_stream_time_audio );
		}
	}
	else
	{	// Sync audio and video back up at this point
		// Not quite sure how to handle this case.
		m_stream_time_video = 
		m_stream_time_audio = std::max( m_stream_time_video, m_stream_time_audio );
	}

	// Unlock the queue
	m_queue_lock.unlock();
}

bool master_clock::is_paused( void ) const		
{	return ( m_state == state_paused );
}

// This allows us to specify whether we want the add calls to currently block or not. If any add calls are already
// blocked they will be released if this is switched off.
void master_clock::add_is_blocking( const bool flag )
{	// Set it as blocking or not. Note that I am just fine not having any critical sections
	// around this, this is because m_add_is_blocking is checked in ::add( * ) before the
	// event is waited on, so setting these will never result in a waiting event. What is more
	// since this is read only inside the ::add calls, there is no problem writing to it off
	// another thread.
	m_add_is_blocking = flag;

	// Release any locked add calls
	if ( !flag )
	{	::SetEvent( m_event_video_available );
		::SetEvent( m_event_audio_available );
	}
}

void master_clock::add( const __int64 time_stamp, const void* p_data, const bool used_with_clock )
{	// There is no point in this if its not being used as the clock
	if ( !used_with_clock ) return;
	
	// Lock the queue
	m_queue_lock.lock();

	// Are both queues empty ?
	const bool both_queues_empty = queues_empty();

	// Check the state
	if ( m_state == state_paused )
	{	// We need to flush any pending queue items
		flush_play_queue();

		// Add this item to the queue marked as a seeking frame
		video_desc new_item = { -1, time_stamp, get_data( p_data ), used_with_clock, NULL };
		m_video_frames.push_back( new_item );
	}
	else
	{	// Add it to the relevant queue
		if ( m_stream_time_video >= m_stream_time_audio )
		{	// Store the video frame
			video_desc new_item = { m_stream_time_video++, time_stamp, get_data( p_data ), used_with_clock, NULL };
			m_video_frames.push_back( new_item );
		}
		else
		{	// Store the audio frame
			audio_desc new_item = { m_stream_time_audio++, time_stamp, get_data( p_data ), used_with_clock, NULL };
			m_audio_frames.push_back( new_item );
		}
	}

	// Trigger there as items on the queue if this is the first item that makes them both available
	if ( both_queues_empty ) 
		::SetEvent( m_frames_available );

	// Unlock the queue
	m_queue_lock.unlock();
}

// Add a video or audio frame to the queue
const bool master_clock::add_direct( const FC3::video::message &msg )
{	// Quick exit
	if ( !m_p_video_dst ) return true;

	// Flush the queues
	flush();

	// Send the frame
	send_frame();
	return msg.send( m_p_video_dst );
}

const bool master_clock::add_direct( const FC3::audio::message &msg )
{	// Quick exit
	if ( !m_p_audio_dst ) return true;

	// Flush the queues
	flush();

	// Send the frame
	send_frame();
	return msg.send( m_p_audio_dst );
}

// Add a video or audio frame to the queue
void master_clock::add( const FC3::video::message &msg, const void* p_data, const bool used_with_clock, const bool display_frame )
{	// Send it directly
	if ( !m_queue_depth_video ) 
	{	// If the frame is to be displayed
		if ( display_frame )
		{	add_direct( msg );
			assert( !p_data );
		}
		return;
	}
	
	// Quick exit
	if ( !m_p_video_dst ) return;

	// Start things up as needed
	send_frame();

	// Data is only used for the clock
	if ( !used_with_clock ) p_data = NULL;

	// Get the frame length (note that this does integer rounding)
	// Note that when scrubbing backwards frames can specify a negative frame-rate.
	const int fr_n = abs( msg.frame_rate().n() ) * ( msg.has_one_field() ? 2 : 1 );
	const int fr_d = abs( msg.frame_rate().d() );	
	const __int64 frame_length = ( fr_d*m_freq + fr_n/2 ) / fr_n;	

	// Wait for space on the list
	const bool time_out = ( ::WaitForSingleObject( m_event_video_available, m_add_is_blocking ? 3000 : 1 ) != WAIT_OBJECT_0 );	

	// Lock the queue
	m_queue_lock.lock();

	// Are both queues empty ?
	const bool both_queues_empty = queues_empty();

	// Do we want to trigger frames ?
	if ( display_frame ) msg.addref();
	if ( m_state == state_paused )
	{	// We need to flush any pending queue items
		flush_play_queue();		

		// Add this item to the queue marked as a seeking frame
		video_desc new_item = { -1, msg.time_stamp(), get_data( p_data ), used_with_clock, display_frame ? &msg : NULL }; 
		m_video_frames.push_back( new_item );
	}
	else
	{	// Add this item to the queue
		video_desc new_item = { m_stream_time_video, msg.time_stamp(), get_data( p_data ), used_with_clock, display_frame ? &msg : NULL }; 		
		m_video_frames.push_back( new_item );

		// Increment the stream time
		m_stream_time_video += frame_length;

		// If the queue is full, wait next time through
		if ( (int)m_video_frames.size() >= m_queue_depth_video ) 
			::ResetEvent( m_event_video_available );
	}

	// Trigger there as items on the queue if this is the first item that makes them both available
	if ( both_queues_empty ) 
		::SetEvent( m_frames_available );

	// Unlock the queue
	m_queue_lock.unlock();
}

void master_clock::add( const FC3::audio::message &msg, const void* p_data, const bool used_with_clock, const bool display_frame )
{	// Send it directly
	if ( !m_queue_depth_audio ) 
	{	// If the frame is to be displayed
		if ( display_frame )
		{	add_direct( msg );
			assert( !p_data );
		}
		return;
	}

	// Quick exit
	if ( !m_p_audio_dst ) return;

	// Start things up as needed
	send_frame();

	// Data is only used for the clock
	if ( !used_with_clock ) p_data = NULL;

	// Dice buffers
	if ( ( !display_frame ) || ( !dice_audio_buffers( msg, p_data, used_with_clock ) ) )
	{	// Wait for space on the list
		const bool time_out = ( ::WaitForSingleObject( m_event_audio_available, m_add_is_blocking ? 3000 : 1 ) != WAIT_OBJECT_0 );	

		// Add the raw buffer
		add_raw( msg, p_data, used_with_clock, display_frame );
	}
}

void master_clock::add_raw( const FC3::audio::message &msg, const void* p_data, const bool used_with_clock, const bool display_frame )
{	// Get the frame length (note that this does integer rounding)
	const int no_samples  = msg.no_samples();
	const int sample_rate = abs( msg.sample_rate() );
	const __int64 frame_length = ( no_samples*m_freq + sample_rate/2 ) / sample_rate;
	
	// Lock the queue
	m_queue_lock.lock();

	// Are both queues empty ?
	const bool both_queues_empty = queues_empty();

	// Do we want to trigger frames ?
	if ( display_frame ) msg.addref();
	if ( m_state == state_paused )
	{	// We need to flush any pending queue items
		flush_play_queue();		

		// Add this item to the queue marked as a seeking frame
		audio_desc new_item = { -1, msg.time_stamp(), get_data( p_data ), used_with_clock, display_frame ? &msg : NULL }; 
		m_audio_frames.push_back( new_item );
	}
	else
	{	// Add this item to the queue
		audio_desc new_item = { m_stream_time_audio, msg.time_stamp(), get_data( p_data ), used_with_clock, display_frame ? &msg : NULL }; 
		m_audio_frames.push_back( new_item );

		// Increment the stream time
		m_stream_time_audio += frame_length;

		// If the queue is full, wait next time through
		if ( (int)m_audio_frames.size() >= m_queue_depth_audio ) 
			::ResetEvent( m_event_audio_available );
	}

	// Trigger there as items on the queue if this is the first item that makes them both available
	if ( both_queues_empty ) 
		::SetEvent( m_frames_available );

	// Unlock the queue
	m_queue_lock.unlock();
}

// The thread proc
DWORD WINAPI master_clock::threadproc( void* lpParameter )
{	// Process
	return ( (master_clock*)lpParameter )->send_all_frames();
}

// Flush all queues, use this judiciously !
void master_clock::flush( void )
{	// Lock the queue
	m_queue_lock.lock();

	// Flush the data
	flush_queues();

	// Unlock the queue
	m_queue_lock.unlock();

	// This is a no-brainer, we can add frames to the queue
	::SetEvent( m_event_video_available );
	::SetEvent( m_event_audio_available );
}

// The thread function
const DWORD master_clock::send_all_frames( void )
{	// Give this thread a name to help in debugging
	{ char temp[ 1024 ];
	  ::sprintf( temp, "master_clock[%ls,%ls]", m_p_video_dst, m_p_audio_dst );
	  FC3i::set_thread_name( temp );
	}	// Make sure temp does not stay on the stack
	
	// While we still need to be running
	while( !m_clock_thread_must_exit )
	{	// If there are no frames available, this efficiently sleeps, which avoids
		// this spinning in a loop doing nothing when there are no frames. This also
		// reduces thread contention around the locks
		if ( ::WaitForSingleObject( m_frames_available, 500 ) == WAIT_TIMEOUT )
		{	// If there was no frame sent directly we should probably consider ourselves idle
			if ( ( m_p_idle ) && ( ::GetTickCount() - m_last_send_time > idle_time_out ) )
			{	// We are idle
				FC3i::auto_lock	idle_lock( m_idle_lock );					
				if ( !m_idle_sent )
				{	m_p_idle->clock_idle();
					m_idle_sent = true;
				}
			}
		}

		// Keep track of whether the audio and video are going to have newly free queues.
		bool trigger_video = false;
		bool trigger_audio = false;		

		// Lock the queue. Note that I do not do a hard lock here since
		// this is on a time critical thread.
		if ( !m_queue_lock.try_lock() )
		{	::Sleep( 1 );
			continue;
		}

		// Check for clock updates
		const clock_type old_clock = m_clock;

		// If we are seeking
		if ( m_state == state_paused )
		{	// Send any audio frames
			while( ( !m_audio_frames.empty() ) && ( m_audio_frames.front().m_time < 0 ) ) send_audio();
			
			// Send any video frames
			while( ( !m_video_frames.empty() ) && ( m_video_frames.front().m_time < 0 ) ) send_video();			
		}
		else
		{	// Start playback if required
			update_reference_time();
		
			// Note that if the stream time is not yet set we are still buffering
			if ( m_stream_reference_time )
			{	// Get the current computer time. We get this time inside of the lock to ensure that any
				// lock time overhead is accounted for in what we display.
				const __int64 clock = get_clock() - m_stream_reference_time;			

				// Send frames in temporal order
				while( true )
				{	// Get the time to play the next sample
					const __int64	audio_time = m_audio_frames.empty() ? _I64_MAX : m_audio_frames.front().m_time;
					const __int64	video_time = m_video_frames.empty() ? _I64_MAX : m_video_frames.front().m_time;

					// Take the one that occurs the sooner
					if ( video_time <= audio_time )
					{	// No frame to sent ?
						if ( video_time > clock ) break;

						// Send the frame
						trigger_video |= send_video();
					}
					else
					{	// No frame to sent ?
						if ( audio_time > clock ) break;

						// Send the frame
						trigger_audio |= send_audio();
					}
				}
			}
		}

		// Has the clock been updated
		const bool clock_updated = ( ::memcmp( &old_clock, &m_clock, sizeof( clock_type ) ) ? true : false );

		// Lock the queue
		m_queue_lock.unlock();

		// If the queues have fallen below length at which new frames can be added,
		// then we go ahead and add frames. Note that we do this outside of the lock.
		if ( trigger_video ) 
			::SetEvent( m_event_video_available );

		if ( trigger_audio ) 
			::SetEvent( m_event_audio_available );

		// Trigget a clock event if needed
		if ( clock_updated ) 
			::SetEvent( m_clock_updated );

		// Throttle us back
		::Sleep( 1 );
	}

	// Finished
	return 0;
}
