#include "HandleMain.h"
#include "newtek/RTVlib.h"

#pragma comment(lib, "dxguid")
#pragma comment(lib, "ddraw")
#pragma comment(lib, "dsound")
#pragma comment(lib, "dplayx")
#pragma comment(lib, "dinput")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "msimg32")
#pragma comment(lib, "vfw32")
#pragma comment(lib, "version")

// Globals
HMENU m_ptr;
toasterclass *toaster;
controlsclass *controls;
previewclass *preview;
captureclass *capture;
dv2rtvclass *dv2rtv;
HBITMAP skinbmp;
HBITMAP skin2bmp;
HBITMAP Exodusbmp;
HANDLE arrayofevents[EVENT_MAXEVENTS];
HWND About;
static HANDLE streamthread=NULL;
static HANDLE previewthread=NULL;
int debug=FALSE;

BOOL killstreamthread=FALSE;
BOOL killpreviewthread=FALSE;

UWORD streamthreadmode=0;

//End Globals

static consoleclass con;

/* Global Vars */
char string[256];  //Temporary for debug
HINSTANCE hInst;   /* current instance */
HWND screen;
LPCTSTR lpszAppName  = "VTDVKIT";
LPCTSTR lpszIcon  = "EXODUS";
LPCTSTR lpszTitle    = "Video Toaster - DV capture kit";
LPINITCOMMONCONTROLSEX initctrls;
CRITICAL_SECTION csglobal;

struct memlist *pmem=NULL; 
struct nodevars *nodeobject=NULL;

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,int nCmdShow) {
	MSG msg;
	WNDCLASS wc;

	InitializeCriticalSection(&csglobal);
   /* screen's, contents */
	wc.style         = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc   = WNDPROC(handlemain);       
	wc.cbClsExtra    = 0;                      
	wc.cbWndExtra    = 0;                      
	wc.hInstance     = hInstance;              
	wc.hIcon         = LoadIcon(hInstance,lpszIcon); 
	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	//wc.hbrBackground = NULL;
	wc.lpszMenuName  = lpszAppName;              
	wc.lpszClassName = lpszAppName;              
   
	if (!RegisterClass(&wc)) return(FALSE);
	
   hInst = hInstance;
	nodeobject=createnode(&pmem,65535,0);

	/*child popup contents*/
	wc.style         = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_PARENTDC; 
   wc.lpfnWndProc   = (WNDPROC)gObjCallback;
	wc.cbClsExtra    = sizeof(WORD);                           
	wc.cbWndExtra    = 0;                           
	wc.hInstance     = hInst;                   
	wc.hIcon         = LoadIcon(hInst, lpszIcon);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 

	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = "OBJWIN";                   
	wc.lpszClassName = "OBJWIN";

	if (!RegisterClass(&wc)) return(FALSE);

	wc.hbrBackground = (HBRUSH)(COLOR_MENU + 1);  
	wc.lpszMenuName  = "OBJGRAYWIN";                   
	wc.lpszClassName = "OBJGRAYWIN";

	if (!RegisterClass(&wc)) return(FALSE);

   /* Open application screen "parent window" */
	screen=CreateWindowEx(0,lpszAppName,lpszTitle,
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
		400,100,350,150,
		NULL,NULL,hInstance,NULL);

   if (!screen) 
      return(FALSE);
   //ShowWindow(screen,nCmdShow);
	ShowWindow(screen,SW_HIDE);

   UpdateWindow(screen);
	//Resize our windows here
	resizewindows();
	initchildren();
	// Open Libs, and other init stuff
	/*
	if (!(InitCommonControlsEx(initctrls))) {
		error("Unable to open the common control dynamic-link library",0);
		}
	*/
	// TODO research why the InitCommonControlsEx doesn't return true
	InitCommonControls();
	sleep(&msg);
   return(msg.wParam); 
	}

void sleep (MSG *msg) {
	while(GetMessage(msg,NULL,0,0))   {
		TranslateMessage(msg); 
		DispatchMessage(msg);  
		}
	}

LRESULT CALLBACK handlemain(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	DWORD streamthreadretid;
	DWORD previewthreadretid;
	UBYTE t;

	switch (uMsg) {
		case WM_CREATE:
			m_ptr=GetMenu(w_ptr);
			/*Go Ahead and Check all pre opened windows here*/
			CheckMenuItem(m_ptr,IDM_CAPTOASTER,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_DV2RTV,MF_BYCOMMAND|MFS_CHECKED);
			//CheckMenuItem(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_CON,MF_BYCOMMAND|MFS_CHECKED);

			//I'd better set up the toaster before I open the threads
			toaster=new toasterclass();
			if (!toaster->videooff)	CheckMenuItem(m_ptr,IDM_VIDEO,MF_BYCOMMAND|MFS_CHECKED);
			controls=new controlsclass();
			preview=new previewclass();
			capture=new captureclass();
			dv2rtv=new dv2rtvclass();

			//This Application will be a stand alone so
			//create events for all our threads
			for (t=0;t<EVENT_MAXEVENTS;t++) {
				arrayofevents[t] = CreateEvent(
					NULL,   // no misc attributes
					FALSE,  // auto-reset event object
					FALSE,  // initial state is nonsignaled
					NULL);  // unnamed object
				if (arrayofevents[t]==NULL) {
					error(0,"CreateEvent error: %d\n", GetLastError());
					}
				}
			streamthread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				streamthreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&streamthreadretid); // returns the thread identifier 
			if (streamthread==NULL) error(0,"CreateThread() failed.");
			//For now the stream thread has normal priority
			previewthread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				previewthreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&previewthreadretid); // returns the thread identifier 
			if (previewthread==NULL) error(0,"CreateThread() failed.");
			//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);

			//Load our graphic skins for all objects in app
			skinbmp=(HBITMAP)LoadImage(hInst,"Resources\\Skins.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			skin2bmp=(HBITMAP)LoadImage(hInst,"Resources\\Skins2.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
			Exodusbmp=(HBITMAP)LoadImage(hInst,"IDB_BITMAP1",IMAGE_BITMAP,0,0,0);

			con.createwindow(w_ptr);
			printc("* * * Console Interface * * *");
			printc(" ");
			controls->createwindow(w_ptr);
			preview->createwindow(w_ptr);
			capture->createwindow(w_ptr);
			dv2rtv->createwindow(w_ptr);
			break;

		case WM_SYSCOMMAND:
			if (wParam==SC_MINIMIZE) preview->hideoverlay();
			if (wParam==SC_RESTORE) preview->showoverlay();
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDM_TEST:
					printc("Test");
					if (toaster->rtmeoutput) toaster->initmasterframe(NULL);
					break;

				/* * * * PROJECT * * * */

				case IDM_EXIT:
					DestroyWindow(screen);
					break;
				/* * * * VIEW * * * */
				case IDM_CONTROLS:
					windowtoggle(screen,controls->window,IDM_CONTROLS);
					break;
				case IDM_VIDEO: {
					UINT isitchecked=GetMenuState(m_ptr,IDM_VIDEO,MF_BYCOMMAND);
					if (toaster->videooff=(isitchecked&MFS_CHECKED)) {
						CheckMenuItem(m_ptr,IDM_VIDEO,MF_BYCOMMAND|MFS_UNCHECKED);
						toaster->videobuf=mynew(&pmem,691200);
						}
					else {
						if (toaster->rtmeoutput) {
							if (toaster->videobuf) dispose((struct memlist *)toaster->videobuf,&pmem);
							CheckMenuItem(m_ptr,IDM_VIDEO,MF_BYCOMMAND|MFS_CHECKED);
							}
						}
					break;
					}
				case IDM_PREVWINDOW: {
					UINT isitchecked=GetMenuState(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND);
					if (preview->previewoff=(isitchecked&MFS_CHECKED)) {
						CheckMenuItem(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND|MFS_UNCHECKED);
						if (debug) printc("Preview Window is now Off");
						preview->hideoverlay();
						ShowWindow(preview->prevwindow,SW_HIDE);
						}
					else {
						CheckMenuItem(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND|MFS_CHECKED);
						if (debug) printc("Preview Window on");
						preview->showoverlay();
						ShowWindow(preview->prevwindow,SW_SHOW);
						}
					break;
					} //end preview window
				case IDM_CAPTURE:
					if (GetMenuState(m_ptr,IDM_CAPTURE,MF_BYCOMMAND)) capture->hidedisplay();
					else {
						if (controls->mycontrolis&8) capture->showdisplay();
						else printc("Please use record button to enter capture mode");
						}
					break;

				case IDM_DV2RTV:
					windowtoggle(screen,dv2rtv->window,IDM_DV2RTV);
					break;

				case IDM_CON:
					windowtoggle(screen,con.consolewindow,IDM_CON);
		 			break;
				/* End All Window Toggles */
				case IDM_CLEARCON:
					con.clearconsole();
					break;
				/* * * * OPTIONS * * */
				case IDM_CAPTOASTER:
					CheckMenuItem(m_ptr,IDM_CAPTOASTER,MF_BYCOMMAND|MFS_CHECKED);
					CheckMenuItem(m_ptr,IDM_CAPOTHER,MF_BYCOMMAND|MFS_UNCHECKED);
					capture->choosedevice(0);
					break;
				case IDM_CAPOTHER:
					CheckMenuItem(m_ptr,IDM_CAPOTHER,MF_BYCOMMAND|MFS_CHECKED);
					CheckMenuItem(m_ptr,IDM_CAPTOASTER,MF_BYCOMMAND|MFS_UNCHECKED);
					capture->choosedevice(1);
					break;
				/* * * * HELP * * * */
				case IDM_ABOUT:
					DialogBox(hInst,"AboutBox",w_ptr,DLGPROC(handleabout));
					break;
				case IDM_DEBUG: {
					UINT isitchecked=GetMenuState(m_ptr,IDM_DEBUG,MF_BYCOMMAND);
					if (isitchecked & MFS_CHECKED) {
						CheckMenuItem(m_ptr,IDM_DEBUG,MF_BYCOMMAND|MFS_UNCHECKED);
						printc("Debug mode is now Off");
						debug=0;
						}
					else {
						CheckMenuItem(m_ptr,IDM_DEBUG,MF_BYCOMMAND|MFS_CHECKED);
						printc("Debug mode Activated to level 1");
						debug=1;
						}
					break;
					} //End Debug
				} //end switch low parm of command
			break;  //end command

		case WM_MOVE:
		case WM_SIZE:
			resizewindows();
			return(0);

		case WM_CLOSE:
			ShowWindow(screen,SW_HIDE);
			return(0L);

		case WM_DESTROY:
			cleanup();
			PostQuitMessage(0);
			break;

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		} /* End switch uMsg */
	return(0L);
	}


void cleanup() {
	UBYTE t;

	//First signal all child threads to end
	dv2rtv->playing=FALSE;
	killstreamthread=
	killpreviewthread=TRUE;
	for (t=0;t<EVENT_MAXEVENTS;t++) if(arrayofevents[t]) SetEvent(arrayofevents[t]);
	//wait for all child threads to succesfully close
	//and then close the handles of the threads
	if (streamthread) {
		WaitForSingleObject(streamthread,INFINITE);
		CloseHandle(streamthread);
		}
	if (previewthread) {
		WaitForSingleObject(previewthread,INFINITE);
		CloseHandle(previewthread);
		}
	//Now to close the events
	for (t=0;t<EVENT_MAXEVENTS;t++) CloseHandle(arrayofevents[t]);

	//end thread stuff
	preview->shutdown();
	controls->shutdown();
	capture->shutdown();
	DestroyWindow(dv2rtv->window);
	delete dv2rtv;
	delete capture;
	delete toaster;
	delete preview;
	delete controls;
	if (Exodusbmp) DeleteObject(Exodusbmp);
	if (skin2bmp) DeleteObject(skin2bmp);
	if (skinbmp) DeleteObject(skinbmp);

	if (pmem) killnode(nodeobject,&pmem);
	if (pmem) disposeall(&pmem);
	if (IsWindowVisible(screen)) DestroyWindow(screen);
	DeleteCriticalSection(&csglobal);
	}


LRESULT CALLBACK handleabout(HWND req_ptr,UINT message,WPARAM wParam,LPARAM lParam) {
   char params[]="";
   char dr[]="";
	switch (message) {
		case WM_INITDIALOG: {
			DWORD versionsize,versiondummy;
			void *PrivateBuild,*SpecialBuild,*ProductName,*LegalCopyright;
			UINT versionbufsize;
			char *version,*exfile="VTNT_DV_kit.exe";
			//RECT rc;
			//int t;
	
			versionsize=GetFileVersionInfoSize(exfile,&versiondummy);
			version=(char *)newnode(nodeobject,versionsize);
			GetFileVersionInfo(exfile,0,versionsize,version);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\PrivateBuild",&PrivateBuild,&versionbufsize);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\ProductName",&ProductName,&versionbufsize);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\SpecialBuild",&SpecialBuild,&versionbufsize);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\LegalCopyright",&LegalCopyright,&versionbufsize);
			wsprintf(string,"%s\n%s-%s %s",(char *)ProductName,
				(char *)PrivateBuild,(char *)SpecialBuild,(char*)LegalCopyright);
			disposenode(nodeobject,(struct memlist *)version);
			SetWindowText(GetDlgItem(req_ptr,IDC_SECURITY),string);
			return (TRUE);
			}

		case WM_DRAWITEM: {
			RECT rc;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;

			lpdis=(LPDRAWITEMSTRUCT) lParam;
			//printc("WM_DRAWITEM");
			if (lpdis->itemID==(UINT)-1) return(TRUE);
			switch (lpdis->itemAction)  {
				case ODA_SELECT:
					//printc("ODA_SELECT");
				case ODA_DRAWENTIRE:
					hdcMem=CreateCompatibleDC(lpdis->hDC); 
					hbmpold=(HDC)SelectObject(hdcMem,Exodusbmp);
					BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
						lpdis->rcItem.right - lpdis->rcItem.left,
						lpdis->rcItem.bottom - lpdis->rcItem.top,
						hdcMem,0,0,SRCCOPY); 
	
					SelectObject(hdcMem, hbmpold); 
					DeleteDC(hdcMem);
					//put border around if selected
					if (lpdis->itemState&ODS_SELECTED) {
						rc.left=lpdis->rcItem.left+2;
						rc.top=lpdis->rcItem.top+2;
						rc.right=lpdis->rcItem.right-2;
						rc.bottom=lpdis->rcItem.bottom-2;
						DrawFocusRect(lpdis->hDC,&rc);
						}
				break; 
				} // end switch item action
			return (TRUE); 
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					//DestroyWindow(About);
					EndDialog(req_ptr, TRUE);
					return (TRUE);
				case IDC_HTTPLINK:
					EndDialog(req_ptr, TRUE);
					printc("http://www.exodusmm.com");
					ShellExecute(req_ptr,"open","http://www.exodusmm.com/",(LPCTSTR)params,(LPCTSTR)dr,SW_MAXIMIZE);
					return (TRUE);
				}
			break;
		}
   return (0L); 
	}


/*Handle import window messages here*/


LRESULT CALLBACK gObjCallback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	class messagebase *messageobj=(class messagebase *)GetWindowLong(w_ptr,GWL_USERDATA);
	if (messageobj) return (messageobj->Callback(w_ptr,uMsg,wParam,lParam));
	else return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
	}

DWORD WINAPI streamthreadfunc(LPVOID parm) {
	return(capture->streamfunc());
	} 

DWORD WINAPI previewthreadfunc(LPVOID parm) { 
	WaitForSingleObject(arrayofevents[EVENT_PREVIEW],INFINITE);
	while (!killpreviewthread) {
		if (toaster->videobuf) preview->updatepreview(/*(ULONG *)toaster->videobuf*/);
		WaitForSingleObject(arrayofevents[EVENT_PREVIEW],INFINITE);
		}
	return(0);
	} 

void resizewindows() {
	LONG y;
	preview->resize();
	y=controls->resize();
	con.resize(y);
	}


void initchildren() {
	capture->startup();
	controls->startup();
	preview->startup();
	con.startup();
	printc("Ready.");
	}


void print(char *sz,...) {
	static TCHAR ach[256];
	va_list va;

	va_start(va, sz);
	wvsprintf (ach,sz, va);
	va_end(va);

	con.print(ach);
	}


void printc(char *sz,...) {
	static TCHAR ach[256];
	va_list va;

	va_start(va, sz);
	wvsprintf (ach,sz, va);
	va_end(va);

	con.printc(ach);
	}


void printerror(char *sz,...) {
	static TCHAR ach[256];
	va_list va;

	if (!IsWindowVisible(screen)) ShowWindow(screen,SW_SHOW);
	va_start(va, sz);
	wvsprintf (ach,sz, va);
	va_end(va);

	con.printc(ach);
	}

/*Error Types are sorted from most critical to least critical*/
int error(char msgtype,char *sz,...) {
    static TCHAR ach[2000];
    va_list va;

    va_start(va, sz);
    wvsprintf (ach,sz, va);
    va_end(va);

	if (msgtype==0) {
		MessageBeep(MB_ICONSTOP);
		MessageBox(screen,ach,"FirePower Error!",MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);
		cleanup();
		throw ach;
		}

	if (msgtype==1) {
		MessageBeep(MB_ICONEXCLAMATION);
		MessageBox(screen,ach,"FirePower Warning",MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
		}

	if (msgtype==2) {
		MessageBeep(MB_ICONEXCLAMATION);
		return(MessageBox(screen,ach,"FirePower Warning",MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_SYSTEMMODAL));
		}

	return(FALSE);
	}

