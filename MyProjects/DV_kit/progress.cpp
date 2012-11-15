
#include "handlemain.h"
#include "progress.h"


int progressclass::Callback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_CLOSE:
			windowtoggle(screen,window,NULL);
			return(0L);

		case WM_DRAWITEM: {
			//RECT rc;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;
			int /*width,*/height;

			lpdis=(LPDRAWITEMSTRUCT) lParam;
			//printc("WM_DRAWITEM");
			if (lpdis->itemID==(UINT)-1) return(TRUE);
			switch (lpdis->itemAction)  {
				case ODA_SELECT:
					//printc("ODA_SELECT");
				case ODA_DRAWENTIRE:
					
					// Is the item selected?  
					hdcMem=CreateCompatibleDC(lpdis->hDC);
					hbmpold=(HDC)SelectObject(hdcMem,backbmp);
					width=lpdis->rcItem.right-lpdis->rcItem.left;
					height=lpdis->rcItem.bottom-lpdis->rcItem.top;

					BitBlt (lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,width,height,
					hdcMem,backloc.x,backloc.y,SRCCOPY);
					SelectObject(hdcMem,barbmp);
					if (lpdis->rcItem.left<progress)
 						TransparentBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							progress,height,
							hdcMem,barloc.x,barloc.y,progress,height,0); 
					SelectObject(hdcMem, hbmpold); 
					DeleteDC(hdcMem);
					break; 
				} // end switch item action
			return (TRUE); 
			}

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}

progressclass::progressclass(struct progressparms *parms) {
	x=parms->x;
	y=parms->y;
	width=parms->width;
	height=parms->height;
	backbmp=parms->backbmp;
	barbmp=parms->barbmp;
	backloc=parms->backloc;
	barloc=parms->barloc;
	//yeah its elogent but minimal
	window=CreateWindowEx(0,"OBJGRAYWIN",parms->name,
		WS_VISIBLE|WS_CHILD,x,y,width,height,parms->parent,
		NULL,hInst,NULL);
	CreateWindowEx(0,"STATIC",NULL,WS_VISIBLE|WS_CHILD|SS_OWNERDRAW,
		0,0,width,height,window,NULL,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(long)this);
	//put progress default to empty for now
	progress=0;
	}

progressclass::~progressclass() {
	}

void progressclass::updateimage(int num,int den) {
	RECT rc;
	rc.left=progress;
	progress=num*width/den;
	rc.right=progress;
	rc.top=0;
	rc.bottom=height;
	InvalidateRect(window,&rc,FALSE);
	}

