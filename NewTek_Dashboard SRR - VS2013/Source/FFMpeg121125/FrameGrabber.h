
#pragma once
#include "../Dashboard/Dashboard_Interfaces.h"
#include "../ProcAmp/ProcAmp.h"

size_t split_arguments(const std::string& str, std::vector<std::string>& arguments, const char *delimiter_list=" \t\n\r");

class FrameGrabber_Interface
{
public:
	virtual ~FrameGrabber_Interface() {}

	virtual void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview)=0;
	virtual bool StartStreaming()=0;
	virtual void StopStreaming()=0;

	//override if you are a Dashboard_Controller_Interface
	virtual Dashboard_Controller_Interface *GetDashboard_Controller_Interface() {return NULL;}
};

class FrameGrabber_TestPattern : public FrameGrabber_Interface
{
public:
	FrameGrabber_TestPattern(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"") : m_pThread(NULL),m_TestMap(640,480),m_Outstream(Preview),
		m_SleepMs(16),m_UseBlackField(false)
	{
		if (IPAddress)
		{
			m_UseBlackField=wcsnicmp(IPAddress,L"Black",5)==0;
			if (IPAddress[5]=='_')
			{
				std::vector<std::string> Args;
				wchar2char(IPAddress);
				split_arguments(wchar2char_pchar,Args,"_xX");
				if (Args.size()>1)
					m_SleepMs=atoi(Args[1].c_str());
				if (Args.size()>3)
				{
					const int xres=atoi(Args[2].c_str());
					const int yres=atoi(Args[3].c_str());
					m_TestMap.resize(xres,yres);
				}
			}
		}
	}
	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_Outstream=Preview;}
	bool StartStreaming()
	{
		m_Counter=0;
		m_pThread = new FrameWork::Threads::thread<FrameGrabber_TestPattern>(this);
		return true;
	}

	void StopStreaming()
	{
		delete m_pThread;
		m_pThread=NULL;
	}

	virtual ~FrameGrabber_TestPattern()
	{
		StopStreaming();
	}
private:
	friend FrameWork::Threads::thread<FrameGrabber_TestPattern>;

	void operator() ( const void* )
	{
		using namespace FrameWork;
		Sleep(m_SleepMs);
		if (!m_UseBlackField)
			DrawField( (PBYTE) m_TestMap(),m_TestMap.xres(),m_TestMap.yres(),m_Counter++ );
		else
			BlackField( (PBYTE) m_TestMap(),m_TestMap.xres() * m_TestMap.yres() * sizeof(WORD) );
			
		//Black field is progressive while the test pattern is interlaced
		m_Outstream->process_frame(&m_TestMap,!m_UseBlackField,(double)time_type::get_current_time(),4.0f/3.0f);
		//printf("%d\n",m_Counter++);
	}
	FrameWork::Threads::thread<FrameGrabber_TestPattern> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

private:
	FrameWork::Bitmaps::bitmap_ycbcr_u8 m_TestMap;
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	DWORD m_SleepMs;
	size_t m_Counter;
	bool m_UseBlackField;
};


class FrameGrabber_FFMpeg : public FrameGrabber_Interface
{
public:
	enum IpURLConversion
	{
		eIpURL_H264,
		eIpURL_MJPEG
	};
	FrameGrabber_FFMpeg(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"");

	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview);
	bool StartStreaming();
	void StopStreaming();

	virtual ~FrameGrabber_FFMpeg();
protected:
	void SetSeekable(bool Seekable) {m_Seekable=Seekable;}
	bool GetSeekable() const {return m_Seekable;}
	void SetFileName(const wchar_t *IPAddress,IpURLConversion format=eIpURL_H264);

	void *m_VideoStream;
	std::string m_URL;
private:
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	FrameGrabber_TestPattern m_TestPattern;  //handle the empty string with something useful
	bool m_Seekable;
};

class FrameGrabber : public FrameGrabber_Interface
{
public:
	enum ReaderFormat
	{
		eTestPattern,
		eFFMPeg_Reader,
		eHttpReader,
		eHttpReader2
	};
	FrameGrabber(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"",ReaderFormat format=eTestPattern,LONG Port=0);
	virtual ~FrameGrabber();

	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_VideoStream->SetOutstream_Interface(Preview);}
	bool StartStreaming() {return m_VideoStream->StartStreaming();}
	void StopStreaming() {m_VideoStream->StopStreaming();}

	//We could make this check for NULL, but this is all setup during the construction of the dashboard, so we assert
	virtual Dashboard_Controller_Interface *GetDashboard_Controller_Interface() 
		{assert ( m_VideoStream);return m_VideoStream->GetDashboard_Controller_Interface();}
protected:

	FrameGrabber_Interface *m_VideoStream;
};

class FFPlay_Controller : public FrameGrabber_FFMpeg, 
						  public Dashboard_Controller_Interface,
						  public FrameWork::Outstream_Interface
{
public:
	FFPlay_Controller(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"",IpURLConversion IP_Format=eIpURL_H264) : 
	  FrameGrabber_FFMpeg(this,IPAddress),m_IP_Format(IP_Format),m_Preview(Preview),m_FailSafe(this,IPAddress)
	{
	}

protected:
	virtual Dashboard_Controller_Interface *GetDashboard_Controller_Interface() {return this;}
	//Intercept frames dispatched as a way to measure heartbeat in failsafe
	virtual void process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer,bool isInterlaced,double VideoClock,float AspectRatio);

	void Flush();
	int Run (void);											// run the filter graph
	int Stop (void);										// stop filter graph
	int Pause (void);										// pause filter graph
	virtual void Seek (__int64 position_10us);				//time stamp in 10 us units
	void Seek (double fraction);
	int SetRate (int);										// set the play speed  (as percentage of normal)
	void SwitchFilename(const wchar_t FileToUse[]);
	void GetFileName(std::wstring &Output) const;
	virtual __int64 GetDuration() const;
	virtual __int64 GetPosition() const;
	void Record(bool start);
	bool GetRecordState(void);
	virtual void SetRecordPath(const char *Path);
	virtual const char *GetRecordPath();
	virtual bool Set_ProcAmp(ProcAmp_enum ProcSetting,double value);
	virtual double Get_ProcAmp(ProcAmp_enum ProcSetting) const;

	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_Preview=Preview;}

private:
	//This class simply listens for a DO exit event, and will SwitchFilename process if it receives this event
	class FailSafe
	{
		public:
			FailSafe(Dashboard_Controller_Interface *pParent,const wchar_t *IPAddress=L"");
			~FailSafe();
			void UpdateHeartBeat() {m_Heartbeat.set();}
			void SwitchFilename(const wchar_t FileToUse[]) {m_FileName=FileToUse;}
		private:
			Dashboard_Controller_Interface * const m_pParent;
			friend FrameWork::Threads::thread<FailSafe>;
			void operator() ( const void* );
			FrameWork::Threads::thread<FailSafe> *m_pThread;
			FrameWork::event m_Heartbeat;
			std::wstring m_FileName;  //cache last filename used
	} m_FailSafe;


	IpURLConversion m_IP_Format;
	FrameWork::Outstream_Interface *m_Preview;
};