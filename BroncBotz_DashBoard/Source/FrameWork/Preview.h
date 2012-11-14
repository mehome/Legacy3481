#pragma once

class Preview;

class Buffer 
{
public:

	Buffer(IDirectDraw7 *pDD, size_t displayDeviceIdx);
	virtual ~Buffer( void );

	/// \param fieldType allows frames to be sent as they come and then properly copied onto the surface buffer
	void process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer);

	const FrameWork::event &GetFrameEvent() {return m_FrameEvent;}
	//This will ensure the surface is allocated on the correct index
	void UpdateDisplayDeviceIdx(size_t displayDeviceIdx,IDirectDraw7 *pDD);
	size_t GetDisplayDeviceIdx( void ) const;
	bool GetError( void );
	//Access to perform the blit
	const CComPtr<IDirectDrawSurface7>& GetSurface( void ) const;
	enum BufferState
	{
		eAvailable,
		eInFlight,
		eReadyToRender
	};
	BufferState GetBufferState() const {return m_BufferState;}
	void SetBufferState(BufferState state);
protected:
	void GetFrameInformation(int &FieldNumber) const;
	int GetFramePitch( void ) const;

	PBYTE pGetBufferData(size_t *pMemorySize,size_t desiredXRes,size_t desiredYRes);
	void ReleaseBufferData( const wchar_t *pErrorMessage = 0); 
	
private:
	size_t m_DisplayDeviceIdx;
	DDSURFACEDESC2 m_LastSurfaceDesc;  //Cache last surface buffer description for proper handling of fielded locking

	float m_Aspect;
	size_t m_cxPels;
	size_t m_cyPels;
	int m_Pitch;

	// Kenny pay attention to this, it might help you
	LONG m_lErrorCondition;

	CComPtr<IDirectDraw7> m_pDD;
	CComPtr<IDirectDrawSurface7> m_pSurface;
	FrameWork::event m_FrameEvent;
	BufferState m_BufferState;
};

//********************************************************************************************************************************************
				
class DisplayDeviceInfo
{
public:

	struct Desc
	{
		GUID m_GUID;
		RECT m_Rect;
	};

public:

	DisplayDeviceInfo( void );
	~DisplayDeviceInfo( void );

	__forceinline bool GetDesc(size_t i, Desc &desc) const;
	__forceinline size_t GetNum( void ) const;

private:

	// The callback function
	static BOOL WINAPI sDDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm);
	static std::vector<Desc> s_Descs;
};

const size_t Preview_NoVideoBuffers=4; //To be safe we need about 3 fields for best results this should be either 2, 4 or 6

/// The DDraw specifics of this class were written by Mike as device, but in this version it has been transformed into a pull kind of model that is 
/// very similar to how HDOut works (Note: HDOut is a push model though).  It is stripped of FC3 management, and using a memcopy within process_frame 
/// makes it possible to release the frames sent to it immediately (thus making it not necessary for anything more complex than a framework bitmap).  
/// If we wanted to avoid the memcopy I would have to see if it was possible for the allocation of surfaces to reference memory, and then create a 
/// generic video frame type like I did in HDout.  In this case I favor simplicity as I suspect we'll need to conduct a colorspace conversion anyhow.
/// Since this is a pull model it is designed for the input to manage the clock and it will monitor the framerate and ensure that it can evenly
/// distribute the render times... even if they are not passed as such (e.g. interleaved frames can send both fields simultaneously).
///  [11/11/2012 James]
class Preview
{		
public:
	
	// Types
	enum eThreadPriority { eThreadPriority_Default, eThreadPriority_Low, eThreadPriority_Med, eThreadPriority_High, eThreadPriority_RT };

	Preview( HWND hWnd, eThreadPriority priority=eThreadPriority_Low);
	virtual ~Preview( void );

	bool Get_IsError() const {return m_IsError;} //make sure everything is happy before pumping frames to it
	void process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer);

	//override these to do things like start / stop the fc3 server
	virtual void StartStreaming( void );
	virtual void StopStreaming( void );

protected:

	// These 2 bad boys return success
	bool InitDirectDraw( void );
	bool CreateAllDisplayDevices( void );

	bool UpdateCurrDisplayDeviceForWnd( void );			// Returns true if changed
	IDirectDraw7 *GetCurrentDisplayDevice( void ) const;

private:
	// My worker thread function that does something useful w/ a buffer after it's been filled
	friend FrameWork::thread<Preview>;
	void operator() ( const void* );
	FrameWork::thread<Preview> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

	Buffer *m_VideoBuffers[Preview_NoVideoBuffers];	//These are the surfaces that get filled
	Buffer *m_LastBufferProcessed;  //keep track of last buffer processed for under-run timeout repaint
	size_t m_CurrentFrameIndex; //This keeps track of the current index we are on

	HWND m_hWnd;
	CComPtr<IDirectDrawSurface7>	m_pPrimary;
	CComPtr<IDirectDrawClipper>		m_pClipper;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DisplayDeviceInfo *m_pDisplayDeviceInfo;

	std::vector<IDirectDraw7*> m_DisplayDevices;
	typedef std::vector<IDirectDraw7*>::iterator DisplayDevicesIter;
	size_t m_CurrDisplayDeviceIdx;

	eThreadPriority m_ThreadPriority;
	FrameWork::event_array m_EventArray; //this consist of the current event index, and the terminate

	FrameWork::event m_TerminateEvent;
	FrameWork::Blend_Averager<double> m_FrameRateAverage; //keeping track of average frame rate in milliseconds
	FrameWork::time_type m_LastTime;  //keep track of time

	bool m_IsError;  //determined during construction
	bool m_IsStreaming;
};

//********************************************************************************************************************************************
//********************************************************************************************************************************************