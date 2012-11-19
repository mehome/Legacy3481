
#include "stdafx.h"
#include <ddraw.h>
#include <atlbase.h>
#include <shellapi.h>
#include "../FrameWork/FrameWork.h"
#include "../FrameWork/Window.h"
#include "../FrameWork/Preview.h"
#pragma comment (lib,"shell32")


// Converts a GUID to a string
inline void GUIDtow(GUID id,wchar_t *string) {
	wsprintfW(string,L"%x-%x-%x-%x%x-%x%x%x%x%x%x",id.Data1,id.Data2,id.Data3,
		id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]);
}

  /*******************************************************************************************************/
 /*										FrameGrabber_TestPattern										*/
/*******************************************************************************************************/

//Throw together the infamous test pattern that streams the frames out
class FrameGrabber_TestPattern
{
public:
	FrameGrabber_TestPattern(FrameWork::Outstream_Interface *Preview=NULL) : m_pThread(NULL),m_TestMap(720,480),m_Outstream(Preview)
	{
	}
	//allow late binding of the output (hence start streaming exists for this delay)
	void SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) {m_Outstream=Preview;}
	void StartStreaming()
	{
		m_Counter=0;
		m_pThread = new FrameWork::tThread<FrameGrabber_TestPattern>(this);
	}

	void StopStreaming()
	{
		delete m_pThread;
		m_pThread=NULL;
	}

	virtual ~FrameGrabber_TestPattern()
	{
		StopStreaming();
	}
private:
	friend FrameWork::tThread<FrameGrabber_TestPattern>;

	void operator() ( const void* )
	{
		using namespace FrameWork;
		Sleep(16);
		//Sleep(33);
		//Sleep(1000);
		DrawField( (PBYTE) m_TestMap(),m_TestMap.xres(),m_TestMap.yres(),m_Counter++ );
		m_Outstream->process_frame(&m_TestMap);
		//printf("%d\n",m_Counter++);
	}
	FrameWork::tThread<FrameGrabber_TestPattern> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

private:
	FrameWork::Bitmaps::bitmap_ycbcr_u8 m_TestMap;
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	size_t m_Counter;
};

const wchar_t * const cwsz_DefaultSmartFile=L"C:\\WindRiver\\WPILib\\SmartDashboard.jar";

class DDraw_Preview 
{
	public:
		enum WindowType
		{
			eStandAlone,
			eSmartDashboard
		};

		void Init(WindowType type);
		DDraw_Preview(WindowType type=eStandAlone, const wchar_t source_name[] = L"Preview",const wchar_t *smart_file=cwsz_DefaultSmartFile,LONG XRes=-1,LONG YRes=-1,float XPos=0.5f,float YPos=0.5f);
		DDraw_Preview(WindowType type, const wchar_t source_name[],const wchar_t *smart_file,LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		virtual ~DDraw_Preview();
		//returns true to quit
		bool CommandLineInterface();
		void RunApp();
		void SignalQuit() { m_Terminate.set(); }

		Preview *GetPreview() {return m_DD_StreamOut;}
		void Reset_DPC();  //This launches reset on a deferred procedure call
	protected:
		virtual void CloseResources();
		virtual void OpenResources();
	private:
		void Reset();
		void DisplayHelp();
		// -1 for x and y res will revert to hard coded defaults (this keeps tweaking inside the cpp file)
		void SetDefaults(const wchar_t source_name[] = L"Preview",LONG XRes=-1,LONG YRes=-1,float XPos=0.5f,float YPos=0.5f);
		void SetDefaults(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		FrameWork::event m_Terminate;
		FrameWork::thread m_thread;  //For DPC support

		HWND m_ParentHwnd;
		Window *m_Window;
		Preview *m_DD_StreamOut;

		std::wstring m_PreviewName,m_SmartDashBoard_FileName;
		RECT m_DefaultWindow;  //left=xRes top=yRes right=xPos bottom=YPos

		FrameGrabber_TestPattern m_FrameGrabber;

		WindowType m_WindowType;
		bool m_IsPopup_LastOpenedState;  //This is only written at the point when window is created
};

using namespace std;

  /*******************************************************************************************************/
 /*											DDraw_Window												*/
/*******************************************************************************************************/

WINDOWPLACEMENT g_WindowInfo;
bool g_IsPopup=true;
bool g_IsSmartDashboardStarted=false;  //only run shell execute one time

class DDraw_Window : public Window
{
	public:
		DDraw_Window(DDraw_Preview *pParent, HWND HWND_Parent=NULL , const bool IsPopup=true , 
			const wchar_t *pWindowName=L"Window" , const RECT *pWindowPosition=NULL ) : Window(HWND_Parent,IsPopup,pWindowName,pWindowPosition), 
			m_pParent(pParent)
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
			eMenu_NoEntries
		};

		virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
		{
			long ret=0;
			switch (message)
			{
			case WM_RBUTTONDOWN:
				{
					HMENU hPopupMenu = CreatePopupMenu();
					InsertMenu(hPopupMenu, 0, ( g_IsPopup?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Floating, L"Floating");
					InsertMenu(hPopupMenu, 0, (!g_IsPopup?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Dockable, L"Dockable");
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
};

  /*******************************************************************************************************/
 /*											DDraw_Preview												*/
/*******************************************************************************************************/

void DDraw_Preview::Init(WindowType type)
{
	m_WindowType=type;

	m_Window=NULL;
	m_ParentHwnd=NULL;
	m_DD_StreamOut=NULL;
	m_FrameGrabber=NULL;
}

DDraw_Preview::DDraw_Preview(WindowType type,const wchar_t source_name[],const wchar_t *smart_file,LONG XRes,LONG YRes,float XPos,float YPos)
{
	m_SmartDashBoard_FileName=smart_file;
	SetDefaults(source_name,XRes,YRes,XPos,YPos);
	Init(type);
}

DDraw_Preview::DDraw_Preview(WindowType type,const wchar_t source_name[],const wchar_t *smart_file,LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
	m_SmartDashBoard_FileName=smart_file;
	SetDefaults(source_name,XRes,YRes,XPos,YPos);
	Init(type);
}

void DDraw_Preview::CloseResources()
{
	//before closing the resources ensure the upstream is not streaming to us
	m_FrameGrabber.StopStreaming();
	m_FrameGrabber.SetOutstream_Interface(NULL);  //pedantic
	delete m_DD_StreamOut;
	m_DD_StreamOut=NULL;
	if (m_Window)
	{
		GetWindowPlacement(*m_Window,&g_WindowInfo);
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


void DDraw_Preview::OpenResources()
{
	CloseResources(); //just ensure all resources are closed

	LONG XRes=m_DefaultWindow.left, YRes=m_DefaultWindow.top, XPos=m_DefaultWindow.right, YPos=m_DefaultWindow.bottom;
	const wchar_t *source_name=m_PreviewName.c_str();

	enum WindowType
	{
		eStandAlone,
		eSmartDashboard
	};


	HWND hWnd_ForDDraw=NULL;
	HWND ParentHwnd=NULL;
	bool IsPopup=g_IsPopup;
	bool IsSmartDashboardStarted=g_IsSmartDashboardStarted;
	if (m_WindowType==eSmartDashboard)
	{
		if (!g_IsSmartDashboardStarted)
		{
			LPTSTR szCmdline = _tcsdup(m_SmartDashBoard_FileName.c_str());
			//Note:
			//The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications. It is not a true HINSTANCE, 
			//however. The only thing that can be done with the returned HINSTANCE is to cast it to an int and compare it with the value 32 or one 
			//of the error codes below.
			HINSTANCE test=ShellExecute(NULL,L"open",szCmdline,NULL,NULL,SW_SHOWNORMAL);
			IsSmartDashboardStarted=true;
		}
		//Give this some time to open
		size_t TimeOut=0;
		do 
		{
			Sleep(100);
			//theAwtToolkitWindow
			//SmartDashboard - /SunAwtFrame
			//ParentHwnd=FindWindow(L"SunAwtToolkit",L"theAwtToolkitWindow");
			ParentHwnd=FindWindow(L"SunAwtFrame",L"SmartDashboard - ");
		} while ((ParentHwnd==NULL)&&(TimeOut++<50)); //This may take a while on cold start
		m_ParentHwnd=ParentHwnd;
	}

	//If we don't have a parent window then we must be a Popup
	if (!ParentHwnd)
		g_IsPopup=IsPopup=true;

	//for a previous ran session... we'll want to grab the last known position for this round
	if (g_IsSmartDashboardStarted)
	{
		//If we still have our parent window or if we have always been a pop-up
		if ((m_ParentHwnd) || (g_IsPopup && m_IsPopup_LastOpenedState))
		{
			//we'll want to to set the defaults to the last set position
			XPos=g_WindowInfo.rcNormalPosition.left;
			YPos=g_WindowInfo.rcNormalPosition.top;
			XRes=g_WindowInfo.rcNormalPosition.right - XPos;
			YRes=g_WindowInfo.rcNormalPosition.bottom - YPos;
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
			SetDefaults(m_PreviewName.c_str(),XRes,YRes,XPos,YPos);
		}
	}
	g_IsSmartDashboardStarted=IsSmartDashboardStarted;

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
	if (m_DD_StreamOut)
	{
		m_FrameGrabber.SetOutstream_Interface(m_DD_StreamOut);
		//Now to start the frame grabber
		m_FrameGrabber.StartStreaming();
	}
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

void DDraw_Preview::SetDefaults(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
	//Note: we use the rect as such
	//left=xRes top=yRes right=xPos bottom=YPos
	m_PreviewName=source_name;
	m_DefaultWindow.left =XRes,
	m_DefaultWindow.top=YRes,
	m_DefaultWindow.right=XPos,
	m_DefaultWindow.bottom=YPos;
}

void DDraw_Preview::SetDefaults(const wchar_t source_name[],LONG XRes,LONG YRes,float XPos,float YPos)
{
	if (XRes==-1)
		XRes=480;
	if (YRes==-1)
		YRes=270;

	LONG X=(LONG)(((float)(::GetSystemMetrics(SM_CXSCREEN)-XRes))*XPos);
	LONG Y=(LONG)(((float)(::GetSystemMetrics(SM_CYSCREEN)-YRes))*YPos);
	SetDefaults(source_name,XRes,YRes,X,Y);
}

void DDraw_Preview::RunApp()
{
	OpenResources();
	m_Terminate.wait();
	CloseResources();
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	wstring SmartDashboard=cwsz_DefaultSmartFile;
	long XRes,YRes,XPos,YPos;
	const char * const csz_FileName="BroncBotz_Dashboard.ini";
	{
		string InFile = csz_FileName;
		std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			const size_t NoEnties=6 << 1;
			string StringEntry[NoEnties];
			for (size_t i=0;i<NoEnties;i++)
			{
				in>>StringEntry[i];
			}
			in.close();
			int left=atoi(StringEntry[1].c_str());
			int top=atoi(StringEntry[3].c_str());
			int right=atoi(StringEntry[5].c_str());
			int bottom=atoi(StringEntry[7].c_str());
			XRes=right-left;
			YRes=bottom-top;
			XPos=left;
			YPos=top;
			char2wchar(StringEntry[9].c_str());
			SmartDashboard=char2wchar_pwchar;
			g_IsPopup=atoi(StringEntry[11].c_str())==0?false:true;
		}
		else
		{
			XRes=320;
			YRes=240;
			XPos=20;
			YPos=10;
		}
	}

	//TODO use .ini to determine standalone case
	DDraw_Preview TheApp(DDraw_Preview::eSmartDashboard,L"Preview",SmartDashboard.c_str(),XRes,YRes,XPos,YPos);
	//DDraw_Preview TheApp(DDraw_Preview::eStandAlone,L"Preview",SmartDashboard.c_str(),XRes,YRes,XPos,YPos);
	TheApp.RunApp();
	
	{
		string OutFile = csz_FileName;
		ofstream out(OutFile.c_str(), std::ios::out );
		out << "left " << g_WindowInfo.rcNormalPosition.left << endl;
		out << "top "  << g_WindowInfo.rcNormalPosition.top << endl;
		out << "right " << g_WindowInfo.rcNormalPosition.right << endl;
		out << "bottom "  << g_WindowInfo.rcNormalPosition.bottom << endl;
		wchar2char(SmartDashboard.c_str());
		out << "SmartDashboard " << wchar2char_pchar << endl;
		out << "IsPopup " << g_IsPopup << endl;
		out.close();
	}
	return 0;
}

