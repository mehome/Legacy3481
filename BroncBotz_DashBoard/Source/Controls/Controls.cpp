#include "stdafx.h"
#include "Controls.h"

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

static void DebugOutput(const char *format, ... )
{	char Temp[2048];
va_list marker;
va_start(marker,format);
vsprintf(Temp,format,marker);
OutputDebugStringA(Temp);
va_end(marker);		
}


enum MenuSelection
{
	eMenu_NoSelection,
	eMenu_Controls,
	eMenu_NoEntries
};

Dashboard_Controller_Interface *g_Controller=NULL;

extern "C" CONTROLS_API void Callback_SmartCppDashboard_Initialize (Dashboard_Controller_Interface *controller)
{
	g_Controller=controller;
}

extern "C" CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
{
	InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, eMenu_NoSelection, NULL);
	InsertMenu(hPopupMenu, -1, MF_BYPOSITION | MF_STRING, eMenu_Controls+StartingOffset, L"File Controls...");
}


extern "C" CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection)
{
	DebugOutput("Selection=%d\n",selection);
	g_Controller->Pause();
	//printf("\n");
}
