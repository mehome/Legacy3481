#include "StdAfx.h"

//**** The DLL Entry Point ****************************************************************
BOOL WINAPI DllMain(HINSTANCE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{	switch(ul_reason_for_call)
	{								//**************************************************************************
		case DLL_PROCESS_ATTACH:	// Perform any initialisation here
			VideoEditor::OnProcessAttach();
									return true;

									//**************************************************************************
		case DLL_PROCESS_DETACH:	// Perform any destruction here
			VideoEditor::OnProcessDetach();
									return true;

									//**************************************************************************
		default:					return true;
	}

	// Return Success
	return true;
}


NewTek_RegisterPluginControl(TimeLine_Editor);
NewTek_RegisterPluginControl(StoryBoard_Editor);
NewTek_RegisterPluginControl(SplitView);
NewTek_RegisterPluginControl(SplitView_Control);
NewTek_RegisterPluginControl(VideoEditor);

NewTek_RegisterPluginControl(InOuts2_Control);
NewTek_RegisterPluginControl(InOuts2_VideoPreviewWindow);
NewTek_RegisterPluginType(InOuts2_VideoPreviewWindow,Control,James Killian,(c)2000 NewTek,1);
NewTek_RegisterPluginControl(InOuts2_MoneyShotPreview);
NewTek_RegisterPluginType(InOuts2_MoneyShotPreview,Control,James Killian,(c)2000 NewTek,1);
