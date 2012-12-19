//**********************************************************************************
#include "stdafx.h"
#include "../FrameWork/FrameWork.h"

#include "profile.h"

__int64	profile::m_freq = 0;

// Constructor
profile::profile( bool used )
	:	m_used( used )
#ifdef	debug_profiler
		, m_last_time_ms( 0.0f )
#endif	debug_profiler
{	if ( !m_used ) return;
	if ( !m_freq ) ::QueryPerformanceFrequency( (LARGE_INTEGER*)&m_freq );

	// Reset
	m_frames = 0;
	m_total_time = 0;
	m_total_bytes = 0;
	m_last_start_time = -1;
}

// Start and stop profiling
void profile::start( void )
{	if ( !m_used ) return;

	// Set the time	
	::QueryPerformanceCounter( (LARGE_INTEGER*)&m_last_start_time );
}

void profile::stop( const DWORD size, const float debug_threshold_in_ms )
{	if ( ( !m_used ) || ( m_last_start_time < 0 ) ) return;

	// Get the time elapsed
	__int64	current_time;
	::QueryPerformanceCounter( (LARGE_INTEGER*)&current_time );

	__int64 delta = current_time - m_last_start_time;
	m_total_time  += delta;
	m_total_bytes += size;

	// This time in ms
	const float time_in_ms = 1000.0f * (float)delta / (float)m_freq;
	if ( ( time_in_ms > debug_threshold_in_ms ) && ( debug_threshold_in_ms ) ) 
	{	// Put a break point here to performance check
		int profile_performance_warning = 0;
	}

	// Store the last time, which is useful in debugging
	m_last_time_ms = time_in_ms;

	// No start time
	m_last_start_time = -1;

	// Reset the minimum and maximum
	if ( !m_frames ) 
			m_min = delta, m_max = delta;
	// Update the minimum and maximum
	else	m_min = min( m_min, delta ),
			m_max = max( m_max, delta );

	m_frames++;	
}

void profile::display( const wchar_t* p_title )
{	if ( !m_used ) return;
	if ( m_frames >= profile_config::display_avg )
	{	// In seconds
		const float time = (float)m_total_time  / (float)( m_frames*m_freq );
		const float size = (float)m_total_bytes / (float)( m_frames*1000*1000 );

		// The bounds on time
		const float min_time = (float)m_min  / (float)m_freq;
		const float max_time = (float)m_max  / (float)m_freq;

		// Output
		FrameWork::DebugOutput( size ? "[%s] avg=%5.2fms (min=%5.2fms, max=%5.2fms), %5.1fMb/s.\n" :
										   "[%s] avg=%5.2fms (min=%5.2fms, max=%5.2fms).\n", 
										   p_title,
										   time*1000.0f, 
										   min_time*1000.0f,
										   max_time*1000.0f,
										   size / time );

		// Reset
		m_frames = 0;
		m_total_time = 0;
		m_total_bytes = 0;
	}
}

