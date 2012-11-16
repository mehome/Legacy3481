/***************************************************************
Module name: HookSpyDll.h
Copyright (c) 2003 Robert Kuster

Notice:	If this code works, it was written by Robert Kuster.
		Else, I don't know who wrote it.

		Use it on your own risk. No responsibilities for
		possible damages of even functionality can be taken.
***************************************************************/
#if !defined HOOKSPY_DLL_H
#define HOOKSPY_DLL_H


#ifdef HOOKSPY_DLL_EXPORTS
#define HOOKDLL_API __declspec(dllexport)
#else
#define HOOKDLL_API __declspec(dllimport)
#endif

///returns true if successful
HOOKDLL_API int BindToProcess(HWND hWnd);
HOOKDLL_API int ReleaseProcess();


#endif // !defined(HOOKSPY_DLL_H)