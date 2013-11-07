#include "stdafx.h"
#include <ddraw.h>
#include <atlbase.h>
#include "Framework.h"

#include "Preview.h"

#undef IS_DDRAW_AVAILABLE
typedef HRESULT (WINAPI *DirectDrawEnumerateExA_t)(LPDDENUMCALLBACKEXA, LPVOID, DWORD);
typedef HRESULT (WINAPI *DirectDrawCreateEx_t)(GUID FAR*, LPVOID*, REFIID, IUnknown FAR*);

#ifdef IS_DDRAW_AVAILABLE
DirectDrawEnumerateExA_t fcn_DirectDrawEnumerateExA = DirectDrawEnumerateExA;
DirectDrawCreateEx_t fcn_DirectDrawCreateEx = DirectDrawCreateEx;
#pragma comment (lib,"Ddraw")
#else
DirectDrawEnumerateExA_t fcn_DirectDrawEnumerateExA = NULL;
DirectDrawCreateEx_t fcn_DirectDrawCreateEx = NULL;

static bool LoadDirectDraw(void)
{
	HMODULE ddrawLib = ::LoadLibrary(L"ddraw.dll");
	if (!ddrawLib)
		return false;

	fcn_DirectDrawEnumerateExA = (DirectDrawEnumerateExA_t)GetProcAddress(ddrawLib, "DirectDrawEnumerateExA");
	fcn_DirectDrawCreateEx = (DirectDrawCreateEx_t)GetProcAddress(ddrawLib, "DirectDrawCreateEx");

	return fcn_DirectDrawEnumerateExA && fcn_DirectDrawCreateEx;
}

static bool DirectDrawLoaded = LoadDirectDraw();
#endif

#pragma comment (lib,"dxguid.lib")

using namespace FrameWork;

static void ShowTimeDelta(char label[]="",bool UsePrintF=false)
{
	using namespace FrameWork;
	static time_type LastTime=(0.0);
	time_type currenttime=time_type::get_current_time();
	time_type delta = currenttime-LastTime;
	if (UsePrintF)
		printf("%s-> %f\n",label,(double)delta*1000.0);
	else
		DebugOutput("%s-> %f\n",label,(double)delta*1000.0);
	LastTime=currenttime;
}

  /*********************************************************************************************************************************/
 /*														DisplayDeviceInfo												 		  */
/*********************************************************************************************************************************/


std::vector<DisplayDeviceInfo::Desc> DisplayDeviceInfo::s_Descs;

BOOL DisplayDeviceInfo::sDDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	// Anything which is not a monitor should be nuked
	if (!hm) 
		return TRUE;
	if (!lpGUID)
	{
		assert(false);
		return TRUE;
	}

	// Get the monitor information
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(hm , &mi);

	Desc newDesc = { *lpGUID, mi.rcMonitor };
	s_Descs.push_back(newDesc);

	return TRUE;
}

DisplayDeviceInfo::DisplayDeviceInfo( void )
{
	if (!s_Descs.size())
	{
		HRESULT hr = fcn_DirectDrawEnumerateExA(sDDEnumCallbackEx, NULL, DDENUM_ATTACHEDSECONDARYDEVICES);
		assert(DD_OK == hr);

		// If there are no monitors attached, used screen resolution only
		if (!s_Descs.size())
		{	
			RECT monRect = { 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };
			Desc newDesc = { GUID_NULL, monRect };
			s_Descs.push_back(newDesc);
		}
	}
}

DisplayDeviceInfo::~DisplayDeviceInfo( void )
{
}

bool DisplayDeviceInfo::GetDesc(size_t i, Desc &desc) const
{
	if (i >= s_Descs.size())
		return false;
	
	desc = s_Descs[i];
	return true;
}

size_t DisplayDeviceInfo::GetNum( void ) const
{
	return s_Descs.size();
}


  /*********************************************************************************************************************************/
 /*																Buffer													 		  */
/*********************************************************************************************************************************/


Buffer::Buffer(IDirectDraw7 *pDD, size_t displayDeviceIdx) :  m_Pitch(0), m_lErrorCondition(0L), m_pDD(pDD), m_Aspect(0.0), 
	m_cxPels(0), m_cyPels(0), m_DisplayDeviceIdx(displayDeviceIdx),m_FrameEvent(false), m_BufferState(eAvailable)
{
	m_FrameEvent.set(false);
}

Buffer::~Buffer( void )
{
	ReleaseBufferData(NULL);
}

void Buffer::GetFrameInformation(int &FieldNumber) const
{
	FieldNumber = 0;
}

int Buffer::GetFramePitch( void ) const
{
	return m_Pitch;
}

PBYTE Buffer::pGetBufferData(size_t *pMemorySize,size_t desiredXRes,size_t desiredYRes)
{
	if (desiredXRes!=m_cxPels || desiredYRes!=m_cyPels)
	{
		//TODO see if we need to worry about releasing this
		//m_pSurface->Release();
		m_pSurface = NULL;
	}

	// Reset my error condition b/c I'm about to be refilled
	::InterlockedExchange(&m_lErrorCondition, 0L);

	if (!m_pSurface)	// Alloc if never alloc'ed before
	{
		DDSURFACEDESC2 ddsd = { 0 };
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;	// Create an flippable scaleable overlay surface
		ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;	// TODO: Is there a flag where won't fail on Video RAM run out???
		ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddsd.ddpfPixelFormat.dwFourCC = ('Y'<<24) + ('V'<<16) + ('Y'<<8) + 'U';
		ddsd.dwWidth = static_cast<DWORD>(desiredXRes);
		ddsd.dwHeight = static_cast<DWORD>(desiredYRes);

		HRESULT hr = m_pDD->CreateSurface(&ddsd, &m_pSurface, NULL);
		assert(SUCCEEDED(hr) && m_pSurface);

		m_cxPels = desiredXRes;
		m_cyPels = desiredYRes;
	}

	if (m_pSurface->IsLost())	// TODO: Check retVal
		m_pSurface->Restore();

	DDSURFACEDESC2 Desc = { 0 };
	Desc.dwSize	= sizeof(Desc);
	
	// We wait until we can lock a surface
	//HRESULT hr = m_pSurface->Lock(NULL, &Desc, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT, NULL);
	//assert(SUCCEEDED(hr));

	while (DD_OK != m_pSurface->Lock(NULL, &Desc, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_DONOTWAIT|DDLOCK_DISCARDCONTENTS, NULL)) 
		::Sleep(1);
	m_LastSurfaceDesc=Desc; //TODO omit
	
	m_Pitch = Desc.lPitch;

	if (pMemorySize)
		*pMemorySize = m_Pitch * desiredYRes;

	return (PBYTE)Desc.lpSurface;
}

void Buffer::process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer)
{
	using namespace FrameWork::Bitmaps;
	assert (m_BufferState==eAvailable);
	//Grab this buffer reserve for fill operation
	if (m_BufferState==eAvailable)
		m_BufferState=eInFlight;
	size_t MemorySize;
	int YResToUse=pBuffer->yres();
	PBYTE frame=pGetBufferData(&MemorySize,pBuffer->xres(),YResToUse);
	bitmap_ycbcr_u8 DestBuffer((pixel_ycbcr_u8 *)frame,pBuffer->xres(),YResToUse,m_Pitch>>2);
	//perform the copy
	DestBuffer=*pBuffer;
	//update the state
	m_BufferState=eReadyToRender;
	ReleaseBufferData(NULL);  //unlock surface for rendering
	//wakeup rendering thread
	m_FrameEvent.set(true);
}

void Buffer::ReleaseBufferData( const wchar_t *pErrorMessage)
{
	if (m_pSurface)
	{
		HRESULT hr = m_pSurface->Unlock(NULL);
		//assert(SUCCEEDED(hr));
	}
	
	// Set any errors
	if (pErrorMessage || !m_pSurface)
		::InterlockedExchange(&m_lErrorCondition, 1L);
}



bool Buffer::GetError( void )
{
	long l_Error = ::InterlockedCompareExchange(&m_lErrorCondition, 0L, 0L);
	return (0L != l_Error);
}

size_t Buffer::GetDisplayDeviceIdx( void ) const
{
	return m_DisplayDeviceIdx;
}

void Buffer::UpdateDisplayDeviceIdx(size_t displayDeviceIdx,IDirectDraw7 *pDD)
{
	if (m_DisplayDeviceIdx!=displayDeviceIdx)
	{
		m_pSurface=NULL; //TODO see if we need to release
		m_BufferState=eAvailable;
		m_DisplayDeviceIdx=displayDeviceIdx;
		m_pDD=pDD;  //change the display device
	}
}

const CComPtr<IDirectDrawSurface7>& Buffer::GetSurface( void ) const
{
	return m_pSurface;
}

void Buffer::SetBufferState(BufferState state) 
{
	//turn on if assert fails
	#if 0
	if (m_BufferState==eInFlight)
		DebugBreak();
	#endif
	assert ((m_BufferState!=eInFlight)&&(state==eAvailable)); 
	m_FrameEvent.reset();  //lock it down
	m_BufferState=state;
}


  /*********************************************************************************************************************************/
 /*																Preview													 		  */
/*********************************************************************************************************************************/


Preview::Preview( HWND hWnd, Preview::eThreadPriority priority) : m_pThread(NULL), m_CurrentFrameIndex(0),m_hWnd(hWnd), 
	m_pDisplayDeviceInfo(new DisplayDeviceInfo()),m_CurrDisplayDeviceIdx(0),m_ThreadPriority(priority),m_FrameRateAverage(0.10),
	m_ConsecutiveRenderDelayCounter(0),m_LastUsingProcessSlot(-1),m_AspectRatio(16.0/9.0),m_IsStreaming(false)
{
	assert(::IsWindow(hWnd));
	if (CreateAllDisplayDevices())
	{
		UpdateCurrDisplayDeviceForWnd(); //start with the correct display device
		//This should always succeed provided the machine that is running the code is up to spec
		m_IsError=! ( this->InitDirectDraw());
	}
	assert(!m_IsError);
}

Preview::~Preview( void )
{
	StopStreaming();
	delete m_pDisplayDeviceInfo;
	for (DisplayDevicesIter iter = m_DisplayDevices.begin(); iter<m_DisplayDevices.end(); iter++)
	{
		if (*iter)
			(*iter)->Release();
	}
}

void Preview::StartStreaming()
{
	if (!m_IsStreaming)
	{
		for (size_t i=0;i<Preview_NoVideoBuffers;i++)
			m_VideoBuffers[i]=new Buffer(this->GetCurrentDisplayDevice(), m_CurrDisplayDeviceIdx);

		{
			//set up the last buffer processed with a black frame
			m_LastBufferProcessed=m_VideoBuffers[0];
			FrameWork::Bitmaps::bitmap_ycbcr_u8 black_frame(720,480);
			const int FieldSize_ =(int) (720*480*2);
			BlackField((PBYTE)black_frame(),FieldSize_);
			m_LastBufferProcessed->process_frame(&black_frame);
		}
		m_LastTime=time_type::get_current_time();

		//Note: We can assume there has been no error if we are here as there is an assertion in the constructor
		m_pThread = new FrameWork::Threads::thread<Preview>(this);
		switch (m_ThreadPriority)
		{
		case eThreadPriority_Low:
			m_pThread->low_priority();
			break;
		case eThreadPriority_Med:
			m_pThread->medium_priority();
			break;
		case eThreadPriority_High:
			m_pThread->high_priority();
			break;
		case eThreadPriority_RT:
			m_pThread->realtime_priority();
			break;
		default:
			// For default thread priority, leave as it is!
			break;
		}
		m_pThread->set_thread_name( "Preview Main Thread" );

		//This must be at the last possible moment this way everything is ready to go where the queue is not accumulating
		m_IsStreaming=true;
	}
}

void Preview::StopStreaming()
{
	if (m_IsStreaming)
	{
		m_IsStreaming=false;
		m_TerminateEvent.set();
		delete m_pThread;	// Stop my thread

		for (size_t i=0;i<Preview_NoVideoBuffers;i++)
		{
			delete m_VideoBuffers[i];
			m_VideoBuffers[i]=NULL;
		}
	}
}

bool Preview::InitDirectDraw( void )
{
	// Gotsa have the direct draw display devices created!!!
	if (!m_DisplayDevices.size())
	{
		assert(false);
		return false;
	}
	IDirectDraw7 *l_pDD = this->GetCurrentDisplayDevice();
	if (!l_pDD)
	{
		assert(false);
		return false;
	}

	// Gotsa reset these
	m_pPrimary.Release();
	m_pClipper.Release();

	HRESULT hr = DD_OK;
	do
	{
		hr = l_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);		// Set Normal coop level
		if (FAILED(hr))
		{
			assert(false);
			break;
		}
		// Create the primary buffer
		DDSURFACEDESC2 ddsd = { 0 };
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		hr = l_pDD->CreateSurface(&ddsd, &m_pPrimary, NULL);
		if (FAILED(hr))
		{
			assert(false);
			break;
		}

		hr = l_pDD->CreateClipper(NULL, &m_pClipper, NULL);	// Create a clipper object for the window
		if (FAILED(hr))
		{
			assert(false);
			break;
		}

		hr = m_pClipper->SetHWnd(NULL, m_hWnd);	// Set clipper to hWnd
		if (FAILED(hr))
		{
			assert(false);
			break;
		}

		hr = m_pPrimary->SetClipper(m_pClipper);
		if (FAILED(hr))
		{
			assert(false);
			break;
		}

		return true;
	}
	while (false);

	// Reset these on error
	m_pPrimary.Release();
	m_pClipper.Release();
	return false;
}

bool Preview::CreateAllDisplayDevices( void )
{	
	// Precondition
	assert(m_pDisplayDeviceInfo);

	if (m_DisplayDevices.size())
		return true;	// Already done this jazz

	bool bRetVal = true;

	// Create a IDirectDraw7 for every display device
	DisplayDeviceInfo::Desc desc = { 0 };
	size_t i=0;
	while (m_pDisplayDeviceInfo->GetDesc(i++, desc))
	{
		IDirectDraw7 *pDD = NULL;
		GUID *pGUID = NULL;
		if (!::IsEqualGUID(GUID_NULL, desc.m_GUID))
			pGUID = &(desc.m_GUID);
		HRESULT hr = fcn_DirectDrawCreateEx(pGUID, (void**)&pDD, IID_IDirectDraw7, NULL);
		if (FAILED(hr) || !pDD)
		{
			assert(false);
			bRetVal = false;
			break;
		}

		m_DisplayDevices.push_back(pDD);
	}

	// Do some cleanup on error
	if (!bRetVal)
	{
		for (DisplayDevicesIter iter = m_DisplayDevices.begin(); iter<m_DisplayDevices.end(); iter++)
		{
			if (*iter)
				(*iter)->Release();
		}
		m_DisplayDevices.clear();
	}

	return bRetVal;
}

bool Preview::UpdateCurrDisplayDeviceForWnd( void )
{
	if (!::IsWindow(m_hWnd))
		return false;

	const size_t l_CurrDisplayDeviceIdx_Prev = m_CurrDisplayDeviceIdx;
	size_t l_CurrDisplayDeviceIdx_New = l_CurrDisplayDeviceIdx_Prev;

	// Get the center of the window
	RECT rect = { 0 };
	::GetWindowRect(m_hWnd, &rect);
	const int wndCenterX = (rect.left+rect.right) / 2;
	const int wndCenterY = (rect.top+rect.bottom) / 2;
	
	// Look which monitor we are closest too ...
	int minDelta = INT_MAX;
	DisplayDeviceInfo::Desc desc = { 0 };
	size_t i=0;
	while (m_pDisplayDeviceInfo->GetDesc(i, desc))
	{
		const int monitorCenterX = (desc.m_Rect.left+desc.m_Rect.right) / 2;
		const int monitorCenterY = (desc.m_Rect.top+desc.m_Rect.bottom) / 2;

		const int delta = ::abs(monitorCenterX-wndCenterX) + ::abs(monitorCenterY-wndCenterY);
		if (delta < minDelta)
		{	
			minDelta = delta;
			l_CurrDisplayDeviceIdx_New  = i;
		}

		i++;
	}

	// This is the monitor number
	m_CurrDisplayDeviceIdx = l_CurrDisplayDeviceIdx_New;

	return (m_CurrDisplayDeviceIdx != l_CurrDisplayDeviceIdx_Prev);
}

IDirectDraw7* Preview::GetCurrentDisplayDevice( void ) const
{
	assert(m_CurrDisplayDeviceIdx < m_DisplayDevices.size());
	return m_DisplayDevices[m_CurrDisplayDeviceIdx];
}

void Preview::operator() ( const void* )
{
	m_EventArray.clear();
	Buffer  *BufferToProcess =m_VideoBuffers[m_CurrentFrameIndex]; 
	m_EventArray.push_back(BufferToProcess->GetFrameEvent());
	m_EventArray.push_back(m_TerminateEvent);
	int EventResult=m_EventArray.wait(false,100);

	if  ((m_IsStreaming)&&((EventResult==0) || (EventResult==-1)))
	{
		HRESULT hr = DD_OK;
		// Update my DD state if display device has changed
		if (UpdateCurrDisplayDeviceForWnd())
		{
			bool bInit = InitDirectDraw();
			assert(bInit);
		}

		BufferToProcess->UpdateDisplayDeviceIdx(m_CurrDisplayDeviceIdx,GetCurrentDisplayDevice());

		//for any problems such as under-run use the last successful buffer
		if  ((BufferToProcess->GetError()) || (BufferToProcess->GetBufferState()!=Buffer::eReadyToRender))
		{
			BufferToProcess=m_LastBufferProcessed;
			//Test last buffer for correct index as well
			BufferToProcess->UpdateDisplayDeviceIdx(m_CurrDisplayDeviceIdx,GetCurrentDisplayDevice());
		}
		size_t NextIndex=(m_CurrentFrameIndex+1) % Preview_NoVideoBuffers; 

		time_type current_time=time_type::get_current_time();
		double Delta=(double)(current_time-m_LastTime);

		if  ((!BufferToProcess->GetError()) && 	(BufferToProcess->GetBufferState()==Buffer::eReadyToRender) )
		{
			m_ConsecutiveRenderDelayCounter=0;
			m_LastTime=current_time;
		

			RECT wndRect = { 0 };
			::GetWindowRect(m_hWnd, &wndRect);

			// Translate rect to owner display device's origin
			DisplayDeviceInfo::Desc desc = { 0 };
			m_pDisplayDeviceInfo->GetDesc(BufferToProcess->GetDisplayDeviceIdx(), desc);
			wndRect.left -= desc.m_Rect.left;
			wndRect.right -= desc.m_Rect.left;
			wndRect.top -= desc.m_Rect.top;
			wndRect.bottom -= desc.m_Rect.top;

			// BitBlt
			//static DDBLTFX BltFX = { sizeof(DDBLTFX), DDBLTFX_NOTEARING };
			// We might need to recover lost surfaces
			if (m_pPrimary->IsLost())
				m_pPrimary->Restore();
			// Perform the blit
			//DebugOutput("rendering %d\n",m_CurrentFrameIndex);
			//hr = m_pPrimary->Blt(&wndRect, pBufferToDraw->GetSurface(), NULL, DDBLT_DDFX|DDBLT_WAIT, &BltFX);
			hr = m_pPrimary->Blt(&wndRect, BufferToProcess->GetSurface(), NULL, DDBLT_ASYNC, NULL);
			//ShowTimeDelta("Blt");

			//For now I'll leave this in as when it exits it could fail... I'll want to find out why this happens
			#if 1
			if (!SUCCEEDED(hr))
				DebugOutput("Preview::operator()  Warning:m_pPrimary->Blt failed\n");
			#else
			assert(SUCCEEDED(hr));
			#endif

			//Now that this buffer has been used... this will be cached as last used (marked as available for next frame received)
			if ((BufferToProcess!=m_LastBufferProcessed) || (m_VideoBuffers[NextIndex]->GetBufferState()==Buffer::eReadyToRender))
			{
				if (m_LastBufferProcessed->GetBufferState()==Buffer::eReadyToRender)
					m_LastBufferProcessed->SetBufferState(Buffer::eAvailable);
				else
					printf("Warning last buffer processed is %d\n",m_LastBufferProcessed->GetBufferState());
				m_LastBufferProcessed=BufferToProcess;
				m_CurrentFrameIndex=NextIndex; //advance the index
			}
			else
				Sleep(16);  //avoid busy wait (this logic happens if no frames have been coming in)
		}
	}
}

void Preview::process_frame_internal(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer)
{
	m_LastUsingProcessSlot=(m_LastUsingProcessSlot+1) % Preview_NoVideoBuffers;
	if (m_VideoBuffers[m_LastUsingProcessSlot]->GetBufferState()!=Buffer::eAvailable)
		Sleep(5);  //give it a chance to open up
	if (m_VideoBuffers[m_LastUsingProcessSlot]->GetBufferState()!=Buffer::eAvailable)
	{
		DebugOutput("   ***Preview::process_frame warning slots may be out of order\n");
		m_LastUsingProcessSlot=-1;
		for (size_t i=0;i<Preview_NoVideoBuffers;i++)
		{
			if (m_VideoBuffers[i]->GetBufferState()==Buffer::eAvailable)
			{
				m_LastUsingProcessSlot=i;
				break;
			}
		}
	}
	m_SignalFrameReady.set();
	//ShowTimeDelta("process_frame");

	if (m_LastUsingProcessSlot!=-1)
	{
		m_VideoBuffers[m_LastUsingProcessSlot]->process_frame(pBuffer);
		//DebugOutput("using %d\n",m_LastUsingProcessSlot);
	}
	else
	{
		//Keep track of failed buffer matches (I can see this happening in some rare cases, but no flooding cases)
		printf("Preview::process_frame failed to find available buffer\n");
		DebugOutput("Preview::process_frame failed to find available buffer\n");
	}
}


void Preview::process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer,bool isInterlaced,double VideoClock,float AspectRatio)
{
	using namespace FrameWork::Bitmaps;
	m_AspectRatio=AspectRatio;
	bitmap_ycbcr_u8 DestBuffer((*pBuffer)(),pBuffer->xres(),pBuffer->yres(),pBuffer->stride());
	if (isInterlaced)
	{
		DestBuffer.reference_even_lines(*pBuffer);
		process_frame_internal(&DestBuffer);
		DestBuffer.reference_odd_lines(*pBuffer);
		process_frame_internal(&DestBuffer);
	}
	else
		process_frame_internal(&DestBuffer);
}