#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"


extern Dashboard_Controller_Interface *g_Controller;
extern MenuSelection_Interface *g_MenuSelection_Vision;
extern const char *csz_Plugin_SquareTargeting;
extern const char *csz_Plugin_Compositor;

//TODO see about disabling edit position when sequence is on bypass (low priority)
class Plugin_Compositor_Interface : public Plugin_Controller_Interface
{
public:
	virtual void SetIsEditable(bool Edit)=0;
	virtual bool GetIsEditable() const=0;
	virtual Plugin_Controller_Interface* GetBypassPluginInterface(void)=0;
} *g_plugin_Composite;

class MenuSelection_Compositor : public MenuSelection_Interface
{
	virtual void Initialize(HWND pParent,Plugin_Controller_Interface *plugin)
	{
		g_plugin_Composite=dynamic_cast<Plugin_Compositor_Interface *>(plugin);
		Plugin_Controller_Interface *bypass_plugin=g_plugin_Composite->GetBypassPluginInterface();
		if (strcmp(bypass_plugin->GetPlugInName(),csz_Plugin_SquareTargeting)==0)
			g_MenuSelection_Vision->Initialize(pParent,bypass_plugin);
	}

	enum MenuSelection
	{
		eMenu_Editable,
		eMenu_NoEntries
	};

	virtual size_t AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
	{
		size_t ret=0;
		InsertMenu(hPopupMenu, -1, ( g_plugin_Composite->GetIsEditable()?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Editable+StartingOffset, L"Edit Position"); 
		ret=eMenu_NoEntries;
		Plugin_Controller_Interface *bypass_plugin=g_plugin_Composite->GetBypassPluginInterface();
		if (strcmp(bypass_plugin->GetPlugInName(),csz_Plugin_SquareTargeting)==0)
			ret+=g_MenuSelection_Vision->AddMenuItems(hPopupMenu,StartingOffset+ret);
		return ret;
	}

	virtual void On_Selection(int selection,HWND pParent)
	{
		switch (selection)
		{
		case eMenu_Editable:
			g_plugin_Composite->SetIsEditable(!g_plugin_Composite->GetIsEditable());
			break;
		default:
			if (selection>eMenu_Editable)
			{
				Plugin_Controller_Interface *bypass_plugin=g_plugin_Composite->GetBypassPluginInterface();
				if (strcmp(bypass_plugin->GetPlugInName(),csz_Plugin_SquareTargeting)==0)
					g_MenuSelection_Vision->On_Selection(selection-eMenu_NoEntries,pParent);
			}
		}
	}
} g_MenuSelection_Compositor_Instance;

MenuSelection_Interface *g_MenuSelection_Compositor=&g_MenuSelection_Compositor_Instance;
