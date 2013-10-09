#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"


extern Dashboard_Controller_Interface *g_Controller;

class Plugin_Compositor_Interface : public Plugin_Controller_Interface
{
public:
	virtual void SetIsEditable(bool Edit)=0;
	virtual bool GetIsEditable() const=0;
} *g_plugin_Composite;

void Compositor_Initialize(HWND pParent,Plugin_Controller_Interface *plugin)
{
	g_plugin_Composite=dynamic_cast<Plugin_Compositor_Interface *>(plugin);
}

enum MenuSelection
{
	eMenu_Editable,
	eMenu_NoEntries
};

size_t Compositor_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
{
	InsertMenu(hPopupMenu, -1, ( g_plugin_Composite->GetIsEditable()?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Editable+StartingOffset, L"Edit Position"); 
	return 1;
}

void Compositor_On_Selection(int selection,HWND pParent)
{
	switch (selection)
	{
	case eMenu_Editable:
		g_plugin_Composite->SetIsEditable(!g_plugin_Composite->GetIsEditable());
		break;
	}
}
