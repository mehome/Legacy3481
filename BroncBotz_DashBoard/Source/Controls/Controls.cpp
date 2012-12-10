#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

void DebugOutput(const char *format, ... )
{	char Temp[2048];
	va_list marker;
	va_start(marker,format);
	vsprintf(Temp,format,marker);
	OutputDebugStringA(Temp);
	va_end(marker);		
}

extern HMODULE g_hModule;
Dashboard_Controller_Interface *g_Controller=NULL;
DLGPROC g_WinProc;

DialogBase *g_pProcamp;
DialogBase *CreateProcampDialog();
void ProcAmp_Initialize();

class FileControls : public DialogBase
{
	public:
		FileControls();
		~FileControls();
	protected:
		virtual size_t GetDialogResource() const {return IDD_FILE_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"File controls for ";}
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	private:
		int m_ScrubValue;
} *g_pFileControls;


  /***********************************************************************************************************************/
 /*														DialogBase														*/
/***********************************************************************************************************************/


DialogBase::DialogBase() : m_IsClosing(false)
{
	//DebugOutput("DialogBase() starting %p\n",this);
}

DialogBase::~DialogBase(void)
{
	//DebugOutput("~DialogBase() ending %p\n",this);
}

int DialogBase::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return(TRUE);
}

void DialogBase::OnEndDialog(void)
{
	if (!m_IsClosing)
	{
		m_IsClosing=true;
		::DestroyWindow( m_hDlg ); 
		m_hDlg=NULL;
		delete this;
	}
}

long DialogBase::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		OnInitDialog( uMsg, wParam, lParam );
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		OnEndDialog();
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

bool DialogBase::Run(HWND pParent)
{
	// Display the main dialog box.
	m_hDlg= CreateDialog( g_hModule, MAKEINTRESOURCE(GetDialogResource()), pParent, g_WinProc );
	SetWindowLongPtr(m_hDlg,GWLP_USERDATA, (LONG_PTR)GetInstance());

	bool bResult_=  m_hDlg!=NULL;
	if (bResult_)
	{
		wchar_t Buffer[128];
		GetWindowText(pParent,Buffer,128);
		std::wstring Name=GetTitlePrefix();
		Name+=Buffer;
		SetWindowText(m_hDlg,Name.c_str());
		ShowWindow(m_hDlg, SW_SHOW);
	}
	return (bResult_);
}


//Agghhh this is not perfect but gets the job done... we have to keep track of our own adjustments since the GetScrollInfo() is not working
//  [12/7/2012 James]
static void ScrollAdjust(WPARAM wParam,SCROLLINFO &si,int numrows=20)
{
	switch (LOWORD(wParam)) 
	{
		case SB_LINEDOWN: //Scrolls one line down. 
			//if (si.nPos+(numrows-1)<si.nMax) si.nPos+=1;
			if (si.nPos<si.nMax) si.nPos+=1;
			break;
		case SB_LINEUP: //Scrolls one line up. 
			if (si.nPos>0) si.nPos-=1;
			break;
		case SB_PAGEDOWN: //Scrolls one page down. 
			//if (numrows+si.nPos<si.nMax-(numrows-1))
			//	si.nPos+=numrows;
			//else
			//	si.nPos=si.nMax-(numrows-1);
			si.nPos+=numrows;
			if (si.nPos>si.nMax)
				si.nPos=si.nMax;
			break;
		case SB_PAGEUP: //Scrolls one page up. 
			si.nPos-=numrows;
			if (si.nPos<si.nMin)
				si.nPos=si.nMin;
			break;
		case SB_THUMBTRACK: 
			si.nPos=HIWORD(wParam);
			break;
	} //End switch
}


  /***********************************************************************************************************************/
 /*														FileControls													*/
/***********************************************************************************************************************/


FileControls::FileControls() : m_ScrubValue(0)
{
}

FileControls::~FileControls()
{
	g_pFileControls=NULL;
}

long FileControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
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
					case IDC_SCRUB:
						DebugOutput("Inside Scrub\n");
						break;
					}
				}
			}
			break;
		case WM_HSCROLL:
			{
				HWND hWndScroller=(HWND)lParam;
				SCROLLINFO si;
				ZeroMemory(&si,sizeof(SCROLLINFO));
				si.nMax=100;

				if (hWndScroller==GetDlgItem(m_hDlg, IDC_SCRUB))
				{
					si.nPos=m_ScrubValue;
					ScrollAdjust(wParam,si);
					m_ScrubValue=si.nPos;
					DebugOutput("Position= %d\n",SendMessage(hWndScroller,TBM_GETPOS,0,0));
				}
			}
			break;
		default:
			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
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
	eMenu_Procamp,
	eMenu_NoEntries
};

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Initialize (Dashboard_Controller_Interface *controller,DLGPROC gWinProc)
{
	g_Controller=controller;
	g_WinProc=gWinProc;
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_StartedStreaming()
{
	ProcAmp_Initialize();
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
{
	//only populate this if we have a controller
	if (g_Controller)
	{
		InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
		InsertMenu(hPopupMenu, -1, (g_pFileControls?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Controls+StartingOffset, L"File Controls...");
		InsertMenu(hPopupMenu, -1, (g_pProcamp?MF_DISABLED|MF_GRAYED:0) | MF_BYPOSITION | MF_STRING, eMenu_Procamp+StartingOffset, L"Procamp...");
	}
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection,HWND pParent)
{
	DebugOutput("Selection=%d\n",selection);
	switch (selection)
	{
		case eMenu_Controls:
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
		case eMenu_Procamp:
			if (!g_pProcamp)
			{
				g_pProcamp=CreateProcampDialog();
				g_pProcamp->Run(pParent);
			}
			else
			{
				DebugOutput("Procamp Dialog already running\n");
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
	if (g_pProcamp)
		g_pProcamp->OnEndDialog();
}
