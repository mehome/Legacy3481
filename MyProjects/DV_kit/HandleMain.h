#ifndef HANDLEMAIN_H
#define HANDLEMAIN_H

#include "../general/system.h"

#define VIDEOX 704
#define VIDEOY 480
#define TOOLBUTTONX 20
#define TOOLBUTTONY 20
#define SKINGRIDX 21
#define SKINGRIDY 21

#define EVENT_STREAM 0
#define EVENT_PREVIEW 1
#define EVENT_MAXEVENTS 2

#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

#include "structgo.h"
#include "controls.h"
#include "preview.h"
#include "console.h"
#include "resource.h"
#include "toaster.h"
#include "capture.h"
#include "DV2RTV.h"
/* end include object headers*/

/*Global Vars*/
extern HMENU m_ptr;
extern toasterclass *toaster;
extern controlsclass *controls;
extern previewclass *preview;
extern captureclass *capture;
extern dv2rtvclass *dv2rtv;
extern HBITMAP skinbmp;
extern HBITMAP skin2bmp;
extern HANDLE arrayofevents[EVENT_MAXEVENTS];
extern HWND About;
extern int debug;
extern BOOL killstreamthread;
extern BOOL killpreviewthread;
extern UWORD streamthreadmode;

extern char string[]; //Temporary for debug
extern HINSTANCE hInst;
extern HWND screen;
extern struct memlist *pmem;
extern struct nodevars *nodeobject; 
extern CRITICAL_SECTION csglobal;

/* End Globals */

/* Here are the function prototypes */
void sleep (MSG *msg);
LRESULT CALLBACK handlemain(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK handleabout(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK gObjCallback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
DWORD WINAPI streamthreadfunc(LPVOID parm);
DWORD WINAPI previewthreadfunc(LPVOID parm);
void cleanup();
void resizewindows();
void initchildren();
void print(char *sz,...);
void printc(char *sz,...);
void printerror(char *sz,...);
int error(char msgtype,char *sz,...);
/* End Global function prototypes */


#endif /* HANDLEMAIN_H */
