#pragma once

#include "stdafx.h"
#include "Controls.h"
#include "Resource.h"


extern Dashboard_Controller_Interface *g_Controller;
extern MenuSelection_Interface *g_MenuSelection_Vision;
extern const char *csz_Plugin_SquareTargeting;
extern const char *csz_Plugin_Compositor;


class Plugin_Compositor_Interface : public Plugin_Controller_Interface
{
public:
	virtual void SetIsEditable(bool Edit)=0;
	virtual bool GetIsEditable() const=0;
	virtual Plugin_Controller_Interface* GetBypassPluginInterface(void)=0;
	//These (like the interface itself must match what is written in the compositor)
	enum ReticleType
	{
		eNone,
		eDefault,
		ePathAlign,
		eShape3D,
		eComposite,
		eBypass,
		eLinePlot
	};
	virtual ReticleType GetCurrentReticalType() const=0;
	virtual void SetStepIntoComposite(bool enableRecursiveStep)=0;
	virtual bool GetStepIntoComposite() const=0;
};


class MenuSelection_Compositor : public MenuSelection_Interface
{
private:
	Plugin_Compositor_Interface *m_plugin_Composite;
	Plugin_Compositor_Interface::ReticleType m_ReticleType;   //cache to ensure the item selection matches what was populated during the menu generation

protected:
	virtual void Initialize(HWND pParent,Plugin_Controller_Interface *plugin)
	{
		m_plugin_Composite=dynamic_cast<Plugin_Compositor_Interface *>(plugin);
		Plugin_Controller_Interface *bypass_plugin=m_plugin_Composite->GetBypassPluginInterface();
		if ((bypass_plugin)&&(strcmp(bypass_plugin->GetPlugInName(),csz_Plugin_SquareTargeting)==0))
			g_MenuSelection_Vision->Initialize(pParent,bypass_plugin);
	}

	enum MenuSelection
	{
		eMenu_Editable,
		eMenu_EditComposite,
		eMenu_NoEntries
	};

	virtual size_t AddMenuItems (HMENU hPopupMenu,size_t StartingOffset)
	{
		m_ReticleType=m_plugin_Composite->GetCurrentReticalType();
		size_t ret=0;
		if (m_ReticleType!=Plugin_Compositor_Interface::eBypass)
		{
			InsertMenu(hPopupMenu, -1, ( m_plugin_Composite->GetIsEditable()?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_Editable+StartingOffset, L"Edit Position"); 
			InsertMenu(hPopupMenu, -1, ( !m_plugin_Composite->GetStepIntoComposite()?MF_CHECKED:MF_UNCHECKED) | MF_BYPOSITION | MF_STRING, eMenu_EditComposite+StartingOffset, L"Group Edit"); 
			ret=eMenu_NoEntries;
		}
		else
		{
			Plugin_Controller_Interface *bypass_plugin=m_plugin_Composite->GetBypassPluginInterface();
			if (bypass_plugin)  //plugin can choose whether or not to have this
			{
				if (strcmp(bypass_plugin->GetPlugInName(),csz_Plugin_SquareTargeting)==0)
					ret+=g_MenuSelection_Vision->AddMenuItems(hPopupMenu,StartingOffset+ret);
			}
		}
		return ret;
	}

	virtual void On_Selection(int selection,HWND pParent)
	{
		if (m_ReticleType!=Plugin_Compositor_Interface::eBypass)
		{
			switch (selection)
			{
			case eMenu_Editable:
				m_plugin_Composite->SetIsEditable(!m_plugin_Composite->GetIsEditable());
				break;
			case eMenu_EditComposite:
				m_plugin_Composite->SetStepIntoComposite(!m_plugin_Composite->GetStepIntoComposite());
				break;
			}
		}
		else
		{
			Plugin_Controller_Interface *bypass_plugin=m_plugin_Composite->GetBypassPluginInterface();
			if (bypass_plugin)  //plugin can choose whether or not to have this
			{
				if (strcmp(bypass_plugin->GetPlugInName(),csz_Plugin_SquareTargeting)==0)
					g_MenuSelection_Vision->On_Selection(selection,pParent);
			}
		}
	}
} g_MenuSelection_Compositor_Instance;

MenuSelection_Interface *g_MenuSelection_Compositor=&g_MenuSelection_Compositor_Instance;
