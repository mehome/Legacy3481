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
		"LogIn <ID or blank to disconnect>\n"
		"Remote <ID to connect to>\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
	);
}

void CommandLineInterface()
{
	PeerCall _peer_call;
	_peer_call.peer_sip_init();

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

			if (!_strnicmp(input_line, "LogIn", 5))
			{
				_peer_call.peer_sip_connect(str_1[0] == 0 ? nullptr : str_1);
			}
			else if (!_strnicmp(input_line, "remote", 6))
			{
				_peer_call.peer_sip_set_remote_id(str_1);
			}
			else if (!_strnicmp(input_line, "Test", 4))
			{
				size_t index = (size_t)atoi(str_1);
				//HandleTests(index, str_2, str_3, str_4);
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

