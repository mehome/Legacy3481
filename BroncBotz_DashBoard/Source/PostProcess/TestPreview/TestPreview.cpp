#include "stdafx.h"
#include <ddraw.h>
#include <atlbase.h>
#include <shellapi.h>
#include "../../FrameWork/FrameWork.h"
#include "../../FrameWork/Window.h"
#include "../../FrameWork/Preview.h"

#pragma comment (lib,"shell32")

// Converts a GUID to a string
inline void GUIDtow(GUID id,wchar_t *string) {
	wsprintfW(string,L"%x-%x-%x-%x%x-%x%x%x%x%x%x",id.Data1,id.Data2,id.Data3,
		id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]);
}

HWND FindConsoleHandle()
{
	static HWND wnd=NULL;
	if (!wnd)
	{
		GUID ID;
		TCHAR title[512];
		TCHAR tempGUIDtitle[512];
		LPCTSTR temptitle;
		if (FAILED(CoCreateGuid(&ID)))
			return NULL;
		GUIDtow(ID,tempGUIDtitle);
		temptitle=tempGUIDtitle;
		if (GetConsoleTitle(title,sizeof(title)/sizeof(TCHAR))==0)
			return NULL; //doh
		SetConsoleTitle(temptitle);
		wnd=FindWindow(NULL,temptitle);
		SetConsoleTitle(title);
	}
	return wnd;
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


class DDraw_Preview 
{
	public:
		enum WindowType
		{
			eStandAlone,
			eSmartDashboard
		};

		void Init(WindowType type);
		DDraw_Preview(WindowType type=eStandAlone, const wchar_t source_name[] = L"Preview",LONG XRes=-1,LONG YRes=-1,float XPos=0.5f,float YPos=0.5f);
		DDraw_Preview(WindowType type, const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos);
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
		Window *m_Window;
		Preview *m_DD_StreamOut;

		std::wstring m_PreviewName;
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
			const wchar_t *pWindowName=L"Window" , const RECT *pWindowPosition=NULL ) : Window(HWND_Parent,IsPopup,pWindowName,pWindowPosition), m_pParent(pParent)
		{
		}
	protected:
		virtual long Dispatcher(HWND window,UINT message, WPARAM w,LPARAM l)
		{
			return m_pParent->Dispatcher(window,message,w,l);
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
	m_DD_StreamOut=NULL;
	m_FrameGrabber=NULL;
}

DDraw_Preview::DDraw_Preview(WindowType type,const wchar_t source_name[],LONG XRes,LONG YRes,float XPos,float YPos)
{
	SetDefaults(source_name,XRes,YRes,XPos,YPos);
	Init(type);
}

DDraw_Preview::DDraw_Preview(WindowType type,const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
	SetDefaults(source_name,XRes,YRes,XPos,YPos);
	Init(type);
}

void DDraw_Preview::CloseResources()
{
	m_FrameGrabber.StopStreaming();
	delete m_DD_StreamOut;
	m_DD_StreamOut=NULL;
	if (m_Window)
	{
		delete m_Window;
		m_Window=NULL;
	}
}

DDraw_Preview::~DDraw_Preview()
{
	CloseResources();
}


void DDraw_Preview::DisplayHelp()
{
	printf(
		"reset [<this apps name=Switcher> [use 16:9=1] [xpos %%0-1] [ypos %%]\n"
		"cls\n"
		"\n"
		"Help  (displays this)\n"
		"Type \"Exit\" at anytime to exit to main\n"
		"Type \"Quit\" at anytime to exit this application\n"
		);
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
	bool IsPopup=true;
	if (m_WindowType==eSmartDashboard)
	{
		IsPopup=true;
		XPos=10;
		YPos=30;
		LPTSTR szCmdline = _tcsdup(TEXT("C:\\WindRiver\\WPILib\\SmartDashboard.jar"));
		//Note:
		//The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications. It is not a true HINSTANCE, 
		//however. The only thing that can be done with the returned HINSTANCE is to cast it to an int and compare it with the value 32 or one 
		//of the error codes below.
		HINSTANCE test=ShellExecute(FindConsoleHandle(),L"open",szCmdline,NULL,NULL,SW_SHOWNORMAL);
		//Give this some time to open
		size_t TimeOut=0;
		do 
		{
			Sleep(100);
			ParentHwnd=FindWindow(L"SunAwtFrame",L"SmartDashboard - ");
		} while ((ParentHwnd==NULL)&&(TimeOut++<50)); //This may take a while on cold start

	}

	{
		{
			assert (!m_Window);
			//apparently there is a racing condition where the window needs a thread to set its handle
			size_t TimeOut=0;
			LONG X=XPos;
			LONG Y=YPos;
			RECT lWindowPosition = {  X,Y,X+XRes,Y+YRes};
			m_Window=new DDraw_Window(this,ParentHwnd,IsPopup,source_name,&lWindowPosition);
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
	case WM_CLOSE:
		m_Terminate.set();
		break;
	default:
		return DefWindowProc(window,message,w,l);
	}
	return 0;
}

void cls(HANDLE hConsole=NULL) 
{   
	if (!hConsole)
		hConsole=::GetStdHandle( STD_OUTPUT_HANDLE );
	COORD coordScreen = { 0, 0 }; /* here's where we'll home the cursor */   
	BOOL bSuccess;   
	DWORD cCharsWritten;   
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */   
	DWORD dwConSize; /* number of character cells in the current buffer */    
	/* get the number of character cells in the current buffer */   
	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);   
	//PERR(bSuccess, "GetConsoleScreenBufferInfo");   
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;   /* fill the entire screen with blanks */   
	bSuccess = FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',       
		dwConSize, coordScreen, &cCharsWritten);   
	//PERR(bSuccess, "FillConsoleOutputCharacter");   /* get the current text attribute */   
	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);   
	//PERR(bSuccess, "ConsoleScreenBufferInfo");   /* now set the buffer's attributes accordingly */   
	bSuccess = FillConsoleOutputAttribute(hConsole, csbi.wAttributes,       dwConSize, coordScreen, &cCharsWritten);   
	//PERR(bSuccess, "FillConsoleOutputAttribute");   /* put the cursor at (0, 0) */   
	bSuccess = SetConsoleCursorPosition(hConsole, coordScreen);   
	//PERR(bSuccess, "SetConsoleCursorPosition");   return; 
} 

size_t split_arguments(const std::string& str, std::vector<std::string>& arguments)
{
	arguments.clear();

	if (str.empty())
		return 0;

	const std::string whitespace = " \t";
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

size_t FillArguments(const char *input_line,char *command,char *str_1,char *str_2,char *str_3,char *str_4)
{
	command[0]=0;
	str_1[0]=0;
	str_2[0]=0;
	str_3[0]=0;
	str_4[0]=0;

	std::vector<std::string> args;
	size_t ret=split_arguments(input_line, args);
	if (split_arguments(input_line, args) >= 1)
	{
		strcpy(command, args[0].c_str());
		if (args.size() > 1)
			strcpy(str_1, args[1].c_str());
		if (args.size() > 2)
			strcpy(str_2, args[2].c_str());
		if (args.size() > 3)
			strcpy(str_3, args[3].c_str());
		if (args.size() > 4)
			strcpy(str_4, args[4].c_str());
	}
	return ret;
}

bool DDraw_Preview::CommandLineInterface()
{
	OpenResources();
	DisplayHelp();
 	cout << "Ready." << endl;

	string AppName;
	{
		wchar2char(m_PreviewName.c_str());
		AppName=wchar2char_pchar;
	}

   	char input_line[128];
    while (cout << AppName << " >",cin.getline(input_line,128))
    {
		char		command[32];
		char		str_1[64];
		char		str_2[64];
		char		str_3[64];
		char		str_4[64];

		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;

		if (FillArguments(input_line,command,str_1,str_2,str_3,str_4)>=1) 
		{
			if (!_strnicmp( input_line, "cls", 3))
				cls();

			//Use this to flush resources
			else if (!_strnicmp( input_line, "reset", 5))
			{
				printf("TODO... not sure if this stress is needed\n");
				#if 0
				CloseResources();
				if (str_1[0])
				{
					AppName=str_1;

					LONG XRes=360;
					LONG YRes=270;

					if (str_2[0]=='1')
						XRes=480;

					float XPos=0.5,YPos=0.5;
					if (str_3[0])
						XPos=(float)atof(str_3);
					if (str_4[0])
						YPos=(float)atof(str_4);
					else
						YPos=XPos;
					char2wchar(str_1);
					SetDefaults(char2wchar_pwchar,XRes,YRes,XPos,YPos);
					OpenResources();
				}
				else
				{
					SetDefaults(m_PreviewName.c_str());
					OpenResources();
				}
				#endif
			}
			else if (!_strnicmp( input_line, "Test", 4))
			{
				#if 0
				bool wait = true;
				// Launch the command
				STARTUPINFO startup_info = { sizeof(STARTUPINFO), 0 };
				startup_info.dwFlags = STARTF_USESHOWWINDOW;
				startup_info.wShowWindow = SW_SHOWMINIMIZED;

				PROCESS_INFORMATION		process_info = { 0 };

				// Just in case someone changed it.
				LPTSTR szCmdline = _tcsdup(TEXT("C:\\WindRiver\\WPILib\\SmartDashboard.jar"));
				LPTSTR CurrentDirectory = _tcsdup(TEXT("C:\\WindRiver\\WPILib\\"));

				if ( ::CreateProcessW( NULL, szCmdline, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, CurrentDirectory, &startup_info, &process_info ) )
				{	// We wait for this process to exit
					if ( wait )
						::WaitForSingleObject( process_info.hProcess, INFINITE );

					// Close the handle
					::CloseHandle( process_info.hProcess );
					::CloseHandle( process_info.hThread );
				}
				#else
				LPTSTR szCmdline = _tcsdup(TEXT("C:\\WindRiver\\WPILib\\SmartDashboard.jar"));
				//Note:
				//The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications. It is not a true HINSTANCE, 
				//however. The only thing that can be done with the returned HINSTANCE is to cast it to an int and compare it with the value 32 or one 
				//of the error codes below.
				HINSTANCE test=ShellExecute(FindConsoleHandle(),L"open",szCmdline,NULL,NULL,SW_SHOWNORMAL);
				//Give this some time to open
				HWND TestHwnd;
				size_t TimeOut=0;
				do 
				{
					Sleep(100);
					TestHwnd=FindWindow(L"SunAwtFrame",L"SmartDashboard - ");
				} while ((TestHwnd==NULL)&&(TimeOut++<50)); //This may take a while on cold start
				printf("test=%p\n",TestHwnd);
				#endif
			}
			else if (!_strnicmp( input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Exit", 4))
			{
				break;
			}
			else if (!_strnicmp( input_line, "Quit", 4))
			{
				return true;
			}
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
	return false;  //just exit
}

void DDraw_Preview::RunApp()
{
	ShowWindow(FindConsoleHandle(),SW_HIDE);
	OpenResources();
	m_Terminate.wait();
	CloseResources();
}

void main()
{
	DDraw_Preview test(DDraw_Preview::eSmartDashboard);
	//DDraw_Preview test(DDraw_Preview::eStandAlone);
	#if 0
	test.CommandLineInterface();
	#else
	test.RunApp();
	#endif
}
