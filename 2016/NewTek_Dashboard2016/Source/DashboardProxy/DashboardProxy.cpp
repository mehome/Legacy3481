// DashboardProxy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlbase.h>
#include <shellapi.h>
#include <strsafe.h>

#pragma comment (lib,"shell32")


int _tmain(int argc, _TCHAR* argv[])
{
	SetCurrentDirectory(L"C:/Program Files (x86)/FRC Dashboard");
	std::wstring FileName=L"D:/Stuff/BroncBotz/Code/2014/2014Dashboard/Dashboard.exe";
	std::string InFile = "Dashboard_Path.ini";
	std::ifstream in(InFile.c_str());
	if (in.is_open())
	{
		std::string TempBuffer;
		in>>TempBuffer;
		char2wchar(TempBuffer.c_str());
		FileName=char2wchar_pwchar;
	}
	HINSTANCE test=ShellExecute(NULL,L"open",FileName.c_str(),NULL,NULL,SW_SHOWNORMAL);
	return 0;
}

