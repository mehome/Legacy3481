#include "pch.h"
// TODO: reference additional headers your program requires here
#include "SineWaveGenerator.h"


generator::generator(size_t no_channels) :	m_waves(0),m_no_channels((int)no_channels),	m_sample_rate ( 48000 )
{
	for (signed i = 0; i < m_no_channels; i++)
	{
		wave_descriptor new_element;
		m_waves.push_back(new_element);
	}
}

generator::~generator()
{
}

void generator::gen_sw_freq( size_t channel, float *dst_buffer, size_t no_samples )
{
	double			 freq_hz = m_waves[channel].m_freq_hz;
	double			 amplitude = m_waves[channel].m_amplitude;
	double			 rho, theta, scale, pi2;
	size_t index=0; //array index of buffer

	rho = m_waves[channel].m_rho;

	pi2 = 3.1415926;
	//Compute the angle ratio unit we are going to use
	theta = freq_hz / m_sample_rate;
	pi2 *= 2.0;
	//Convert the angle ratio unit into radians
	theta *= pi2;

	//set our scale... this is also the size of the radius 
	scale = amplitude;

	{
		size_t				 siz=no_samples;
		double				 sample;


		while( siz-- )
		{
			//Find Y given the hypotenuse (scale) and the angle (rho)
			//Note: using sin will solve for Y, and give us an initial 0 size
			sample = sin( rho ) * scale;
			//increase our angular measurement
			rho += theta;
			//bring back the angular measurement by the length of the circle when it has completed a revolution
			if ( rho > pi2 )
				rho -= pi2;

			*(((float *)(dst_buffer))+(index++))=(float)sample;
			
		}
	}
	//save the sine wave state
	m_waves[channel].m_rho = rho;
}

void generator::gen_sw_short(size_t channel, short *dst_buffer, size_t no_samples, bool addsample)
{
	double			 freq_hz = m_waves[channel].m_freq_hz;
	double			 amplitude = m_waves[channel].m_amplitude;
	double			 rho, theta, scale, pi2;
	size_t index = 0; //array index of buffer

	rho = m_waves[channel].m_rho;

	pi2 = 3.1415926;
	//Compute the angle ratio unit we are going to use
	theta = freq_hz / m_sample_rate;
	pi2 *= 2.0;
	//Convert the angle ratio unit into radians
	theta *= pi2;

	//set our scale... this is also the size of the radius 
	scale = amplitude;

	{
		size_t				 siz = no_samples;
		double				 sample;


		while (siz--)
		{
			//Find Y given the hypotenuse (scale) and the angle (rho)
			//Note: using sin will solve for Y, and give us an initial 0 size
			//if frequency is zero we'll keep silent
			sample = (freq_hz != 0.0)? sin(rho) * scale : 0.0;
			//increase our angular measurement
			rho += theta;
			//bring back the angular measurement by the length of the circle when it has completed a revolution
			if (rho > pi2)
				rho -= pi2;

			//*(((float *)(dst_buffer)) + (index++)) = (float)sample;
			if (!addsample)
				*(((short *)(dst_buffer)) + (index+= m_no_channels)) = (short)(sample*(double)0x7fff);
			else
				*(((short *)(dst_buffer)) + (index += m_no_channels)) += (short)(sample*(double)0x7fff);
		}
	}
	//save the sine wave state
	m_waves[channel].m_rho = rho;
}


#if 0
void generator::process_xml( const FrameWork::Communication2::message::xml *p_msg )
{
	FrameWork::xml::tree xml_tree((const wchar_t *)*p_msg);

	//<sinwgen channels="4">
	//<channel num="1" freq="20.0" amp="0.5"/>
	//<channel num="2" freq="40.0" amp="0.75"/>
	//<channel num="3" freq="20.0" amp="0.5"/>
	//<channel num="4" freq="40.0" amp="0.75"/>
	//<sinwgen/>

	//if there is no frame source feeding frames to the sine wave generator, then there is also the parameter
	//<sinwgen send="true"/>
	//to begin sending with default settings
	
	//defaults
	//channels = 2
	//freq = 1000.0
	//amp = 1.0

	if(!::wcsicmp(xml_tree.type(), L"exit"))
	{
		m_exit = true;
	}
	else if(!::wcsicmp(xml_tree.type(), L"sinwgen"))
	{
		if( m_p_refresh_timer )
		{
			delete m_p_refresh_timer;
			m_p_refresh_timer = NULL;
		}

		if( m_own_send_thread )
		{
			if(!::wcsicmp(xml_tree.parameter(L"send"), L"true"))
			{
			}
		}
	
		m_settings_lock.write_lock();

		if(xml_tree.parameter(L"sample_rate") && xml_tree.parameter(L"sample_rate"))
			m_sample_rate = ::_wtoi(xml_tree.parameter(L"sample_rate"));

		for(signed i=0; i < m_no_channels; i++)
			delete m_waves[i];
	
		delete [] m_waves;

		m_no_channels = 2;

		if(xml_tree.parameter(L"channels") && xml_tree.parameter(L"channels")[0])
		{
			m_no_channels = ::_wtoi(xml_tree.parameter(L"channels"));
		}

		m_waves = new wave_descriptor*[m_no_channels];

		for( signed i=0; i < m_no_channels; i++ )
		{
			//the following is implicitly done in the ctor of wave_descriptor
			/*m_waves[i]->m_freq_hz = 1000.0
			m_waves[i]->m_amplitude = 1.0;
			m_waves[i]->m_rho = 0;*/
			m_waves[i] = new wave_descriptor;
		}
		
		for( signed i = 0; i < xml_tree.no_children(); i++) 
		{
			if(!::wcsicmp(xml_tree.child(i).type(), L"channel"))
			{
				for( signed j = 0; j < xml_tree.child(i).no_parameters(); j++ )
				{
					signed l_channel = ::_wtoi(xml_tree.child(i).parameter(L"num"));
					if( l_channel < m_no_channels )
					{
						m_waves[i]->m_freq_hz = ::_wtof(xml_tree.parameter(L"freq"));
						m_waves[i]->m_amplitude = ::_wtof(xml_tree.parameter(L"amp"));
					}
				}
			}
		}

		m_settings_lock.write_unlock();
	}
}

void generator::process_frame( const FrameWork::Communication2::message::frame *p_msg )
{
	//FrameWork::Threads::work::call_fcn_wait<> l_generator_wait;

	//p_msg->no_channels()

	//well.... since we can't change the number of channels in the message we received we have to construct a 
	//new one using the video from the one we just got

	int no_samples = 1601;
	
	m_settings_lock.read_lock();
	
	FrameWork::Communication2::message::frame l_frame_w_audio(p_msg->xres(), p_msg->yres(), false, no_samples, m_no_channels);
	
	l_frame_w_audio.time_stamp() = p_msg->time_stamp();
	l_frame_w_audio.field_type() = p_msg->field_type();
	l_frame_w_audio.aspect_ratio() = p_msg->aspect_ratio();
	l_frame_w_audio.sample_rate() = m_sample_rate;
	l_frame_w_audio.ycbcr() = p_msg->ycbcr();

	for ( signed i=0; i < m_no_channels; i++ )
	{
		gen_sw_freq( i, const_cast<float *>(&(l_frame_w_audio.audio()()[i*no_samples])),  no_samples);	
		//wait+ = cpp_call_fcn( this, &generator::gen_sw_freq, i, p_msg->audio(), p_msg->no_samples());
	}

	//l_generator_wait.wait();
	
	m_settings_lock.read_unlock();

	l_frame_w_audio.send( m_frame_dst_name );

	
}

#endif