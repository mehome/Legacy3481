#include "stdafx.h"
#include <CommDlg.h>
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
DialogBase *g_pFileControls;

DialogBase *CreateProcampDialog();
DialogBase *CreateFileControlsDialog();

void ProcAmp_Initialize(HWND pParent);

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

BOOL DialogBase::getopenfile(wchar_t *dest,wchar_t *filename,const wchar_t *defpath,const wchar_t *defext,const wchar_t *inputprompt,const wchar_t *filter,BOOL musthave) 
{
	OPENFILENAME ofn;
	BOOL bResult;

	//Clear out and fill in an OPENFILENAME structure in preparation
	//for creating a common dialog box to open a file.
	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	= sizeof(OPENFILENAME);
	ofn.hwndOwner	= m_hDlg;
	ofn.hInstance	= NULL;  //ignored
	ofn.lpstrFilter	= filter;
	ofn.nFilterIndex	= 0;
	dest[0]	= '\0';
	ofn.lpstrFile	= dest;
	ofn.nMaxFile	= MAX_PATH;
	ofn.lpstrFileTitle = filename;
	ofn.nMaxFileTitle	= MAX_PATH;
	ofn.lpstrInitialDir = defpath;
	ofn.lpstrDefExt	= defext;
	ofn.lpstrTitle	= inputprompt;
	if (musthave) ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	else ofn.Flags=OFN_HIDEREADONLY;

	return (bResult=GetOpenFileName(&ofn));
}


bool DialogBase::getopenfilename(const wchar_t *inputprompt,std::wstring &Output,BOOL musthave,const wchar_t *defaultPath,const wchar_t *defaultExt) 
{
	wchar_t PathBuffer[MAX_PATH]; //full path
	wchar_t FileBuffer[MAX_PATH];
	//No filters
	BOOL bResult=getopenfile(PathBuffer,FileBuffer,defaultPath,defaultExt,inputprompt,NULL,musthave);
	//DebugOutput("%ls name=%ls",PathBuffer,FileBuffer);
	Output=PathBuffer;
	return bResult!=0;
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

extern "C" CONTROLS_API void Callback_SmartCppDashboard_StartedStreaming(HWND pParent)
{
	if (g_Controller)
		ProcAmp_Initialize(pParent);
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
				g_pFileControls=CreateFileControlsDialog();
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
