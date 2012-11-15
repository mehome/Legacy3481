#include "HandleMain.h"
#define FT_MTEDIT		200
#define FT_MTSCRUB	201
#define FT_LEFT		202
#define FT_RIGHT		203

int miniscrub::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	class storyboardclass *storyboard=getstoryboard();
	char buffer[24];

	switch(uMsg) {

		case WM_HSCROLL:
			switch LOWORD(wParam) {
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_THUMBTRACK:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
					itoa(SendMessage(mediatimescrub,TBM_GETPOS,0,0),string,10);
					SetWindowText(mediatimeedit,string);
					return(0);
				case TB_BOTTOM:
					itoa(controls->streamptr->actualframes,string,10);
					SetWindowText(mediatimeedit,string);
					return(0);
				case TB_TOP:
					itoa(0,string,10);
					SetWindowText(mediatimeedit,string);
					return(0);
				default: goto noprocessHSCROLL;
				}
			return(0);
noprocessHSCROLL:
			break;

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			switch (notifycode) {

				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case FT_LEFT:
							idleminiscrub=NULL;
							break;
						case FT_RIGHT:
							idleminiscrub=NULL;
							break;
						default: goto noprocessCOMMAND;
						}
					return(0);
					} //end id bn clicked


				case EN_UPDATE: {
					struct imagelist *streamptr=controls->streamptr;
					long source=0;
					long dest=0;
					if (GetWindowText((HWND)lParam,buffer,6)) {
						source=atol(buffer);
						if (debug==2) {
							wsprintf(string,"EN_UPDATE %d",source);
							printc(string);
							}
						//since we only have one edit no need to switch button id
						SendMessage(controls->scrub,TBM_SETPOS,TRUE,mediatimeroffset+source);
						SendMessage(controls->window,WM_HSCROLL,MAKELONG(0,TB_THUMBPOSITION),(LONG)controls->scrub);
						} //end if able to get text
					else {
						if (debug) printc("Update error");
						goto noprocessCOMMAND;
						}
					break;
					} // end EN_UPDATE
				default: goto noprocessCOMMAND;
				} //End switch notifycode
			return(0);
noprocessCOMMAND:
			break;
			} //end WM_COMMAND



		case WM_DRAWITEM: {
			//RECT rc;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;
			int keeplit;

			lpdis=(LPDRAWITEMSTRUCT) lParam;
			//printc("WM_DRAWITEM");
			if (lpdis->itemID==(UINT)-1) return(TRUE);
			switch (lpdis->itemAction)  {
				case ODA_SELECT:
					//printc("ODA_SELECT");
				case ODA_DRAWENTIRE:
					
					/* Is the item selected? */ 
 					if (lpdis->itemState & ODS_SELECTED) { 
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
							hdcMem,((lpdis->CtlID)-195)*SKINGRIDX+1,SKINGRIDY*3+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						//Handle our Button Down
						switch (lpdis->CtlID) {
							case FT_LEFT:
								idleminiscrub=this;
								idlesignals|=IDLE_FTLEFT;
								if (toaster->rtmeoutput) toaster->resync();
								SetEvent(arrayofevents[EVENT_IDLE]);
								break;
							case FT_RIGHT:
								idleminiscrub=this;
								idlesignals|=IDLE_FTRIGHT;
								if (toaster->rtmeoutput) toaster->resync();
								SetEvent(arrayofevents[EVENT_IDLE]);
								break;
							}
						} //end if selected
					else { //Not selected
						//printc("ODA_DRAWENTIRE");
						if ((lpdis->CtlID==FT_LEFT)&&(lpdis->CtlID==FT_RIGHT))
							keeplit=SKINGRIDY+1;
						else keeplit=1;
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
 						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
						hdcMem,((lpdis->CtlID)-195)*SKINGRIDX+1,SKINGRIDY*2+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} // end not selected
					break;
				} // end switch item action
			return (TRUE); 
			}


		case WM_SIZE:
			//TODO configure size
			return (0);


		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


void miniscrub::adjustscrub(int amount) {
	SendMessage(mediatimescrub,TBM_SETPOS,TRUE,SendMessage(mediatimescrub,TBM_GETPOS,0,0)+amount);
	SendMessage(window,WM_HSCROLL,MAKELONG(0,TB_THUMBPOSITION),(LONG)mediatimescrub);
	}

void miniscrub::initminiscrub() {
	struct imagelist *streamptr;
	ULONG mediatimer;
	UINT actualrange;

	if (IsWindowVisible(window)) {
		streamptr=controls->streamptr;
		mediatimer=controls->mediatimer;
		//Init the mini scrub
		if (streamptr) {
			actualrange=streamptr->actualframes;
			if (streamptr->id==id_media) {
				if (streamptr->prev) if (streamptr->prev->id==id_dve)
					actualrange-=streamptr->prev->actualframes;
				if (streamptr->next) if (streamptr->next->id==id_dve)
					actualrange-=streamptr->next->actualframes;
				}
			}
		else actualrange=1;
		SendMessage(mediatimescrub,TBM_SETRANGE,FALSE,MAKELONG(1L,(UWORD)actualrange));
		SendMessage(mediatimescrub,TBM_SETPOS,TRUE,mediatimer);
		wsprintf(string,"%d",mediatimer);
		mediatimeroffset=controls->frameindex-mediatimer;
		SetWindowText(mediatimeedit,string);
		}
	}


miniscrub::miniscrub(miniscrubinput *miniparms) {
	RECT rc;
	int toolx=0;
	int tooly=0;

	parentwindow=miniparms->parent;

	GetClientRect(parentwindow,&rc);

	window=CreateWindowEx(0,"OBJWIN",NULL,
		WS_CHILD|WS_VISIBLE,miniparms->x,miniparms->y,rc.right-miniparms->x,CONTROLBUTTONY,parentwindow,
		NULL,hInst,NULL);

	CreateWindowEx(0,"STATIC","Frame:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,56,CONTROLBUTTONY,window,NULL,hInst,NULL);
	toolx+=56;
	mediatimeedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,window,(HMENU)FT_MTEDIT,hInst,NULL);
	toolx+=48;
	CreateWindowEx(0,"BUTTON","<",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		toolx,tooly,TOOLBUTTONX,CONTROLBUTTONY,window,
		(HMENU)FT_LEFT,hInst,NULL);
	toolx+=TOOLBUTTONX;
	mediatimescrub=CreateWindowEx(0,TRACKBAR_CLASS,"",WS_VISIBLE|WS_CHILD|
		TBS_NOTICKS|TBS_HORZ|TBS_TOP,
		toolx,tooly,rc.right-toolx-miniparms->x-18,CONTROLBUTTONY,window,
		(HMENU)FT_MTSCRUB,hInst,NULL);
	toolx=rc.right-miniparms->x-TOOLBUTTONX;
	CreateWindowEx(0,"BUTTON",">",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		toolx,tooly,TOOLBUTTONX,CONTROLBUTTONY,window,
		(HMENU)FT_RIGHT,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	}

miniscrub::~miniscrub() {
	DestroyWindow(window);
	}

