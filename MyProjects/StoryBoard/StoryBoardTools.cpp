#include "HandleMain.h"  

int storyboardclass::storyboardtools::Callback(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
	{
	char buffer[24];
	class storyboardclass *storyboard=getstoryboard();

	switch(uMsg) {

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			int t;
			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case IDC_SELECT:
							storyboard->pointermode=selection;
							storyboard->selectimageobj->resetlist(TRUE);
							storyboard->updateimagelist();
							break;
						case IDC_DRAGDROP:
							storyboard->pointermode=dragdrop;
							break;
						case IDC_YINYAN:
							storyboard->updateimagelist();
							storyboard->resetframecounter();
							break;
						case IDC_CLIPIN:
							GetWindowText(miniscrubobj->mediatimeedit,buffer,6);
							t=atol(buffer);
							t+=controls->streamptr->cropin-1;
							wsprintf(buffer,"%d",t);
							SetWindowText(inwindow,buffer);
							break;
						case IDC_CLIPOUT:
							GetWindowText(miniscrubobj->mediatimeedit,buffer,6);
							SetWindowText(durwindow,buffer);
							break;
						default: goto skip3;
						}
					InvalidateRect(toolwindow,NULL,FALSE);
					return(0);
					} //end id bn clicked
				case EN_UPDATE: {
					struct imagelist *streamptr=controls->streamptr;
					long source=0;
					long dest=0;

					if (streamptr) {
						if (GetWindowText((HWND)lParam,buffer,6)) {
							source=atol(buffer);
							if (debug==2) {
								wsprintf(string,"EN_UPDATE %d",source);
								printc(string);
								}
							switch (buttonid) {
								case IDC_IN:	
									dest=streamptr->cropin;
									if ((source>=0)&&(source!=dest)&&
										(source<streamptr->totalframes)) {
										if (debug) printc("cropin Changed");
										streamptr->cropin=(short)source;
										//Move up cropout if it is smaller
										if (streamptr->cropout<=source) {
											streamptr->cropout=min(streamptr->actualframes+(short)source,streamptr->totalframes);
											itoa(streamptr->cropout,string,10);
											SetWindowText(outwindow,string);
											}
										controls->adjustframecounter(storyboard,streamptr->linenumber,(short)((streamptr->cropout-source)-streamptr->actualframes),TRUE);
										streamptr->actualframes=(short)(streamptr->cropout-source);
										itoa(streamptr->cropout-source,string,10);
										SetWindowText(durwindow,string);
										}
									break;	
								case IDC_OUT:
									dest=streamptr->cropout;
									if ((source>0)&&(source!=dest)&&
										(source>streamptr->cropin)) {
										if (debug) printc("cropout Changed");
										if	(source>streamptr->totalframes) {
											source=streamptr->totalframes;
											wsprintf(string,"The maximum frames for this media is %ld",source);
											printc(string);
											}
										streamptr->cropout=(UWORD)source;
										//cropout our rtv audio too if exists
										if (streamptr->rtvaudio) ((struct wavinfo *)streamptr->rtvaudio)->out=(UWORD)source;
										audio->updateimagelist();
										streamptr->actualframes+=(short)(source-dest);
										controls->adjustframecounter(storyboard,streamptr->linenumber,(short)(source-dest),TRUE);
										itoa(source-streamptr->cropin,string,10);
										SetWindowText(durwindow,string);
										//Manually set the out to the highest value
										if (source==streamptr->totalframes) {
											itoa(source,string,10);
											SetWindowText((HWND)lParam,string);
											}
										}
									break;
								case IDC_DUR: {
									long medias;
									//Determine whether were on a media or dve
									if (streamptr->id==id_dve) {
										//next and prev might be NULL
										dest=streamptr->duration;
										if (streamptr->next) medias=streamptr->next->actualframes;
										if (streamptr->prev) if (streamptr->prev->actualframes<medias)
											medias=streamptr->prev->actualframes;

										if ((source>0)&&(source!=dest)&&
											(source<=medias)) {
											if (debug) printc("duration Changed");
											streamptr->duration=(short)source;
											controls->frameindex+=dest-source;
											controls->oldframeindex+=dest-source;
											controls->adjustframecounter(storyboard,streamptr->linenumber,(short)(dest-source),TRUE);
											}
										}
									else { //Setting actualframes and cropout
										dest=streamptr->actualframes;
										if ((source>0)&&(source!=dest)) {
											if (debug) printc("actualframes Changed");
											if	(source+streamptr->cropin>streamptr->totalframes) {
												source=streamptr->totalframes-streamptr->cropin;
												wsprintf(string,"With \"In:\" set to %ld, the maximum duration for this media is %ld",streamptr->cropin,source);
												printc(string);
												}
											streamptr->cropout=(UWORD)(source+streamptr->cropin);
											//cropout our rtv audio too if exists
											if (streamptr->rtvaudio) ((struct wavinfo *)streamptr->rtvaudio)->out=(UWORD)source;
											audio->updateimagelist();
											streamptr->actualframes+=(short)(source-dest);
											controls->adjustframecounter(storyboard,streamptr->linenumber,(short)(source-dest),TRUE);
											itoa(source+streamptr->cropin,string,10);
											SetWindowText(outwindow,string);
											//Manually set the out to the highest value
											if (source==streamptr->totalframes-streamptr->cropin) {
												itoa(source,string,10);
												SetWindowText((HWND)lParam,string);
												}
											}
										}
									miniscrubobj->initminiscrub();
									break;
									}
								} //end switch button id
							}
						else if (debug) printc("Update error");
						break;
						} //end if streamptr
					} // end EN_UPDATE
				} //End switch notifycode
			skip3:
			break;
			}

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
							hdcMem,((lpdis->CtlID)-TB_IDCOFFSET)*SKINGRIDX+1,SKINGRIDY*3+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} //end if selected
					else { //Not selected
						//printc("ODA_DRAWENTIRE");
						if (((lpdis->CtlID==IDC_SELECT)&&(storyboard->pointermode==selection))||
							((lpdis->CtlID==IDC_DRAGDROP)&&(storyboard->pointermode==dragdrop)))
							keeplit=SKINGRIDY*3+1;
						else keeplit=SKINGRIDY*2+1;
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
 						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
						hdcMem,((lpdis->CtlID)-TB_IDCOFFSET)*SKINGRIDX+1,keeplit,SRCCOPY); 
	
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


void storyboardclass::storyboardtools::createwindow(HWND w_ptr) {

	toolwindow=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|
		WS_DLGFRAME|WS_CHILD,235,200,300,220,w_ptr,
		(HMENU)IDC_TOOLS,hInst,NULL);

	SetWindowLong(toolwindow,GWL_USERDATA,(long)this);
	}


void storyboardclass::storyboardtools::startup() {
	HMENU idc_tools[5] = {
		(HMENU)IDC_YINYAN,
		(HMENU)IDC_SELECT,
		(HMENU)IDC_CLIPIN,
		(HMENU)IDC_DRAGDROP,
		(HMENU)IDC_CLIPOUT};
	struct miniscrubinput miniparms={0,0,toolwindow,hInst};
	int toolx;
	UBYTE t;

	for (t=0;t<TB_NUMOFBUTTONS;t++) {

		CreateWindowEx(0,"BUTTON","test",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
			t*TOOLBUTTONX,0,TOOLBUTTONX,CONTROLBUTTONY,toolwindow,
			idc_tools[t],hInst,NULL);
		}
	toolx=t*TOOLBUTTONX;

	// Now to dreate the child IOD textbox imagelist edits
	CreateWindowEx(0,"STATIC","In:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,0,24,CONTROLBUTTONY,toolwindow,NULL,hInst,NULL);
	toolx+=24;
	inwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		WS_DISABLED|ES_CENTER|ES_NUMBER,
		toolx,0,48,CONTROLBUTTONY,toolwindow,(HMENU)IDC_IN,hInst,NULL);
	toolx+=48;
	CreateWindowEx(0,"STATIC","Out:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,0,30,CONTROLBUTTONY,toolwindow,NULL,hInst,NULL);
	toolx+=30;
	outwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		WS_DISABLED|ES_CENTER|ES_NUMBER,
		toolx,0,48,CONTROLBUTTONY,toolwindow,(HMENU)IDC_OUT,hInst,NULL);
	toolx+=48;
	CreateWindowEx(0,"STATIC","Dur:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,0,30,CONTROLBUTTONY,toolwindow,NULL,hInst,NULL);
	toolx+=30;
	durwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		WS_DISABLED|ES_CENTER|ES_NUMBER,
		toolx,0,48,CONTROLBUTTONY,toolwindow,(HMENU)IDC_DUR,hInst,NULL);
	toolx+=48;
	miniparms.x=toolx;
	miniscrubobj=new miniscrub(&miniparms);
	}


void storyboardclass::storyboardtools::shutdown() {
	delete miniscrubobj;
	}

  /**************************************************/
 /*      Here is all the StoryBoard Base stuff     */
/**************************************************/

storyboardclass::storyboardclass() {
	pointermode=dragdrop;
	numrows=numcolumns=2;  //dummy init
	smartrefreshclip=NULL;
	imagelisthead=scrollpos=removethis=NULL;
	olditem=-1;
	newitem=-1;
	freezeglow=FALSE;
	framecounter=0;
	//Lets set our blendfunction for updateglow()
	bf.BlendOp=AC_SRC_OVER;
	bf.BlendFlags=NULL;
	bf.SourceConstantAlpha=60; //original 60
	bf.AlphaFormat=AC_SRC_ALPHA;
	oldglowpos=0;
	oldstreamptr=NULL;
	imagecount=0;
	scrolloffset=0;
	selectimageobj=new selectimageclass();
	}

void storyboardclass::createwindow(HWND w_ptr) 
	{
	HDC hdc,smartdc;
	RECT rc;

	tools.createwindow(w_ptr);

	storywindow=CreateWindowEx(0,"OBJWIN","",WS_VISIBLE|WS_DLGFRAME|
		WS_VSCROLL|WS_CHILD,235,220,300,260,w_ptr,
		(HMENU)IDC_STORYBOARD,hInst,NULL);

	SetWindowLong(storywindow,GWL_USERDATA,(long)this);

	//Let's set up the filler.
	//fillerimage=(HBITMAP)LoadImage(hInst,"Resources\\filler.bmp",IMAGE_BITMAP,XBITMAP-2,YBITMAP-2,LR_LOADFROMFILE);
	/* This will create a SmartRefresh Bitmap*/
	hdc=GetDC(storywindow);
	smartdc=CreateCompatibleDC(hdc);
	rc.top=rc.left=0;
	rc.right=XBITMAP;
	rc.bottom=YBITMAP;
	fillerimage=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
	/* Initialize the area */
	SelectObject(smartdc,fillerimage);
	FillRect(smartdc,&rc,(HBRUSH)(COLOR_WINDOW+1));
	DeleteDC(smartdc);
	ReleaseDC(storywindow,hdc);
	}


void storyboardclass::resize() {
	RECT rc;
	// This is to get maxsize for mediaedit only
	// To be moved once this becomes it's own object
	GetWindowRect(screen,&rc);

	LONG y=size100notheight(screen,tools.toolwindow,27,0,100,TOOLBUTTONY+6);
	size100noty(screen,storywindow,27,y,100,65);

	configuresize();
	olditem=-1;newitem=-1;
	if (imagelisthead) updateimagelist();
	}


void storyboardclass::configuresize() {
	RECT rc;

	GetClientRect(storywindow,&rc);
	numcolumns=((rc.right)/XBITMAP);
	sb_offsetx=((rc.right)%XBITMAP)/2;
	numrows=(rc.bottom)/YBITMAP;
	//sb_offsety=((rc.bottom)%YBITMAP)/2;
	sb_offsety=1; //We need a throwaway area
	if (debug) {wsprintf(string,"x->%d,y->%d",numcolumns,numrows);printc(string);}
	}

void storyboardclass::startup() {
	SCROLLINFO si;
	tools.startup();
	//StoryBoard Main Window startup here
	smartrefreshclip=createclipwin(storywindow,(HBRUSH)(COLOR_WINDOW+1));
	//Set storyboards scrollbar to full position
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_PAGE|SIF_RANGE|SIF_POS;
	GetScrollInfo(storywindow,SB_VERT,&si);
	si.nMax=0;
	si.nMin=0;
	si.nPage=si.nMax;
	si.nPos=si.nMin;
	SetScrollInfo(storywindow,SB_VERT,&si,TRUE);
	}


void storyboardclass::shutdown() {
	struct imagelist *index=imagelisthead;

	tools.shutdown();
	if (index) {
		DeleteObject(index->image);
		while (index=index->next) {
			DeleteObject(index->image);
			postmediafx(index);
			}
		}
	//destroy the filler images
	if (fillerimage) DeleteObject(fillerimage);

	if (smartrefreshclip) DeleteObject(smartrefreshclip);
	}


storyboardclass::~storyboardclass() {
	delete selectimageobj;
	//Nothing to clean yet...
	}
