#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

static void DebugOutput(const char *format, ... )
{	char Temp[2048];
	va_list marker;
	va_start(marker,format);
	vsprintf(Temp,format,marker);
	OutputDebugStringA(Temp);
	va_end(marker);		
}

extern HMODULE g_hModule;
Dashboard_Controller_Interface *g_Controller=NULL;
HWND g_hDialogHWND=NULL;

class MyDlg
{
    public:
        // Constructor and Destructor for MyDlg.
        MyDlg();
        ~MyDlg(void);

		// Runs the Dialog.
        bool Run(HWND pParent);

    private:
        HWND        m_hDlg;                     // HWND of Dialog.

		//char		m_WindowTitle[128];

		//HANDLE		m_hDisplayTerminate;

		//void OnTimer(void);
		//void OnPaint(void);
		//void RePaint( HDC hDC );

		int DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
        static int CALLBACK BaseDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
        int OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam );
        void OnEndDialog(void);
		void MessagePump();
		bool m_IsClosing;
} *g_pMyDlg;

MyDlg::MyDlg() : m_IsClosing(false)
{
	//m_hDisplayTerminate = CreateEvent( NULL, true, false, NULL );
}

MyDlg::~MyDlg(void)
{
	//Do not use PostQuitMessage... as this will force app to exit early... instead just use a bool to stop the message pump
	//PostQuitMessage(0);
	m_IsClosing=true;
}

int MyDlg::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return(TRUE);
}

void MyDlg::OnEndDialog(void)
{
	//KillTimer( m_hDlg, 1 );
	g_pMyDlg=NULL;
	delete this;
}

int MyDlg::DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
		case WM_COMMAND: 
			{
				//nostaticpreview=nostaticxypreview=-1;
				WORD notifycode = HIWORD(wParam);
				WORD buttonid = LOWORD(wParam);
				if (notifycode==BN_CLICKED) 
				{
					//Handle our button up
					switch (buttonid) 
					{
					case IDC_PAUSE:
						g_Controller->Pause();
						break;
					case IDC_STOP:
						g_Controller->Stop();
						break;
					case IDC_PLAY:
						g_Controller->Run();
						break;
					case IDC_BROWSE:
						break;
					}
				}
			}
			break;
		case WM_INITDIALOG:
			OnInitDialog( uMsg, wParam, lParam );
			break;

		//case WM_PAINT:
		//	OnPaint();
		//	break;
		//case WM_TIMER:
		//	OnTimer();
		//	break;

		case WM_CLOSE:
			OnEndDialog();
			break;
		default:
			return FALSE;
	}
	return TRUE;
}


int CALLBACK MyDlg::BaseDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	MyDlg *pThis_;

	if (uMsg == WM_INITDIALOG)
	{   
		pThis_ = (MyDlg*) lParam;
		pThis_->m_hDlg = hDlg;
		SetWindowLongPtr( hDlg, GWL_USERDATA, lParam );
	}
	else
	   pThis_ = (MyDlg*) GetWindowLongPtr( hDlg, GWL_USERDATA );
	

	int Result_ = pThis_->DlgProc( uMsg, wParam, lParam );
	return(Result_);
}

void MyDlg::MessagePump()
{
	MSG Msg;
	do
	{
		while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if(!IsDialogMessage(g_hDialogHWND, &Msg))
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
		Sleep(100);
	} while ((WM_QUIT != Msg.message)&&(!m_IsClosing));
	// Close the window
	::DestroyWindow( g_hDialogHWND ); 
	g_hDialogHWND=NULL;
}

bool MyDlg::Run(HWND pParent)
{
	// Display the main dialog box.
	g_hDialogHWND= CreateDialogParam( g_hModule, MAKEINTRESOURCE(IDD_FILE_DIALOG), pParent, BaseDlgProc, (LPARAM) this );
	bool bResult_=  g_hDialogHWND!=NULL;
	if (bResult_)
	{
		wchar_t Buffer[128];
		GetWindowText(pParent,Buffer,128);
		std::wstring Name=L"File controls for ";
		Name+=Buffer;
		SetWindowText(g_hDialogHWND,Name.c_str());
		ShowWindow(g_hDialogHWND, SW_SHOW);
	}
	MessagePump();
	return (bResult_);
}



enum MenuSelection
{
	eMenu_NoSelection,
	eMenu_Controls,
	eMenu_NoEntries
};

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Initialize (Dashboard_Controller_Interface *controller)
{
	g_Controller=controller;
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
{
	//only populate this if we have a controller
	if (g_Controller)
	{
		InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
		InsertMenu(hPopupMenu, -1, (g_pMyDlg?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Controls+StartingOffset, L"File Controls...");
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection,HWND pParent)
{
	DebugOutput("Selection=%d\n",selection);
	if (!g_pMyDlg)
	{
		g_pMyDlg=new MyDlg;
		g_pMyDlg->Run(pParent);
	}
	else
	{
		DebugOutput("Controls Dialog already running\n");
		assert(false);
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Shutdown()
{
	delete g_pMyDlg;
	//While this is set to NULL implicitly, setting it here will get it NULL much quicker to avoid any racing condition
	if (g_pMyDlg)		//using if for debugging
		g_pMyDlg=NULL;
}
