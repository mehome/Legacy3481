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

class DialogBase
{
    public:
        DialogBase();
        virtual ~DialogBase(void);

		// Runs the Dialog.
        bool Run(HWND pParent);
		void OnEndDialog(void);

	protected:
		virtual size_t GetDialogResource() const =0;
		virtual LPARAM GetInstance() const =0;
		virtual const wchar_t * const GetTitlePrefix() const =0;
		virtual int DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    private:
        HWND        m_hDlg;                     // HWND of Dialog.
		
        static int CALLBACK BaseDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
        int OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam );
		void MessagePump();
		bool m_IsClosing;
};

class FileControls : public DialogBase
{
	protected:
		virtual size_t GetDialogResource() const {return IDD_FILE_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"File controls for ";}
		virtual int DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
} *g_pFileControls;


  /***********************************************************************************************************************/
 /*														DialogBase														*/
/***********************************************************************************************************************/


DialogBase::DialogBase() : m_IsClosing(false)
{
	DebugOutput("DialogBase() starting %p",this);
}

DialogBase::~DialogBase(void)
{
	//Note: Do not use PostQuitMessage... as this will force app to exit early... instead just use a bool to stop the message pump
	DebugOutput("~DialogBase() ending %p",this);
}

int DialogBase::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return(TRUE);
}

void DialogBase::OnEndDialog(void)
{
	m_IsClosing=true;
}

int DialogBase::DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		OnInitDialog( uMsg, wParam, lParam );
		break;

	case WM_CLOSE:
		OnEndDialog();
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

int CALLBACK DialogBase::BaseDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	DialogBase *pThis_;

	if (uMsg == WM_INITDIALOG)
	{   
		pThis_ = (DialogBase*) lParam;
		pThis_->m_hDlg = hDlg;
		SetWindowLongPtr( hDlg, GWL_USERDATA, lParam );
	}
	else
	   pThis_ = (DialogBase*) GetWindowLongPtr( hDlg, GWL_USERDATA );
	

	int Result_=FALSE;
	if (pThis_)
		Result_ = pThis_->DlgProc( uMsg, wParam, lParam );
	return(Result_);
}

void DialogBase::MessagePump()
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
	g_pFileControls=NULL;
	delete this;
}

bool DialogBase::Run(HWND pParent)
{
	// Display the main dialog box.
	g_hDialogHWND= CreateDialogParam( g_hModule, MAKEINTRESOURCE(GetDialogResource()), pParent, BaseDlgProc, GetInstance() );
	bool bResult_=  g_hDialogHWND!=NULL;
	if (bResult_)
	{
		wchar_t Buffer[128];
		GetWindowText(pParent,Buffer,128);
		std::wstring Name=GetTitlePrefix();
		Name+=Buffer;
		SetWindowText(g_hDialogHWND,Name.c_str());
		ShowWindow(g_hDialogHWND, SW_SHOW);
	}
	MessagePump();
	return (bResult_);
}


  /***********************************************************************************************************************/
 /*														FileControls													*/
/***********************************************************************************************************************/

int FileControls::DlgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
		case WM_COMMAND: 
			{
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
		default:
			return __super::DlgProc(uMsg,wParam,lParam);
	}
	return TRUE;
}


  /***********************************************************************************************************************/
 /*													C Global functions													*/
/***********************************************************************************************************************/


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
		InsertMenu(hPopupMenu, -1, (g_pFileControls?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Controls+StartingOffset, L"File Controls...");
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection,HWND pParent)
{
	switch (selection)
	{
		case eMenu_Controls:
			DebugOutput("Selection=%d\n",selection);
			if (!g_pFileControls)
			{
				g_pFileControls=new FileControls;
				g_pFileControls->Run(pParent);
			}
			else
			{
				DebugOutput("Controls Dialog already running\n");
				assert(false);
			}
			break;
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Shutdown()
{
	//Just signal... it will destroy itself from within the message pump
	if (g_pFileControls)
		g_pFileControls->OnEndDialog();
}
