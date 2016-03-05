#include "StdAfx.h"
#include "FrameWork.Audio2.h"

using namespace FrameWork::Audio2::effects;

__forceinline const float sqr( const float x ) { return x*x; }

compressor_limiter::compressor_limiter( const compressor_limiter& from )
{	// Copy the settings
	operator= ( from );

	// Everything defaults to no gain
	m_reset = true;
}

compressor_limiter::compressor_limiter( void )
	:	m_threshold( +50.0f ),		// The upper limit is very high
		m_attack( 50.0f ),			// The attack is very fast
		m_release( 50.0f ),			// The release is very fast
		m_ratio( 10.0f ),			// 10:1 compression ratio
		m_makeup_gain( 0.0f ),		// Do not boost the signal in any additional way
		m_stereo( false ),			// Are we acting as a stereo compressor-limiter
		m_enable( true )
{	// Everything defaults to no gain
	m_reset = true;
}

// Apply the effect to a buffer
void compressor_limiter::operator() ( FrameWork::Audio2::buffer_f32& src, const int sample_rate )
{	// Quick exit
	if ( !m_enable ) 
	{	// Reset the gains so it is not weird when you switch it back on
		m_reset = true;
		
		// Finished
		return;
	}
	
	// Some constants
	const int	no_channels	= src.no_channels();
	const int	no_samples	= src.no_samples();
	const float buffer_length_ms = 1000.0f * (float)no_samples / (float)sample_rate;
	
	// The first step is to measure the RMS power across all of the channels
	float vu_levels[ max_no_channels ], max_vu_level = 0.0f;
	for( int ch=0; ch<max_no_channels; ch++ )
	{	vu_levels[ ch ] = get_vu( src, ch );
		max_vu_level = std::max( max_vu_level, vu_levels[ ch ] );
	}

	// If not in stereo, make each channel correspond to the maximum
	if ( !m_stereo ) for( int ch=0; ch<max_no_channels; vu_levels[ ch++ ] = max_vu_level );	

	// Get the Threshold in dB
	const float threshold_db = (float)utils::db::ratio( m_threshold );

	// We now cycle over all channels and determine what to do.
	for( int ch=0; ch<max_no_channels; ch++ )
	{	// Compute the audio level that we want to get to
		float gain_db = 0.0f;
		if ( vu_levels[ ch ] > threshold_db )
		{	// Get the VU level in dB
			const float vu_db = (float)utils::ratio::db( vu_levels[ ch ] );

			// This is how much to loud it is
			gain_db = -( ( m_ratio - 1.0f ) * vu_db + ( 1.0f - m_ratio ) * m_threshold ) / m_ratio;
		}

		// Apply the make-up gain
		gain_db += m_makeup_gain;		

		// If resetting, start with this value to avoid ramping when audio starts
		if ( m_reset )
				// We want to start at the gain db value to avoid sudden jumps when audio starts or stops
				m_current_db[ ch ] = 0;

		// If we are ramping down the volume		
		float ratio;
		if ( m_current_db[ ch ] >= gain_db )
				// In the next buffer this is how close we want to get
				ratio = std::min( 1.0f, buffer_length_ms / std::max( m_attack, 0.1f ) );

		else	// In the next buffer this is how close we want to get
				ratio = std::min( 1.0f, buffer_length_ms / std::max( m_release, 0.1f ) );

		// Get the distance to the new dB value
		const float new_db = ratio*gain_db + (1.0f-ratio)*m_current_db[ ch ];

		// Get the volumes as ratios
		const float start_ratio = (float)utils::db::ratio( m_current_db[ ch ] );
		const float end_ratio   = (float)utils::db::ratio( new_db );

		// Ramp the volume
		ramp_volume( src, ch, start_ratio, end_ratio );

		// Store the current level
		m_current_db[ ch ] = new_db;
	}

	// Reset the reset flag.
	m_reset = false;
}

float compressor_limiter::get_vu( FAUD::buffer_f32& src, const int ch )
{	// Error handline
	if ( ch >= src.no_channels() ) return 0.0f;

	// Get the values
	const int	no_samples = src.no_samples();
	const float *p_src     = src( ch );

	// Get the DC offset
	float sum = 0;
	for( int i=0; i<no_samples; sum += p_src[ i ], i++ );
	const float zero = sum / (float)no_samples;

	// Get the RMS value
	float	sum2 = 0;
	for( int i=0; i<no_samples; sum2 += sqr(p_src[ i ]-zero), i++ );
	const float rms = sqrt( sum2 / (float)no_samples );

	// Return the value scaled up to a peak
	return rms * 1.414213562373096f;
}

void compressor_limiter::ramp_volume( FAUD::buffer_f32& src, const int ch, const float vol_start, const float vol_end )
{	// Error handline
	if ( ch >= src.no_channels() ) return;

	// Get the values
	const int	no_samples = src.no_samples();
	float		*p_src     = src( ch );

	// No volume change
	if ( vol_start == vol_end )
	{	// Skip it
		if ( vol_start == 1.0f ) return;
		
		// Just scalue the values
		for( int i=0; i<no_samples; i++ )
			p_src[ i ] *= vol_start;
	}
	else
	{	// Get the volume ramp
		float dvolume = ( vol_end - vol_start ) / (float)no_samples;
		float volume  = vol_start + dvolume;

		// Apply the ramp
		for( int i=0; i<no_samples; i++, volume += dvolume )
			p_src[ i ] *= volume;
	}
}

// Threshold  - also called ceiling - This sets the point at which the automatic volume reduction kicks in. Below that volume the compressor 
// does nothing. When the input gets above that level, the compressor reduces the volume automatically to keep the signal from getting much louder.
// Measured in dB
const float compressor_limiter::threshold( void ) const
{	return m_threshold;
}

float& compressor_limiter::threshold( void )
{	return m_threshold;
}

// Attack time  - This is how quickly the volume is reduced once the input exceeds the threshold. If it's too slow, then a short burst of loud music 
// can get through and possibly cause distortion. So when using a compressor as a tool to prevent overload you generally want a very fast attack time. 
// But when used on an electric bass to get a little more punch, 20-50 milliseconds is often good because that lets a little burst of the attack get 
// through before the volume is reduced. So each note has a little extra "definition" but without the full length of the note being too loud.
// Measured in ms
const float compressor_limiter::attack( void ) const
{	return m_attack;
}

float& compressor_limiter::attack( void )
{	return m_attack;
}

// Release time  - This determines how quickly the volume comes back up when the input is no longer above the threshold. If it's too fast you'll hear 
// the volume as it goes up and down. That sound is called "pumping" or "breathing." Sometimes this sound is desirable for adding presence to vocals, 
// drums, and other instruments, but often it is not wanted. The best setting depends on whether you're using the compressor as a tool to prevent 
// overloading, or as an effect to create a cool sound or add more sustain to an instrument. If you don't want to hear the compressor work, set the 
// release time fairly long - one second or more. If you want an "aggressive" sound use a shorter release time. Note that as the release time is made 
// shorter, distortion increases at low frequencies. This is often used by audio engineers as an intentional effect.
// Measured in ms
const float compressor_limiter::release( void ) const
{	return m_release;
}

float& compressor_limiter::release( void )
{	return m_release;
}

// Compression ratio  - This dictates how much the volume is reduced versus how far above the threshold the signal is. A ratio of 1:1 does nothing. 2:1 
// means if the input rises to 2 dB above the threshold, the compressor will reduce the level by only 1 dB so the output will now be 1 dB louder. 10:1 
// means the signal must be 10 dB above the threshold for the output to increase by 1 dB. When a compressor is used with a high ratio - say, 5:1 or 
// greater - it is considered a limiter. In fact, the compression ratio is the only distinction between a compressor and a limiter.
const float compressor_limiter::compression_ratio( void ) const
{	return m_ratio;
}

float& compressor_limiter::compression_ratio( void )
{	return m_ratio;
}

// Makeup Gain  - since a compressor can only reduce the volume when the incoming signal is too high, the Makeup Gain (output volume) control lets you 
// bring the compressed audio back up to an acceptable level.
const float	compressor_limiter::makeup_gain( void ) const
{	return m_makeup_gain;
}

float& compressor_limiter::makeup_gain( void )
{	return m_makeup_gain;
}

// Enable or disable the effect entirely
const bool  compressor_limiter::enable( void ) const
{	return m_enable;
}

bool &compressor_limiter::enable( void )
{	return m_enable;
}

// Apply the operation in stereo. When true this means that there is in effect one compressor/limiter per channel. When this is false the
// total level for all channels is combined so that the volume across all of them remains constant. For instance when you have multiple microphones
// attached to a system, you would set this to false
const bool	compressor_limiter::multichannel( void ) const
{	return m_stereo;
}

bool &compressor_limiter::multichannel( void )
{	return m_stereo;
}

void compressor_limiter::reset( void )
{	// Reset the gains so it is not weird when you switch it back on
	m_reset = true;
}

// Copy the values
void compressor_limiter::operator= ( const compressor_limiter& from )
{	m_threshold = from.m_threshold;
	m_attack = from.m_attack;
	m_release = from.m_release;
	m_ratio = from.m_ratio;
	m_makeup_gain = from.m_makeup_gain;
	m_stereo = from.m_stereo;
	m_enable = from.m_enable;
}