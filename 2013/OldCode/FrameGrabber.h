
#pragma once
#include "../Dashboard/Dashboard_Interfaces.h"
#include "../ProcAmp/ProcAmp.h"

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
	FrameGrabber_TestPattern(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"") : m_pThread(NULL),m_TestMap(720,480),m_Outstream(Preview)
	{
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
		Sleep(16);
		//Sleep(33);
		//Sleep(1000);
		DrawField( (PBYTE) m_TestMap(),m_TestMap.xres(),m_TestMap.yres(),m_Counter++ );
		m_Outstream->process_frame(&m_TestMap,true,(double)time_type::get_current_time());
		//printf("%d\n",m_Counter++);
	}
	FrameWork::Threads::thread<FrameGrabber_TestPattern> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

private:
	FrameWork::Bitmaps::bitmap_ycbcr_u8 m_TestMap;
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	size_t m_Counter;
};

class FailSafeReset_Interface
{
public:
	virtual void Reset()=0;
};

class FrameGrabber_FFMpeg : public FrameGrabber_Interface,
							public FailSafeReset_Interface
{
public:
	enum IpURLConversion
	{
		eIpURL_H264,
		eIpURL_MJPEG
	};
	FrameGrabber_FFMpeg(Dashboard_Controller_Interface *parent,FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"");

	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview);
	bool StartStreaming();
	void StopStreaming();

	virtual ~FrameGrabber_FFMpeg();
protected:
	virtual void Reset();
	void Reset_Thread();  //This launches reset on a deferred procedure call
	void SetFileName(const wchar_t *IPAddress,IpURLConversion format=eIpURL_H264);

	void *m_VideoStream;
private:
	size_t split_arguments(const std::string& str, std::vector<std::string>& arguments);
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	Dashboard_Controller_Interface * const m_Parent; //used to implement the failsafe()
	FrameGrabber_TestPattern m_TestPattern;  //handle the empty string with something useful
	std::string m_Options; //Support options to see what options we would need
	//Used for failsafe
	FrameWork::Work::thread m_thread;  //For DPC support
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
	FrameGrabber(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"",ReaderFormat format=eTestPattern);
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
						  public Dashboard_Controller_Interface
{
public:
	FFPlay_Controller(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"",IpURLConversion IP_Format=eIpURL_H264) : 
	  FrameGrabber_FFMpeg(this,Preview,IPAddress),m_IP_Format(IP_Format)
	{
	}

protected:
	virtual Dashboard_Controller_Interface *GetDashboard_Controller_Interface() {return this;}

	void Flush();
	int Run (void);											// run the filter graph
	int Stop (void);										// stop filter graph
	int Pause (void);										// pause filter graph
	void Seek (double fraction);	
	int SetRate (int);										// set the play speed  (as percentage of normal)
	void SwitchFilename(const wchar_t FileToUse[]);
	void GetFileName(std::wstring &Output) const;
	virtual bool Set_ProcAmp(ProcAmp_enum ProcSetting,double value);
	virtual double Get_ProcAmp(ProcAmp_enum ProcSetting) const;
private:
	IpURLConversion m_IP_Format;
};
