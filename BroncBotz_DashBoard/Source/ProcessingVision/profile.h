#pragma once


	namespace profile_config
	{	
		// Display timings
		static const int	display_avg				 = 100;	// 100
		static const bool	profile_enabled			 = 1;
		static const bool	profile_audio_write_packet_time = profile_enabled && 0;	
		static const bool	profile_video_write_packet_time = profile_enabled && 0;	
		static const bool   profile_codec_time = profile_enabled && 1;
		static const bool   profile_audio_codec_time = profile_enabled && 0;
		static const bool   profile_video_codec_time = profile_enabled && 1;
	};

struct profile
{			// Constructor
			profile( const bool used = true );

			// Start and stop profiling
			void start( void );
			void stop( const DWORD size = 0, const float debug_threshold_in_ms = 0.0 );
			void display( const wchar_t* p_title );

private:	// The total time
				   __int64	m_last_start_time;
			static __int64	m_freq;

			// The average time elapsed
			__int64	m_total_time;
			__int64	m_total_bytes;
			__int64	m_min, m_max;

			// Used to help debugging
			float	m_last_time_ms;

			// The number of frames that have elapsed
			int		m_frames;

			// Are we used ?
			bool	m_used;
};
