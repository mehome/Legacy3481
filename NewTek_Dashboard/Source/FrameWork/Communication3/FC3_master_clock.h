#pragma once

struct FRAMEWORKCOMMUNICATION3_API master_clock
{			// Constructor
			master_clock( // The audio and video FC3 server destinations to send frames too.
				          const wchar_t *p_video_dst, 
						  const wchar_t *p_audio_dst, 
						  // With each frame you can provide a block of data that can be used to 
						  // identify the last frames displayed. This is the size of that data
						  const int per_frame_data_size = 0,
						  // The internal queue depth to use. Once you have delivered this many frames
						  // that have not yet "come due" (i.e. sent to destination, or the current
						  // distance ahead of output) the calls to add new frames will block until
						  // the queue depths are correct.
						  const int video_queue_depth = 8, 
						  const int audio_queue_depth = 8);

			// Destructor
			~master_clock( void );

			// Play and pause the output
			void play( void );
			void pause( void );

			// This allows us to specify whether we want the add calls to currently block or not. If any add calls are already
			// blocked they will be released if this is switched off.
			void add_is_blocking( const bool flag );

			// Because the audio and video in a file might not have exactly the same length this allows you to re-sync them at the playbacl
			// start time.
			void insert_sync_point( void );

			// Add a video or audio frame to the queue
			void operator += ( const FrameWork::Communication3::video::message &msg );
			void operator += ( const FrameWork::Communication3::audio::message &msg );			

			// Add a video and audio frame with a identifier.			
			void add( const FrameWork::Communication3::video::message &msg, const void* p_data = NULL, const bool used_with_clock = true, const bool display_frame = true );
			void add( const FrameWork::Communication3::audio::message &msg, const void* p_data = NULL, const bool used_with_clock = true, const bool display_frame = true );

			// This adds a "dummy" frame the queue that you will see notification of in the clock callback.
			// This frame has no duration and is just a marker in the queue and so is not used for clocking (etc...)
			void add( const __int64 time_stamp, const void* p_data = NULL, const bool used_with_clock = true );

			// Get the current time-value		
			const __int64 clock( void* p_data = NULL ) const;

			// This returns an event to you that is triggered each time the clock gets updated
			// (i.e. a frame is sent to the switcher.) Making an immediate call to the clock()
			// function upon this trigger is likely very fast (i.e. this is triggered at a good time.)
			const HANDLE clock_event( void ) const;

			// Flush all queues, use this judiciously !
			void flush( void );

			// Return "true" if paused; "false" if playing
			bool is_paused( void ) const;
			
private:	// The start graph delay. This ensures that the first and second samples
			// delivered are slightly closer together than they actually should be.
			// Making it easier on the switcher and audio mixer to avoid drops. This
			// is not really a very bug deal.
			static const int start_delay = 8;			// ms

			// After this long, always start playing
			static const int start_time_out = 150;		// ms

			// The maximum length we want for audio buffers
			static const int audio_buffer_len = 75;		// ms

			// Raw adding of audio
			void add_raw( const FrameWork::Communication3::audio::message &msg, const void* p_data = NULL, const bool used_with_clock = true, const bool display_frame = true );
	
			// The destinations
			wchar_t *m_p_video_dst;
			wchar_t *m_p_audio_dst;

			// The current "state"
			enum { state_playing, state_paused } m_state;

			// The queue depths
			const int m_queue_depth_video;
			const int m_queue_depth_audio;

			// The data block size
			const int m_data_size;

			// The clock freqeuence
			__int64	m_freq;
			__int64 m_time_out;

			// The lock on the audio queue
			typedef FrameWork::Communication3::implementation::critical_section critical_section;
			typedef FrameWork::Communication3::implementation::auto_lock auto_lock;
			mutable critical_section m_queue_lock;

			// Audio and video frame descriptions
			struct	audio_desc
			{	__int64	m_time, m_time_stamp;
				void*	m_p_data;
				bool	m_used_with_clock;
				const FrameWork::Communication3::audio::message *m_p_frame;
			};

			struct	video_desc
			{	__int64	m_time, m_time_stamp;
				void*	m_p_data;
				bool	m_used_with_clock;
				const FrameWork::Communication3::video::message *m_p_frame;
			};

			// The audio and video queues
			std::deque< audio_desc >	m_audio_frames;
			std::deque< video_desc >	m_video_frames;

			// The reference clock time
			__int64	m_stream_playback_started_at;
			__int64	m_stream_reference_time;

			__int64	m_stream_time_video;
			__int64	m_stream_time_audio;

			// The current output clock time
			struct	clock_type
			{	__int64	m_stream_time;
				__int64	m_time_stamp;
				void*   m_p_data;
				bool	m_is_video;

			}	m_clock;

			// This event signals that the queue is currently has empty spots in it
			HANDLE	m_event_video_available;
			HANDLE	m_event_audio_available;
			HANDLE	m_frames_available;
			HANDLE	m_clock_updated;
			bool	m_add_is_blocking;

			// The thread
			bool	m_clock_thread_must_exit;
			HANDLE	m_clock_thread;

			// The current set of data block
			std::vector< void* >	m_data_list;
			
			// The thread proc
			static DWORD WINAPI threadproc( void* lpParameter );

			// The thread function
			const DWORD send_all_frames( void );

			// Send the front video frame
			const bool send_video( void );
			const bool send_audio( void );

			// Get the master system clock in 10ns units
			const __int64 get_clock( void ) const;

			// Get and release data items
			void*	get_data( const void* p_src );
			void	release_data( void* p_src );

			// Flush all buffers on the queue
			void flush_queues( void );
			void flush_play_queue( void );
			void flush_seek_queue( void );			

			// Update the reference time
			void update_reference_time( void );

			// This will dice audio buffers into smaller pieces
			const bool dice_audio_buffers( const FrameWork::Communication3::audio::message &msg, const void* p_data, const bool used_with_clock );
};