#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"

extern DialogBase *g_pVisionControls;
extern Dashboard_Controller_Interface *g_Controller;

class VisionControls : public DialogBase
{
	public:
		VisionControls();
		~VisionControls();
		virtual bool Run(HWND pParent);
	protected:
		virtual size_t GetDialogResource() const {return IDD_VISION_DIALOG;}
		virtual LPARAM GetInstance() const {return (LPARAM) this;}
		virtual const wchar_t * const GetTitlePrefix() const  {return L"Vision controls for ";}
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
	private:
		std::string m_BLS_Filename; //keep note of the formed name for exit
};

DialogBase *CreateVisionControlsDialog() {return new VisionControls;}

const char * const csz_Filename="Vision_";
static void GetVisionFilename(HWND pParent,std::string &Output)
{
	Output=csz_Filename;
	wchar_t Buffer[128];
	GetWindowText(pParent,Buffer,128);
	{
		wchar2char(Buffer);
		Output+=wchar2char_pchar;
		Output+=".ini";
	}
}

  /***********************************************************************************************************************/
 /*														FileControls													*/
/***********************************************************************************************************************/


VisionControls::VisionControls() 
{
}

bool VisionControls::Run(HWND pParent)
{
	bool ret=__super::Run(pParent);
#if 0
	using namespace std;
	string InFile;
	GetVisionFilename(GetParent(m_hDlg),InFile);
	m_BLS_Filename=InFile;  //keep copy for exit
	std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		const size_t NoEnties=e_no_procamp_items;
		string StringEntry[e_no_procamp_items<<1];
		{
			char Buffer[1024];
			for (size_t i=0;i<NoEnties;i++)
			{
				in>>StringEntry[i<<1];
				in.getline(Buffer,1024);
				StringEntry[(i<<1)+1]=Buffer;
			}
		}
		in.close();
		for (size_t i=0;i<8;i++)
		{
			m_ProcAmpValues[i]=atof(StringEntry[(i<<1)+1].c_str());
		}
	}

	for (int i=0;i<e_no_procamp_items;i++)
	{
		UpdateSlider((ProcAmp_enum)i,true);
		UpdateText((ProcAmp_enum)i,true);
	}
#endif
	return ret;
}

VisionControls::~VisionControls()
{
	g_pVisionControls=NULL;
}

long VisionControls::Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	assert(g_Controller);

	switch(uMsg)
	{
		case WM_COMMAND: 
#if 0			
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
					}
				}
			}
#endif			
			break;
		case WM_HSCROLL:
#if 0			
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
#endif			
			break;
		case WM_DESTROY:
#if 0			
			{
				HWND hwndEdit=GetDlgItem(m_hDlg,IDC_FILENAME);
				//restore the original edit procedure call for this control
				SetWindowLongPtr(hwndEdit, GWL_WNDPROC, (LONG) m_wpOrigEditProc); 
			}

			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
#endif				
			break;
		default:
			return __super::Dispatcher(w_ptr,uMsg,wParam,lParam);
	}
	return TRUE;
}
