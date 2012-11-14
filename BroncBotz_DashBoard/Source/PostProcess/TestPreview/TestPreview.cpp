#include "stdafx.h"
#include <ddraw.h>
#include <atlbase.h>
#include "../../FrameWork/FrameWork.h"
#include "../../FrameWork/Window.h"
#include "../../FrameWork/Preview.h"


class DDraw_Preview 
{
	public:
		DDraw_Preview(const wchar_t source_name[] = L"Preview",LONG XRes=-1,LONG YRes=-1,float XPos=0.5f,float YPos=0.5f);
		DDraw_Preview(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos);
		virtual ~DDraw_Preview();
		//returns true to quit
		bool CommandLineInterface();
		Preview *GetPreview() {return m_DD_StreamOut;}
	private:
		void DisplayHelp();
		void CloseResources();
		// -1 for x and y res will revert to hard coded defaults (this keeps tweaking inside the cpp file)
		void OpenResources(const wchar_t source_name[] = L"Preview",LONG XRes=-1,LONG YRes=-1,float XPos=0.5f,float YPos=0.5f);
		void OpenResources(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos);

		Window *m_Window;
		Preview *m_DD_StreamOut;

		//Audio stuff TODO
		std::wstring m_PreviewName;
};

using namespace std;

  /*******************************************************************************************************/
 /*											DDraw_Preview												*/
/*******************************************************************************************************/

DDraw_Preview::DDraw_Preview(const wchar_t source_name[],LONG XRes,LONG YRes,float XPos,float YPos) : m_Window(NULL),m_DD_StreamOut(NULL)
{
	OpenResources(source_name,XRes,YRes,XPos,YPos);
}

DDraw_Preview::DDraw_Preview(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos): m_Window(NULL),m_DD_StreamOut(NULL)
{
	OpenResources(source_name,XRes,YRes,XPos,YPos);
}

void DDraw_Preview::CloseResources()
{
	delete m_DD_StreamOut;
	m_DD_StreamOut=NULL;

	//Audio
}

DDraw_Preview::~DDraw_Preview()
{
	CloseResources();
	if (m_Window)
	{
		delete m_Window;
		m_Window=NULL;
	}
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

void DDraw_Preview::OpenResources(const wchar_t source_name[],LONG XRes,LONG YRes,LONG XPos,LONG YPos)
{
	if (!m_DD_StreamOut)
	{
		m_PreviewName=source_name;
		{
			//Doh have to reopen the window to reposition it... (should have methods to manipulate but oh well
			if (m_Window)
			{
				delete m_Window;
				m_Window=NULL;
			}

			//apparently there is a racing condition where the window needs a thread to set its handle
			size_t TimeOut=0;
			LONG X=XPos;
			LONG Y=YPos;
			RECT lWindowPosition = {  X,Y,X+XRes,Y+YRes};
			m_Window=new Window(NULL,true,source_name,&lWindowPosition);
			while ((!(HWND)*m_Window)&&(TimeOut++<100))
				Sleep(10);
			assert((HWND)*m_Window);

			m_DD_StreamOut=new Preview((HWND)*m_Window);
			if (!m_DD_StreamOut->Get_IsError())
			{
				m_DD_StreamOut->StartStreaming();
			}
			else
				printf("DDraw_Preview::OpenResources Error detected in setting up DDraw environment \n");
		}
	}
}

void DDraw_Preview::OpenResources(const wchar_t source_name[],LONG XRes,LONG YRes,float XPos,float YPos)
{
	if (XRes==-1)
		XRes=480;
	if (YRes==-1)
		YRes=270;

	LONG X=(LONG)(((float)(::GetSystemMetrics(SM_CXSCREEN)-XRes))*XPos);
	LONG Y=(LONG)(((float)(::GetSystemMetrics(SM_CYSCREEN)-YRes))*YPos);
	OpenResources(source_name,XRes,YRes,X,Y);
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
					OpenResources(char2wchar_pwchar,XRes,YRes,XPos,YPos);
				}
				else
				{
					OpenResources(m_PreviewName.c_str());
				}
				#endif
			}
			else if (!_strnicmp( input_line, "Select", 6))
			{
				char2wchar(str_1);
				//m_Switcher->SelectSenderToProcess(char2wchar_pwchar);
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

  /*******************************************************************************************************/
 /*										FrameGrabber_TestPattern										*/
/*******************************************************************************************************/

//Throw together the infamous test pattern that streams the frames out
class FrameGrabber_TestPattern
{
	public:
		FrameGrabber_TestPattern(FrameWork::Outstream_Interface *Preview) : m_pThread(NULL),m_TestMap(720,480),m_Outstream(Preview)
		{
		}

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

		void DrawField( PBYTE pField, const int FrameWidth, const int FieldHeight, const int FieldNumber )
		{
			{ // aka Black Field section
				const int FrameSize = FrameWidth * FieldHeight * sizeof(USHORT);
				PWORD pField_ = (PWORD) pField, pEnd_ = pField_ + (FrameSize/sizeof(WORD));

				int FieldHeight_ = FrameSize / (FrameWidth * sizeof(USHORT));
				int OneThird_ = ( FieldHeight_ * 1 ) / 3;
				int TwoThird_ = ( FieldHeight_ * 2 ) / 3;

				while(pField_ != pEnd_)
				{	*pField_++ = 0x1080;
				}
				// Draw gradient
				for (int Idx_ = 0; Idx_ < OneThird_; Idx_++)
				{	PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

				for (int X_ = 0; X_ < FrameWidth; X_++)
				{	WORD w_ = ((X_&0xFF)<<8)+ 0x80;
				*pw_++ = w_;
				}
				}
				// Draw bars
				for (int Idx_ = TwoThird_; Idx_ < FieldHeight_; Idx_++)
				{	PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

				for (int X_ = 0; X_ < FrameWidth; X_++)
				{	WORD w_ = 0xC000 | ((X_*256)/FrameWidth);
				*pw_++ = w_;
				}
				}

			}
			int ThreeNine_ = ( FieldHeight * 3 ) / 9;
			int FourNine_  = ( FieldHeight * 4 ) / 9;
			int FiveNine_  = ( FieldHeight * 5 ) / 9;
			int SixNine_   = ( FieldHeight * 6 ) / 9;

			// Draw stationary vertical lines
			for (int Idx_ = ThreeNine_; Idx_ < FourNine_; Idx_++)
			{	
				PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

				for (int X_ = 0; X_ < FrameWidth; X_++)
				{	
					int Of16_ = (X_ % 16);
					if (Of16_ == 0 || Of16_ == 1)
					{	
						*pw_++ = 0xE080;
					}
					else
					{	
						*pw_++ = 0x1080;
					}
				}
			}

			// Draw moving horizontal lines
			for (int Idx_ = FourNine_; Idx_ < FiveNine_; Idx_++)
			{	PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

			int Of16_ = ((Idx_ + FieldNumber) % 16);
			//int Of16_ = (Idx_  % 16);

			if (Of16_ == 0)
			{	
				for (int X_ = 0; X_ < FrameWidth; X_++)
					*pw_++ = 0xE080;
			}
			else
			{	
				for (int X_ = 0; X_ < FrameWidth; X_++)
					*pw_++ = 0x1080;
			}
			}
			//Draw moving diagonal lines
			for (int Idx_ = FiveNine_; Idx_ < SixNine_; Idx_++)
			{	
				PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

				for (int X_ = 0; X_ < FrameWidth; X_++)
				{	int Of16_ = ((X_ + Idx_ + FieldNumber) % 16);
				if (Of16_ == 0 || Of16_ == 1)
				{	*pw_++ = 0xE080;
				}
				else
				{	*pw_++ = 0x1080;
				}
				}
			}
			//--------------------------------------------------------------------------------Field alignment test
			// Draw black on field line 480, F0 Left, F1 Right.
			// These would be frame lines 960 and 961.
			// Effect should be Left Line above Right Line.
			{
				int Line480_ = ( FieldHeight * 8 ) / 9;
				PWORD pwLine_ = ((PWORD) pField + (Line480_ * FrameWidth));

				// If F1, advance to right half.
				if (FieldNumber & 1)
				{
					pwLine_ += (FrameWidth/2);
				}
				// Draw half a line.
				for (int X_ = 0; X_ < (FrameWidth/2); X_++)
				{	*pwLine_++ = 0x1080;
				}
			}
			// Line 0: F0 Black / White, F1 White Black.
			// These would be frame lines 0 and 1.
			// Effect should be Frame Line 0 Black / White,
			// and Frame Line 1 White Black, ie Left is
			// Black ontop of White, and Right is White
			// ontop of Black. These are so we can verify
			// output with a digital scope.
			{
				PWORD pwLine_ = (PWORD) pField;
				WORD wLeft_, wRight_;

				if (FieldNumber & 1)
					wLeft_ = 0xF080, wRight_ = 0x1080;
				else
					wLeft_ = 0x1080, wRight_ = 0xF080;

				for (int X_ = 0; X_ < FrameWidth; X_++)
				{
					if (X_ < (FrameWidth/2))
						*pwLine_++ = wLeft_;
					else
						*pwLine_++ = wRight_;
				}
			}
		}

		void operator() ( const void* )
		{
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
		FrameWork::Outstream_Interface * const m_Outstream;
		size_t m_Counter;
};

void main()
{
	DDraw_Preview test;
	FrameGrabber_TestPattern grabber(test.GetPreview());
	grabber.StartStreaming();
	test.CommandLineInterface();
}
