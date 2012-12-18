#include "StdAfx.h"

NewTek_RegisterPluginControl(StoryBoard);
NewTek_RegisterPluginControl(StoryBoard_View);
NewTek_RegisterPluginControl(StoryBoard_Crouton);
NewTek_RegisterPluginControl(StoryBoard_Crouton_NoOwn);
NewTek_RegisterPluginClass	(StoryBoard_Item);
NewTek_RegisterPluginControl(StoryBoardPath);
NewTek_RegisterPluginControl(InOuts_Control);
NewTek_RegisterPluginControl(VideoPreviewWindow);
NewTek_RegisterPluginControl(InOuts_Control_Default);

HINSTANCE h_sb_HINSTANCE;

//**** The DLL Entry Point ****************************************************************
BOOL WINAPI DllMain(HINSTANCE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{	switch(ul_reason_for_call)
	{								//**************************************************************************
		case DLL_PROCESS_ATTACH:	//CoInitialize(NULL);
									h_sb_HINSTANCE=hInst;
									StoryBoard_Init();
									StoryBoard_Crouton::StoryBoard_Crouton_LocalCroutonBitmap();									
									return true;

									//**************************************************************************
		case DLL_PROCESS_DETACH:	StoryBoard_Free();
									//CoUninitialize();
									h_sb_HINSTANCE=NULL;
									return true;

									//**************************************************************************
		default:					return true;
	}

	// Return Success
	return true;
}

