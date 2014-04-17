// TestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"

using namespace std;

/************************************************************************
* FUNCTION: cls(HANDLE hConsole)                                        * 
*                                                                       * 
* PURPOSE: clear the screen by filling it with blanks, then home cursor * 
*                                                                       * 
* INPUT: the console buffer to clear                                    * 
*                                                                       * 
* RETURNS: none                                                         * 
*************************************************************************/  
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

void DisplayHelp()
{
	printf(
		"cls clears screen\n"
		"Sequence <#>\n"
		"Edit <off on>\n"
		"Velocity <feet per second>\n"
		"Turn <in radians e.g. 0.01>\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
		);
}

#pragma warning(disable : 4996)

bool GetBool(const char *arg)
{
	bool ret=false;
	if (_stricmp("on",arg)==0)
		ret=true;
	else if (arg[0]=='1' || arg[0]=='y' || arg[0]=='Y')
		ret=true;
	return ret;
}

void CommandLineInterface()
{
	char input_line[128];

	char		command[32];
	char		str_1[MAX_PATH];
	char		str_2[MAX_PATH];
	char		str_3[MAX_PATH];
	char		str_4[MAX_PATH];
	char		str_5[MAX_PATH];

	const char * const Args[]=
	{
		str_1,str_2,str_3,str_4,str_5
	};

	while (cout << ">",cin.getline(input_line,128))
	{
		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;
		str_5[0]=0;

		if (sscanf( input_line,"%s %s %s %s %s %s",command,str_1,str_2,str_3,str_4,str_5)>=1)
		{
			if (!_strnicmp( input_line, "cls", 3))
			{
				cls();
			}
			else if (!_strnicmp( input_line, "sequence", 3))  //or seq for short
			{
				SmartDashboard::PutNumber("Sequence",atof(str_1));
			}
			else if (!_strnicmp( input_line, "edit", 4)) 
			{
				SmartDashboard::PutBoolean("Edit Position Main",GetBool(str_1));
			}
			else if (!_strnicmp( input_line, "Velocity", 3))  //or vel for short
			{
				SmartDashboard::PutNumber("Velocity",atof(str_1));
			}
			else if (!_strnicmp( input_line, "Turn", 4))
			{
				SmartDashboard::PutNumber("Rotation Velocity",atof(str_1));
			}
			else if (!_strnicmp( input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Quit", 4))
				break;
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
}

int main(int argc, char** argv)
{
	//There should never be a desired case for more than one instance of this server running, and for convenience I can have the
	//configuration auto load this program when launching the dashboard.  Creating a system wide event is a way to ensure there
	//is only one instance running

	// Create a system wide named event
	HANDLE	SystemWideNamedEvent = ::CreateEventW( NULL, FALSE, FALSE, L"SmartDashboard_LocalTestServer" );		
	if ( SystemWideNamedEvent )
	{	// Check for another app with the same name running
		if ( ::GetLastError() == ERROR_ALREADY_EXISTS )
		{
			printf("Another copy of this application is already running.\n");
			Sleep(1000); //allow user to see this message
			return 1;
		}
	}

	SmartDashboard::init();
	printf("\n\n--------------------------------------\n");
	DisplayHelp();
	CommandLineInterface();
	SmartDashboard::shutdown();

	CloseHandle(SystemWideNamedEvent);
	return 0;
}
