// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//Module::Surface::g_h_module = hModule;
		 //This can reduce the size of the working set for some applications
		DisableThreadLibraryCalls(hModule);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;		
	case DLL_PROCESS_DETACH:
		//Module::Surface::g_h_module = NULL;
		break;
	}

	return TRUE;
}
