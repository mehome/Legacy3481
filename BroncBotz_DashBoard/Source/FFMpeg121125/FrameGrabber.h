
#pragma once

class FrameGrabber_TestPattern
{
public:
	FrameGrabber_TestPattern(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"") : m_pThread(NULL),m_TestMap(720,480),m_Outstream(Preview)
	{
	}
	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_Outstream=Preview;}
	void StartStreaming()
	{
		m_Counter=0;
		m_pThread = new FrameWork::tThread<FrameGrabber_TestPattern>(this);
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
	friend FrameWork::tThread<FrameGrabber_TestPattern>;

	void operator() ( const void* )
	{
		using namespace FrameWork;
		Sleep(16);
		//Sleep(33);
		//Sleep(1000);
		DrawField( (PBYTE) m_TestMap(),m_TestMap.xres(),m_TestMap.yres(),m_Counter++ );
		m_RGB=m_TestMap;
		m_Outstream->process_frame(&m_RGB);
		//printf("%d\n",m_Counter++);
	}
	FrameWork::tThread<FrameGrabber_TestPattern> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

private:
	FrameWork::Bitmaps::bitmap_ycbcr_u8 m_TestMap;
	FrameWork::Bitmaps::bitmap_bgra_u8 m_RGB;
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	size_t m_Counter;
};

struct AVCodecContext;
struct SwsContext;
class FrameGrabber_HttpStream
{
	enum ThreadType
	{
		ReceivingThread,
		ProcessingThread,
	};

	typedef FrameWork::tThread<FrameGrabber_HttpStream, const ThreadType&> thread_t;

public:
	FrameGrabber_HttpStream(FrameWork::Outstream_Interface* Preview=NULL, const wchar_t* IPAddress=L"");
	virtual ~FrameGrabber_HttpStream(void);

	void SetOutstream_Interface(FrameWork::Outstream_Interface* Preview);

	bool StartStreaming(void);
	void StopStreaming(void);

private:
	bool ReceiveData(void);
	bool ProcessData(void);
	void operator()(const ThreadType&);

private:
	bool m_Error;

	FrameWork::Outstream_Interface* m_pOutstream;
	thread_t* m_pRecvThread;
	thread_t* m_pProcThread;

	int m_PortNum;
	std::wstring m_HostName;
	std::wstring m_Resource;
	std::wstring m_UserName;
	std::wstring m_Password;
	DWORD m_AuthScheme;

	HINTERNET m_hSession;
	HINTERNET m_hConnection;
	HINTERNET m_hRequest;

	std::wstring m_BoundaryToken;
	std::vector<BYTE> m_RecvBuffer;
	size_t m_RecvBufferSize;
	size_t m_ParserOffset;

	typedef std::pair<size_t, BYTE*> PreProcessedData;
	std::queue<PreProcessedData> m_ProcessingQueue;
	FrameWork::critical_section m_ProcessingQueue_CS;

	AVCodecContext* m_pCodecCtx;
	SwsContext* m_pSwsContext;

	friend thread_t;
};

class FrameGrabber
{
public:
	FrameGrabber(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"");

	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview);
	void StartStreaming();
	void StopStreaming();

	virtual ~FrameGrabber();

protected:

	void *m_VideoStream;
private:
	size_t split_arguments(const std::string& str, std::vector<std::string>& arguments);
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	FrameGrabber_TestPattern m_TestPattern;  //handle the empty string with something useful
	std::string m_Options; //Support options to see what options we would need
};

class FFPlay_Controller : public FrameGrabber
{
public:
	FFPlay_Controller(FrameWork::Outstream_Interface *Preview=NULL,const wchar_t *IPAddress=L"") : FrameGrabber(Preview,IPAddress)
	{
	}

	void Flush();
	int Run (void);											// run the filter graph
	int Stop (void);										// stop filter graph
	int Pause (void);										// pause filter graph
	int Seek (double, double,  bool scrubbing =false);		// seek to start/stop positions (in seconds)
	int SetRate (int);										// set the play speed  (as percentage of normal)
};
