// TestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PIF
#define M_PIF 3.141592654f
#endif
#define M_PID 3.14159265358979323846

#define DEG_2_RAD(x)		((x)*M_PI/180.0)
#define RAD_2_DEG(x)		((x)*180.0/M_PI)
#define ARRAY_SIZE(things)	((sizeof(things)/sizeof(*(things))))

#define Inches2Meters(x)	((x)*0.0254)
#define Feet2Meters(x)		((x)*0.3048)
#define Meters2Feet(x)		((x)*3.2808399)
#define Meters2Inches(x)	((x)*39.3700787)

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
		"test <#>\n"
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
			else if (!_strnicmp( input_line, "Test", 4))
			{
				int Test=atoi(str_1);
				enum
				{
					eCurrent,
					eFOVSweepTest,
					eRobotSweepTest,
				};
				const char * const TestName[]=
				{
					"current",
					"FOVTest",
					"RobotSweepTest",
				};

				//if the first character is not a number then translate the string
				if (((str_1[0]<'0')||(str_1[0]>'9'))&&(str_1[0]!=0))
				{
					Test=-1;
					for (size_t i=0;i<_countof(TestName);i++)
					{
						if (stricmp(TestName[i],str_1)==0)
						{
							Test=i;
							break;
						}
					}
					if (Test==-1)
					{
						printf("No match found.  Try:\n");
						for (size_t i=0;i<_countof(TestName);i++)
							printf("%s, ",TestName[i]);
						printf("\n");
						return;
					}
				}

				switch(Test)
				{
				
				case eCurrent:
				case eFOVSweepTest:
					{
						//Note: I usually work with radians and meters... but in this small example I've
						//chosen to work in degrees and inches for easier readability
						const double min_angle=-17.158;
						const double max_angle=17.158*3.0;
						const double NoSteps=100.0;

						const double StandAdjustedAngle=min_angle;  //it so happens they are equal in this test
						const double StandFrameDiameter_in=3.1;
						const double StandFrameRadius_in=StandFrameDiameter_in/2;
						const double pivot_radius_in=sqrt((6*6)+(StandFrameDiameter_in*StandFrameDiameter_in));
						const double Camera_Z_offset=-1.954 + -5.0;
						const double Camera_Y_offset=-0.4;

						const double Arm_Length_in=17.0;
						const double CubeSize=5.3;

						for (double i=0.0; i<NoSteps; i++)
						{
							const double Scalar=i/NoSteps;
							const double pitch=max_angle*Scalar + min_angle;
							//pitch is the sensed angle use this to determine other geometry
							const double stand_angle=pitch+StandAdjustedAngle;  //determine angle of stand  
							const double height=sin(DEG_2_RAD(pitch)) * Arm_Length_in + CubeSize;  //5.3 is cube height
							SmartDashboard::PutNumber("Camera_rot_y",pitch);
							SmartDashboard::PutNumber("Camera_y",cos(DEG_2_RAD(pitch))*pivot_radius_in+Camera_Y_offset);
							SmartDashboard::PutNumber("Camera_z",sin(DEG_2_RAD(-pitch))*pivot_radius_in+Camera_Z_offset);
							SmartDashboard::PutNumber("Height",height);
							SmartDashboard::PutNumber("height_indicator_y",height);
							SmartDashboard::PutNumber("ArmAngle",pitch);
							SmartDashboard::PutBoolean("medium_box_enabled",height>CubeSize);
							SmartDashboard::PutBoolean("high_box_enabled",height>CubeSize*2);
							Sleep(33);
						}
						Sleep(2000);
						SmartDashboard::PutNumber("Camera_rot_y",min_angle);
						SmartDashboard::PutNumber("Camera_y",6.0);
						SmartDashboard::PutNumber("Camera_z",-5.0);
						SmartDashboard::PutNumber("Height",0.0);
						SmartDashboard::PutNumber("height_indicator_y",0.0);
						SmartDashboard::PutNumber("ArmAngle",min_angle);
						SmartDashboard::PutBoolean("medium_box_enabled",false);
						SmartDashboard::PutBoolean("high_box_enabled",false);
					}
					break;
				case eRobotSweepTest:
					{
						//These angles were pulled from .\Design\2015\Prototyping\Arm Prototype\ArmSketch4.SLDPRT
						//subject to change
						const double min_angle=-43.32;
						const double max_angle=52.36;
						const double NoSteps=100.0;

						//This is a rough estimate of a 5.75 height looking down a 50 inch arm... will know better once tower assembly
						//is cleaned up with camera mounted
						const double StandAdjustedAngle=-6.56;  
						const double StandFrameDiameter_in=3.1;  //this is correct
						const double StandFrameRadius_in=StandFrameDiameter_in/2;
						//TODO will need to find pivot point and use distance formula on it
						const double pivot_radius_in=sqrt((6*6)+(StandFrameDiameter_in*StandFrameDiameter_in));
						//TODO get these from solid works once camera is mounted
						const double Camera_Z_offset=-1.954 + -5.0;
						const double Camera_Y_offset=-0.4;

						const double Arm_Length_in=17.0;
						const double CubeSize=5.3;

						for (double i=0.0; i<NoSteps; i++)
						{
							const double Scalar=i/NoSteps;
							const double pitch=max_angle*Scalar + min_angle;
							//pitch is the sensed angle use this to determine other geometry
							const double stand_angle=pitch+StandAdjustedAngle;  //determine angle of stand  
							const double height=sin(DEG_2_RAD(pitch)) * Arm_Length_in + CubeSize;  //5.3 is cube height
							SmartDashboard::PutNumber("Camera_rot_y",pitch);
							SmartDashboard::PutNumber("Camera_y",cos(DEG_2_RAD(pitch))*pivot_radius_in+Camera_Y_offset);
							SmartDashboard::PutNumber("Camera_z",sin(DEG_2_RAD(-pitch))*pivot_radius_in+Camera_Z_offset);
							SmartDashboard::PutNumber("Height",height);
							SmartDashboard::PutNumber("height_indicator_y",height);
							SmartDashboard::PutNumber("ArmAngle",pitch);
							//TODO use a different naming convention for the totes
							//SmartDashboard::PutBoolean("medium_box_enabled",height>CubeSize);
							//SmartDashboard::PutBoolean("high_box_enabled",height>CubeSize*2);
							Sleep(33);
						}
						Sleep(2000);
						SmartDashboard::PutNumber("Camera_rot_y",min_angle);
						SmartDashboard::PutNumber("Camera_y",6.0);
						SmartDashboard::PutNumber("Camera_z",-5.0);
						SmartDashboard::PutNumber("Height",0.0);
						SmartDashboard::PutNumber("height_indicator_y",0.0);
						SmartDashboard::PutNumber("ArmAngle",min_angle);
						//TODO use a different naming convention for the totes
						//SmartDashboard::PutBoolean("medium_box_enabled",false);
						//SmartDashboard::PutBoolean("high_box_enabled",false);
					}
					break;
				}
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
