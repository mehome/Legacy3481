#ifndef GBASIC_H
#define GBASIC_H


/* Globlal External Vars */
extern char string[]; //Temporary for debug
extern HINSTANCE hInst;
extern HWND screen;
extern struct memlist *pmem;
extern struct nodevars *nodeobject; 
extern CRITICAL_SECTION csglobal;
/* End Global External Vars*/

/* Here are the function prototypes */
void windowtoggle(HWND w_ptr,HWND child,long mitem);
void smartrefresh(HDC hdc,HBITMAP pic,int x,int y,int widthx,int widthy);
HBITMAP createclipwin(HWND w_ptr,HBRUSH color);
HBITMAP createbitmapwin(HWND w_ptr,char *bmpfile);
LONG size100noty(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
LONG size100notheight(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
LONG size100notyorh(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
void size100(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height);
void sizetabchild(HWND child,HWND parent);
ULONG fliplong(ULONG x);
UWORD flipword(UWORD x);
char *getdirectory(char *inputprompt,char *pathholder);
BOOL getopenfile(char *dest,char *filename,char *defpath,char *defext,char *inputprompt,char *filter,BOOL musthave);
//Dos lowlever file IO API
HANDLE OpenReadSeq(char *filename);
__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod);
__int64 myTell64 (HANDLE hf);
DWORD myTell (HANDLE hf);
DWORD mySeek (HANDLE hf,DWORD distance,DWORD MoveMethod);
int myRead(HANDLE hf,void *buf,DWORD count);
int myClose(HANDLE hf);
/* End Global function prototypes */


#endif /* GBASIC_H */
