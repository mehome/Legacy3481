#include "HandleMain.h"  

bmplistbox::bmplistbox() {
	itemindex=0;
	//printc("bmplistbox construct called");
	}

void bmplistbox::measureitem(LPARAM lParam) {
	//printc("WM_MEASUREITEM");
	lpmis=(LPMEASUREITEMSTRUCT) lParam;
	lpmis->itemHeight=YBITMAP;
	lpmis->itemWidth=XBITMAP;
	}

int bmplistbox::drawitem(LPARAM lParam) {
	HDC hdcMem;
	LPDRAWITEMSTRUCT lpdis;
	RECT rcBitmap;
	
	/*  Declarations for TextOut functions
	TEXTMETRIC tm;
	TCHAR tchBuffer[MAX_PATH];
	int y;
	*/

	//printc("WM_DRAWITEM");
	lpdis=(LPDRAWITEMSTRUCT) lParam;
	if (lpdis->itemID==-1) return(TRUE);
	switch (lpdis->itemAction) {
		case ODA_SELECT:
			//printc("ODA_SELECT");
		case ODA_DRAWENTIRE:
			//printc("ODA_DRAWENTIRE");
			hbmppicture=(HBITMAP) SendMessage(lpdis->hwndItem,LB_GETITEMDATA,lpdis->itemID,(LPARAM)0);
			hdcMem=CreateCompatibleDC(lpdis->hDC); 
			hbmpold=(HBITMAP)SelectObject(hdcMem,hbmppicture); 
 				BitBlt(lpdis->hDC, 
					 lpdis->rcItem.left, lpdis->rcItem.top,
					 lpdis->rcItem.right - lpdis->rcItem.left, 
					 lpdis->rcItem.bottom - lpdis->rcItem.top,
					 hdcMem,0,0,SRCCOPY); 
			/* Display the text associated with the item. */
			/*		
			SendMessage(lpdis->hwndItem, LB_GETTEXT, 
							lpdis->itemID, (LPARAM) tchBuffer); 
			GetTextMetrics(lpdis->hDC,&tm); 
 			y=(lpdis->rcItem.bottom+lpdis->rcItem.top-tm.tmHeight)/2; 
			TextOut(lpdis->hDC,XBITMAP + 6,y,tchBuffer,strlen(tchBuffer)); 
			*/

			SelectObject(hdcMem, hbmpold); 
			DeleteDC(hdcMem); 
					
			/* Is the item selected? */ 
 			if (lpdis->itemState & ODS_SELECTED) { 
				/*Set RECT coordinates to surround only the bitmap*/ 
				rcBitmap.left = lpdis->rcItem.left;
				rcBitmap.top = lpdis->rcItem.top; 
				rcBitmap.right = lpdis->rcItem.left + XBITMAP;
				rcBitmap.bottom = lpdis->rcItem.top + YBITMAP; 
				/*Draw a rectangle around bitmap to indicate the selection.*/
				DrawFocusRect(lpdis->hDC, &rcBitmap);
				} 
			break; 

		case ODA_FOCUS: 
		   /* 
			Do not process focus changes. The focus caret 
			(outline rectangle) indicates the selection. 
			The Which one? (IDOK) button indicates the final 
			selection. 
			*/
			break;
		}
	return (TRUE); 
	}

void bmplistbox::additem (HWND w_ptr,char *itemname,HBITMAP bmp_ptr) {
	int item;
	item=SendMessage(w_ptr,LB_ADDSTRING,0,(long)itemname);
	SendMessage(w_ptr,LB_SETITEMDATA,item,(long)bmp_ptr);
	}

void bmplistbox::addbmp() {
	itembmp_ptr[itemindex]=(HBITMAP)LoadImage(hInst,"bitmaptest.bmp",IMAGE_BITMAP,XBITMAP-2,YBITMAP-2,LR_LOADFROMFILE);
	//wsprintf(string,"bmp->%lx index= %d",itembmp_ptr[itemindex],itemindex);
	//printc(string);
	additem (bmplstwindow,"ItWorks",itembmp_ptr[itemindex]);
	//SetFocus(bmplstwindow);
	//SendMessage(bmplstwindow,LB_SETCURSEL,0,0);
	itemindex++;
	}

void bmplistbox::shutdown() {
	UWORD t;
	if (itemindex) {
		for (t=0;t<itemindex;t++) {
			DeleteObject(itembmp_ptr[t]);
			}
		}
	}

bmplistbox::~bmplistbox() {
	//nothing to clean yet...
	}



