#include "stdafx.h"
#include "Framework.h"
#include "Window.h"

Window::operator HWND( void )
{	return m_hWnd;
}

// Constructor
Window::Window( HWND Parent , const bool IsPopup , 
				const wchar_t *pWindowName ,
				const RECT *pWindowPosition )
	:	m_hWnd( NULL ) , 
		m_pThread( NULL ) ,
		m_Parent_hWnd( Parent ) , 
		m_PopupWindow( IsPopup ) ,
		m_pWindowName( pWindowName )
{	// Store the position
	if (!pWindowPosition)
	{	static RECT lWindowPosition = { ::GetSystemMetrics(SM_CXSCREEN)/4 , 
										::GetSystemMetrics(SM_CYSCREEN)/4 , 
										::GetSystemMetrics(SM_CXSCREEN)*3/4 , 
										::GetSystemMetrics(SM_CYSCREEN)*3/4 };
		pWindowPosition = &lWindowPosition;
	}
	m_WindowPosn = *pWindowPosition;
	
	// Start the thread
	m_pThread = new FrameWork::tThread<Window>( this );
}

// Destructor
Window::~Window( void )
{	if (m_hWnd) 
		::PostMessage( m_hWnd , WM_USER , (long)(size_t)this , 0 );
	if (m_pThread)
		delete m_pThread;
}

long Window::Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
{
	switch (message)
	{
		case WM_CLOSE:
			printf("\nYou cannot close the window here yet... sorry\n>");
			//for now ignore
			break;
		default:
			return DefWindowProc(window,message,w,l);
	}
	return 0;
}

long CALLBACK Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
{
	Window *BaseClass=(Window*)GetWindowLongPtr(window,GWLP_USERDATA);		
	long ret;
	if (BaseClass) 
	{
		// If we could get access to the canvas, then we will use it's callback to
		// get the result
		ret=BaseClass->Dispatcher(window,message,w,l);
	}
	else
	{
		// If there is no item, just a default callback
		ret=DefWindowProc(window,message,w,l);
	}
	return ret;
}

// Converts a GUID to a string
inline void GUIDtow(GUID id,wchar_t *string) {
	swprintf(string,L"%x-%x-%x-%x%x-%x%x%x%x%x%x",id.Data1,id.Data2,id.Data3,
		id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]);
}

// The thread callback
void Window::operator() ( const void* )
{	
	GUID UniqueClassID;
	if (FAILED(CoCreateGuid(&UniqueClassID))) 
		assert(false);
	wchar_t Unique_ClassName[256];
	GUIDtow(UniqueClassID,Unique_ClassName);

	DWORD PopupStyls=WS_OVERLAPPEDWINDOW;
	{
		WNDCLASS wc;
		memset(&wc,0,sizeof(WNDCLASS));
		wc.style=CS_HREDRAW|CS_VREDRAW;
		wc.lpfnWndProc=(WNDPROC)::Dispatcher;
		wc.lpszMenuName=m_pWindowName;
		wc.lpszClassName=Unique_ClassName;
		if (!RegisterClass(&wc))
			assert(false);
	}
	// Open a window
	m_hWnd = ::CreateWindowW(	Unique_ClassName , m_pWindowName , WS_VISIBLE | ( m_PopupWindow ? PopupStyls : WS_CHILD ), 
								m_WindowPosn.left , m_WindowPosn.top , 
								m_WindowPosn.right-m_WindowPosn.left , m_WindowPosn.bottom-m_WindowPosn.top , 
								m_Parent_hWnd , NULL , NULL , NULL );
	assert(m_hWnd);
	// Setup the window long values	
	SetWindowLongPtr(m_hWnd,GWLP_USERDATA, (LONG_PTR)this);

	{	//Fill w/ black
		HDC hdc=GetDC(m_hWnd);
		RECT rc;
		GetClientRect(m_hWnd,&rc);
		BitBlt(hdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,NULL,0,0,BLACKNESS);
		ReleaseDC(m_hWnd,hdc);
	}
	InitializeWindow();
	// The threaded message loop
	MSG msg;
	#if 0
	while(::GetMessage(&msg,NULL,0,0)!=0)
	{	// Is this a quit message
		if ( (msg.hwnd==m_hWnd) && 
			 (msg.message==WM_USER) &&
			 (msg.wParam==(long)(size_t)this) 
			 )
        {	break;
        }
        else
        {	TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
	}
	#else
	//Using PeekMessage allows us to not lockup if the window is a child on a separate process
	do
	{
		while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) > 0) //Or use an if statement
		{
			if ( (msg.hwnd==m_hWnd) && 	(msg.message==WM_USER) && (msg.wParam==(long)(size_t)this) )
				break;

			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		if ( (msg.hwnd==m_hWnd) && 	(msg.message==WM_USER) && (msg.wParam==(long)(size_t)this) )
			break;
		Sleep(100);
	} while (WM_QUIT != msg.message);
	#endif
	// Close the window
	::DestroyWindow( m_hWnd ); 
}