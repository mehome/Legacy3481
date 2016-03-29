#pragma once

struct FRAMEWORKAUDIO2_API compressor_limiter
{			// Constructor
			compressor_limiter( void );
			compressor_limiter( const compressor_limiter& from );

			// Apply the effect to a buffer
			void operator() ( buffer_f32& src, const int sample_rate );

			// Reset internal counts. This is used if there is an audio discontinuity to avoid
			// weird glitches from one buffer to the next
			void reset( void );

			// Enable or disable the effect entirely
			const bool  enable( void ) const;
				  bool &enable( void );

			// Apply the operation in stereo. When true this means that there is in effect one compressor/limiter per channel. When this is false the
			// total level for all channels is combined so that the volume across all of them remains constant. For instance when you have multiple microphones
			// attached to a system, you would set this to false
			const bool	multichannel( void ) const;
				  bool &multichannel( void );

			// Threshold  - also called ceiling - This sets the point at which the automatic volume reduction kicks in. Below that volume the compressor 
			// does nothing. When the input gets above that level, the compressor reduces the volume automatically to keep the signal from getting much louder.
			// Measured in dB
			const float  threshold( void ) const;
				  float& threshold( void );

			// Attack time  - This is how quickly the volume is reduced once the input exceeds the threshold. If it's too slow, then a short burst of loud music 
			// can get through and possibly cause distortion. So when using a compressor as a tool to prevent overload you generally want a very fast attack time. 
			// But when used on an electric bass to get a little more punch, 20-50 milliseconds is often good because that lets a little burst of the attack get 
			// through before the volume is reduced. So each note has a little extra "definition" but without the full length of the note being too loud.
			// Measured in ms
			const float  attack( void ) const;
				  float& attack( void );

			// Release time  - This determines how quickly the volume comes back up when the input is no longer above the threshold. If it's too fast you'll hear 
			// the volume as it goes up and down. That sound is called "pumping" or "breathing." Sometimes this sound is desirable for adding presence to vocals, 
			// drums, and other instruments, but often it is not wanted. The best setting depends on whether you're using the compressor as a tool to prevent 
			// overloading, or as an effect to create a cool sound or add more sustain to an instrument. If you don't want to hear the compressor work, set the 
			// release time fairly long - one second or more. If you want an "aggressive" sound use a shorter release time. Note that as the release time is made 
			// shorter, distortion increases at low frequencies. This is often used by audio engineers as an intentional effect.
			// Measured in ms
			const float  release( void ) const;
				  float& release( void );

			// Compression ratio  - This dictates how much the volume is reduced versus how far above the threshold the signal is. A ratio of 1:1 does nothing. 2:1 
			// means if the input rises to 2 dB above the threshold, the compressor will reduce the level by only 1 dB so the output will now be 1 dB louder. 10:1 
			// means the signal must be 10 dB above the threshold for the output to increase by 1 dB. When a compressor is used with a high ratio - say, 5:1 or 
			// greater - it is considered a limiter. In fact, the compression ratio is the only distinction between a compressor and a limiter.
			// This is a ratio
			const float  compression_ratio( void ) const;
				  float& compression_ratio( void );

			// Makeup Gain  - since a compressor can only reduce the volume when the incoming signal is too high, the Makeup Gain (output volume) control lets you 
			// bring the compressed audio back up to an acceptable level.
			// Measured in dB
			const float	 makeup_gain( void ) const;
				  float& makeup_gain( void );

			// Copy the values
			void operator= ( const compressor_limiter& from );

private:	// Keep thins sane
			static const int max_no_channels = 8;
	
			// The current settings
			float	m_threshold;	// In dB
			float	m_attack;		// In milli-seconds
			float	m_release;		// In milli-seconds
			float	m_ratio;		// The compression ratio, in dB
			float	m_makeup_gain;	// A dB gain value
			bool	m_stereo;		// Are we acting as a stereo compressor-limiter
			bool	m_enable;

			// The current gain level
			bool	m_reset;
			float	m_current_db[ max_no_channels ];

			// Utility functions
			static float get_vu( buffer_f32& src, const int ch );
			static void ramp_volume( buffer_f32& src, const int ch, const float vol_start, const float vol_end );
};