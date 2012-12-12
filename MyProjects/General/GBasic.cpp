// GBasic.cpp
// These Functions are Global accross multiple applications,
// but only Good for the PC platform

#include "system.h"
#include <shlobj.h> //this is for the browse for folder

void size100base(HWND parent,HWND window,LONG *x,LONG *y,LONG *width,LONG *height,UBYTE skip) {
	RECT rc;

	GetClientRect(parent,&rc);

	if (!((skip==1)||(skip==3))) *y=(rc.bottom*(*y))/100;
	if (!((skip==2)||(skip==3))) *height=((rc.bottom*(*height))/100)-*y;
	*x=(rc.right*(*x))/100;
	*width=((rc.right*(*width))/100)-*x;
	}


LONG size100noty(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(parent,window,&x,&y,&width,&height,1);
	SetWindowPos(window,NULL,x,y,
		width,height,SWP_NOACTIVATE|SWP_NOZORDER);
	return (y+height);
	}


LONG size100notheight(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(parent,window,&x,&y,&width,&height,2);
	SetWindowPos(window,NULL,x,y,
		width,height,SWP_NOACTIVATE|SWP_NOZORDER);
	return (y+height);
	}

LONG size100notyorh(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(parent,window,&x,&y,&width,&height,3);
	SetWindowPos(window,NULL,x,y,
		width,height,SWP_NOACTIVATE|SWP_NOZORDER);
	return (y+height);
	}

void size100(HWND parent,HWND window,LONG x,LONG y,LONG width,LONG height) {
	size100base(parent,window,&x,&y,&width,&height,0);
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

HBITMAP createbitmapwin(HWND w_ptr,char *bmpfile)
	{
	HBITMAP smartrefreshclip;
	HDC hdc,smartdc;
	RECT rc;

	/* This will create a SmartRefresh Bitmap*/
	hdc=GetDC(w_ptr);
	smartdc=CreateCompatibleDC(hdc);
	GetClientRect(w_ptr,&rc);
	//smartrefreshclip=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
	smartrefreshclip=(HBITMAP)LoadImage(hInst,bmpfile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	/* Initialize the area */
	SelectObject(smartdc,smartrefreshclip);
	//FillRect(smartdc,&rc,color);
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
		//TransparentBlt(hdc,x,y,widthx,widthy,hmem,x,y,widthx,widthy,0);
		BitBlt(hdc,x,y,widthx,widthy,hmem,x,y,SRCCOPY);
		/*wsprintf(string,"x=%d,y=%d,wx=%d,wy=%d",x,y,widthx,widthy);printc(string);*/
		DeleteDC(hmem);
		}
	}	

ULONG fliplong(ULONG x) {
	__asm {
		mov	eax,x
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		mov	x,edx
		}
	return (x);
	}

UWORD flipword(UWORD x) {
	__asm {
		mov	ax,x
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		mov	x,dx
		}
	return (x);
	}

char *getdirectory(char *inputprompt,char *pathholder) {
	BROWSEINFOA bif;
	LPMALLOC Malloc=NULL;

	LPITEMIDLIST result;
	bif.hwndOwner=screen;
	bif.pidlRoot=NULL;
	bif.pszDisplayName=pathholder;
	bif.lpszTitle=inputprompt;
	bif.ulFlags=BIF_RETURNONLYFSDIRS;
	bif.lpfn=NULL;
	bif.lParam=NULL;
	bif.iImage=NULL;

	result=SHBrowseForFolder(&bif);
	if (result) {
		SHGetPathFromIDList(result,pathholder);
		if (strlen(pathholder)>4) strcat(pathholder,"\\");
		}
	//ok time to free the result
	if (SHGetMalloc(&Malloc)==NOERROR) Malloc->Free(result);
	return(0);
	}


BOOL getopenfile(char *dest,char *filename,char *defpath,char *defext,char *inputprompt,char *filter,BOOL musthave) {
	OPENFILENAME ofn;
	BOOL bResult;

	//Clear out and fill in an OPENFILENAME structure in preparation
	//for creating a common dialog box to open a file.
	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	= sizeof(OPENFILENAME);
	ofn.hwndOwner	= screen;
	ofn.hInstance	= hInst;
	ofn.lpstrFilter	= filter;
	ofn.nFilterIndex	= 0;
	dest[0]	= '\0';
	ofn.lpstrFile	= dest;
	ofn.nMaxFile	= MAX_PATH;
	ofn.lpstrFileTitle = filename;
	ofn.nMaxFileTitle	= MAX_PATH;
	ofn.lpstrInitialDir = defpath;
	ofn.lpstrDefExt	= defext;
	ofn.lpstrTitle	= inputprompt;
	if (musthave) ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	else ofn.Flags=OFN_HIDEREADONLY;

	return (bResult=GetOpenFileName(&ofn));
	}

HANDLE OpenReadSeq(char *filename) {
	HANDLE hf;
	hf=CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		//FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hf==INVALID_HANDLE_VALUE) hf=(void *)-1;
	return (hf);
	}

__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod) {
   LARGE_INTEGER li;
   li.QuadPart = distance;
   li.LowPart = SetFilePointer (hf,li.LowPart,&li.HighPart,MoveMethod);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

__int64 myTell64 (HANDLE hf) {
   LARGE_INTEGER li;
   li.QuadPart = 0;
   li.LowPart = SetFilePointer (hf,li.LowPart,&li.HighPart,FILE_CURRENT);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

DWORD myTell (HANDLE hf) {
	DWORD distance;
   distance = SetFilePointer (hf,0,NULL,FILE_CURRENT);
	if (distance == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		distance = -1;
		}
	return distance;
	}

DWORD mySeek (HANDLE hf,DWORD distance,DWORD MoveMethod) {
	DWORD bytes_read;
   bytes_read=SetFilePointer (hf,distance,NULL,MoveMethod);
	if (bytes_read == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		bytes_read = -1;
		}
	return (bytes_read);
	}
 
int myRead(HANDLE hf,void *buf,DWORD count) {
	DWORD bytes_read;
	if (!(ReadFile(hf,buf,count,&bytes_read,NULL))) bytes_read=-1;
	return ((int)bytes_read);
	}

int myClose(HANDLE hf) {
	int value=-1;
	if (CloseHandle(hf)) value=0;
	return (value);
	}
