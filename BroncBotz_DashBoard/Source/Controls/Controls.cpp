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
} *g_pMyDlg;

MyDlg::MyDlg()
{
	//m_hDisplayTerminate = CreateEvent( NULL, true, false, NULL );
}

MyDlg::~MyDlg(void)
{
}

int MyDlg::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return(TRUE);
}

void MyDlg::OnEndDialog(void)
{
	g_pMyDlg=NULL;
	//KillTimer( m_hDlg, 1 );
	EndDialog( m_hDlg, TRUE );
}

int MyDlg::DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int Result_ = FALSE;
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
		Result_ = OnInitDialog( uMsg, wParam, lParam );
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
	}
	return(Result_);
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

bool MyDlg::Run(HWND pParent)
{
	// Display the main dialog box.
	bool bResult_ = (DialogBoxParam( g_hModule, MAKEINTRESOURCE(IDD_FILE_DIALOG), pParent, BaseDlgProc, (LPARAM) this ) != 0);
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

//Note: the dialog is currently modal, but I have treated the logic as if it is not... in case this needs to change
//for now I'll keep it modal

extern "C" CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
{
	//only populate this if we have a controller
	if (g_Controller)
	{
		InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
		InsertMenu(hPopupMenu, -1, (g_pMyDlg?MF_DISABLED:0) | MF_BYPOSITION | MF_STRING, eMenu_Controls+StartingOffset, L"File Controls...");
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
