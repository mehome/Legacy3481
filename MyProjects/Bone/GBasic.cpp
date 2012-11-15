// GBasic.cpp
// These Functions are Global accross multiple applications,
// but only Good for the PC platform

#include "HandleMain.h"
#pragma comment(lib, "dxguid")
#pragma comment(lib, "ddraw")
#pragma comment(lib, "dsound")
#pragma comment(lib, "dplayx")
#pragma comment(lib, "dinput")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "msimg32")
#pragma comment(lib, "vfw32")
/* Global Vars */
char string[256];  //Temporary for debug
HINSTANCE hInst;   /* current instance */
HWND screen;
LPCTSTR lpszAppName  = "MYAPP";
LPCTSTR lpszTitle    = "Generic Interface";
LPINITCOMMONCONTROLSEX initctrls;

struct memlist *pmem=NULL; 
struct nodevars *nodeobject=NULL;

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,int nCmdShow) {
	MSG msg;
	WNDCLASS wc;

   /* screen's, contents */
	wc.style         = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc   = WNDPROC(handlemain);       
	wc.cbClsExtra    = 0;                      
	wc.cbWndExtra    = 0;                      
	wc.hInstance     = hInstance;              
	wc.hIcon         = LoadIcon(hInstance,lpszAppName); 
	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
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
	wc.hIcon         = LoadIcon(hInst, "MYAPP"); 
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
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_TABSTOP,
		0,0,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL,NULL,hInstance,NULL);

   if (!screen) 
      return(FALSE);
   ShowWindow(screen,nCmdShow);

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


void size100base(HWND window,LONG *x,LONG *y,LONG *width,LONG *height,UBYTE skip) {
	RECT rc;

	GetClientRect(screen,&rc);

	if (!(skip==1)) *y=(rc.bottom*(*y))/100;
	if (!(skip==2)) *height=((rc.bottom*(*height))/100)-*y;
	*x=(rc.right*(*x))/100;
	*width=((rc.right*(*width))/100)-*x;
	}


LONG size100noty(HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(window,&x,&y,&width,&height,1);
	SetWindowPos(window,NULL,x,y,
		width,height,SWP_NOACTIVATE|SWP_NOZORDER);
	return (y+height);
	}


LONG size100notheight(HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(window,&x,&y,&width,&height,2);
	SetWindowPos(window,NULL,x,y,
		width,height,SWP_NOACTIVATE|SWP_NOZORDER);
	return (y+height);
	}


void size100(HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(window,&x,&y,&width,&height,0);
	SetWindowPos(window,NULL,x,y,
		width,height,SWP_NOACTIVATE|SWP_NOZORDER);
	}


void sizetabchild(HWND child,HWND parent) {

	RECT cp;
	GetClientRect(parent,&cp);
	//Now that the child is the same size and position will fine tune it
	cp.left+=3;
	cp.right-=6;
	cp.top+=22;
	cp.bottom-=25;
	SetWindowPos(child,NULL,cp.left,cp.top,
		cp.right,cp.bottom,SWP_NOACTIVATE|SWP_NOZORDER);
	}


void windowtoggle(HWND w_ptr,HWND child,long mitem) {
	HMENU m_ptr;
	m_ptr=GetMenu(w_ptr);
	if (IsWindowVisible(child)) {
		ShowWindow(child,SW_HIDE);
		if (mitem) CheckMenuItem(m_ptr,mitem,MF_BYCOMMAND|MFS_UNCHECKED);
		}

	else {
		ShowWindow(child,SW_SHOW);
		if (mitem) CheckMenuItem(m_ptr,mitem,MF_BYCOMMAND|MFS_CHECKED);
		}
	}


HBITMAP createclipwin(HWND w_ptr,HBRUSH color)
	{
	HBITMAP smartrefreshclip;
	HDC hdc,smartdc;
	RECT rc;

	/* This will create a SmartRefresh Bitmap*/
	hdc=GetDC(w_ptr);
	smartdc=CreateCompatibleDC(hdc);
	GetClientRect(w_ptr,&rc);
	smartrefreshclip=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
	/* Initialize the area */
	SelectObject(smartdc,smartrefreshclip);
	FillRect(smartdc,&rc,color);
	DeleteDC(smartdc);
	ReleaseDC(w_ptr,hdc);
	return (smartrefreshclip);
	}


void smartrefresh(
	HDC hdc,HBITMAP pic,int x,int y,int widthx,int widthy) 
	{
	HDC hmem;

	if ((widthx>0) && (widthy>0)) {
		hmem=CreateCompatibleDC(hdc);
		SelectObject(hmem,pic);
		BitBlt(hdc,x,y,widthx,widthy,hmem,x,y,SRCCOPY);
		/*wsprintf(string,"x=%d,y=%d,wx=%d,wy=%d",x,y,widthx,widthy);printc(string);*/
		DeleteDC(hmem);
		}
	}	
