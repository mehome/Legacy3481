
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
		m_pThread = new FrameWork::thread<FrameGrabber_TestPattern>(this);
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
	friend FrameWork::thread<FrameGrabber_TestPattern>;

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
	FrameWork::thread<FrameGrabber_TestPattern> *m_pThread;	// My worker thread that does something useful w/ a buffer after it's been filled

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
		Preview *GetPreview() {return m_DD_StreamOut;}
		virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l);
	protected:
		virtual void CloseResources();
		virtual void OpenResources();
	private:
		void DisplayHelp();
		// -1 for x and y res will revert to hard coded defaults (this keeps tweaking inside the cpp file)
		void SetDefaults(const wchar_t source_name[] = L"Preview",LONG XRes=-1,LONG YRes=-1,float XPos=0.5f,float YPos=0.5f);
		void SetDefaults(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		FrameWork::event m_Terminate;
		HWND m_ParentHwnd;
		Window *m_Window;
		Preview *m_DD_StreamOut;

		std::wstring m_PreviewName,m_SmartDashBoard_FileName;
		RECT m_DefaultWindow;  //left=xRes top=yRes right=xPos bottom=YPos

		FrameGrabber_TestPattern m_FrameGrabber;

		WindowType m_WindowType;
};

using namespace std;

  /*******************************************************************************************************/
 /*											DDraw_Window												*/
/*******************************************************************************************************/

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
		virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
		{
			return m_pParent->Dispatcher(window,message,w,l);
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

WINDOWPLACEMENT g_WindowInfo;
bool g_IsPopup=true;

void DDraw_Preview::CloseResources()
{
	m_FrameGrabber.StopStreaming();
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
	if (m_WindowType==eSmartDashboard)
	{
		LPTSTR szCmdline = _tcsdup(m_SmartDashBoard_FileName.c_str());
		//Note:
		//The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications. It is not a true HINSTANCE, 
		//however. The only thing that can be done with the returned HINSTANCE is to cast it to an int and compare it with the value 32 or one 
		//of the error codes below.
		HINSTANCE test=ShellExecute(NULL,L"open",szCmdline,NULL,NULL,SW_SHOWNORMAL);
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

	{
		{
			assert (!m_Window);
			//apparently there is a racing condition where the window needs a thread to set its handle
			size_t TimeOut=0;
			LONG X=XPos;
			LONG Y=YPos;
			RECT lWindowPosition = {  X,Y,X+XRes,Y+YRes};
			//Note: A child option can only be presented if we have the parent window
			m_Window=new DDraw_Window(this,ParentHwnd,ParentHwnd?IsPopup:true,source_name,&lWindowPosition);
			while ((!(HWND)*m_Window)&&(TimeOut++<100))
				Sleep(10);
			assert((HWND)*m_Window);
			hWnd_ForDDraw=(HWND)*m_Window;
		}
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

void DDraw_Preview::SetDefaults(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
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

long DDraw_Preview::Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
{
	switch (message)
	{
	#if 0
	case WM_TIMER:
		if (m_ParentHwnd)
		{
			//agghhhh polling for now yuck
			m_ParentHwnd=FindWindow(L"SunAwtFrame",L"SmartDashboard - ");
			if (!m_ParentHwnd)
				m_Terminate.set();
		}
		break;
	#endif
	case WM_CLOSE:
	case WM_DESTROY:
		m_Terminate.set();
		break;
	default:
		return DefWindowProc(window,message,w,l);
	}
	return 0;
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
	//DDraw_Preview test(DDraw_Preview::eStandAlone,L"Preview",SmartDashboard.c_str(),XRes,YRes,XPos,YPos);
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

