#pragma once

struct Window
{				// Constructor
				Window( HWND Parent=NULL , const bool IsPopup=true , 
						const wchar_t *pWindowName=L"Window" , const RECT *pWindowPosition=NULL );

				// Destructor
				~Window( void );

				// Cast to an HWND
				operator HWND( void );

				virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l);

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
				friend FrameWork::thread<Window>;

				// The thread
				FrameWork::thread<Window>	*m_pThread;
};