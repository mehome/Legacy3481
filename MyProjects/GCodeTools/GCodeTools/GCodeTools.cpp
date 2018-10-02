//I'm keeping these default comments... nice to see something different

// PeerCall.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file



#include "pch.h"
#include <iostream>
#pragma comment (lib,"winmm")
#include "GCodeTools.h"

void DisplayHelp()
{
	printf(
		"Connect\n"
		"sound_start\n"
		"sound_stop\n"
		"load_ct [filename]\n"
		"playb <block number=0>\n"
		"plays <position=0.0>\n"
		"stop\n"
		"pause (toggles state)\n"
		"reversech <0 normal 1 reversed>\n"
		"export [filename]\n"
		"setbounds <x,y,z>\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
	);
}

void CommandLineInterface()
{
	GCodeTools _gcode_tools;
	_gcode_tools.GCodeTools_init();
	bool IsPaused = false;

	using namespace std;
	cout << endl;
	cout << "Ready." << endl;

	char input_line[128];
	while (cout << ">", cin.getline(input_line, 128))
	{
		char		command[32];
		char		str_1[MAX_PATH];
		char		str_2[MAX_PATH];
		char		str_3[MAX_PATH];
		char		str_4[MAX_PATH];

		command[0] = 0;
		str_1[0] = 0;
		str_2[0] = 0;
		str_3[0] = 0;
		str_4[0] = 0;

		if (sscanf(input_line, "%s %s %s %s %s", command, str_1, str_2, str_3, str_4) >= 1)
		{

			if (!_strnicmp(input_line, "connect", 5))
			{
				_gcode_tools.GCodeTools_connect();
			}
			else if (!_strnicmp(input_line, "sound_start", 11))
			{
				_gcode_tools.TestSound_Start();
			}
			else if (!_strnicmp(input_line, "sound_stop", 10))
			{
				_gcode_tools.TestSound_Stop();
			}
			else if (!_strnicmp(input_line, "load_ct", 4))
			{
				bool ret=_gcode_tools.LoadSequence_CT(str_1);
				printf("result=%s\n",ret?"success":"load error");
			}
			else if (!_strnicmp(input_line, "playb", 5))
			{
				_gcode_tools.PlayBlock(atoi(str_1));
			}
			else if (!_strnicmp(input_line, "plays", 5))
			{
				_gcode_tools.PlaySong(atof(str_1));
			}
			else if (!_strnicmp(input_line, "stop", 4))
			{
				_gcode_tools.Stop_NotePlayer();
			}
			else if (!_strnicmp(input_line, "pause", 5))
			{
				IsPaused = !IsPaused; //toggle
				_gcode_tools.Pause_NotePlayer(IsPaused);
			}
			else if (!_strnicmp(input_line, "reversech", 7))
			{
				int Reversed = atoi(str_1);
				_gcode_tools.ReveseChannels(Reversed != 0);
			}
			else if (!_strnicmp(input_line, "export", 5))
			{
				_gcode_tools.ExportGCode(str_1[0]==0?nullptr:str_1);
			}
			else if (!_strnicmp(input_line, "setbounds", 7))
			{
				const double x = atof(str_1);
				const double y = atof(str_2);
				const double z = atof(str_3);
				_gcode_tools.SetBounds(x,y,z);
				printf("Bounds x=%.2f y=%.2f z=%.2f\n",x,y,z);
			}
			else if (!_strnicmp(input_line, "test", 4))
			{
				_gcode_tools.Test();
			}
			else if (!_strnicmp(input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp(input_line, "Quit", 4))
				break;
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
}


int main()
{
	{ // Three magic lines of code
		TIMECAPS TimeCaps_;
		timeGetDevCaps(&TimeCaps_, sizeof(TimeCaps_));
		timeBeginPeriod(TimeCaps_.wPeriodMin);
	}

	DisplayHelp();
	CommandLineInterface();
}

