#include "stdafx.h"
#include "../TestFiles.cpp"
#define __IncludedTestFiles__
#include "Common.h"

/************************************************************************
* FUNCTION: cls(HANDLE hConsole)                                        * 
*                                                                       * 
* PURPOSE: clear the screen by filling it with blanks, then home cursor * 
*                                                                       * 
* INPUT: the console buffer to clear                                    * 
*                                                                       * 
* RETURNS: none                                                         * 
*************************************************************************/  
void cls(HANDLE hConsole) 
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

long Hex2Long (const char *string)
{
	long value;
	//convert string to a long value
	const char *hexstring=strrchr(string,'x');
	//todo check for zero before x
	if (hexstring)
	{
		value=0;
		unsigned index=1;
		char Digit;
		while (Digit=hexstring[index++])
		{
			value=value<<4;
			Digit-='0';
			if (Digit>16)
				Digit-=7;
			if (Digit>16)
				Digit-=32;
			value+=Digit;
		}
	}
	else
		value=atol(string);
	return value;
}
const char *InputFileName=NULL;
char ManualFilename[MAX_PATH]={'0'};

void ShowCurrentSelection()
{
	printf("Current Selection-> %s\n",InputFileName);
}
void ShowFileSelections()
{
	for (size_t i=0;i<g_FileStuff->NoSelections;i++)
		printf("%d.\t%s\n",i,g_FileStuff->inputfile[i]);
}

void DisplayFileSelectionHelp()
{
	printf(
		"---------File Stuff----------\n"
		"ShowFiles\n"
		"SelectGroup <group#> \n"
		"Sel <File#> \n"
		"SetFileName <File with Path>\n"
		"cls\n"
		"-------End File Stuff--------\n"
		);
}

size_t InitFileSelection()
{
	size_t FileSelection=g_FileStuff->DefaultSelection;
	if (!InputFileName)
		InputFileName=g_FileStuff->inputfile[FileSelection];
	return FileSelection;
}

bool HandleFileSelectionCommands(size_t &FileSelection,const char *input_line,const char *str_1)
{
	bool ret=true;
	if (!_strnicmp( input_line, "SelectGroup", 11))
	{
		long GroupSelection=min(Hex2Long(str_1),(long)(c_NoGroups-1));
		g_FileStuff=c_Groups[GroupSelection];
		FileSelection=g_FileStuff->DefaultSelection;
		ShowFileSelections();
		cout << endl;
		InputFileName=g_FileStuff->inputfile[FileSelection];
		ShowCurrentSelection();
	}
	else if (!_strnicmp( input_line, "Sel", 3))
	{
		g_FileStuff->DefaultSelection=FileSelection=Hex2Long(str_1);
		InputFileName=g_FileStuff->inputfile[FileSelection];
		ShowCurrentSelection();
	}
	else if (!_strnicmp( input_line, "SetFileName", 11))
	{
		strcpy(ManualFilename,str_1);
		InputFileName=ManualFilename;
		printf("FileName = %s\n",InputFileName);
	}
	else if (!_strnicmp( input_line, "ShowFiles", 9))
		ShowFileSelections();
	else if (!_strnicmp( input_line, "cls", 3))
		cls();
	else
		ret=false;
	return ret;
}

void SetGroupSelection(string GroupID,size_t Selection)
{
	for (size_t i=0;i<c_NoGroups;i++)
	{
		if (strcmp(c_Groups[i]->GroupID,GroupID.c_str())==0)
		{
			g_FileStuff=c_Groups[i];
			g_FileStuff->DefaultSelection=Selection;
			break;
		}
	}
}

size_t LoadDefaults(const char *DefaultFileName,size_t *BuildEnvironment)
{
	size_t DefaultConsole=0;
	//Load saved selection
	string InFile = string(c_outpath) + DefaultFileName;
	std::ifstream in(InFile.c_str(), std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		string GroupID;
		size_t Selection;
		size_t l_BuildEnvironment=0;
		bool UsedManualFileName=false;
		//wow this is neat
		in>>GroupID>>Selection>>DefaultConsole>>l_BuildEnvironment>>UsedManualFileName;
		if (BuildEnvironment)
			*BuildEnvironment=l_BuildEnvironment;
		//find the group
		SetGroupSelection(GroupID,Selection);
		if (UsedManualFileName)
		{
			string FileName;
			in>>FileName;
			strcpy(ManualFilename,FileName.c_str());
			if (ManualFilename[0]!=0)
				InputFileName=ManualFilename;
		}
		in.close();
	}
	return DefaultConsole;
}

void SaveDefaults(const char *DefaultFileName,size_t DefaultConsole,size_t BuildEnvironment)
{
	//Save selections
	string OutFile = string(c_outpath) + DefaultFileName;
	ofstream out(OutFile.c_str(), std::ios::out | std::ios::binary);
	out.write(g_FileStuff->GroupID,(streamsize)(strlen(g_FileStuff->GroupID)+1));
	out << endl;
	out << g_FileStuff->DefaultSelection << endl;
	out << DefaultConsole << endl;
	out << BuildEnvironment << endl;
	bool UsedManualFileName=(InputFileName==ManualFilename);
	out << UsedManualFileName << endl;
	if (UsedManualFileName)
	{
		out.write(InputFileName,(streamsize)(strlen(InputFileName)+1));
		out << endl;
	}
	out.close();
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