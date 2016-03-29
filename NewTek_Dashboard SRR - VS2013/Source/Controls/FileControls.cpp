#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

extern DialogBase *g_pFileControls;
extern Dashboard_Controller_Interface *g_Controller;

  /***********************************************************************************************************************/
 /*														FileControls													*/
/***********************************************************************************************************************/


class FileControls : public DialogBase
{
	public:
		FileControls();
		~FileControls();
		virtual bool Run(HWND pParent);
		//only to be called from the dispatcher
		long Edit_Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	protected:
		virtual size_t GetDialogResource() const {return IDD_FILE_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"File controls for ";}
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	private:
		WNDPROC m_wpOrigEditProc;
		int m_ScrubValue;
};

DialogBase *CreateFileControlsDialog() {return new FileControls;}




FileControls::FileControls() : m_ScrubValue(0)
{
}

long FileControls::Edit_Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CHAR:
			//DebugOutput("WM_CHAR=%d\n",wParam);
			if (wParam==13)
			{
				//We've entered a filename kill focus by setting it to parent window
				SetFocus(GetParent(w_ptr));
				wchar_t Buffer[MAX_PATH<<1];
				GetWindowText(w_ptr,Buffer,MAX_PATH<<1);
				DebugOutput("Switch to=%ls\n",Buffer);
				assert(g_Controller);
				g_Controller->SwitchFilename(Buffer);
			}

			break;
	}
	return CallWindowProc(m_wpOrigEditProc, w_ptr, uMsg, wParam, lParam); 
}

LRESULT APIENTRY EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	//note: this should ALWAYS work
	FileControls *BaseClass=(FileControls*)GetWindowLongPtr(hwnd,GWLP_USERDATA);		
	long ret=TRUE;
	if (BaseClass) 
		ret=BaseClass->Edit_Dispatcher(hwnd,uMsg,wParam,lParam);
	else
		assert(false);
	return ret;
} 

bool FileControls::Run(HWND pParent)
{
	bool ret=__super::Run(pParent);
	//http://msdn.microsoft.com/en-us/library/ms633570%28VS.85%29.aspx#subclassing_window
	//In this section we want to retrieve when the carriage return key is pressed within our edit control
	//Retrieve handle to edit control
	HWND hwndEdit=GetDlgItem(m_hDlg,IDC_FILENAME);
	//subclass the edit control
	m_wpOrigEditProc=(WNDPROC) SetWindowLongPtr(hwndEdit,GWL_WNDPROC,(LONG)EditSubclassProc);
	SetWindowLongPtr(hwndEdit,GWLP_USERDATA, (LONG_PTR)this);
	//One more thing to do with the edit control... set it to its current file
	std::wstring FileName;
	assert(g_Controller);
	g_Controller->GetFileName(FileName);
	SetWindowText(hwndEdit,FileName.c_str());
	SendDlgItemMessage(m_hDlg, IDC_RECORD, BM_SETCHECK, g_Controller->GetRecordState(), 0);
	return ret;
}

FileControls::~FileControls()
{
	g_pFileControls=NULL;
}

long FileControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	assert(g_Controller);

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
						{
							std::wstring Output;
							if (getopenfilename(L"Choose file for playback",Output,true))
							{
								HWND hwndEdit=GetDlgItem(m_hDlg,IDC_FILENAME);
								SetWindowText(hwndEdit,Output.c_str());
								DebugOutput("Switch to=%ls\n",Output.c_str());
								g_Controller->SwitchFilename(Output.c_str());
							}
						}
						break;
					case IDC_RECORD:
						bool bChecked = SendDlgItemMessage(m_hDlg, IDC_RECORD, BM_GETCHECK, 0, 0) > 0;
						g_Controller->Record(bChecked);
					}
				}
			}
			break;
		case WM_HSCROLL:
			{
				HWND hWndScroller=(HWND)lParam;
				if (hWndScroller==GetDlgItem(m_hDlg, IDC_SCRUB))
				{
					int position=SendMessage(hWndScroller,TBM_GETPOS,0,0);
					//DebugOutput("Position= %d\n",position);
					g_Controller->Seek((double)position/100.0);
					break;
				}
			}
			break;
		case WM_DESTROY:
			{
				HWND hwndEdit=GetDlgItem(m_hDlg,IDC_FILENAME);
				//restore the original edit procedure call for this control
				SetWindowLongPtr(hwndEdit, GWL_WNDPROC, (LONG) m_wpOrigEditProc); 
			}

			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
			break;
		default:
			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
	}
	return TRUE;
}

