#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3;
using namespace FC3::timecode;


// Initialize the clock
clock::clock( void )
	:	m_current_time( 0 )
{
}

// Build a clock and sync with an existing clock
clock::clock( clock& sync_with )
	:	m_current_time( sync_with.m_current_time )
{
}

const __int64 clock::operator() ( const __int64 time_elapsed )
{	// If there is no time that has elapsed since we started
	if ( !m_current_time )
	{	// Get the current system time
		m_current_time = get_current_clock_time();
	}
	else
	{	// Update the current time
		m_current_time += time_elapsed;
		
		// Get the difference between the predicted and real time
		const __int64 time_error = get_current_clock_time() - m_current_time;

		// If we are more than 1s away from the system clock, we jump to the correct time.
		if ( ( time_error > 10000000LL ) || ( time_error < -10000000LL ) )
		{	// Correct entirely
			m_current_time += time_error;
		}
		else
		{	// Apply a correction that gets us 0.1% closer to the system clock
			m_current_time += time_error / 1024;		
		}
	}

	// Return the result
	return m_current_time;
}

// The the current system clock time in 100ns units
const __int64 clock::get_current_clock_time( void ) const
{	// Even though this conversion can be done directly, MSDN recommends otherwise :
	// Do not cast a pointer to a FILETIME structure to either a LARGE_INTEGER* or __int64* value because it can cause alignment faults on 64-bit Windows.
	FILETIME	ft;
	::GetSystemTimeAsFileTime( &ft );
	::FileTimeToLocalFileTime( &ft, &ft );

	// Get it as a 64-bit number
	LARGE_INTEGER li;
	li.LowPart  = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	// Return the 64bit integer
	return li.QuadPart;
}