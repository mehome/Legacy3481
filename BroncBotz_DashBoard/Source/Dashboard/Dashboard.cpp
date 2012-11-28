
#include "stdafx.h"
#include <ddraw.h>
#include <atlbase.h>
#include <shellapi.h>
#include "../FrameWork/FrameWork.h"
#include "../FrameWork/Window.h"
#include "../FrameWork/Preview.h"
#include "../ProcessingVision/ProcessingVision.h"
#include "../FFMpeg121125/FrameGrabber.h"
#pragma comment (lib,"shell32")
#pragma comment (lib,"../FFMpeg121125/lib/avcodec.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avdevice.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avfilter.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avformat.lib")
#pragma comment (lib , "../FFMpeg121125/lib/avutil.lib")
//#pragma comment (lib , "lib/postproc.lib")
#pragma comment (lib , "../FFMpeg121125/lib/swresample.lib")
#pragma comment (lib , "../FFMpeg121125/lib/swscale.lib")
#pragma comment (lib , "../FFMpeg121125/SDL/lib/x86/SDL.lib")
#pragma comment (lib , "../FFMpeg121125/SDL/lib/x86/SDLmain.lib")

// Converts a GUID to a string
inline void GUIDtow(GUID id,wchar_t *string) {
	wsprintfW(string,L"%x-%x-%x-%x%x-%x%x%x%x%x%x",id.Data1,id.Data2,id.Data3,
		id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]);
}

class ProcessingVision : public FrameWork::Outstream_Interface
{
	public:
		ProcessingVision(FrameWork::Outstream_Interface *Preview=NULL) : m_DriverProc(NULL),m_PlugIn(NULL),m_Outstream(Preview) {}
		~ProcessingVision()
		{
			FlushPlugin();
		}

		void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_Outstream=Preview;}
		void LoadPlugIn(const wchar_t Plugin[])
		{
			FlushPlugin();  //ensure its not already loaded
			m_PlugIn=LoadLibrary(Plugin);
			if (m_PlugIn)
			{
				m_DriverProc=(DriverProc_t) GetProcAddress(m_PlugIn,"ProcessFrame_RGB32");
				if (!m_DriverProc)
					FlushPlugin();
			}
		}
		void StartStreaming() {m_IsStreaming=true;}
		void StopStreaming() {m_IsStreaming=false;}

		virtual void process_frame(const FrameWork::Bitmaps::bitmap_bgra_u8 *pBuffer)
		{
			if (m_IsStreaming)
			{
				if (m_DriverProc)
				{
					using namespace FrameWork::Bitmaps;
					Bitmap_Frame frame((PBYTE)(*pBuffer)(),pBuffer->xres(),pBuffer->yres(),pBuffer->stride());
					Bitmap_Frame out_frame=*((*m_DriverProc)(&frame));
					bitmap_bgra_u8 dest((pixel_bgra_u8 *)out_frame.Memory,out_frame.XRes,out_frame.YRes,out_frame.Stride);
					m_Outstream->process_frame(&dest);
				}
				else
					m_Outstream->process_frame(pBuffer); //just passing through			
			}
		}
	private:
		typedef Bitmap_Frame * (*DriverProc_t)(Bitmap_Frame *Frame);
		DriverProc_t m_DriverProc;
		void FlushPlugin()
		{
			if (m_PlugIn)
			{
				FreeLibrary(m_PlugIn);
				m_PlugIn = NULL;
			}
		}

		HMODULE m_PlugIn;
		FrameWork::Outstream_Interface * m_Outstream; //I'm not checking for NULL so stream must be stopped while pointer is invalid
		bool m_IsStreaming;
};


const wchar_t * const cwsz_DefaultSmartFile=L"C:\\WindRiver\\WPILib\\SmartDashboard.jar";
const wchar_t * const cwsz_PlugInFile=L"ProcessingVision.dll";
const wchar_t * const cwsz_ClassName=L"SunAwtFrame";
const wchar_t * const cwsz_WindowName=L"SmartDashboard - ";

class DDraw_Preview 
{
	public:
		struct DDraw_Preview_Props
		{
			enum WindowType
			{
				eStandAlone,
				eSmartDashboard
			} window_type;
			std::wstring source_name;
			std::wstring IP_Address;
			std::wstring smart_file;			//startup this parent window app
			std::wstring ClassName,WindowName;  //find this window
			std::wstring plugin_file;
			std::wstring aux_startup_file,aux_startup_file_Args;	//startup an aux... (no window finding for this)
			LONG XRes,YRes,XPos,YPos;
			//provide an alternate way to determine window coordinates
			// -1 for x and y res will revert to hard coded defaults (this keeps tweaking inside the cpp file)
			void SetDefaults(LONG XRes_=-1,LONG YRes_=-1,float XPos_=0.5f,float YPos_=0.5f);
		};
		typedef DDraw_Preview_Props::WindowType WindowType;

		DDraw_Preview(const DDraw_Preview_Props &props);
		virtual ~DDraw_Preview();
		//returns true to quit
		bool CommandLineInterface();
		void RunApp();
		void SignalQuit() { m_Terminate.set(); }

		Preview *GetPreview() {return m_DD_StreamOut;}
		void Reset_DPC();  //This launches reset on a deferred procedure call
		void StopStreaming();
		void StartStreaming();
	protected:
		virtual void CloseResources();
		virtual void OpenResources();
	private:
		void Reset();
		void DisplayHelp();
		void SetDefaults(LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		void UpdateDefaultsFromWindowPlacement();
		FrameWork::event m_Terminate;
		FrameWork::thread m_thread;  //For DPC support

		HWND m_ParentHwnd;
		Window *m_Window;
		Preview *m_DD_StreamOut;

		DDraw_Preview_Props m_Props;
		RECT m_DefaultWindow;  //left=xRes top=yRes right=xPos bottom=YPos

		#if 0
		FrameGrabber_TestPattern m_FrameGrabber;
		#else
		FrameGrabber m_FrameGrabber;
		#endif

		ProcessingVision m_ProcessingVision;

		bool m_IsPopup_LastOpenedState;  //This is only written at the point when window is created
		bool m_IsStreaming;
};

using namespace std;

  /*******************************************************************************************************/
 /*											DDraw_Window												*/
/*******************************************************************************************************/

WINDOWPLACEMENT g_WindowInfo;
bool g_IsPopup=true;
bool g_IsSmartDashboardStarted=false;  //only run shell execute one time

void HighlightWindow( HWND hwnd, BOOL fDraw )
{
	HWND parent=GetParent(hwnd);
	assert(parent);
	if (!parent) return;
	const LONG DINV=3;
	HDC hdc;
	RECT rc;
	BOOL bBorderOn;
	bBorderOn = fDraw;

	if (hwnd == NULL || !IsWindow(hwnd))
		return;

	hdc = ::GetWindowDC(parent);
	::GetWindowRect(hwnd, &rc);
	//::OffsetRect(&rc, -rc.left, -rc.top);
	rc.left-=DINV;
	rc.top-=DINV;
	rc.right+=DINV<<1;
	rc.bottom+=DINV<<1;

	if (!IsRectEmpty(&rc))
	{
		PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, DINV,  DSTINVERT);
		PatBlt(hdc, rc.left, rc.bottom - DINV, DINV,
			-(rc.bottom - rc.top - 2 * DINV), DSTINVERT);
		PatBlt(hdc, rc.right - DINV, rc.top + DINV, DINV,
			rc.bottom - rc.top - 2 * DINV, DSTINVERT);
		PatBlt(hdc, rc.right, rc.bottom - DINV, -(rc.right - rc.left),
			DINV, DSTINVERT);
	}

	::ReleaseDC(parent, hdc);
} 

class DDraw_Window : public Window
{
	public:
		DDraw_Window(DDraw_Preview *pParent, HWND HWND_Parent=NULL , const bool IsPopup=true , 
			const wchar_t *pWindowName=L"Window" , const RECT *pWindowPosition=NULL ) : Window(HWND_Parent,IsPopup,pWindowName,pWindowPosition), 
			m_pParent(pParent),m_AspectRatio(4.0/3.0),m_Editable(false),m_IsDragging(false)
		{
		}
		~DDraw_Window()
		{
			//if (!m_PopupWindow)
			//	KillTimer(m_hWnd,1);
		}
	protected:
		enum MenuSelection
		{
			eMenu_NoSelection,
			eMenu_Floating=100,	//typically win32 starts these at 100  (not sure why, but it is probably optional)
			eMenu_Dockable,
			eMenu_Editable,
			eMenu_Lock4x3,
			eMenu_Lock16x9,
			eMenu_Stretch,
			eMenu_Show480i,
			eMenu_ShowHalfRes,
			eMenu_NoEntries
		};

		virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
		{
			long ret=0;
			switch (message)
			{
			case WM_LBUTTONDOWN:
				if (m_Editable)
				{
					WINDOWPLACEMENT preview_pos;
					GetWindowPlacement(*this,&preview_pos);
					struct tagPOINT mouseloc;
					GetCursorPos(&mouseloc);
					m_XGrab=preview_pos.rcNormalPosition.left-mouseloc.x;
					m_YGrab=preview_pos.rcNormalPosition.top-mouseloc.y;
					m_IsDragging=true;
					HighlightWindow(*this,false);
				}
				break;
			case WM_LBUTTONUP:
				if (m_IsDragging)
					HighlightWindow(*this,true);

				m_IsDragging=false;
				break;
			case WM_MOUSEMOVE:
				if (m_IsDragging)
				{
					struct tagPOINT mouseloc;
					GetCursorPos(&mouseloc);
					SetWindowPos(*this,NULL,mouseloc.x+m_XGrab,mouseloc.y+m_YGrab,0,0,SWP_NOSIZE|SWP_NOZORDER);
				}
				break;
			//This sizing was translated from here:
			//http://cboard.cprogramming.com/windows-programming/98998-uniform-resize-window.html
			//It works well...the only discrepancy is the the aspect ratio is height/width form (not the end of the world)
			//  [11/21/2012 James]
			case WM_SIZING:
				{
					double aspectRatio=(m_AspectRatio!=0.0)?1.0 / m_AspectRatio : 0.0;
					WPARAM &wParam=w;
					LPARAM &lParam=l;

					if (m_AspectRatio!=0.0)
					{
						RECT sz;    
						int w, h;
						// Copy over the new size that was determined by windows        
						memcpy(&sz, (const void *) lParam, sizeof(RECT));         
						// Calculate the width and height of the window        
						w = sz.right - sz.left;        
						h = sz.bottom - sz.top;         
						switch (wParam)        
						{            
						case WMSZ_LEFT:            
						case WMSZ_RIGHT:                
							// Modify the Height of the window                
							sz.bottom = LONG(w * aspectRatio) + sz.top;                
							break;             
						case WMSZ_TOP:            
						case WMSZ_BOTTOM:                
							// Modify the Width of the window                
							sz.right = LONG(h * (1 / aspectRatio)) + sz.left;                
							break;             
						case WMSZ_TOPRIGHT:            
						case WMSZ_TOPLEFT:            
						case WMSZ_BOTTOMRIGHT:            
						case WMSZ_BOTTOMLEFT:                
							// Adjust the width and height of the window to match aspect ratio                
							if (double(h) / double(w) > aspectRatio)
								w = int(double(h) / aspectRatio);                
							else
								h = int(double(w) * aspectRatio);

							// Adjust Height                
							if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)                
								sz.top = sz.bottom - h;                
							else
								sz.bottom = sz.top + h;

							// Adjust Width                
							if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
								sz.left = sz.right - w;
							else
								sz.right = sz.left + w;
							break;        
						}         
						// Copy back the size of the window        
						memcpy((void *) lParam, &sz, sizeof(RECT));
					}
				}
				break;
			case WM_RBUTTONDOWN:
				{
					HMENU hPopupMenu = CreatePopupMenu();
					if (!g_IsPopup)
						InsertMenu(hPopupMenu, -1, ( m_Editable?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Editable, L"Editable");
					InsertMenu(hPopupMenu, -1, ( g_IsPopup?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Floating, L"Floating");
					InsertMenu(hPopupMenu, -1, (!g_IsPopup?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Dockable, L"Dockable");
					if (g_IsPopup)
					{
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==4.0/3.0)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Lock4x3, L"Lock 4x3 aspect");
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==16.0/9.0)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Lock16x9, L"Lock 16x9 aspect");
						InsertMenu(hPopupMenu, -1, ((m_AspectRatio==0.0)?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Stretch, L"Stretch");
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_Show480i, L"Size 480i");
						InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_ShowHalfRes, L"Size half res");
					}
					SetForegroundWindow(window);

					//TODO omit this... I thought I needed to exclude from DDraw surface but it works fine
					//TPMPARAMS excludeRegion;
					//excludeRegion.cbSize=sizeof(TPMPARAMS);
					//GetWindowRect(window,&excludeRegion.rcExclude);
					//TODO omit this... If I want to keep it centered
					//UINT L_R_Alignment=GetSystemMetrics(SM_MENUDROPALIGNMENT)==0?TPM_LEFTALIGN:TPM_RIGHTALIGN;
					int XPos=0;
					int YPos=0;
					if (!g_IsPopup)
					{
						WINDOWPLACEMENT parent;
						GetWindowPlacement(GetParent(*this),&parent);
						XPos=parent.rcNormalPosition.left;
						YPos=parent.rcNormalPosition.top;
					}
					GetWindowPlacement(*this,&g_WindowInfo);
					XPos+=(g_WindowInfo.rcNormalPosition.left+g_WindowInfo.rcNormalPosition.right) >> 1;
					YPos+=(g_WindowInfo.rcNormalPosition.top+g_WindowInfo.rcNormalPosition.bottom) >> 1;
					MenuSelection selection=(MenuSelection)TrackPopupMenuEx(hPopupMenu, TPM_CENTERALIGN | TPM_RETURNCMD, XPos, YPos	, window, NULL);
					if (selection!=eMenu_NoSelection)
					{
						FrameWork::DebugOutput("Selection=%d\n",selection);
						switch (selection)
						{
							case eMenu_Editable:
								m_Editable=!m_Editable;
								HighlightWindow(*this,m_Editable);
								break;
							case eMenu_Floating:
								if (!g_IsPopup)		//only commit if it was changed
								{
									g_IsPopup=true;
									m_pParent->Reset_DPC();
								}
								break;
							case eMenu_Dockable:
								if (g_IsPopup)		//only commit if it was changed
								{
									g_IsPopup=false;
									m_pParent->Reset_DPC();
								}
								break;
							case eMenu_Lock4x3:
								m_AspectRatio=4.0/3.0;
								break;
							case eMenu_Lock16x9:
								m_AspectRatio=16.0/9.0;
								break;
							case eMenu_Stretch:
								m_AspectRatio=0.0;
								break;
							case eMenu_Show480i:
								if (m_AspectRatio==16.0/9.0)
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									SetWindowPos(*this,NULL,0,0,853,480,SWP_NOMOVE|SWP_NOZORDER);
								}
								else
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									SetWindowPos(*this,NULL,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER);
								}
								break;
							case eMenu_ShowHalfRes:
								if (m_AspectRatio==16.0/9.0)
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									SetWindowPos(*this,NULL,0,0,427,240,SWP_NOMOVE|SWP_NOZORDER);
								}
								else
								{
									RECT rc;
									GetWindowRect(*this,&rc);
									SetWindowPos(*this,NULL,0,0,320,240,SWP_NOMOVE|SWP_NOZORDER);
								}
								break;
						}
					}
				}
				break;
			case WM_CLOSE:
			case WM_DESTROY:
				m_pParent->SignalQuit();
				break;
			default:
				ret=DefWindowProc(window,message,w,l);
			}
			return ret;
		}
		virtual void InitializeWindow() 
		{
			//if (!m_PopupWindow)
			//	SetTimer(m_hWnd,1,1000,NULL);
		}

	private:
		DDraw_Preview * const m_pParent;
		LONG m_XGrab,m_YGrab;
		//if this is 0 it does not lock
		double m_AspectRatio;
		bool m_Editable,m_IsDragging;
};


using namespace FrameWork;

  /*******************************************************************************************************/
 /*									DDraw_Preview::DDraw_Preview_Props									*/
/*******************************************************************************************************/

void DDraw_Preview::DDraw_Preview_Props::SetDefaults(LONG XRes_,LONG YRes_,float XPos_,float YPos_)
{
	XRes=(XRes_==-1)?480:XRes_;
	YRes=(YRes_==-1)?270:YRes_;

	XPos=(LONG)(((float)(::GetSystemMetrics(SM_CXSCREEN)-XRes))*XPos_);
	YPos=(LONG)(((float)(::GetSystemMetrics(SM_CYSCREEN)-YRes))*YPos_);
}

  /*******************************************************************************************************/
 /*											DDraw_Preview												*/
/*******************************************************************************************************/


DDraw_Preview::DDraw_Preview(const DDraw_Preview_Props &props) : m_Window(NULL),m_ParentHwnd(NULL),m_DD_StreamOut(NULL),
	m_FrameGrabber(NULL,props.IP_Address.c_str()),m_ProcessingVision(NULL),m_IsStreaming(false)
{
	m_Props=props;
	SetDefaults(props.XRes,props.YRes,props.XPos,props.YPos);
}

void DDraw_Preview::UpdateDefaultsFromWindowPlacement()
{
	//If we still have our parent window or if we have always been a pop-up
	if ((m_ParentHwnd) || (g_IsPopup && m_IsPopup_LastOpenedState))
	{
		//we'll want to to set the defaults to the last set position
		LONG XPos=g_WindowInfo.rcNormalPosition.left;
		LONG YPos=g_WindowInfo.rcNormalPosition.top;
		LONG XRes=g_WindowInfo.rcNormalPosition.right - XPos;
		LONG YRes=g_WindowInfo.rcNormalPosition.bottom - YPos;
		//translate from child to pop-up or vise versa

		if (g_IsPopup!=m_IsPopup_LastOpenedState)
		{
			RECT Parent;
			GetWindowRect(m_ParentHwnd,&Parent);
			if (g_IsPopup)  //child to pop-up
				XPos+=Parent.left,YPos+=Parent.top;
			else
				XPos-=Parent.left,YPos-=Parent.top;
		}
		//Pedantic but we should keep this updated
		SetDefaults(XRes,YRes,XPos,YPos);
	}
}

void DDraw_Preview::StopStreaming()
{
	if (m_IsStreaming)
	{
		//before closing the resources ensure the upstream is not streaming to us
		m_FrameGrabber.StopStreaming();
		m_ProcessingVision.StopStreaming();
		m_FrameGrabber.SetOutstream_Interface(NULL);  //pedantic
		m_ProcessingVision.SetOutstream_Interface(NULL);
		m_IsStreaming=false;
	}
}

void DDraw_Preview::CloseResources()
{
	StopStreaming();
	delete m_DD_StreamOut;
	m_DD_StreamOut=NULL;
	if (m_Window)
	{
		GetWindowPlacement(*m_Window,&g_WindowInfo);
		UpdateDefaultsFromWindowPlacement();
		delete m_Window;
		m_Window=NULL;
	}
	if (m_ParentHwnd)
	{
		//TODO determine if popup can listen to WM_DESTROY from parent (and Nullify m_ParentHwnd)
		//or use timer (I think I may just leave this alone)
		#if 0
		if (g_IsPopup)
			PostMessage(m_ParentHwnd,WM_CLOSE,0,0);
		#endif
		m_ParentHwnd=NULL;
	}
}

DDraw_Preview::~DDraw_Preview()
{
	CloseResources();
}


void DDraw_Preview::StartStreaming()
{
	if (!m_IsStreaming && m_DD_StreamOut)
	{
		m_IsStreaming=true;
		#if 0
		m_FrameGrabber.SetOutstream_Interface(m_DD_StreamOut);
		#else
		m_ProcessingVision.SetOutstream_Interface(m_DD_StreamOut);
		m_FrameGrabber.SetOutstream_Interface(&m_ProcessingVision);
		#endif
		//Now to start the frame grabber
		m_FrameGrabber.StartStreaming();
		m_ProcessingVision.StartStreaming();
	}

}

void DDraw_Preview::OpenResources()
{
	typedef DDraw_Preview::DDraw_Preview_Props PrevProps;
	CloseResources(); //just ensure all resources are closed

	m_ProcessingVision.LoadPlugIn(m_Props.plugin_file.c_str());
	LONG XRes=m_DefaultWindow.left, YRes=m_DefaultWindow.top, XPos=m_DefaultWindow.right, YPos=m_DefaultWindow.bottom;
	const wchar_t *source_name=m_Props.source_name.c_str();

	HWND hWnd_ForDDraw=NULL;
	HWND ParentHwnd=NULL;
	bool IsPopup=g_IsPopup;
	bool IsSmartDashboardStarted=g_IsSmartDashboardStarted;
	if ((m_Props.smart_file.c_str()[0]!=0)&&(m_Props.window_type!=PrevProps::eStandAlone))
	{
		if (!g_IsSmartDashboardStarted)
		{
			LPTSTR szCmdline = _tcsdup(m_Props.smart_file.c_str());
			//Note:
			//The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications. It is not a true HINSTANCE, 
			//however. The only thing that can be done with the returned HINSTANCE is to cast it to an int and compare it with the value 32 or one 
			//of the error codes below.
			HINSTANCE test=ShellExecute(NULL,L"open",szCmdline,NULL,NULL,SW_SHOWNORMAL);
			IsSmartDashboardStarted=true;
		}
	}
	if ((m_Props.WindowName.c_str()[0]!=0)&&(m_Props.window_type!=PrevProps::eStandAlone))
	{
		//Give this some time to open
		size_t TimeOut=0;
		do 
		{
			Sleep(100);
			//SmartDashboard - /SunAwtFrame
			//ParentHwnd=FindWindow(L"SunAwtFrame",L"SmartDashboard - ");
			ParentHwnd=FindWindow(m_Props.ClassName.c_str(),m_Props.WindowName.c_str());
		} while ((ParentHwnd==NULL)&&(TimeOut++<50)); //This may take a while on cold start
		m_ParentHwnd=ParentHwnd;
	}

	//If we don't have a parent window then we must be a Popup
	if (!ParentHwnd)
		g_IsPopup=IsPopup=true;

	{
		assert (!m_Window);
		//apparently there is a racing condition where the window needs a thread to set its handle
		size_t TimeOut=0;
		LONG X=XPos;
		LONG Y=YPos;
		RECT lWindowPosition = {  X,Y,X+XRes,Y+YRes};
		//Note: A child option can only be presented if we have the parent window
		assert(IsPopup || ParentHwnd);
		m_IsPopup_LastOpenedState=IsPopup;  //this is the only place this is written
		m_Window=new DDraw_Window(this,ParentHwnd,IsPopup,source_name,&lWindowPosition);
		while ((!(HWND)*m_Window)&&(TimeOut++<100))
			Sleep(10);
		assert((HWND)*m_Window);
		hWnd_ForDDraw=(HWND)*m_Window;
	}
	
	if (hWnd_ForDDraw)
	{
		assert (!m_DD_StreamOut);
		m_DD_StreamOut=new Preview(hWnd_ForDDraw);
		if (!m_DD_StreamOut->Get_IsError())
		{
			m_DD_StreamOut->StartStreaming();
		}
		else
			printf("DDraw_Preview::OpenResources Error detected in setting up DDraw environment \n");
	}
	else
		printf("No HWnd for DDraw\n");
	StartStreaming();
	//see if there is another file to launch
	if ((!g_IsSmartDashboardStarted)&& (m_Props.aux_startup_file.c_str()[0]!=0))
	{
		const wchar_t *Args=m_Props.aux_startup_file_Args.c_str();
		if (Args[0]==0)
			Args=NULL;
		HINSTANCE test=ShellExecute(NULL,L"open",m_Props.aux_startup_file.c_str(),Args,NULL,SW_SHOWNORMAL);
		DebugOutput("Launching %ls, result=%x",m_Props.aux_startup_file.c_str(),test);
	}

	g_IsSmartDashboardStarted=IsSmartDashboardStarted;
}

void DDraw_Preview::Reset()
{
	CloseResources();
	OpenResources();
}

void DDraw_Preview::Reset_DPC()
{
	//This has to be a deferred procedure call because the primitive callback must complete!
	using namespace FrameWork;
	cpp::threadcall_ex( do_not_wait, m_thread, this, &DDraw_Preview::Reset);
}

void DDraw_Preview::SetDefaults(LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
	//Note: we use the rect as such
	//left=xRes top=yRes right=xPos bottom=YPos
	m_DefaultWindow.left =XRes,
	m_DefaultWindow.top=YRes,
	m_DefaultWindow.right=XPos,
	m_DefaultWindow.bottom=YPos;
}

void DDraw_Preview::RunApp()
{
	OpenResources();
	m_Terminate.wait();
	CloseResources();
}

size_t split_arguments(const std::string& str, std::vector<std::string>& arguments)
{
	arguments.clear();

	if (str.empty())
		return 0;

	const std::string whitespace = " \t\n\r";
	const char group_char = '"';
	bool in_argument = false;

	arguments.push_back(std::string());
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		if (*it == group_char)
			in_argument = !in_argument;
		else if (in_argument || whitespace.find(*it) == std::string::npos)
			arguments.back().push_back(*it);
		else if (!arguments.back().empty())
			arguments.push_back(std::string());
	}

	if (arguments.back().empty())
		arguments.pop_back();

	assert(!in_argument); // Uneven quotes?

	return arguments.size();
}

void AssignInput(wstring &output,const char *input)
{
	std::vector<std::string> Args;
	split_arguments(std::string(input),Args);
	//just use the first argument found
	char2wchar(Args[0].c_str());
	output=char2wchar_pwchar;
	if (wcscmp(output.c_str(),L"none")==0)
		output=L"";
}

void AssignOutput(string &output,const wchar_t *input)
{
	wchar2char(input);
	output=wchar2char_pchar;
	if (output[0]==0)
		output="none";
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	DDraw_Preview::DDraw_Preview_Props props;
	wstring SmartDashboard=cwsz_DefaultSmartFile;
	wstring Title=L"Preview";
	wstring Plugin=cwsz_PlugInFile;
	wstring AuxStart=L"none";
	wstring AuxArgs=L"none";
	wstring ClassName=cwsz_ClassName;
	wstring WindowName=cwsz_WindowName;
	wstring IP_Address=L"none";

	string sz_FileName="Video1.ini";
	wchar_t *ext=wcsrchr(lpCmdLine,L'.');
	if ((ext)&&(wcsicmp(ext,L".ini")==0))
	{
		wchar2char(lpCmdLine);
		sz_FileName=wchar2char_pchar;
	}
	{
		string InFile = sz_FileName.c_str();
		std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			const size_t NoEnties=13;
			string StringEntry[NoEnties<<1];
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
			AssignInput(Title,StringEntry[1].c_str());
			AssignInput(IP_Address,StringEntry[3].c_str());
			int left=atoi(StringEntry[5].c_str());
			int top=atoi(StringEntry[7].c_str());
			int right=atoi(StringEntry[9].c_str());
			int bottom=atoi(StringEntry[11].c_str());
			props.XRes=right-left;
			props.YRes=bottom-top;
			props.XPos=left;
			props.YPos=top;
			AssignInput(SmartDashboard,StringEntry[13].c_str());
			AssignInput(ClassName,StringEntry[15].c_str());
			AssignInput(WindowName,StringEntry[17].c_str());
			g_IsPopup=atoi(StringEntry[19].c_str())==0?false:true;
			AssignInput(Plugin,StringEntry[21].c_str());
			AssignInput(AuxStart,StringEntry[23].c_str());
			AssignInput(AuxArgs,StringEntry[25].c_str());
		}
		else
		{
			props.XRes=320;
			props.YRes=240;
			props.XPos=20;
			props.YPos=10;
		}
	}

	#if 1
	props.window_type=DDraw_Preview::DDraw_Preview_Props::eSmartDashboard;
	#else
	props.window_type=DDraw_Preview::DDraw_Preview_Props::eStandAlone;
	#endif

	props.source_name=Title;
	props.IP_Address=IP_Address;
	props.smart_file=SmartDashboard;
	props.ClassName=ClassName;
	props.WindowName=WindowName;
	props.plugin_file=Plugin;
	props.aux_startup_file=AuxStart;
	props.aux_startup_file_Args=AuxArgs;

	DDraw_Preview TheApp(props);

	TheApp.RunApp();
	
	{
		string OutFile = sz_FileName.c_str();
		string output;

		ofstream out(OutFile.c_str(), std::ios::out );

		AssignOutput(output,Title.c_str());
		out << "title= " << output << endl;
		AssignOutput(output,IP_Address.c_str());
		out << "IP_Address= " << output << endl;

		out << "left= " << g_WindowInfo.rcNormalPosition.left << endl;
		out << "top= "  << g_WindowInfo.rcNormalPosition.top << endl;
		out << "right= " << g_WindowInfo.rcNormalPosition.right << endl;
		out << "bottom= "  << g_WindowInfo.rcNormalPosition.bottom << endl;

		AssignOutput(output,SmartDashboard.c_str());
		out << "SmartDashboard= " << output << endl;
		AssignOutput(output,ClassName.c_str());
		out << "ClassName= " << output << endl;
		AssignOutput(output,WindowName.c_str());
		out << "WindowName= " << "\"" << output << "\"" << endl;
		out << "IsPopup= " << g_IsPopup << endl;
		AssignOutput(output,Plugin.c_str());
		out << "PlugIn= " << output << endl;
		AssignOutput(output,AuxStart.c_str());
		out << "AuxStartupFile= " << output << endl;
		AssignOutput(output,AuxArgs.c_str());
		out << "AuxStartupFileArgs= " << output << endl;
		out.close();
	}
	return 0;
}

