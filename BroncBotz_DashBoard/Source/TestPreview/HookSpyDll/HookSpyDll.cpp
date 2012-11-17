/***************************************************************
Module name: HookSpyDll.cpp
Copyright (c) 2003 Robert Kuster

Notice:	If this code works, it was written by Robert Kuster.
		Else, I don't know who wrote it.

		Use it on your own risk. No responsibilities for
		possible damages of even functionality can be taken.
***************************************************************/

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "HookSpyDll.h"


//-------------------------------------------------------
// shared data 
// Notice:	seen by both: the instance of "HookSpy.dll" maped
//			into the remote process as well as by the instance
//			of "HookSpy.dll" mapped into our "HookSpy.exe"
#pragma data_seg (".shared")
int		g_bSubclassed = 0;	// START button subclassed?
UINT	WM_HOOKEX = 0;
HWND	g_hWnd	= 0;		// control containing the password
HHOOK	g_hHook = 0;
#pragma data_seg ()

#pragma comment(linker,"/SECTION:.shared,RWS") 


//-------------------------------------------------------
// global variables (unshared!)
//
HINSTANCE hDll;

// New & old window procedure of the subclassed App
WNDPROC				OldProc = NULL;	
LRESULT CALLBACK	NewProc( HWND,UINT,WPARAM,LPARAM );

//-------------------------------------------------------
// DllMain
//
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if( ul_reason_for_call == DLL_PROCESS_ATTACH ) 
	{
		hDll = (HINSTANCE) hModule;	
		::DisableThreadLibraryCalls( hDll );

		if( WM_HOOKEX==NULL )
			WM_HOOKEX = ::RegisterWindowMessage( "WM_HOOKEX_RK" );			
	}

	return TRUE;
}


//-------------------------------------------------------
// HookProc
// Notice: - executed by the "remote" instance of "HookSpy.dll";
//		   - unhooks itself right after it gets the password;
//
#define pCW ((CWPSTRUCT*)lParam)

LRESULT HookProc (
  int code,       // hook code
  WPARAM wParam,  // virtual-key code
  LPARAM lParam   // keystroke-message information
)
{	
	//if( pCW->message == WM_HOOKSPY ) {
	//	//::MessageBeep(MB_OK);
	//	//::SendMessage( g_hWnd,WM_GETTEXT,128,(LPARAM)g_szPassword );
	//	::UnhookWindowsHookEx( g_hHook );
	//}
	#if 0
	CWPSTRUCT *CWPmessage=(CWPSTRUCT *)lParam;
	if (CWPmessage)
	{
		printf("hwnd=%x msg=%d l=%x w=%x\n",CWPmessage->hwnd,CWPmessage->message,CWPmessage->lParam,CWPmessage->wParam);
		switch (CWPmessage->message)
		{
			case WM_CLOSE:
				printf("WM_Close\n");
				break;
			case WM_DESTROY:
				printf("WM_Destroy\n");
				break;
		}
	}
	else
		printf("No CWP message -> %d %x %x\n",code,wParam,lParam);
	#endif
	//return ::CallNextHookEx(g_hHook, code, wParam, lParam);

	if( (pCW->message == WM_HOOKEX) && pCW->lParam ) 
	{
		::UnhookWindowsHookEx( g_hHook );

		if( g_bSubclassed ) 
			goto END;		// already subclassed?

		// Let's increase the reference count of the DLL (via LoadLibrary),
		// so it's NOT unmapped once the hook is removed;
		char lib_name[MAX_PATH]; 
		::GetModuleFileName( hDll, lib_name, MAX_PATH );

		if( !::LoadLibrary( lib_name ) )
			goto END;		

		// Subclass START button
		OldProc = (WNDPROC) 
			::SetWindowLong( g_hWnd, GWL_WNDPROC, (long)NewProc );
		if( OldProc==NULL )			// failed?
			::FreeLibrary( hDll );
		else {						// success -> leave "HookInjEx.dll"
			::MessageBeep(MB_OK);	// mapped into "explorer.exe"
			g_bSubclassed = true;
		}		
	}
	else if( pCW->message == WM_HOOKEX ) 
	{
		::UnhookWindowsHookEx( g_hHook );

		// Failed to restore old window procedure? => Don't unmap the
		// DLL either. Why? Because then "explorer.exe" would call our
		// "unmapped" NewProc and  crash!!
		if( !SetWindowLong( g_hWnd, GWL_WNDPROC, (long)OldProc ) )
			goto END;

		::FreeLibrary( hDll );

		::MessageBeep(MB_OK);
		g_bSubclassed = false;	
	}

END:
	return ::CallNextHookEx(g_hHook, code, wParam, lParam);

}

LRESULT CALLBACK NewProc(
						 HWND hwnd,      // handle to window
						 UINT uMsg,      // message identifier
						 WPARAM wParam,  // first message parameter
						 LPARAM lParam   // second message parameter
						 )
{
	//switch (uMsg) 
	//{
	//case WM_LBUTTONDOWN: uMsg = WM_RBUTTONDOWN; break;
	//case WM_LBUTTONUP:	 uMsg = WM_RBUTTONUP;	break;

	//case WM_RBUTTONDOWN: uMsg = WM_LBUTTONDOWN; break;
	//case WM_RBUTTONUP:   uMsg = WM_LBUTTONUP;	break;

	//case WM_LBUTTONDBLCLK: uMsg = WM_RBUTTONDBLCLK; break;
	//case WM_RBUTTONDBLCLK: uMsg = WM_LBUTTONDBLCLK; break;
	//}
	printf("hwnd=%x msg=%d w=%x l=%x\n",hwnd,uMsg,wParam,lParam);

	return CallWindowProc( OldProc,hwnd,uMsg,wParam,lParam );
}


//-------------------------------------------------------
// GetWindowTextRemote
// Notice: - injects "HookSpy.dll" into the remote process 
//			 (via SetWindowsHookEx);
//		   - gets the password from the remote edit control;
//
//	Return value: - number of characters retrieved 
//					by remote WM_GETTEXT;
//
int BindToProcess(HWND hWnd)
{	
	g_hWnd = hWnd;
	DWORD TID=GetWindowThreadProcessId(hWnd,NULL);
	g_hHook = SetWindowsHookEx( WH_CALLWNDPROC,(HOOKPROC)HookProc,hDll, TID );
	printf("result=%p\n",g_hHook);
	bool ret=( g_hHook!=NULL ) ;

	if( g_hHook!=NULL ) 
	{
		SendMessage( hWnd,WM_HOOKEX,0,1 );
	}
	else
		g_bSubclassed=0;

	return g_bSubclassed;
}

int ReleaseProcess()
{
	g_hHook = SetWindowsHookEx( WH_CALLWNDPROC,(HOOKPROC)HookProc,
		hDll, GetWindowThreadProcessId(g_hWnd,NULL) );

	if( g_hHook==NULL )
		return 0;	

	assert(g_hWnd);
	if (g_hWnd)
		SendMessage( g_hWnd,WM_HOOKEX,0,0 );

	return (g_bSubclassed == NULL);
}