#include "HandleMain.h"
#include "../include/newtek/RTVlib.h"

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
char defaultpath[MAX_PATH]; //default path upon execution
//Should have done this to all the objects a long time ago
char username[32];
toasterclass *toaster;
loadersclass *medialoaders;
filtersclass *filters;
controlsclass *controls;
previewclass *preview;
dragthisclass *dragthis;
projectclass *project;
securityclass *security;
audioclass *audio;
captureclass *capture;
storysourceclass *tabs1;
HFONT mediafont;
HBITMAP skinbmp;
HBITMAP Exodusbmp;
HDRAWDIB drawdib=NULL;
HANDLE arrayofevents[EVENT_MAXEVENTS];
HWND About;
static HANDLE playthread=NULL;
static HANDLE streamthread=NULL;
static HANDLE videothread=NULL;
static HANDLE previewthread=NULL;
static HANDLE idlethread=NULL;
static HANDLE dve1thread=NULL;
static HANDLE dve2thread=NULL;
static HANDLE aviaudthread=NULL;
int debug=FALSE;

BOOL killplaythread=FALSE;
BOOL killstreamthread=FALSE;
BOOL killvideothread=FALSE;
BOOL killpreviewthread=FALSE;
BOOL killidlethread=FALSE;
BOOL killdve1thread=FALSE;
BOOL killdve2thread=FALSE;
BOOL killaviaudthread=FALSE;
UWORD idlesignals=0;
UWORD streamthreadmode=0;
class storysourcebase *IdleChangeDirParm1;
class miniscrub *idleminiscrub=NULL;
char IdleChangeDirParm2[MAX_PATH];
int nostaticpreview=-1; //for filters need speed direct access
int nostaticxypreview=-1; //for filters need speed direct access
int test=0;
//Statics members of other classes
HBITMAP storysourcebase::folder=NULL;
HBITMAP storysourcebase::errorimage=NULL;
HBITMAP storysourcebase::audioimage=NULL;
HBITMAP storysourcebase::audiohalfimage=NULL;
HBITMAP selectimageclass::multiselectimage=NULL;
HBITMAP storyboardclass::glowbmp=NULL;
char storysourcebase::drvspecbuf[MAXDRVBUFSIZE];
DWORD storysourcebase::drvbufsize=0;
//End Globals

// Global Vars from gbasic
char string[MAX_PATH];  //Temporary for debug
HINSTANCE hInst;   /* current instance */
HWND screen;
LPCTSTR lpszAppName  = "STORYBOARD";
LPCTSTR lpszIcon  = "EXODUS";
LPCTSTR lpszTitle    = "Story Board Interface";
LPINITCOMMONCONTROLSEX initctrls;
CRITICAL_SECTION csglobal;

struct memlist *pmem=NULL; 
struct nodevars *nodeobject=NULL;
static HACCEL haccel;

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
	wc.lpszMenuName  = lpszAppName;              
	wc.lpszClassName = lpszAppName;              
   
	if (!RegisterClass(&wc)) return(FALSE);
	
   hInst = hInstance;
	nodeobject=createnode(&pmem,65536,0);

	/*child popup contents*/
	wc.style         = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_PARENTDC; 
   wc.lpfnWndProc   = (WNDPROC)gObjCallback;
	wc.cbClsExtra    = sizeof(WORD);                           
	wc.cbWndExtra    = 0;                           
	wc.hInstance     = hInst;                   
	wc.hIcon         = LoadIcon(hInst,lpszIcon); 
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
		0,0,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL,NULL,hInstance,NULL);

   if (!screen) 
      return(FALSE);
   ShowWindow(screen,nCmdShow|SW_SHOWMAXIMIZED);

   UpdateWindow(screen);
	//Resize our windows here
	resizewindows();
	initchildren();
	// Open Libs, and other init stuff
	/*
	if (!(InitCommonControlsEx(initctrls))) {
		error(0,"Unable to open the common control dynamic-link library");
		}
	*/
	// TODO research why the InitCommonControlsEx doesn't return true
	InitCommonControls();
	haccel=LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	sleep(&msg);
   return(msg.wParam); 
	}

void sleep (MSG *msg) {
	while(GetMessage(msg,NULL,0,0))   {
		if (!TranslateAccelerator(screen,haccel,msg)) {
			TranslateMessage(msg); 
			DispatchMessage(msg);  
			}
		}
	}

static consoleclass con;
static storyboardclass storyboard;

LRESULT CALLBACK handlemain(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	DWORD playthreadretid;
	DWORD streamthreadretid;
	DWORD videothreadretid;
	DWORD previewthreadretid;
	DWORD idlethreadretid;
	DWORD dve1threadretid;
	DWORD dve2threadretid;
	DWORD aviaudthreadretid;
	UBYTE t;

	switch (uMsg) {
		case WM_CREATE:
			memset(arrayofevents,0,EVENT_MAXEVENTS*4);
			m_ptr=GetMenu(w_ptr);
			/*Go Ahead and Check all pre opened windows here*/
			CheckMenuItem(m_ptr,IDM_CON,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_CONTROLS,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_IMPORTS,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_TRANSITIONS,MF_BYCOMMAND|MFS_CHECKED);
			//CheckMenuItem(m_ptr,IDM_FILTERS,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_STORYBOARD,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND|MFS_CHECKED);
			CheckMenuItem(m_ptr,IDM_CAPTOASTER,MF_BYCOMMAND|MFS_CHECKED);

			//I'd better set up the toaster before I open the threads
			toaster=new toasterclass();
			//Same goes for the streamthread
			medialoaders=new loadersclass();
			filters=new filtersclass();
			controls=new controlsclass();
			preview=new previewclass();
			dragthis=new dragthisclass();
			security=new securityclass();
			project=new projectclass();
			audio=new audioclass();
			capture=new captureclass();
			tabs1=new storysourceclass();
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

			//Now Set up our child threads
			playthread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				playthreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&playthreadretid); // returns the thread identifier 
			if (playthread==NULL) error(0,"CreateThread() failed.");
			SetThreadPriority(playthread,THREAD_PRIORITY_TIME_CRITICAL);
			streamthread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				streamthreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&streamthreadretid); // returns the thread identifier 
			if (streamthread==NULL) error(0,"CreateThread() failed.");
			//For now the stream thread has normal priority
			//Heres the video thread
			if (toaster->rtmeoutput) {
				videothread=CreateThread( 
					NULL, // no misc attributes 
					0, // use default stack size  
					videothreadfunc, // thread function 
					NULL, // argument to thread function 
					0, // use default creation flags 
					&videothreadretid); // returns the thread identifier 
				if (videothread==NULL) error(0,"CreateThread() failed.");
				//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);
				}
			else {
				videothread=CreateThread( 
					NULL, // no misc attributes 
					0, // use default stack size  
					videothreadNoCard, // thread function 
					NULL, // argument to thread function 
					0, // use default creation flags 
					&videothreadretid); // returns the thread identifier 
				if (videothread==NULL) error(0,"CreateThread() failed.");
				//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);
				}
			previewthread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				previewthreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&previewthreadretid); // returns the thread identifier 
			if (previewthread==NULL) error(0,"CreateThread() failed.");
			//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);

			//And finally the Idle Thread
			idlethread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				idlethreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&idlethreadretid); // returns the thread identifier 
			if (idlethread==NULL) error(0,"CreateThread() failed.");
			SetThreadPriority(idlethread,THREAD_PRIORITY_IDLE);

			dve1thread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				dve1threadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&dve1threadretid); // returns the thread identifier 
			if (dve1thread==NULL) error(0,"CreateThread() failed.");
			else SetThreadIdealProcessor(dve1thread,0);
			//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);

			dve2thread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				dve2threadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&dve2threadretid); // returns the thread identifier 
			if (dve2thread==NULL) error(0,"CreateThread() failed.");
			else SetThreadIdealProcessor(dve2thread,1);
			//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);


			aviaudthread=CreateThread( 
				NULL, // no misc attributes 
				0, // use default stack size  
				aviaudthreadfunc, // thread function 
				NULL, // argument to thread function 
				0, // use default creation flags 
				&aviaudthreadretid); // returns the thread identifier 
			if (aviaudthread==NULL) error(0,"CreateThread() failed.");
			//SetThreadPriority(videothread,THREAD_PRIORITY_TIME_CRITICAL);

			//Make a permanent record of storyboards path
			GetCurrentDirectory(MAX_PATH,defaultpath);
			//Open vfw.dll only once per application
			AVIFileInit();
			//Allocate one drawing dib for avi blt
			drawdib=DrawDibOpen();
			//We'll set up our font here
			mediafont=CreateFont(12,4,0,0,FW_NORMAL,
				0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,
				"Arial");
			//Load our graphic skins for all objects in app
			skinbmp=(HBITMAP)LoadImage(hInst,"Resources\\Skins.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			Exodusbmp=(HBITMAP)LoadImage(hInst,"IDB_BITMAP1",IMAGE_BITMAP,0,0,0);
			con.createwindow(w_ptr);
			printc("* * * StoryBoard Console Interface * * *");
			printc(" ");
			//printc("2.3 Gigabytes Free");
			//printc(" ");

			//ShowWindow(console,SW_HIDE);

			//init the static members of storysource
			storysourcebase::folder=(HBITMAP)LoadImage(hInst,"resources\\folder.bmp",IMAGE_BITMAP,HALFXBITMAP-2,HALFYBITMAP-2,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			storysourcebase::errorimage=(HBITMAP)LoadImage(hInst,"resources\\error.bmp",IMAGE_BITMAP,HALFXBITMAP-2,HALFYBITMAP-2,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			storysourcebase::audioimage=(HBITMAP)LoadImage(hInst,"resources\\speaker.bmp",IMAGE_BITMAP,XBITMAP-2,YBITMAP-2,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			storysourcebase::audiohalfimage=(HBITMAP)LoadImage(hInst,"resources\\speaker.bmp",IMAGE_BITMAP,HALFXBITMAP-2,HALFYBITMAP-2,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			selectimageclass::multiselectimage=(HBITMAP)LoadImage(hInst,"resources\\multiselect.bmp",IMAGE_BITMAP,XBITMAP-2,YBITMAP-2,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
			storysourcebase::drvbufsize=GetLogicalDriveStrings(MAXDRVBUFSIZE,storysourcebase::drvspecbuf);
			//storyboardclass::glowbmp=(HBITMAP)LoadImage(hInst,"resources\\glow.bmp",IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
			//We'll have to use our own load bmp to get the correct DIB for alphablend
			{
			BITMAPINFOHEADER *bmi;
			struct imagelist *glowimage=(struct imagelist *)newnode(nodeobject,sizeof(imagelist));
			glowimage->filesource="resources\\glow.bmp";
			storyboardclass::glowbmp=medialoaders->bmpobj.getthumbbmp(glowimage,&bmi);
			disposenode(nodeobject,(struct memlist *)glowimage);
			}
			/*End Checking all pre opened windows*/
			tabs1->createwindow(&storyboard,w_ptr);

			controls->createwindow(w_ptr);
			preview->createwindow(w_ptr);
			storyboard.createwindow(w_ptr);
			dragthis->createwindow(w_ptr);
			filters->createwindow(w_ptr);
			audio->createwindow(w_ptr);
			capture->createwindow(w_ptr);
			break;

		case WM_SYSCOMMAND:
			if (wParam==SC_MINIMIZE) preview->hideoverlay();
			if (wParam==SC_RESTORE) preview->showoverlay();
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/* * * * PROJECT * * * */
				case IDM_TEST: {
					BuildRTVFile("c:\\myprojects\\storyboard\\imports\\test.rtv",2,0,720,240,29.97f);
					WriteRTVFile("c:\\myprojects\\storyboard\\imports\\test.rtv",toaster->videobuf);
					CloseRTVFile("c:\\myprojects\\storyboard\\imports\\test.rtv");
					break;
					}
				case IDM_NEW:
					project->projectnew(&storyboard);
					break;
				case IDM_OPEN: {
					char *filename;
					printc("opening project...");
					if (filename=project->getopenfilename("Open StoryBoard Project...",TRUE))
						project->projectopen(&storyboard,filename);
					printc("Ready.");
					break;
					}
				case IDM_CLOSE:
					printc("closing project");
					project->projectclose(&storyboard);
					break;
				case IDM_SAVE:
					if (*(project->projectname)) {
						project->projectsave(&storyboard,project->projectname);
						printc("updated project.");
						break;
						}
				case IDM_SAVEAS: {
					char *filename;
					if (storyboard.imagelisthead) {
						printc("Saving As...");
						if (filename=project->getsavefilename("Save StoryBoard Project...")) {
							project->projectsave(&storyboard,filename);
							printc("project saved");
							}
						else printc("The save request has been canceled.");
						}
					else printc("There is nothing to save");
					break;
					}
				case KEY_ESCAPE:
				case IDM_EXIT:
					DestroyWindow(screen);
					break;

				/* * * * VIEW * * * */
				case IDM_CONTROLS:
					windowtoggle(screen,controls->window,IDM_CONTROLS);
					break;
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
				case IDM_IMPORTS:
					windowtoggle(screen,tabs1->media.window,IDM_IMPORTS);
					break;
				case IDM_TRANSITIONS:
					windowtoggle(screen,tabs1->fxobject.window,IDM_TRANSITIONS);
					break;
				case IDM_STORYBOARD:
					windowtoggle(screen,storyboard.storywindow,IDM_STORYBOARD);
					break;
				case IDM_FILTERS:
					windowtoggle(screen,filters->window,IDM_FILTERS);
					filters->updateimagelist();
					break;
				case IDM_AUDIO:
					windowtoggle(screen,audio->window,IDM_AUDIO);
					audio->updateimagelist();
					break;
				case IDM_CAPTURE:
					if (GetMenuState(m_ptr,IDM_CAPTURE,MF_BYCOMMAND)) capture->hidedisplay();
					else {
						if (controls->mycontrolis&8) capture->showdisplay();
						else printc("Please use record button to enter capture mode");
						}
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
				case IDM_ABOUT: {
					DialogBox(hInst,"AboutBox",w_ptr,DLGPROC(handleabout));
					break;
					}
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
				case KEY_DELETE:
					//TODO figure out how to setfocus to different windows luckily the delete key
					//is only used for StoryBoard so it will not matter at this time.
					return storyboard.Callback(w_ptr,uMsg,wParam,lParam);
				}
			break;

		case WM_MOVE:
		case WM_SIZE:
			resizewindows();
			return(0);

		case WM_DESTROY:
			SetCurrentDirectory(defaultpath);
			if (toaster->rtmeoutput) toaster->initmasterframe("resources\\shutdown.rtv");
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
	killplaythread=
	killstreamthread=
	killvideothread=
	killpreviewthread=
	killidlethread=
	killdve1thread=
	killdve2thread=
	killaviaudthread=TRUE;
	for (t=0;t<EVENT_MAXEVENTS;t++) if(arrayofevents[t]) SetEvent(arrayofevents[t]);
	//wait for all child threads to succesfully close
	//and then close the handles of the threads
	if (playthread) {
		WaitForSingleObject(playthread,INFINITE);
		CloseHandle(playthread);
		}
	if (streamthread) {
		WaitForSingleObject(streamthread,INFINITE);
		CloseHandle(streamthread);
		}
	if (videothread) {
		WaitForSingleObject(videothread,INFINITE);
		CloseHandle(videothread);
		}
	if (previewthread) {
		WaitForSingleObject(previewthread,INFINITE);
		CloseHandle(previewthread);
		}
	if (idlethread) {
		WaitForSingleObject(idlethread,INFINITE);
		CloseHandle(idlethread);
		}
	if (dve1thread) {
		WaitForSingleObject(dve1thread,INFINITE);
		CloseHandle(dve1thread);
		}
	if (dve2thread) {
		WaitForSingleObject(dve2thread,INFINITE);
		CloseHandle(dve2thread);
		}
	if (aviaudthread) {
		WaitForSingleObject(aviaudthread,INFINITE);
		CloseHandle(aviaudthread);
		}
	//Now to close the events
	for (t=0;t<EVENT_MAXEVENTS;t++) CloseHandle(arrayofevents[t]);

	//end thread stuff
	tabs1->shutdown();
	filters->shutdown();
	preview->shutdown();
	controls->shutdown();
	medialoaders->shutdown();
	audio->shutdown();
	capture->shutdown();
	delete tabs1;
	delete capture;
	delete audio;
	delete project;
	delete security;
	delete dragthis;
	delete filters;
	delete medialoaders;
	storyboard.shutdown();
	dragthis->shutdown();
	delete toaster;
	delete preview;
	delete controls;
	if (Exodusbmp) DeleteObject(Exodusbmp);
	if (skinbmp) DeleteObject(skinbmp);
	if (storysourcebase::folder) DeleteObject(storysourcebase::folder);
	if (storysourcebase::errorimage) DeleteObject(storysourcebase::errorimage);
	if (storysourcebase::audioimage) DeleteObject(storysourcebase::audioimage);
	if (storysourcebase::audiohalfimage) DeleteObject(storysourcebase::audiohalfimage);
	if (selectimageclass::multiselectimage) DeleteObject(selectimageclass::multiselectimage);
	if (storyboardclass::glowbmp) DeleteObject(storyboardclass::glowbmp);
	DeleteObject(mediafont);
	// release DrawDib stuff
	DrawDibClose(drawdib);
	//Release the vfw dll
	AVIFileExit();

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
			char *version;
			static char demoreg[2][16]={
				{'-','G','G','j','(','0','H','I','&','+','R','h','P','P','4',0},
				{'e','G','P','E','q',')','V','^','g','R','6','~','P','4',0}
				};
			//RECT rc;
			char *temppass="*#@!@#*%^";
			int t;
	
			strcpy(string,defaultpath);
			strcat(string,"\\StoryBoard.exe");
			versionsize=GetFileVersionInfoSize(string,&versiondummy);
			version=(char *)newnode(nodeobject,versionsize);
			GetFileVersionInfo(string,0,versionsize,version);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\PrivateBuild",&PrivateBuild,&versionbufsize);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\ProductName",&ProductName,&versionbufsize);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\SpecialBuild",&SpecialBuild,&versionbufsize);
			VerQueryValue((LPVOID)version,"\\StringFileInfo\\000004B0\\LegalCopyright",&LegalCopyright,&versionbufsize);
			for (t=0;t<2;t++) security->encrypt(temppass,demoreg[0],TRUE);
			for (t=0;t<2;t++) security->encrypt(temppass,demoreg[1],TRUE);
			//wsprintf(string,"%s\n%s-%s %s\n%s\n\n%s",(char *)ProductName,
			wsprintf(string,"%s\n%s-%s \n\n%s",(char *)ProductName,
				(char *)PrivateBuild,(char *)SpecialBuild,
				//demoreg[security->pass()],username,(char*)LegalCopyright);
				(char*)LegalCopyright);
			for (t=0;t<2;t++) security->encrypt(temppass,demoreg[0],FALSE);
			for (t=0;t<2;t++) security->encrypt(temppass,demoreg[1],FALSE);
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

DWORD WINAPI playthreadfunc(LPVOID parm) { 
	return(controls->playfunc(&storyboard,NULL));
	} 

DWORD WINAPI streamthreadfunc(LPVOID parm) {
	return(medialoaders->streamfunc());
	} 

DWORD WINAPI videothreadfunc(LPVOID parm) { 
	return(controls->updatevideo());
	} 

DWORD WINAPI previewthreadfunc(LPVOID parm) { 
	return (preview->updatepreviewthread());
	} 

DWORD WINAPI videothreadNoCard(LPVOID parm) { 
	return(controls->updatevideoNoCard());
	}

DWORD WINAPI idlethreadfunc(LPVOID parm) {
		ULONG *videobuf=toaster->NoToaster;
	//lean and mean
	WaitForSingleObject(arrayofevents[EVENT_IDLE],INFINITE);
	while (!killidlethread) {
		if (idlesignals&IDLE_CHANGEDIR) {
			IdleChangeDirParm1->changedir(&storyboard,IdleChangeDirParm2);
			idlesignals^=IDLE_CHANGEDIR;
			}
		else if (idlesignals&IDLE_FTLEFT) {
			idleminiscrub->adjustscrub(-1);
			Sleep(200L);
			while (idleminiscrub) {
				idleminiscrub->adjustscrub(-1);
				Sleep(31L);
				if (controls->frames_behind) while (controls->frames_behind<(-4)) Sleep(0L);
				}
			idlesignals^=IDLE_FTLEFT;
			}
		else if (idlesignals&IDLE_FTRIGHT) {
			idleminiscrub->adjustscrub(1);
			Sleep(200L);
			while (idleminiscrub) {
				idleminiscrub->adjustscrub(1);
				Sleep(31L);
				if (controls->frames_behind) while (controls->frames_behind<(-4)) Sleep(0L);
				}
			idlesignals^=IDLE_FTRIGHT;
			}
		else if (idlesignals&IDLE_TOOLBAR) {
			Sleep(31L);
			storyboard.tools.updatetoolbar(controls->streamptr);
			idlesignals^=IDLE_TOOLBAR;
			}

		WaitForSingleObject(arrayofevents[EVENT_IDLE],INFINITE);
		} //end while not killidlethread

	return(0);
	} 

DWORD WINAPI dve1threadfunc(LPVOID parm) {
	//dofx parms
	struct imagelist *streamptr;
	class generalFX *DVEprefs;
	ULONG *imagea;
	ULONG *imageb;
	ULONG *videobuf;
	ULONG mediatimer;

	WaitForSingleObject(arrayofevents[EVENT_DVE1],INFINITE);
	while (!killdve1thread) {
		streamptr=controls->streamptr;
		DVEprefs=streamptr->DVEprefs;
		imagea=DVEprefs->imagea;
		imageb=DVEprefs->imageb;
		videobuf=DVEprefs->videobuf;
		mediatimer=DVEprefs->dvetime;
		DVEprefs->doFX(streamptr,videobuf,imageb,videobuf,mediatimer,0);
		SetEvent(arrayofevents[EVENT_DVE1F]);
		WaitForSingleObject(arrayofevents[EVENT_DVE1],INFINITE);
		} //end while not killdve1thread
	return(0);
	}

DWORD WINAPI dve2threadfunc(LPVOID parm) {
	//dofx parms
	struct imagelist *streamptr;
	class generalFX *DVEprefs;
	ULONG *imagea;
	ULONG *imageb;
	ULONG *videobuf;
	ULONG mediatimer;

	WaitForSingleObject(arrayofevents[EVENT_DVE2],INFINITE);
	while (!killdve2thread) {
		streamptr=controls->streamptr;
		DVEprefs=streamptr->DVEprefs;
		imagea=DVEprefs->imagea;
		imageb=DVEprefs->imageb;
		videobuf=DVEprefs->videobuf;
		mediatimer=DVEprefs->dvetime;
		DVEprefs->doFX(streamptr,videobuf,imageb,videobuf,mediatimer,1);
		SetEvent(arrayofevents[EVENT_DVE2F]);
		WaitForSingleObject(arrayofevents[EVENT_DVE2],INFINITE);
		} //end while not killdve2thread
	return(0);
	}


DWORD WINAPI aviaudthreadfunc(LPVOID parm) {
	DWORD event;
	event=WaitForMultipleObjects(4,&arrayofevents[EVENT_AVIAUD1],FALSE,INFINITE);
	while (!killaviaudthread) {
		medialoaders->aviobj.streamaudio(++event);
		event=WaitForMultipleObjects(4,&arrayofevents[EVENT_AVIAUD1],FALSE,INFINITE);
		} //end while not killdve2thread
	return(0);
	}

void resizewindows() {
	LONG y;
	storyboard.resize();
	audio->resize();
	filters->resize();
	y=controls->resize();
	preview->resize(y);
	con.resize(y);
	tabs1->resize();
	}


void initchildren() {
	capture->startup();
	audio->startup();
	filters->startup();
	storyboard.startup();
	controls->startup();
	preview->startup();
	con.startup();
	tabs1->startup();
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

	va_start(va, sz);
	wvsprintf (ach,sz, va);
	va_end(va);

	con.printc(ach);
	}


struct imagelist *duplicateimage(struct imagelist *source)
	{
	struct imagelist *dest;
	HBITMAP newimage;
	RECT rc;
	HDC hdc,sourcedc,destdc;

	//This will create a new Bitmap Image so that we can switch dirs in the Media
	hdc=GetDC(screen);
	sourcedc=CreateCompatibleDC(hdc);
	destdc=CreateCompatibleDC(hdc);
	newimage=CreateCompatibleBitmap(hdc,XBITMAP,YBITMAP);
	SelectObject(sourcedc,source->image);
	SelectObject(destdc,newimage);
	//Clear out newbitmap
	rc.left=rc.top=0;
	rc.right=XBITMAP;
	rc.bottom=YBITMAP;
	FillRect(destdc,&rc,(HBRUSH)(COLOR_WINDOW+1));
	BitBlt(destdc,0,0,XBITMAP,YBITMAP,sourcedc,0,0,SRCCOPY);
	DeleteDC(destdc);
	DeleteDC(sourcedc);
	ReleaseDC(screen,hdc);
	dest=(struct imagelist *)newnode(nodeobject,sizeof(struct imagelist));
	clrmem((char *)dest,sizeof(struct imagelist));
	dest->image=newimage;
	dest->filesource=(char *)newnode(nodeobject,strlen(source->filesource)+1);
	strcpy(dest->filesource,source->filesource);
	dest->text=(char *)newnode(nodeobject,strlen(source->text)+1);
	strcpy(dest->text,source->text);
	dest->cropout=dest->actualframes=dest->totalframes=source->totalframes;
	dest->id=source->id;
	dest->idobj=source->idobj;
	dest->mediatype=source->mediatype;
	dest->next=NULL;dest->prev=NULL;
	dest->audio=source->audio;
	dest->DVEprefs=source->DVEprefs;
	dest->rtvaudio=source->rtvaudio;
	dest->mediafilter=source->mediafilter; //We are not copying contents of filters for now
	dest->mediaother=source->mediaother; //same for resources of loaders
	return (dest);
	}


void g_updateframedisplay()  {
	controls->updateframesdisplay(&storyboard);
	}

class storyboardclass *getstoryboard() {
	return (&storyboard);
	}

void	toastercallback(void *userdata,long frames_behind) {
	controls->frames_behind=frames_behind;
	}

void	toasterdcallback(void *userdata,ULONG source_time) {
	controls->source_time=source_time;
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
		MessageBox(screen,ach,"StoryBoard Error!",MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);
		cleanup();
		throw ach;
		}

	if (msgtype==1) {
		MessageBeep(MB_ICONEXCLAMATION);
		MessageBox(screen,ach,"StoryBoard Warning",MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
		}

	if (msgtype==2) {
		MessageBeep(MB_ICONEXCLAMATION);
		return(MessageBox(screen,ach,"StoryBoard Warning",MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_SYSTEMMODAL));
		}

	return(FALSE);
	}
