#pragma once
class MessageBase 
{
public:
	virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
};

struct Window : public MessageBase
{				// Constructor
				Window( HWND Parent=NULL , const bool IsPopup=true , 
						const wchar_t *pWindowName=L"Window" , const RECT *pWindowPosition=NULL );

				// Destructor
				~Window( void );

				// Cast to an HWND
				operator HWND( void );

				virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l);
				DLGPROC GetDispatcherBase();
protected:
				virtual void InitializeWindow() {}  //override to do special customization to window once its created
				HWND	m_hWnd;
				HWND	m_Parent_hWnd;
				RECT	m_WindowPosn;
				bool	m_PopupWindow;
				const wchar_t *m_pWindowName;

private:		// The child window pointer
				// The thread callback
				void operator() ( const void* );

				// A friend
				friend FrameWork::Threads::thread<Window>;

				// The thread
				FrameWork::Threads::thread<Window>	*m_pThread;
				bool m_IsClosing;
};