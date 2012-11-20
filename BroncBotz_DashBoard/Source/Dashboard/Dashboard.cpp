
#include "stdafx.h"
#include <ddraw.h>
#include <atlbase.h>
#include <shellapi.h>
#include "../FrameWork/FrameWork.h"
#include "../FrameWork/Window.h"
#include "../FrameWork/Preview.h"
#include "../ProcessingVision/ProcessingVision.h"
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
		m_RGB=m_TestMap;
		m_Outstream->process_frame(&m_RGB);
		//printf("%d\n",m_Counter++);
	}
	FrameWork::tThread<FrameGrabber_TestPattern> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

private:
	FrameWork::Bitmaps::bitmap_ycbcr_u8 m_TestMap;
	FrameWork::Bitmaps::bitmap_bgr_u8 m_RGB;
	FrameWork::Outstream_Interface * m_Outstream; //could be dynamic, but most-likely just late binding per stream session
	size_t m_Counter;
};


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
				m_DriverProc=(DriverProc_t) GetProcAddress(m_PlugIn,"ProcessFrame_RGB24");
				if (!m_DriverProc)
					FlushPlugin();
			}
		}
		void StartStreaming() {m_IsStreaming=true;}
		void StopStreaming() {m_IsStreaming=false;}

		virtual void process_frame(const FrameWork::Bitmaps::bitmap_bgr_u8 *pBuffer)
		{
			if (m_IsStreaming)
			{
				if (m_DriverProc)
				{
					using namespace FrameWork::Bitmaps;
					Bitmap_Frame frame((PBYTE)(*pBuffer)(),pBuffer->xres(),pBuffer->yres(),pBuffer->stride());
					Bitmap_Frame out_frame=*((*m_DriverProc)(&frame));
					bitmap_bgr_u8 dest((pixel_bgr_u8 *)out_frame.Memory,out_frame.XRes,out_frame.YRes,out_frame.Stride);
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
			std::wstring smart_file;
			std::wstring plugin_file;
			std::wstring aux_startup_file;
			std::wstring aux_startup_file_Args;
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
	protected:
		virtual void CloseResources();
		virtual void OpenResources();
	private:
		void Reset();
		void DisplayHelp();
		void SetDefaults(LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		FrameWork::event m_Terminate;
		FrameWork::thread m_thread;  //For DPC support

		HWND m_ParentHwnd;
		Window *m_Window;
		Preview *m_DD_StreamOut;

		DDraw_Preview_Props m_Props;
		RECT m_DefaultWindow;  //left=xRes top=yRes right=xPos bottom=YPos

		FrameGrabber_TestPattern m_FrameGrabber;
		ProcessingVision m_ProcessingVision;

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
	m_FrameGrabber(NULL),m_ProcessingVision(NULL)
{
	m_Props=props;
	SetDefaults(props.XRes,props.YRes,props.XPos,props.YPos);
}

void DDraw_Preview::CloseResources()
{
	//before closing the resources ensure the upstream is not streaming to us
	m_FrameGrabber.StopStreaming();
	m_ProcessingVision.StopStreaming();
	m_FrameGrabber.SetOutstream_Interface(NULL);  //pedantic
	m_ProcessingVision.SetOutstream_Interface(NULL);
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
	typedef DDraw_Preview::DDraw_Preview_Props PrevProps;
	CloseResources(); //just ensure all resources are closed

	m_ProcessingVision.LoadPlugIn(m_Props.plugin_file.c_str());
	LONG XRes=m_DefaultWindow.left, YRes=m_DefaultWindow.top, XPos=m_DefaultWindow.right, YPos=m_DefaultWindow.bottom;
	const wchar_t *source_name=m_Props.source_name.c_str();

	HWND hWnd_ForDDraw=NULL;
	HWND ParentHwnd=NULL;
	bool IsPopup=g_IsPopup;
	bool IsSmartDashboardStarted=g_IsSmartDashboardStarted;
	if (m_Props.window_type==PrevProps::eSmartDashboard)
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
			SetDefaults(XRes,YRes,XPos,YPos);
		}
	}

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

	string sz_FileName="BroncBotz_Dashboard.ini";
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
			const size_t NoEnties=10 << 1;
			string StringEntry[NoEnties];
			for (size_t i=0;i<NoEnties;i++)
			{
				in>>StringEntry[i];
			}
			in.close();
			{
				char2wchar(StringEntry[1].c_str());
				Title=char2wchar_pwchar;
			}
			int left=atoi(StringEntry[3].c_str());
			int top=atoi(StringEntry[5].c_str());
			int right=atoi(StringEntry[7].c_str());
			int bottom=atoi(StringEntry[9].c_str());
			props.XRes=right-left;
			props.YRes=bottom-top;
			props.XPos=left;
			props.YPos=top;
			{
				char2wchar(StringEntry[11].c_str());
				SmartDashboard=char2wchar_pwchar;
			}
			g_IsPopup=atoi(StringEntry[13].c_str())==0?false:true;
			{
				char2wchar(StringEntry[15].c_str());
				Plugin=char2wchar_pwchar;
			}
			{
				char2wchar(StringEntry[17].c_str());
				AuxStart=char2wchar_pwchar;
			}
			{
				char2wchar(StringEntry[19].c_str());
				AuxArgs=char2wchar_pwchar;
			}
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
	if (wcsicmp(SmartDashboard.c_str(),L"none")==0)
		props.window_type=DDraw_Preview::DDraw_Preview_Props::eStandAlone;
	#else
	DDraw_Preview::WindowType window_type=DDraw_Preview::DDraw_Preview_Props::eStandAlone;
	#endif

	props.source_name=Title.c_str();
	props.smart_file=SmartDashboard.c_str();
	props.plugin_file=Plugin.c_str();
	props.aux_startup_file=AuxStart.c_str();
	if (wcsicmp(props.aux_startup_file.c_str(),L"none")==0)
		props.aux_startup_file=L"";
	props.aux_startup_file_Args=AuxArgs.c_str();
	if (wcsicmp(props.aux_startup_file_Args.c_str(),L"none")==0)
		props.aux_startup_file_Args=L"";

	DDraw_Preview TheApp(props);

	TheApp.RunApp();
	
	{
		string OutFile = sz_FileName.c_str();
		ofstream out(OutFile.c_str(), std::ios::out );
		{
			wchar2char(Title.c_str());
			out << "title= " << wchar2char_pchar << endl;
		}
		out << "left= " << g_WindowInfo.rcNormalPosition.left << endl;
		out << "top= "  << g_WindowInfo.rcNormalPosition.top << endl;
		out << "right= " << g_WindowInfo.rcNormalPosition.right << endl;
		out << "bottom= "  << g_WindowInfo.rcNormalPosition.bottom << endl;
		{
			wchar2char(SmartDashboard.c_str());
			out << "SmartDashboard= " << wchar2char_pchar << endl;
		}
		out << "IsPopup= " << g_IsPopup << endl;
		{
			wchar2char(Plugin.c_str());
			out << "PlugIn= " << wchar2char_pchar << endl;
		}
		{
			wchar2char(AuxStart.c_str());
			out << "AuxStartupFile= " << wchar2char_pchar << endl;
		}
		{
			wchar2char(AuxArgs.c_str());
			out << "AuxStartupFileArgs= " << wchar2char_pchar << endl;
		}
		out.close();
	}
	return 0;
}

