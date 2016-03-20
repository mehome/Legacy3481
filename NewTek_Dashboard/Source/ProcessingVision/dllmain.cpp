// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ProcessingVision.h"
#include "OCV_VisionProcessingBase.h"

VisionTracker* g_pTracker[eNumTrackers];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		memset( g_pTracker, 0, sizeof(VisionTracker*) * eNumTrackers);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		for( int i = 0; i < eNumTrackers; i++)
			delete g_pTracker[i];
		break;
	}

	return TRUE;
}

