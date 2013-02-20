// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "NI_VisionProcessingBase.h"

VisionTracker* g_pTracker = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_pTracker;
		break;
	}

	return TRUE;
}

