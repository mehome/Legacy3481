
#include "pch.h"
#include "Time_Type.h"
namespace DirectSound
{
	namespace Output
	{

#include "DS_Output.h"
	}
}
#include "ThreadLib.h"
#include "SineWaveGenerator.h"   //for internal testing

#include <algorithm>
#include <dsound.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include "FunctionDiscoveryKeys_devpkey.h"

using namespace std;

//#define __TestFailure__

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#undef DS_Output_Verbose
#define __UseTestSineWave__
#define __DisableResampling__
#undef  __AlwaysUseThreading__

#ifdef DS_Output_Verbose
#define DOUT_DS_Output(x,...) debug_output(__VA_ARGS__)
#else
#define DOUT_DS_Output(x,...)
#endif


#define TestIterationRate \
{ \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	printf("rate=%f\n",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

#define TestIterationRate2 \
{ \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	printf("\r rate=%f                ",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

#define TestIterationRate3 \
{ \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	debug_output(p_debug_category,L"rate=%f\n",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

void debug_output(const char *format, ...)
{
	va_list marker;
	va_start(marker, format);
	static char Temp[2048];
	vsprintf_s(Temp, format, marker);
	OutputDebugStringA(Temp);
	va_end(marker);
}

namespace DirectSound
{
	namespace Output
	{

DirectSound_Globals *g_DirectSoundInitializer=NULL;

class DS_Output_Internal
{
	public:
		DS_Output_Internal();
		~DS_Output_Internal();

		void SetCallbackFillBuffer(std::function<void(size_t, short *, size_t)> callback) { m_FillBufferCallback = callback; }
		void deliver_audio(const audio_frame *p_frame);

		virtual void StartStreaming(void);
		virtual void StopStreaming(void);
		//returns time in seconds left before needing a new packet
		double FillBuffer(void);

		void SetUseDisplayedCallbacks(bool UseDisplayedCallbacks);
		bool GetUsingThreading() const {return m_UseThreading;}

		bool GetIsStreaming() const { return m_IsStreaming; }
	protected:
		bool m_IsStreaming;
		size_t m_AudioOutput;

	public:
		void operator() (const void*); //TODO work out how to call this directly
	private:
		//The extra thread is primarily for non-displayed clients which gives more tolerance for scheduling
		std::future<void> m_TaskState;
		event m_Event;

		LPDIRECTSOUNDBUFFER m_lpdsb;
		DWORD m_BufferSizeInBytes;
		DWORD m_FillPosition; //keeps track of where the buffer has been filled
		DWORD m_PreviousFillPosition;
		size_t m_IsClocking_Counter=0;  //avoid false positive by checking for consecutive zero positions
		__int64 m_ClockPhaseOffset;  //This tunes the phase of the clock with the audio clock

		std::function<void (size_t,short *,size_t)>	m_FillBufferCallback;

		//This is for GetTimeElapsed
		long m_GTE_LastCalled;
		bool m_UseDisplayedCallbacks;
		//This helps manage whether or not to use threading
		bool m_UseThreading=true;
		//Some "digital" audio devices do not clock until they are active, this keeps a track on them via GetCursorPosition()
		bool m_IsClocking = false;
};

class DS_Output_Core
{
	public:
		DS_Output_Core();
		~DS_Output_Core();

		//If Null this will use the desktop window
		bool StartUp(HWND ParentWindow=NULL, const std::vector<std::wstring>* p_device_list=NULL);
		void ShutDown();
		void SetWaveFormat(WAVEFORMATEX &wfx,WORD wFormatTag,WORD nChannels,DWORD nSamplesPerSecond,WORD wBitsPerSample);

		double GetTimeElapsed();

		// Enable/disable Windows default ducking of audio when communication devices are active
		// for the specified device or NULL == default audio device
		// true == use system setting for ducking, false == don't duck
		const HRESULT SystemAudioDucking(const bool enable, const wchar_t* p_device_name=NULL) const;

		size_t GetSampleRate(void) const;
		size_t GetNoChannels(void) const;
		//This is the size of the buffer (in samples) that DSound will allocate
		size_t GetBufferSampleSize(void) const;
		size_t GetBitsPerSample(void) const;


		WAVEFORMATEX *GetWaveFormatToUse(void);
		LPDIRECTSOUND8 GetDirectSoundObject(void);

		static DS_Output_Core &GetDS_Output_Core(void);

	private:
		static BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCWSTR lpszDesc, LPCWSTR lpszDrvName, LPVOID lpContext );
	protected:
		critical_section m_BlockAudioStreamsInterface;
	private:
		LPDIRECTSOUND8 m_IDS8_Interface;
		LPDIRECTSOUNDBUFFER m_IDS8_Buffer; // primary
		DWORD m_BufferSizeInBytes;
		DWORD m_FillPosition; //keeps track of where the buffer has been filled
		HANDLE m_NotificationTimer;	
		WAVEFORMATEX m_FormatToUse;

		long m_GTE_LastCalled;
};

class DirectSound_Globals
{
	public:
		DirectSound_Globals();
		~DirectSound_Globals();
		bool init( const std::vector<std::wstring>* p_device_list );  //no work is to be done within the constructor
		/// \return true if hardware is present

		DS_Output_Core &GetDS_Output_Core() {return *m_DS_Output_Core;}

	private:
		DS_Output_Core * m_DS_Output_Core;
};


	}
}

using namespace DirectSound::Output;

  /*************************************************************************/
 /*							 DS_Output_Internal							  */			
/*************************************************************************/

//we'll callback to a function like this to fill in the buffer
void client_fillbuffer_default(size_t no_channels, short *dst_buffer, size_t no_samples)
{
	//internal testing
	#if 0
	static size_t count = 0;
	printf("Test %d\n", count++);
	#endif
	#if 0
	static generator sine_wave_test;
	sine_wave_test.frequency(0, 100.0);
	sine_wave_test.gen_sw_short(0, dst_buffer, no_samples);
	sine_wave_test.frequency(1, 101.0);
	sine_wave_test.gen_sw_short(1, dst_buffer+1, no_samples);
	#endif
}

DS_Output_Internal::DS_Output_Internal() : 
	m_IsStreaming(false),m_AudioOutput(),
	m_lpdsb(NULL), m_BufferSizeInBytes(0), m_FillPosition(0), m_PreviousFillPosition(0), m_ClockPhaseOffset(0),
	m_GTE_LastCalled(0)
{ 
	m_FillBufferCallback = client_fillbuffer_default;
	DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
	//We shouldn't make this range the same value
	m_FillPosition=0;
	m_PreviousFillPosition=(DWORD)DSOC.GetBufferSampleSize() * DSOC.GetWaveFormatToUse()->nBlockAlign;

	//Set up the secondary buffer
	DSBUFFERDESC dsbdesc;
	INIT_DIRECTX_STRUCT(dsbdesc);
	dsbdesc.dwFlags         = 
		DSBCAPS_GLOBALFOCUS			|	//If my window is minimized I'm still going to be heard!
		DSBCAPS_STATIC				|	//Use hardware buffers if I can
		DSBCAPS_GETCURRENTPOSITION2;	//We are a new product use the best accurate code

	WAVEFORMATEX *FormatToUse=DSOC.GetWaveFormatToUse();
	dsbdesc.dwBufferBytes=m_BufferSizeInBytes=(DWORD)(FormatToUse->nBlockAlign * (DSOC.GetBufferSampleSize()));
	dsbdesc.lpwfxFormat=FormatToUse;
	try
	{
		if FAILED(DSOC.GetDirectSoundObject()->CreateSoundBuffer(&dsbdesc, &m_lpdsb, NULL))
		{
			#ifndef __TestFailure__
			assert(false);
			#endif
			throw"Unable to Create Secondary Sound Buffer" __FUNCSIG__;
		}

		{	//Fill our buffer with silence
			void *dsbuf1,*dsbuf2;
			DWORD dsbuflen1,dsbuflen2;

			if FAILED(m_lpdsb->Lock(0,m_BufferSizeInBytes,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0))
				assert(false),throw"Unable to lock secondary buffer" __FUNCSIG__;
			if (dsbuf1)
				FillMemory((char *)dsbuf1,dsbuflen1,(BYTE)(FormatToUse->wBitsPerSample == 8 ? 128 : 0));
			if (dsbuf2)
				FillMemory((char *)dsbuf2,dsbuflen2,(BYTE)(FormatToUse->wBitsPerSample == 8 ? 128 : 0));
			if FAILED(m_lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
				assert(false),throw"Unable to unlock secondary buffer" __FUNCSIG__;
		}
		//Might as well start playing it
		m_lpdsb->SetCurrentPosition(0);
	}
	catch (char *Error)  //Close resources in constructor throws
	{
		if (m_lpdsb)
		{
			m_lpdsb->Stop();
			m_lpdsb->Release();
			m_lpdsb=NULL;
		}
		//TODO determine why this throw is needed... we should recover gracefully!
		//throw Error;
		debug_output("Warning: DS_Output_Internal() failed  %ls",Error);
	}
}

DS_Output_Internal::~DS_Output_Internal()
{
	StopStreaming();
	if (m_lpdsb)
	{
		m_lpdsb->Stop();
		m_lpdsb->Release();
		m_lpdsb=NULL;
	}
}

void DS_Output_Internal::StopStreaming(void) 
{
	if (m_IsStreaming)
	{
		m_IsStreaming = false;
		//join... wait for thread to close:
		size_t TimeOut = 0;
		while (m_TaskState.wait_for(std::chrono::milliseconds(500)) != std::future_status::ready)
		{
			printf("waiting %d\n", TimeOut++);  //put else where so I can monitor both
		}
		m_lpdsb->Stop();
	}
}

template<typename F, typename... Ts>
inline std::future<typename std::result_of<F(Ts...)>::type> reallyAsync(F&& f, Ts&&... params) // return future for asynchronous call to f(params...)
{
	return std::async(std::launch::async,
		std::forward<F>(f),
		std::forward<Ts>(params)...);
}

__inline void MySleep(double Seconds)
{
	const int time_ms = (int)(Seconds*1000.0);
	std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
}

void task_proc(DS_Output_Internal *instance)
{
	printf("starting task_proc()\n");
	void *dummy_ptr=nullptr;
	while (instance->GetIsStreaming())
	{
		//no spin management here:... delegate to client
		(*instance)(dummy_ptr);
	}
	printf("ending task_proc()\n");
}

void DS_Output_Internal::StartStreaming(void)
{
	if (!m_IsStreaming)
	{
		m_lpdsb->Play(0,0,DSBPLAY_LOOPING);
		m_IsStreaming=true;
		if (m_UseThreading)
		{
			//m_pThread = new thread<DS_Output_Internal>(this);
			//m_pThread->priority_above_normal();
			//m_pThread->set_thread_name("Module.DirectSound.Output Main Thread");
			m_TaskState=reallyAsync(task_proc,this);
		}

		//now to set up the initial clock phase with the audio clock
		{
			DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
			DWORD BlockAlign=DSOC.GetWaveFormatToUse()->nBlockAlign;
			//m_BufferSampleSize=DSOC.GetBufferSampleSize();
			double SampleRate=(double)DSOC.GetSampleRate();

			Sleep(16); //give some time for the cursor to get going

			DWORD playpos;
			m_lpdsb->GetCurrentPosition(&playpos,NULL);
			playpos/=BlockAlign; //convert to samples

			time_type CurrentTime=time_type::get_current_time();
			time_type CPUClock=((__int64)CurrentTime) % 10000000;
			double CalibratePlayPos=(double)CPUClock*SampleRate;
			int PhaseOffset=(int)playpos-(int)CalibratePlayPos;
			//printf("playpos %d, %d  phase=%d \n",playpos,(size_t)(CalibratePlayPos),PhaseOffset);
			//m_ClockPhaseOffset= ((__int64)PhaseOffset * 10000000) / (__int64)DSOC.GetSampleRate();
			m_ClockPhaseOffset=(__int64)(((double)PhaseOffset / (double)DSOC.GetSampleRate()) * 10000000.0);

			//Check my math :)
			//CPUClock=((__int64)CurrentTime + m_ClockPhaseOffset) % 10000000;
			//CalibratePlayPos=(double)CPUClock*SampleRate;
			//printf("Test-> playpos %d, %d \n",playpos,(size_t)(CalibratePlayPos));
		}
	}
}

void DS_Output_Internal::SetUseDisplayedCallbacks(bool UseDisplayedCallbacks) 
{
	StopStreaming();
	#ifdef __AlwaysUseThreading__
	m_UseThreading=true;
	#else
	m_UseThreading=!UseDisplayedCallbacks;
	#endif
	m_UseDisplayedCallbacks=UseDisplayedCallbacks;
	StartStreaming();
}

void DS_Output_Internal::deliver_audio(const audio_frame *p_frame)
{
	if (m_UseThreading)
		m_Event.set();
	else
		FillBuffer();
}

double DS_Output_Internal::FillBuffer()
{
	class Internal
	{
		public:
			Internal(LPDIRECTSOUNDBUFFER lpdsb, __int64 &ClockPhaseOffset, const  bool &IsStreaming, bool &IsClocking, size_t &IsClocking_Counter) : m_lpdsb(lpdsb),
				m_ClockPhaseOffset(ClockPhaseOffset), m_IsStreaming(IsStreaming), m_IsClocking(IsClocking),m_IsClocking_Counter(IsClocking_Counter)
			{
				DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
				m_BlockAlign=DSOC.GetWaveFormatToUse()->nBlockAlign;
				m_BufferSampleSize=DSOC.GetBufferSampleSize();
				m_SampleRate=(double)DSOC.GetSampleRate();
			}
			///This works in sample units
			size_t AdvancePosition(size_t CurrentFillSamplePosition,size_t AdvInSamples)
			{
				size_t ret=CurrentFillSamplePosition+AdvInSamples;
				//since the buffer size may not be a power of 2 we'll need to branch (no biggy)
				//If we overflow subtract the length of the buffer
				if (ret>=m_BufferSampleSize)
					ret-=m_BufferSampleSize;
				return ret;
			}
			size_t GetPlayPos()
			{
				DWORD playpos;
				//size_t SampleOffset=0;
				size_t SampleOffset=(size_t)(0.038 * m_SampleRate);
				m_lpdsb->GetCurrentPosition(&playpos,NULL);
				const size_t Clocking_Counter_Threshold = 3;
				m_IsClocking = playpos != 0 || m_IsClocking_Counter++<Clocking_Counter_Threshold;  //keep the logic simple, if this is a false positive it will resolve itself the next cycle
				if (m_IsClocking)
					m_IsClocking_Counter = 0;  //reset the counter each successful test of an advanced cursor
				//Testing for false positive
				#if 0
				if (playpos==0)
					printf("%d->%d|\n",playpos, m_IsClocking_Counter);
				if (!m_IsClocking)
				{
					printf("Failed--\n");
					m_IsClocking = true;
				}
				#endif			
				playpos/=m_BlockAlign; //convert to samples

				//Note: This design assumes the buffer size (of our secondary buffer) is the same size as the sample rate.  This is fine for now, but at some
				//point I may need to make a distinction if I have to change the buffer size.
				
				time_type CPUClock=((__int64)time_type::get_current_time()+m_ClockPhaseOffset) % 10000000;
				double CalibratePlayPos=(double)CPUClock*m_SampleRate;
				int PhaseOffset=(int)playpos-(int)CalibratePlayPos;
				int iSampleRate=(int)m_SampleRate;
				//Check wrap-around case
				if (PhaseOffset > iSampleRate>>1)
					PhaseOffset= ((int)playpos-iSampleRate) - (int)CalibratePlayPos;
				else if (PhaseOffset < -(iSampleRate>>1))
					PhaseOffset= playpos - ((int)CalibratePlayPos-iSampleRate);

				double Current_ClockPhaseOffset=(double)m_ClockPhaseOffset + (((double)PhaseOffset / m_SampleRate) * 10000000.0);

				//check math
				//CPUClock=((__int64)time_type::get_current_time() + (__int64)Current_ClockPhaseOffset) % 10000000;
				//CalibratePlayPos=(double)CPUClock*m_SampleRate;
				//printf("Test-> playpos %d, %d \n",playpos,(size_t)(CalibratePlayPos));

				const double dSmoothingValue=0.1;
				//blend the current phase error to the current phase offset
				m_ClockPhaseOffset=(__int64) (((1.0-dSmoothingValue) * (double)m_ClockPhaseOffset) + (dSmoothingValue * Current_ClockPhaseOffset));
				DOUT_DS_Output(L"^~^", L"playpos=%d, calibrated=%d, phase_offset=%d\n", playpos, (size_t)(CalibratePlayPos), PhaseOffset);
				size_t Error=abs(PhaseOffset);
				//printf("\r%d          ",Error);
				//if (Error>1000)
				//	debug_output(p_debug_category,L"%d\n",Error);

				return AdvancePosition((size_t)(CalibratePlayPos),SampleOffset);
			}
			///Note in a circular buffer the begin range may be greater than the end range
			///This is all in samples
			/// \ret 0=in range 
			///		   less than 0 is somewhere before the range  
			///		greater than 0 is somewhere after  the range
			int IsInRange(size_t playpos, size_t BeginRange,size_t EndRange)
			{
				//geez.. there is a roladex going in my mind how to solve this.  This technique well branch into certain cases and use the
				//known assumptions to their fullest to solve.  This is probably a simplistic approach with the cost of some redundancy.
				int ret=0;
				double UnselectedRangePosition=-1; //If playpos is in the unselected range this will point position in a normalized scale
				size_t UnselectedRangeSize;
				//Determine what kind of range we have
				if (BeginRange<EndRange)  //normal range  -------BxxxxxE-------
				{
					size_t NormalRangeEndPointToEnd=m_BufferSampleSize-EndRange;
					UnselectedRangeSize=BeginRange + NormalRangeEndPointToEnd;
					if (playpos<BeginRange)
						UnselectedRangePosition=(double)(playpos+NormalRangeEndPointToEnd)/ (double)UnselectedRangeSize;
					else if (playpos>EndRange)
						UnselectedRangePosition=((double)(playpos-EndRange))/(double)UnselectedRangeSize;
				}
				else if (EndRange<BeginRange) //inverted range  xxxxE---------Bxxxx
				{
					UnselectedRangeSize=BeginRange - EndRange;
					if ((playpos>EndRange) && (playpos<BeginRange))
						UnselectedRangePosition=(double)(playpos-EndRange) / (double)UnselectedRangeSize;
				}
				else
					assert(false);  // this should not be happening
				int ScaledPosition=(int)((double)UnselectedRangePosition * UnselectedRangeSize);
				return UnselectedRangePosition==-1?  0 :
					(UnselectedRangePosition<0.5)?  ScaledPosition :  //this many samples after the end
					ScaledPosition - (int)UnselectedRangeSize; //This many samples before the beginning (will be negative)
			}
			//wait until the play cursor has hit within the range
			double WaitForPlay(size_t BeginRange,size_t EndRange)
			{
				#undef __WaitForPlay_Verbose__
				#ifdef __WaitForPlay_Verbose__
				vector<size_t> playpos_list;
				#endif
				//Get current play position
				size_t playpos,playpos_latencyguard;
				int result;
				{
					//This piece will give more cpu time by giving a larger sleep
					#if 1
					playpos=GetPlayPos();
					//playpos_latencyguard=AdvancePosition(playpos,(size_t)(0.004 * m_SampleRate));
					playpos_latencyguard=playpos;
					double SleepTime_ms= (((double)BeginRange-(double)playpos) * 1.0 / m_SampleRate) * 1000.0;
					DWORD dwSleepTime_ms=(SleepTime_ms>=1.0)?(DWORD)SleepTime_ms:1;
					//printf("%d\n",dwSleepTime_ms);
					//debug_output(p_debug_category,L"%d\n",dwSleepTime_ms);
					Sleep(dwSleepTime_ms);
					result=IsInRange(playpos_latencyguard,BeginRange,EndRange);
					//We need to ensure we have slept enough so we'll poll to see where we are... typically this may happen once after the initial big sleep
					if (result<0)
					#endif
					{
						//On some machines that have lower quality sound cards (e.g. the playpos is more inaccurate).  We allow a threshold setting
						//in here to ensure that we do not sleep too long.  Unfortunately, this has to be tuned for long buffers to work properly... for now 
						//this will be left at zero... (as it was before without it), and we can address this if we find we do need it... most likely the
						//typical stress is shorter buffers so it wouldn't matter to have this set to say -300
						//  [8/29/2013 James]
						const int threshold=0;
						do 
						{
							//debug_output(p_debug_category,L"%d\n",result);
							if (result < threshold)
							{
								//Note: if we are not clocking give a generous time interval between checks (may want to dial it down if it takes to long to switch states)
								DWORD TimeMS = m_IsClocking ? 1 : 1000;
								Sleep(TimeMS);
							}
							//printf("Sleep 1\n");
							playpos=GetPlayPos();
							//The latency guard was needed for the old clocking mechanism of adding time for redundant cursor positions
							//It appears the new clocking mechanism no longer needs this; however, I'll want to keep this around incase the
							//overshooting problem happens in rare cases
							//playpos_latencyguard=AdvancePosition(playpos,(size_t)(0.001 * m_SampleRate));
							playpos_latencyguard=playpos;
							result=IsInRange(playpos_latencyguard,BeginRange,EndRange);
							#ifdef __WaitForPlay_Verbose__
							playpos_list.push_back(playpos);
							#endif
							//debug_output(p_debug_category,L"r=%d pp=%d b=%d,e=%d\n\n",result,playpos,BeginRange,EndRange);
						} while (result<threshold && m_IsStreaming);
					}
				}
				double ret=0.0;
				if (result<=0)
				{
					double SampleTime=1.0/m_SampleRate;
					if (BeginRange<EndRange)
						ret=(double)(EndRange-playpos_latencyguard)* SampleTime;
					else
					{
						size_t NormalRangeEndPointToEnd=m_BufferSampleSize-BeginRange;
						size_t InvertedRangeSize=EndRange + NormalRangeEndPointToEnd;

						if (playpos_latencyguard>=BeginRange)
							ret=double(InvertedRangeSize - (playpos_latencyguard-BeginRange)) * SampleTime;
						else
							ret= double(InvertedRangeSize - (playpos_latencyguard + NormalRangeEndPointToEnd)) * SampleTime;
					}
					#ifdef __WaitForPlay_Verbose__
					DWORD playpos;
					m_lpdsb->GetCurrentPosition(&playpos,NULL);
					playpos/=m_BlockAlign; //convert to samples
					//if (playpos < playpos_list[playpos_list.size()-1])
					if (false)
					{
						wstring sList;
						wchar_t Buffer[16];
						for (size_t i=0;i<playpos_list.size();i++)
						{
							_itow((int)playpos_list[i],Buffer,10);
							sList+=Buffer;
							sList+=L",";
						}
						_itow((int)playpos,Buffer,10);
						sList+=L"Actual(";
						sList+=Buffer;
						sList+=L")";

						debug_output(p_debug_category,L"%s\n",sList.c_str());
					}
					#endif

				}
				else if (result>0)
				{
					result=IsInRange(playpos,BeginRange,EndRange);
					if (result>0)
					{
						debug_output("FillBuffer wait for play overshot %d samples \n",result);
						#ifdef __WaitForPlay_Verbose__
						debug_output(p_debug_category,L"range from %d to %d\n",BeginRange,EndRange);
						wstring sList;
						wchar_t Buffer[16];
						for (size_t i=0;i<playpos_list.size();i++)
						{
							_itow((int)playpos_list[i],Buffer,10);
							sList+=Buffer;
							sList+=L",";
						}
						debug_output(p_debug_category,L"%s\n",sList.c_str());
						#endif
					}
				}
				//Case 82506:  track how this can be a corrupt value
				assert(ret < 2.0);  //recoverable down stream... but would like to trace code to determine why this could fail
				return ret;
			}
		private:
			LPDIRECTSOUNDBUFFER m_lpdsb;
			DWORD m_BlockAlign;
			size_t m_BufferSampleSize;
			size_t &m_IsClocking_Counter;
			double m_SampleRate;
			__int64 &m_ClockPhaseOffset;
			double m_ClockScalar;
			const bool &m_IsStreaming;
			bool &m_IsClocking;
	} _(m_lpdsb,m_ClockPhaseOffset,m_IsStreaming,m_IsClocking, m_IsClocking_Counter);

	double ret=0.0;  //if we are not streaming this value will not really matter
	DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
	DWORD BlockAlign=DSOC.GetWaveFormatToUse()->nBlockAlign;

	if (m_IsStreaming)
	{
		double SampleRate=(double)DSOC.GetSampleRate();
		//Get current play position
		size_t playpos=_.GetPlayPos();
		size_t WritePos=m_FillPosition/BlockAlign;

		//Note from previous to fill is the area that was just filled where the fill position is set to the beginning point where it is about to get filled
		//check if the position is in its correct range.  It should be in this range; however, due to the nature of the cursor skipping an update to
		//20ms intervals, it is possible that the cursor is on that phase and would show a stagnant update from the previous iteration, this should
		//be fine since it doesn't impact the write position, and will get resolve during the wait.  All we really care about here is if the
		//play cursor overshot
		if ((_.IsInRange(playpos,m_PreviousFillPosition/BlockAlign,WritePos))>0)
		{
			debug_output("Error not in range cursor=%d,from=%d,to=%d\n",playpos,m_PreviousFillPosition/BlockAlign,WritePos);
			//Place fill position about 20ms of time ahead of where we are now
			m_FillPosition=((DWORD)_.AdvancePosition(playpos,(size_t)(0.020 * SampleRate))) * BlockAlign;
		}

		//printf("%d play=%d write=%d\n",WritePos-playpos,playpos,WritePos);
		DOUT_DS_Output(L"^~^", L"play_dif=%d play=%d write=%d\n", WritePos - playpos, playpos, WritePos);
		//Grab the next frame to process we'll need its size to lock the buffer
		//AudioMessageConverter::SourcePacket SourcePkt=m_AudioMessageConverter.GetNextFrame();
		//const buffer_f32 *Source=SourcePkt.BufferToProcess;
		//const float *Source;
		//size_t SampleSizeToFill=Source?Source->no_samples():(size_t)(SampleRate * 0.020);
		size_t SampleSizeToFill = (size_t)(SampleRate * 0.020);

		//Attempt to lock
		size_t TimeOut=0;
		bool LockStatus=false;
		void *dsbuf1,*dsbuf2;
		DWORD dsbuflen1,dsbuflen2;
		do
		{
			if (m_lpdsb->Lock(m_FillPosition,(DWORD)SampleSizeToFill*BlockAlign,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)==DS_OK)
			{
				LockStatus=true;
			}
			else
				Sleep(1);  //retry
		} while ((TimeOut++<5)&&(LockStatus==false));

		if (LockStatus)
		{
			//Success fill the buffer
			size_t Sample1Length=dsbuflen1/BlockAlign;
			assert(dsbuf1); //sanity check, should always have this if the lock was successful
			//m_AudioMessageConverter.FillOutput(Source,0,(PBYTE)dsbuf1,Sample1Length);
			m_FillBufferCallback(2, (short *)dsbuf1, Sample1Length);  //TODO cache number of channels

			//May have a the wrap around case buffer, so we fill it accordingly
			if (dsbuf2)
			{
				m_FillBufferCallback(2, (short *)dsbuf2, dsbuflen2 / BlockAlign);
				//printf("test\n");
			}
			//	m_AudioMessageConverter.FillOutput(Source,Sample1Length,(PBYTE)dsbuf2,dsbuflen2/BlockAlign);
			if FAILED(m_lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
				printf("Unable to unlock DS buffer\n");
		}
		else
			debug_output("Error cursor=%d,buf1=%p,buf1len=%d,buf2=%p,buf2len=%d\n",playpos,dsbuf1,dsbuflen1,dsbuf2,&dsbuflen2);

		//Advance the position markers to the new target play region
		m_PreviousFillPosition=m_FillPosition;
		//The advance works in sample units
		m_FillPosition=((DWORD)_.AdvancePosition(m_FillPosition/BlockAlign,SampleSizeToFill)) * BlockAlign;
		//Wait for play to hit our new filled range
		ret=_.WaitForPlay(m_PreviousFillPosition/BlockAlign,m_FillPosition/BlockAlign);
		//We should be in the play cursor range now we can send the displayed
		//if ((m_UseDisplayedCallbacks)&&(SourcePkt.Message))
		//	SourcePkt.Message->displayed();

		//clean / tidy up
		//m_AudioMessageConverter.ClosePacket(SourcePkt);
	}
	return ret;
}

void DS_Output_Internal::operator() (const void*)
{
	double TimeOut = FillBuffer(); //notify our stream to update
	//printf("%f\n",TimeOut*1000.0);
	if ((TimeOut > 0.0) && (TimeOut < 2.0))
		m_Event.wait((DWORD)(TimeOut * 0.50 * 1000.0));  //only a small percentage before to get enough time to fill
	else
	{
		//Enable if there is playback problems... this does git hit from time to time
		//printf("Unexpected return time from FillBuffer\n");
		m_Event.wait(1000);
	}
}
  /************************************************************************/
 /*							  DS_Output_Core							 */
/************************************************************************/
DS_Output_Core::DS_Output_Core() : m_IDS8_Interface(NULL), m_IDS8_Buffer(NULL), m_BufferSizeInBytes(0), m_FillPosition(0), m_NotificationTimer(NULL), m_GTE_LastCalled(0)
{
	memset(&m_FormatToUse, 0, sizeof(WAVEFORMATEX));
	CoInitialize(NULL);

	m_NotificationTimer=CreateWaitableTimer(NULL,false,NULL);
	assert(m_NotificationTimer);
	m_FillPosition=(DWORD)GetBufferSampleSize(); //this way the first buffer will get filled when it starts
}

DS_Output_Core::~DS_Output_Core()
{
	ShutDown();
	CloseHandle(m_NotificationTimer);

	CoUninitialize();
}

const HRESULT DS_Output_Core::SystemAudioDucking(const bool enable, const wchar_t* p_device_name) const
{	HRESULT hr			= S_OK;
	UINT	deviceCount = 0;
	LPWSTR	pwszID		= NULL;
	bool	deviceFound	= false;

    IMMDeviceEnumerator*	pDeviceEnumerator	= NULL;
	IMMDeviceCollection*	pDeviceCollection	= NULL;
    IMMDevice*				pEndpoint			= NULL;
    IAudioSessionManager2*	pSessionManager2	= NULL;
    IAudioSessionControl*	pSessionControl		= NULL;
    IAudioSessionControl2*	pSessionControl2	= NULL;

    //  Start with the default endpoint.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
    
	if(p_device_name)
	{
		// enumerate the available output devices
		if(SUCCEEDED(hr))
		{	hr = pDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDeviceCollection);
		
			pDeviceEnumerator->Release();
			pDeviceEnumerator = NULL;
		}

		// how many devices?
		if(SUCCEEDED(hr))
		{	hr = pDeviceCollection->GetCount(&deviceCount);
			if(deviceCount == 0)
				hr = E_FAIL;
		}
	
		// find the specified end point
		if(SUCCEEDED(hr))
		{	for (UINT i = 0 ; i < deviceCount; i++)
			{	PROPVARIANT varName;
				PropVariantInit(&varName);
				IPropertyStore*	pProps = NULL;
				
				// Get pointer to endpoint number i.
				hr = pDeviceCollection->Item(i, &pEndpoint);
			
				if(SUCCEEDED(hr))
				{	// Get the endpoint ID string.
					hr = pEndpoint->GetId(&pwszID);
				}

				if(SUCCEEDED(hr))
				{	// Get the endpoint ID string.
					hr = pEndpoint->GetId(&pwszID);
				}

				if(SUCCEEDED(hr))
				{	// Get the endpoint ID string.
					hr = pEndpoint->GetId(&pwszID);
				}

				if(SUCCEEDED(hr))
				{	// Get the property store for this endpoint
					hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
				}

				if(SUCCEEDED(hr))
				{	// Get the endpoint's friendly-name property.
					hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);

					// see if it matches the requested device
					if(SUCCEEDED(hr) && 0==::wcscmp(p_device_name, varName.pwszVal))
					{	deviceFound = true;

						// cleanup now, we're leaving
						PropVariantClear(&varName);
						if(pwszID)
						{	CoTaskMemFree(pwszID); pwszID = NULL;
						}
						SAFE_RELEASE(pProps);

						break;
					}
				}

				// free on each loop for reuse
				PropVariantClear(&varName);
				if(pwszID)
				{	CoTaskMemFree(pwszID); pwszID = NULL;
				}
				SAFE_RELEASE(pProps);
			}
		}
	}
	
	// if we didn't find what we wanted but there are no errors, use the default device
	if(!deviceFound && SUCCEEDED(hr))
	{	hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
		pDeviceEnumerator->Release();
		pDeviceEnumerator = NULL;
	}

    // Activate session manager.
    if (SUCCEEDED(hr))
    {	hr = pEndpoint->Activate(__uuidof(IAudioSessionManager2),  CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&pSessionManager2));
        pEndpoint->Release();
        pEndpoint = NULL;
    }

    if (SUCCEEDED(hr))
    {	hr = pSessionManager2->GetAudioSessionControl(NULL, 0, &pSessionControl);
        
        pSessionManager2->Release();
        pSessionManager2 = NULL;
    }

    if (SUCCEEDED(hr))
    {	hr = pSessionControl->QueryInterface( __uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
                
        pSessionControl->Release();
        pSessionControl = NULL;
    }

    //  Sync the ducking state with the specified preference.
    if (SUCCEEDED(hr))
    {   if (enable)
        {	hr = pSessionControl2->SetDuckingPreference(FALSE);
        }
        else
        {	hr = pSessionControl2->SetDuckingPreference(TRUE);
        }

        pSessionControl2->Release();
        pSessionControl2 = NULL;
    }

	SAFE_RELEASE(pDeviceCollection);

    return hr;
}

void DS_Output_Core::SetWaveFormat(WAVEFORMATEX &wfx,WORD wFormatTag,WORD nChannels,DWORD nSamplesPerSecond,WORD wBitsPerSample)
{
	memset(&wfx,0,sizeof(WAVEFORMATEX));
	wfx.wFormatTag=wFormatTag;
	wfx.nChannels=nChannels;
	wfx.nSamplesPerSec=nSamplesPerSecond;
	wfx.wBitsPerSample=wBitsPerSample;
	wfx.nBlockAlign=wfx.wBitsPerSample/8*wfx.nChannels;
	wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
}

size_t DS_Output_Core::GetSampleRate(void) const 
{	
	return (size_t)m_FormatToUse.nSamplesPerSec;
}
size_t DS_Output_Core::GetNoChannels(void) const 
{	
	return (size_t)m_FormatToUse.nChannels;
}
size_t DS_Output_Core::GetBufferSampleSize(void) const
{	
	//There size is really flexible, we just want it large enough to avoid it circling too much 1 second seems to be good
	return (size_t)m_FormatToUse.nSamplesPerSec;
}
size_t DS_Output_Core::GetBitsPerSample(void) const
{
	return (size_t)m_FormatToUse.wBitsPerSample;
}

double DS_Output_Core::GetTimeElapsed(void)
{
	DWORD playpos;
	//assert m_IDS8_Buffer
	m_IDS8_Buffer->GetCurrentPosition(&playpos,NULL);
	playpos/=GetWaveFormatToUse()->nBlockAlign; //convert to samples

	long PointerDifference=long(playpos)-m_GTE_LastCalled;
	if (PointerDifference<0.0)
	{
		size_t BufferSize=GetBufferSampleSize()<<1;
		PointerDifference=((long)BufferSize-m_GTE_LastCalled) + (long)playpos;
	}
	m_GTE_LastCalled=(long)playpos;
	return (double)PointerDifference / (double)GetSampleRate();
	//return (double)PointerDifference;
 
}

WAVEFORMATEX *DS_Output_Core::GetWaveFormatToUse(void) 
{	
	return &m_FormatToUse;
}

LPDIRECTSOUND8 DS_Output_Core::GetDirectSoundObject(void) 
{
	return m_IDS8_Interface;
}
DS_Output_Core &DS_Output_Core::GetDS_Output_Core(void){
	return g_DirectSoundInitializer->GetDS_Output_Core();
}
bool DS_Output_Core::StartUp(HWND ParentWindow, const std::vector<std::wstring>* p_device_list)
{
	m_IDS8_Buffer=NULL;
	enum ErrorList
	{
		eErrCreatingDSObject,
		eErrSettingCooperativeLevel,
		eErrCreatingPrimarySoundBuffer,
		eErrGetCapsFailed,
		eErrCouldNotFindFormat
	};
	//This is overkill but I didn't want to give up previous work
	const wchar_t * const ErrorMsg[]= 
	{
		L"Unable to create DirectSound object",
		L"Unable to set CooperativeLevel in DS",
		L"Unable to create DS primary sound buffer",
		L"Unable to get capabilities",
		L"Unable to format DS default standard"
	};
	try
	{
		bool device_found = false;
						
		if(p_device_list)
		{	// keep track of all our devices
			std::map<std::wstring, GUID> device_map;		

			// enumerate the devices into our map
			if (FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)&DS_Output_Core::DSEnumProc, (VOID*)&device_map)))
			{	assert(false);
			}

			// try our list of preferred devices, in order until one works
			for(auto pref_it = p_device_list->begin(); pref_it != p_device_list->end(); ++pref_it)
			{	// find it in the map, by name
				auto devit = device_map.find(*pref_it);
				if(devit != device_map.end())
				{	// attempt to use it
					if(!FAILED(DirectSoundCreate8(&devit->second, &m_IDS8_Interface, NULL)))
					{	device_found = true;

						// try to disable ducking for this device
						// go ahead and run though, it's better than nothing
						if(FAILED(SystemAudioDucking(false, pref_it->c_str())))
						{	assert(false);
						}

						break;
					}
				}
			}
		}

		// fall back to default device?
		if(!device_found)
		{	if (FAILED(DirectSoundCreate8(NULL, &m_IDS8_Interface, NULL))) throw (int)eErrCreatingDSObject;
			
			// try to disable ducking for the default device
			// go ahead and run though, it's better than nothing
			if(FAILED(SystemAudioDucking(false, NULL)))
			{	assert(false);
			}
		}

		//Note: Cooperative level of DSSCL priority allows formating the primary buffer
		HWND hwndToUse=(ParentWindow)?ParentWindow:GetDesktopWindow();
		if (FAILED(m_IDS8_Interface->SetCooperativeLevel(hwndToUse,DSSCL_PRIORITY))) throw (int)eErrSettingCooperativeLevel;

		//Obtain primary buffer
		DSBUFFERDESC dsbdesc;
		INIT_DIRECTX_STRUCT(dsbdesc);
		dsbdesc.dwFlags=DSBCAPS_PRIMARYBUFFER;
		if FAILED(m_IDS8_Interface->CreateSoundBuffer(&dsbdesc,&m_IDS8_Buffer,NULL)) throw (int)eErrCreatingPrimarySoundBuffer;

		{ //Well try some formats (until I can figure out how to do this better!)
			size_t i=0;
			struct formatdata
			{
				WORD wFormatTag;
				WORD nChannels;
				DWORD nSamplesPerSecond;
				WORD wBitsPerSample;

			} trythis[]=
			{
				//{WAVE_FORMAT_IEEE_FLOAT,4,96000,32},//....01
				//{WAVE_FORMAT_PCM,4,96000,32},//...........02
				//{WAVE_FORMAT_PCM,4,96000,16},//...........04
				//{WAVE_FORMAT_IEEE_FLOAT,2,96000,32},//....08
				//{WAVE_FORMAT_PCM,2,96000,32},//...........10
				//{WAVE_FORMAT_PCM,2,96000,16},//...........20

				//{WAVE_FORMAT_IEEE_FLOAT,4,48000,32},//....40
				//{WAVE_FORMAT_PCM,4,48000,32},//...........80
				//{WAVE_FORMAT_PCM,4,48000,16},//..........100
				//{WAVE_FORMAT_IEEE_FLOAT,2,48000,32},//...200
				//{WAVE_FORMAT_PCM,2,48000,32},//..........400
				{WAVE_FORMAT_PCM,2,48000,16},//..........800

				//{WAVE_FORMAT_IEEE_FLOAT,4,44100,32},//..1000
				//{WAVE_FORMAT_PCM,4,44100,32},//.........2000
				//{WAVE_FORMAT_PCM,4,44100,16},//.........4000
				//{WAVE_FORMAT_IEEE_FLOAT,2,44100,32},//..8000
				//{WAVE_FORMAT_PCM,2,44100,32},//........10000
				//{WAVE_FORMAT_PCM,2,44100,16}//.........20000
			};
			const size_t NoElements = SIZEOF_ARRAY(trythis);

			DSCAPS capabilities;
			INIT_DIRECTX_STRUCT(capabilities);

			unsigned long filter=0xffff; //32 bits... 32 things to try
			if (m_IDS8_Interface->GetCaps(&capabilities)!=DS_OK) throw (int)eErrGetCapsFailed;
			
			//enable all 32 bit types by default since there is not a dwFlags to represent them
			filter=0x01|0x02|0x08|0x10|0x40|0x80|0x200|0x400|0x1000|0x2000|0x8000|0x10000; 
			DWORD MinSampleRate=capabilities.dwMinSecondarySampleRate;
			DWORD MaxSampleRate=capabilities.dwMaxSecondarySampleRate;
			if  (!(
				(MaxSampleRate==96000) || (MinSampleRate=96000) ||
				((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>96000)&&(MinSampleRate<96000))
				))
				filter &= (0x01|0x02|0x04|0x08|0x10|0x20);

			if  (!(
				(MaxSampleRate==48000) || (MinSampleRate=48000) ||
				((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
				))
				filter &= (0x40|0x80|0x100|0x200|0x400|0x800);

			if (capabilities.dwFlags & DSCAPS_SECONDARY16BIT)
			{
				if (capabilities.dwFlags & DSCAPS_SECONDARYSTEREO)
				{
					if  (
						(MaxSampleRate==96000) || (MinSampleRate=96000) ||
						((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>96000)&&(MinSampleRate<96000))
						)
						filter |= (0x04|0x20); //16bit 96000 for 2 and 4 channel

					if  (
						(MaxSampleRate==48000) || (MinSampleRate=48000) ||
						((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
						)
						filter |= (0x100|0x800); //16bit 48000 for 2 and 4 channel

					if  (
						(MaxSampleRate==44100) || (MinSampleRate=44100) ||
						((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
						)
						filter |= (0x4000|0x20000); //16bit 44100 for 2 and 4 channel
				}
			}
			
			bool Success=false;
			#ifdef __TestFailure__
			throw (int)eErrCouldNotFindFormat;  //Testing failure case...
			#endif
			do 
			{
				if (!((1<<i) & filter))
					continue;
				SetWaveFormat(m_FormatToUse,trythis[i].wFormatTag,trythis[i].nChannels,trythis[i].nSamplesPerSecond,trythis[i].wBitsPerSample);
				Success=(m_IDS8_Buffer->SetFormat(&m_FormatToUse)==DS_OK);
				//Just because it works in the primary doesn't mean it will work in the secondary... drat!
				if (Success)
				{
					//Test the secondary buffer too
					LPDIRECTSOUNDBUFFER lpdsb;
					DSBUFFERDESC dsbdesc;
					INIT_DIRECTX_STRUCT(dsbdesc);
					dsbdesc.dwFlags         = 
						DSBCAPS_GLOBALFOCUS			|	//If my window is minimized I'm still going to be heard!
						DSBCAPS_STATIC				|	//Use hardware buffers if I can
						DSBCAPS_CTRLPOSITIONNOTIFY	|	//I'm going to be using a notification scheme
						DSBCAPS_GETCURRENTPOSITION2;	//We are a new product use the best accurate code

					WAVEFORMATEX *FormatToUse=&m_FormatToUse;
					dsbdesc.dwBufferBytes=(DWORD)FormatToUse->nBlockAlign * ((DWORD)GetBufferSampleSize()*2);
					dsbdesc.lpwfxFormat=FormatToUse;
					if FAILED(m_IDS8_Interface->CreateSoundBuffer(&dsbdesc,&lpdsb,NULL))
						Success=false;
					if (lpdsb)
						lpdsb->Release();

				}
			}while ((!Success)&&(i++<(NoElements-1)));
			if (i==NoElements) throw (int)eErrCouldNotFindFormat;
		}
	}
	catch (int ErrorCode)
	{
		m_IDS8_Buffer = NULL;
		#ifndef __TestFailure__
		assert(false);
		#endif
		debug_output("DS_Output_Core::StartUp %s %d",ErrorMsg[ErrorCode],ErrorCode);
	}
	return m_IDS8_Buffer != NULL;
}

BOOL CALLBACK DS_Output_Core::DSEnumProc(LPGUID lpGUID, LPCWSTR lpszDesc, LPCWSTR lpszDrvName, LPVOID lpContext )
{	// NULL only for "Primary Sound Driver".
	if(lpGUID == NULL) return (TRUE);

	// get access to our device map
    std::map<std::wstring, GUID>* p_device_map = (std::map<std::wstring, GUID>*)lpContext;
    
	// get the description as a string (driver name is a GUID as a string)
	std::wstring desc = lpszDesc;
	if(desc.empty()) return (TRUE);

	// make sure we're not adding it twice
	auto it = p_device_map->find(desc);
	if(it == p_device_map->end())
	{	(*p_device_map)[desc] = *lpGUID;
	}
	    
	return(TRUE);
}

void DS_Output_Core::ShutDown()
{
	//m_AbortThread = true;
	SetEvent(m_NotificationTimer);
	//StopThread();

	if (m_IDS8_Buffer)
	{
		m_IDS8_Buffer->Stop();
		m_IDS8_Buffer->Release();
		m_IDS8_Buffer=NULL;
	}

	if (m_IDS8_Interface)
	{
		m_IDS8_Interface->Release();
		m_IDS8_Interface=NULL;
	}
}
  /************************************************************************/
 /*						  DirectSound_Globals							 */
/************************************************************************/
bool DirectSound_Globals::init( const std::vector<std::wstring>* p_device_list )
{
	m_DS_Output_Core=new DS_Output_Core;
	return m_DS_Output_Core->StartUp(NULL, p_device_list);
}

DirectSound_Globals::DirectSound_Globals()
{

}

DirectSound_Globals::~DirectSound_Globals()
{
	delete m_DS_Output_Core;
	m_DS_Output_Core=NULL;
}

  /***********************************************************************/
 /*								DS_Output								*/			
/***********************************************************************/

DS_Output::DS_Output() 
{
	m_DS = std::make_shared<DS_Output_Internal>();
}

void DS_Output::SetCallbackFillBuffer(std::function<void(size_t, short *, size_t)> callback)
{
	m_DS->SetCallbackFillBuffer(callback);
}

DS_Output::~DS_Output()
{
	#if 0
	debug_output(p_debug_category,L"Stopping server %s",m_VideoName.c_str());
	#endif

	StopStreaming();
}

void DS_Output::StartStreaming(void)
{
	m_IsStreaming=true;
	m_DS->StartStreaming();
}

void DS_Output::StopStreaming(void)
{
		m_IsStreaming=false;
		m_DS->StopStreaming();
}


  /************************************************************************/
 /*						  DirectSound_Initializer					     */
/************************************************************************/

long DirectSound_Initializer::s_ref = 0;
critical_section	lock_directsound_startup;

bool DirectSound_Initializer::StartUp_DirectSound( const std::vector<std::wstring>* p_device_list )
{
	auto_lock	lock_hw( lock_directsound_startup );
	s_ref++;

	if ( s_ref == 1 )
	{
		if (!g_DirectSoundInitializer)
		{
			g_DirectSoundInitializer= new DirectSound_Globals;
			if (!g_DirectSoundInitializer->init(p_device_list))
				ShutDown_DirectSound();
		}
		else
			ShutDown_DirectSound();
	}

	return (g_DirectSoundInitializer!=NULL);
}

void DirectSound_Initializer::ShutDown_DirectSound(void)
{	
	auto_lock	lock_hw( lock_directsound_startup );
	s_ref--;

	if ( !s_ref )
	{
		if (g_DirectSoundInitializer)
		{	delete g_DirectSoundInitializer;
		g_DirectSoundInitializer=NULL;
		}
	}
}

void SetCallbackFillBuffer(std::function<void(size_t, short *, size_t)> callback);