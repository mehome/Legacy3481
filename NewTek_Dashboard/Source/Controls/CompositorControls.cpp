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

class MenuSelection_Compositor : public MenuSelection_Interface
{
	virtual void Initialize(HWND pParent,Plugin_Controller_Interface *plugin)
	{
		g_plugin_Composite=dynamic_cast<Plugin_Compositor_Interface *>(plugin);
	}

	enum MenuSelection
	{
		eMenu_Editable,
		eMenu_NoEntries
	};

	virtual size_t AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
	{
		InsertMenu(hPopupMenu, -1, ( g_plugin_Composite->GetIsEditable()?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Editable+StartingOffset, L"Edit Position"); 
		return 1;
	}

	virtual void On_Selection(int selection,HWND pParent)
	{
		switch (selection)
		{
		case eMenu_Editable:
			g_plugin_Composite->SetIsEditable(!g_plugin_Composite->GetIsEditable());
			break;
		}
	}
} g_MenuSelection_Compositor_Instance;

MenuSelection_Interface *g_MenuSelection_Compositor=&g_MenuSelection_Compositor_Instance;
