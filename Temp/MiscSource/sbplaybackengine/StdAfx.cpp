#include "StdAfx.h"

HINSTANCE	h_SB_HINSTANCE;
CPU_Limiter GlobalCPULimiter;

//**** The DLL Entry Point ****************************************************************
BOOL WINAPI DllMain(HINSTANCE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{	switch(ul_reason_for_call)
	{								//**************************************************************************
		case DLL_PROCESS_ATTACH:	h_SB_HINSTANCE=hInst;
									g_BufferCache_Init(true);
									DriveItems_Cache_Init(true);
									DisableThreadLibraryCalls(hInst);
									return true;

									//**************************************************************************
		case DLL_PROCESS_DETACH:	h_SB_HINSTANCE=NULL;
									g_BufferCache_Init(false);
									DriveItems_Cache_Init(false);
									return true;

									//**************************************************************************
		default:					return true;
	}

	// Return Success
	return true;
}

