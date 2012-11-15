#include "HandleMain.h"

int audioclass::audioguiclass::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	char buffer[24];
	wavinfo *audiowav;
	struct filterswindowlist *audiowindowptr;

	switch(uMsg) {

		case WM_HSCROLL:
			nostaticpreview=0;
			audiowav=getnode(w_ptr,&audiowindowptr);
			switch LOWORD(wParam) {
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_THUMBTRACK:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
					itoa(SendMessage((((struct audiogui *)audiowindowptr)->audioscrub),TBM_GETPOS,0,0),string,10);
					SetWindowText((((struct audiogui *)audiowindowptr)->audioed),string);
					return(0);
				case TB_BOTTOM:
					itoa(controls->streamptr->totalframes,string,10);
					SetWindowText((((struct audiogui *)audiowindowptr)->audioed),string);
					return(0);
				case TB_TOP:
					itoa(0,string,10);
					SetWindowText((((struct audiogui *)audiowindowptr)->audioed),string);
					return(0);
				default: goto noprocessHSCROLL;
				}
			return(0);
noprocessHSCROLL:
			break;

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			struct audiogui *audiowindowindex;
			audiowav=getnode(w_ptr,&audiowindowptr);
			audiowindowindex=(struct audiogui *)audiowindowptr;
			struct imagelist *streamptr=controls->streamptr;

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case AD_CLOSE:
							goto closeaudioskip;
						case AD_XP:
							GetWindowText(audio->miniscrubobj->mediatimeedit,buffer,6);
							SetWindowText(audiowindowindex->inwindow,buffer);
							break;
						case AD_YP:
							GetWindowText(audio->miniscrubobj->mediatimeedit,buffer,6);
							SetWindowText(audiowindowindex->durwindow,buffer);
							break;
						default: goto noprocessCOMMAND;
						}
					return(0);
					} //end id bn clicked

				case EN_UPDATE: {
					long source=0;
					long dest=0;

					if (GetWindowText((HWND)lParam,buffer,6)) {
						source=atol(buffer);
						if (debug==2) {
							wsprintf(string,"EN_UPDATE %d",source);
							printc(string);
							}
						switch (buttonid) {
							case AD_IN:	
								dest=audiowav->in;
								if ((source>=0)&&(source!=dest)&&
									(source<streamptr->totalframes)) {
									if (debug) printc("cropin Changed");
									audiowav->in=(UWORD)source;
									//Move up cropout if it is smaller
									if (audiowav->out<=source) {
										audiowav->out=min(streamptr->actualframes+source,streamptr->totalframes);
										itoa(audiowav->out,string,10);
										SetWindowText(audiowindowindex->outwindow,string);
										}
									itoa(audiowav->out-source,string,10);
									SetWindowText(audiowindowindex->durwindow,string);
									}
								break;	
							case AD_OUT:
								if (!source) audiowav->out=0; //special zero case to allow sound to reset to no crop
								dest=audiowav->out;
								if ((source>0)&&(source!=dest)&&
									(source>audiowav->in)) {
									if (debug) printc("cropout Changed");
									if	(source>streamptr->totalframes) {
										source=streamptr->totalframes;
										wsprintf(string,"The maximum frames for this media is %ld",source);
										printc(string);
										}
									audiowav->out=(UWORD)source;
									itoa(source-audiowav->in,string,10);
									SetWindowText(audiowindowindex->durwindow,string);
									//Manually set the out to the highest value
									if (source==streamptr->totalframes) {
										itoa(source,string,10);
										SetWindowText((HWND)lParam,string);
										}
									}
								break;
							case AD_DUR:
								if (!source) audiowav->out=0; //special zero case to allow sound to reset to no crop
								//Setting actualframes and cropout
								dest=audiowav->out-audiowav->in;
								if ((source>0)&&(source!=dest)) {
									if (debug) printc("actualframes Changed");
									if	(source+audiowav->in>streamptr->totalframes) {
										source=streamptr->totalframes-audiowav->in;
										wsprintf(string,"With \"In:\" set to %ld, the maximum duration for this media is %ld",audiowav->in,source);
										printc(string);
										}
									audiowav->out=(UWORD)(source+audiowav->in);
									itoa(source+audiowav->in,string,10);
									SetWindowText(audiowindowindex->outwindow,string);
									//Manually set the out to the highest value
									if (source==streamptr->totalframes-audiowav->in) {
										itoa(source,string,10);
										SetWindowText((HWND)lParam,string);
										}
									}
								break;
							case AD_OFFSETED:
								audiowav->frameoffset=(short)source;
								break;
							default: goto noprocessCOMMAND;
							} //end switch button id
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

		case WM_CLOSE: {
closeaudioskip:
			struct audionode *audioprev,*audionext;
			if (debug) printc("Close Audio");
			closeaudio(audiowav=getnode(w_ptr,&audiowindowptr));
			//remove audio from link
			audioprev=audiowav->node.prev;
			audionext=audiowav->node.next;
			if (audioprev) {
				audioprev->next=audionext;
				if (audioprev->prev==NULL) controls->streamptr->audio=audioprev;
				}
			else controls->streamptr->audio=audionext;
			if (audionext) audionext->prev=audioprev;
			disposenode(nodeobject,(struct memlist *)audiowav);
			//updategui
			audio->updateimagelist();
			return(0L);
			} //end WM_CLOSE 

		case WM_DRAWITEM: {
			//RECT rc;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;

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
							hdcMem,((lpdis->CtlID)-202)*SKINGRIDX+1,SKINGRIDY*3+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} //end if selected
					else { //Not selected
						//printc("ODA_DRAWENTIRE");
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
 						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
						hdcMem,((lpdis->CtlID)-202)*SKINGRIDX+1,SKINGRIDY*2+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} // end not selected
					break; 
				} // end switch item action
			return (TRUE); 
			}

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


struct wavinfo *audioclass::audioguiclass::getnode(HWND w_ptr,struct filterswindowlist **audiowindowindex) {
	struct imagelist *streamptr=controls->streamptr;
	audionode *audioindex=NULL;
	*audiowindowindex=audio->windowlist;

	if ((streamptr)&&(*audiowindowindex)) {
		audioindex=streamptr->audio;
		}
	else return(0);

	//Take w_ptr compare against CGwindowlist to get our node

	if (audioindex) {
		do {
			//See if we have any windows created already
			if (*audiowindowindex) {
				if ((*audiowindowindex)->window==w_ptr) break;
				else *audiowindowindex=(*audiowindowindex)->next;
				}
			} while (audioindex=audioindex->next);
		}

	return ((struct wavinfo *)audioindex);
	}


void audioclass::audioguiclass::closeaudio(struct wavinfo *audiowav) {
	if (audiowav) {
		audio->player.closevoices();
		if (audiowav->filesource) {
			disposenode(nodeobject,(struct memlist *)audiowav->filesource);
			audiowav->filesource=NULL;
			}
		if (audiowav->name) {
			disposenode(nodeobject,(struct memlist *)audiowav->name);
			audiowav->name=NULL;
			}
		}
	}


void audioclass::audioguiclass::initcontrols(struct filterswindowlist *audiowindowptr,struct audionode *audioptr) {
	struct audiogui *audiowindowindex=(struct audiogui *)audiowindowptr;
	struct wavinfo  *audioindex=(struct wavinfo  *)audioptr;
	SetWindowText(((struct audiogui *)audiowindowptr)->caption,audioindex->name);

	wsprintf(string,"%d",audioindex->in);
	SetWindowText(audiowindowindex->inwindow,string);
	wsprintf(string,"%d",audioindex->out);
	SetWindowText(audiowindowindex->outwindow,string);
	wsprintf(string,"%d",audioindex->out-audioindex->in);
	SetWindowText(audiowindowindex->durwindow,string);
	wsprintf(string,"%d",audioindex->frameoffset);
	SetWindowText(audiowindowindex->audioed,string);
	SendMessage(audiowindowindex->audioscrub,TBM_SETPOS,TRUE,audioindex->frameoffset);
	SendMessage(audiowindowindex->audioscrub,TBM_SETRANGE,TRUE,MAKELONG(0L,controls->streamptr->totalframes));
	}


HWND audioclass::audioguiclass::getnewgui(struct filterswindowlist *audiowindowptr,HWND audiowindow,ULONG count) {
	//Gui stuff
	struct audiogui *audiowindowindex=(struct audiogui *)audiowindowptr;
	HWND maincontrol;
	RECT rc;
	int toolx=0;
	int tooly=0;

	GetClientRect(audiowindow,&rc);

	//Init all windows that will be opened later
	maincontrol=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_DLGFRAME|
		WS_CHILD,0,0,200,100,audiowindow,NULL,hInst,NULL);

	//TODO move resize to separate function for handler to call
	size100notyorh(audiowindow,maincontrol,0,100*count+CONTROLBUTTONY,100,100);
	SetWindowLong(maincontrol,GWL_USERDATA,(long)this);

	//And now the gadgets for the window
	audiowindowindex->caption=CreateWindowEx(0,"STATIC","",WS_VISIBLE|WS_CHILD,
		toolx+8,tooly,rc.right-40,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx=rc.right-28;
	CreateWindowEx(0,"BUTTON","x",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly+4,18,18,maincontrol,
		(HMENU)AD_CLOSE,hInst,NULL);
	toolx=0;
	tooly+=24;

	CreateWindowEx(0,"STATIC","In:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,24,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=24;
	CreateWindowEx(0,"BUTTON","<",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		toolx,tooly,TOOLBUTTONX,CONTROLBUTTONY,maincontrol,
		(HMENU)AD_XP,hInst,NULL);
	toolx+=TOOLBUTTONX;
	audiowindowindex->inwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)AD_IN,hInst,NULL);
	toolx+=48;
	CreateWindowEx(0,"STATIC","Out:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,30,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=30;
	audiowindowindex->outwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)AD_OUT,hInst,NULL);
	toolx+=48;
	CreateWindowEx(0,"BUTTON",">",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		toolx,tooly,TOOLBUTTONX,CONTROLBUTTONY,maincontrol,
		(HMENU)AD_YP,hInst,NULL);
	toolx+=TOOLBUTTONX;
	CreateWindowEx(0,"STATIC","Dur:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,30,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=30;
	audiowindowindex->durwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)AD_DUR,hInst,NULL);
	toolx=8;
	tooly+=24;


	CreateWindowEx(0,"STATIC","Offset:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,56,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=56;
	audiowindowindex->audioed=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)AD_OFFSETED,hInst,NULL);
	toolx+=48;
	audiowindowindex->audioscrub=CreateWindowEx(0,TRACKBAR_CLASS,"",WS_VISIBLE|WS_CHILD|
		TBS_NOTICKS|TBS_HORZ|TBS_TOP,
		toolx,tooly,rc.right-toolx-8,CONTROLBUTTONY,maincontrol,
		(HMENU)AD_OFFSETTB,hInst,NULL);
	toolx=0;
	tooly+=24;

	return (maincontrol);
	}

ULONG audioclass::audioguiclass::getwindowlistsize() {
	return (sizeof(struct audiogui));
	}
