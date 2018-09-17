#pragma once

///This is the heart of the sine wave generator.  It works by first establishing the angle unit to use (theta [in radians]) and then increments the 
///angle on each iteration (rho) finally a sample is obtained by finding Y given the angle (rho) and the hypotenuse 
///(i.e. radius, amplitude or scaling factor).  This will initially start the sine wave at 0 peak to positive then peak to negative and end with 0
///for one cycle, the angle measurement (rho) is preserved for subsequent calls to create a seamless waveform.

struct generator 
{
	public:
		generator(size_t no_channels=2);

		virtual ~generator( void );
		
		//This one works with a fixed rate and keeps track of where the wave leaves off from a previous buffer packet.
		void gen_sw_freq(size_t channel, float *dst_buffer, size_t no_samples);
		void gen_sw_short(size_t channel, short *dst_buffer, size_t no_samples);
		//sets the buffer fill list's set frequency		
		void frequency(size_t channel, double Frequency) { m_waves[channel].m_freq_hz = Frequency; }
		
		//sets the buffer fill list's set amplitude
		void amplitude(size_t channel, double Amplitude) { m_waves[channel].m_amplitude = Amplitude; }
		protected:
		//sample rate to generate at
		int m_sample_rate;

		//no of channels to fill with audio
		int m_no_channels;

		struct wave_descriptor
		{
			wave_descriptor() :
				m_freq_hz ( 1000.0 ), 
				//m_amplitude = (float) 1.41421356;  //sqrt(2) 
				m_amplitude ( 1.0 ),
				m_rho ( 0 )
				{}

			///How many revolutions per second
			double m_freq_hz;
			
			///The scaling factor to apply default is 1.0
			double m_amplitude;
		
			///keep track of the last angle measurement to create seamless waves
			double m_rho;
		};
		
		//ptr to array of wave descriptors to be used in generating individual waves for each channel
		std::vector<wave_descriptor> m_waves;
};

