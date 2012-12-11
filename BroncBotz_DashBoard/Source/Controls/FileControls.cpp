#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

extern DialogBase *g_pFileControls;
extern Dashboard_Controller_Interface *g_Controller;

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
};

DialogBase *CreateFileControlsDialog() {return new FileControls;}


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
				if (hWndScroller==GetDlgItem(m_hDlg, IDC_SCRUB))
				{
					int position=SendMessage(hWndScroller,TBM_GETPOS,0,0);
					DebugOutput("Position= %d\n",position);
					break;
				}
			}
			break;
		default:
			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
	}
	return TRUE;
}

