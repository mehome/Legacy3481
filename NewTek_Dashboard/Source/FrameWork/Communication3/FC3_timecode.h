#pragma once

/*	fc3::timecode::clock is a clock that will accurately follow the system clock over long
	periods of time, in order to provide the capability to use TOD time-code for use in stamping
	video frames with the current time-code.

	If you are sending video frames from a device in a loop, the following would be an example
	of how to use this.

	FC3::time_code::clock	myclock;
	while( true )
	{	FC3::video::message *p_msg = render_frame();
		p_msg->time_code() = myclock( p_msg->frame_rate().frame_time() );
		p_msg->send( somewhere );
	}

	If you have multiple inputs that you want to have locked to the exact same reference
	clock. The best way to do this is most likely. Do not copy this code as is without
	understanding exactly how it works.

	FC3::time_code	*p_clocks[ no_clocks ] = { NULL };
	FC3::time_code	*p_master_clock = NULL;

	// Then when sending each frame on input n
	if ( !p_clocks[ n ] )
	{	if ( p_master_clock ) p_clocks[ n ] = new FC3::time_code::clock( *p_master_clock ); 
		else p_clocks[ n ] = p_master_clock = new FC3::time_code::clock();
	}
	p_msg->time_code() = ( *p_clocks[ n ] )( p_msg->frame_rate().frame_time() );
	p_msg->send( somewhere );


	The __int64 values are in the same units as FILETIME (i.e. the number of 100-nanosecond intervals since January 1, 1601). If you
	need to compute them relative to some other time, then simply get your other time as a __int64 and perform math to 
	get the relative time.

	It might not be totally obvious, but because this only uses time_elapsed_100ns to guage approximatly how fast frames are running
	you need to not worry about prevision on that value as long as it is reasonable (i.e. do not worry about whether the real time
	elapsed is 666666 or 666667.)
*/

struct FRAMEWORKCOMMUNICATION3_API clock
{			// Initialize the clock
			clock( void );
			clock( clock& sync_with );

			// This will return the current time. You tell it how long you expect the frame to be
			// in 100ns units, and it will combine this result with the current system time to get
			// a clock that accurately follows the system time over a long duration.
			const __int64 operator() ( const __int64 time_elapsed_100ns );

private:	// The current time, in 100ns units
			__int64	m_current_time;

			// The the current system clock time in 100ns units
			const __int64 get_current_clock_time( void ) const;
};