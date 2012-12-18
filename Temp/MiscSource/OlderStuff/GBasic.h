#ifndef GBASIC_H
#define GBASIC_H

#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95   (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

/* Globlal External Vars */
extern char string[]; //Temporary for debug
extern HINSTANCE hInst;
extern HWND screen;
extern LPCTSTR lpszTitle;
extern struct memlist *pmem;
extern struct nodevars *nodeobject; 
extern CRITICAL_SECTION csglobal;
/* End Global External Vars*/

/* Here are the function prototypes */
BOOL RegisterWin95(CONST WNDCLASS *lpwc);
void sleep (MSG *msg);
void windowtoggle(HWND w_ptr,HWND child,long mitem);
void smartrefresh(HDC hdc,HBITMAP pic,int x,int y,int widthx,int widthy);
HBITMAP createclipwin(HWND w_ptr,HBRUSH color);
LONG size100noty(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
LONG size100notheight(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
LONG size100notyorh(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
void size100(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
void sizetabchild(HWND child,HWND parent);
void cleanup();
ULONG fliplong(ULONG x);
UWORD flipword(UWORD x);
/* End Global function prototypes */

class messagebase {
	public:
	virtual int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
	};


#endif /* GBASIC_H */
