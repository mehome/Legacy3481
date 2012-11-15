#include "handlemain.h"

#define DR_CLIPNAME		200
#define DR_AUDIOCHK		201
#define DR_AUDIOPATH		202
#define DR_AUDIOASL		203
#define DR_VIDEOCHK		204
#define DR_VIDEOPATH		205
#define DR_VIDEOASL		206
#define DR_START			207
#define DR_SOURCEFILE	208
#define DR_SOURCEASL		209
#define DR_MINBT			210
#define DR_CLOSEBT		211
#define DR_OPTIONSBT		212

const static int controlx[13]={0,301,0,241,281,0,241,81,0,241,1,41,161};
static POINT mask[13];



int dv2rtvclass::Callback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	char buffer[MAX_PATH];

	switch(uMsg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(window,&ps);
			smartrefresh(ps.hdc,smartrefreshclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(window,&ps);
			return (0);
			}
		case WM_MOUSEMOVE: {
			struct tagPOINT mouseloc;
			if (dragtoggle) {
				GetCursorPos(&mouseloc);
				SetWindowPos(window,NULL,mouseloc.x-xcoord,mouseloc.y-ycoord,0,0,SWP_NOSIZE|SWP_NOZORDER);
				}
			break;
			}
		case WM_LBUTTONUP:
			if (dragtoggle) {
				dragtoggle=FALSE;
				ReleaseCapture();
				}
			break;

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case DR_AUDIOCHK:
							//if (SendMessage(audiochk,BM_GETCHECK,0,0)==BST_CHECKED) {
							//	SendMessage(audiochk,BM_SETCHECK,BST_UNCHECKED,0);
							if (audioon) {
								audioon=FALSE;
								EnableWindow(audiopathedit,FALSE);
								EnableWindow(audioasl,FALSE);
								InvalidateRect(audiochk,NULL,FALSE);
								}
							else {
								//SendMessage(audiochk,BM_SETCHECK,BST_CHECKED,0);
								audioon=TRUE;
								EnableWindow(audiopathedit,TRUE);
								EnableWindow(audioasl,TRUE);
								InvalidateRect(audiochk,NULL,FALSE);
								}
							break;
						case DR_VIDEOCHK:
							//if (SendMessage(videochk,BM_GETCHECK,0,0)==BST_CHECKED) {
							//	SendMessage(videochk,BM_SETCHECK,BST_UNCHECKED,0);
							if (videoon) {
								videoon=FALSE;
								EnableWindow(videopathedit,FALSE);
								EnableWindow(videoasl,FALSE);
								InvalidateRect(videochk,NULL,FALSE);
								}
							else {
								//SendMessage(videochk,BM_SETCHECK,BST_CHECKED,0);
								videoon=TRUE;
								EnableWindow(videopathedit,TRUE);
								EnableWindow(videoasl,TRUE);
								InvalidateRect(videochk,NULL,FALSE);
								}
							break;
						case DR_AUDIOASL:
							getdirectory("Select Audio Path",audiopath);
							SetWindowText(audiopathedit,audiopath);
							break;
						case DR_VIDEOASL:
							getdirectory("Select Video Path",videopath);
							SetWindowText(videopathedit,videopath);
							break;
						case DR_SOURCEASL:
getsource:
							if (getopenfile(sourceavi,clipname,NULL,NULL,"Please select AVI DV file.",NULL,0))
							SetWindowText(sourcefileedit,sourceavi);
							//lets assume the filename has the .avi extension
							//The Filters will in the browser will enforce this
							if (clipname[strlen(clipname)-4]=='.') clipname[strlen(clipname)-4]=0;
							SetWindowText(clipnameedit,clipname);
							break;
						case DR_START:
							if (playing) playing=0;
							else {
								if (sourceavi[0]==0) {
									printerror("Please Select a source file to Convert...");
									goto getsource;
									}
								else {
									//print("dv2rtv...");
									streamthreadmode=0;
									SetEvent(arrayofevents[EVENT_STREAM]);
									}
								}
							break;
						//Heres the extra buttons
						case DR_MINBT:
							ShowWindow(window,SW_MINIMIZE);
							break;
						case DR_CLOSEBT:
							if (toaster->rtmeoutput) toaster->initmasterframe("resources\\shutdown.rtv");
							DestroyWindow(screen);
							break;
						case DR_OPTIONSBT:
							//ShowWindow(screen,SW_SHOW);
							windowtoggle(screen,screen,NULL);
							break;
						default: goto noprocessCOMMAND;
						}
					return(0);
					} //end id bn clicked

				case EN_UPDATE: {
					long source=0;
					long dest=0;

					if (GetWindowText((HWND)lParam,buffer,MAX_PATH)) {
						source=atol(buffer);
						if (debug==2) {
							wsprintf(string,"EN_UPDATE %d",source);
							printc(string);
							}
						switch (buttonid) {
							case DR_CLIPNAME:
								GetWindowText(clipnameedit,clipname,32);
								break;
							case DR_AUDIOPATH:
								GetWindowText(audiopathedit,audiopath,MAX_PATH);
								break;
							case DR_VIDEOPATH:
								GetWindowText(videopathedit,videopath,MAX_PATH);
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

		case WM_DRAWITEM: {
			//RECT rc;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;
			int keeplit,width,height;

			lpdis=(LPDRAWITEMSTRUCT) lParam;
			//printc("WM_DRAWITEM");
			if (lpdis->itemID==(UINT)-1) return(TRUE);
			switch (lpdis->itemAction)  {
				case ODA_SELECT:
					//printc("ODA_SELECT");
				case ODA_DRAWENTIRE:
					
					// Is the item selected?  
 					if (lpdis->itemState & ODS_SELECTED) { 
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skin2bmp);
						width=lpdis->rcItem.right-lpdis->rcItem.left;
						height=lpdis->rcItem.bottom-lpdis->rcItem.top;
						TransparentBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							width,height,
							hdcMem,controlx[((lpdis->CtlID)-200)],81,width,height,0); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} //end if selected
					else { //Not selected
						//printc("ODA_DRAWENTIRE");
						//if ((lpdis->CtlID==DR_START)&&playing)
						//	keeplit=81;
						//else keeplit=1;
						keeplit=1;
						switch (lpdis->CtlID) {
							case DR_START:
								if (playing) goto keepon;
								break;
							case DR_AUDIOCHK:
								if (audioon) goto keepon;
								break;
							case DR_VIDEOCHK:
								if (videoon) goto keepon;
								break;
keepon:
								keeplit=81;
							}
						hdcMem=CreateCompatibleDC(lpdis->hDC);
						hbmpold=(HDC)SelectObject(hdcMem,smartrefreshclip);
						width=lpdis->rcItem.right-lpdis->rcItem.left;
						height=lpdis->rcItem.bottom-lpdis->rcItem.top;

						BitBlt (lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,width,height,
							hdcMem,mask[(lpdis->CtlID)-200].x,mask[(lpdis->CtlID)-200].y,SRCCOPY);
						SelectObject(hdcMem,skin2bmp);
 						TransparentBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							width,height,
						hdcMem,controlx[((lpdis->CtlID)-200)],keeplit,width,height,0); 
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} // end not selected
					break; 
				} // end switch item action
			return (TRUE); 
			}

		case WM_LBUTTONDOWN: {
			POINT mouseloc;
			RECT rc;

			GetCursorPos(&mouseloc);
			GetWindowRect(window,&rc);
			// x=4 y=22 if windowframe is on
			xcoord=mouseloc.x-rc.left;
			ycoord=mouseloc.y-rc.top;
			dragtoggle=TRUE;
			SetCapture(window);
			//printc("x=%d y=%d",xcoord,ycoord);
			//We are not going to process this
			break;
			}

		case WM_CLOSE:
			windowtoggle(screen,window,IDM_DV2RTV);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}



void dv2rtvclass::createwindow(HWND w_ptr) {
	struct progressparms prginput= {"Progress..",20,10,250,4,NULL,
		NULL,skin2bmp,NULL,NULL};

	int toolx=0;
	int tooly=0;
	POINT polypts[24]={45,22,45+3,22,78-10,22,74+4,0+22,99+4,6+23,259+4,68+23,297+4,85+24,327+4,104+24,339+4,115+23,
		355,175,332+4,213+21,310+4,228+22,267+4,249+22,100+4,316+22,
		4+4,258+22,6+4,68+22,7+4,47+22};
	HRGN hrgn,hrgn2;

	window=CreateWindowEx(WS_EX_APPWINDOW,"OBJGRAYWIN","Video Toaster - DV capture kit",
		WS_VISIBLE|WS_POPUP|WS_CAPTION,4,4,360,348,w_ptr,
		NULL,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(long)this);

	hrgn = CreatePolygonRgn(polypts,17,WINDING);
	hrgn2 = CreateEllipticRgn(12+5,1+23,78+5,71+23);

	CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);
	hrgn2 = CreateEllipticRgn(7+5,22+22,27+5,102+22);
	CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);
	hrgn2 = CreateEllipticRgn(287+4,108+22,351+4,180+22);
	CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);
	hrgn2 = CreateEllipticRgn(289+4,110+23,353+4,214+23);
	CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);
	hrgn2 = CreateEllipticRgn(5+4,180+22,97+4,320+22);
	CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);
	hrgn2 = CreateEllipticRgn(21+6,270+25,113+6,322+25);
	CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);

	SetWindowRgn(window,hrgn,TRUE);
	DeleteObject(hrgn);
	DeleteObject(hrgn2);

	smartrefreshclip=createbitmapwin(window,"Resources\\background.bmp");

	//since we are not going to resize we'll put the children here
	startstopbt=CreateWindowEx(0,"BUTTON","Start",WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		20,235,77,77,window,
		(HMENU)DR_START,hInst,NULL);
	mask[DR_START-200].x=20;
	mask[DR_START-200].y=235;
	hrgn = CreateEllipticRgn(0,0,77,77);
	SetWindowRgn(startstopbt,hrgn,TRUE);
	DeleteObject(hrgn);

	tooly=68;
	toolx=100;
/*
	CreateWindowEx(0,"STATIC","Source File:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,80,CONTROLBUTTONY,window,NULL,hInst,NULL);
*/
	tooly+=24;

	sourceasl=CreateWindowEx(0,"BUTTON","...",WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		20,tooly,27,24,window,
		(HMENU)DR_SOURCEASL,hInst,NULL);
	mask[DR_SOURCEASL-200].x=20;
	mask[DR_SOURCEASL-200].y=tooly;
	toolx=50;
	hrgn = CreateEllipticRgn(0,0,27,24);
	SetWindowRgn(sourceasl,hrgn,TRUE);
	DeleteObject(hrgn);

	sourcefileedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
		,toolx,tooly,180,CONTROLBUTTONY,window,(HMENU)DR_SOURCEFILE,hInst,NULL);
	SetWindowText(sourcefileedit,"Select AVI DV file");

	toolx=20;
	tooly+=24;
/*
	CreateWindowEx(0,"STATIC","Clip Name:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,80,CONTROLBUTTONY,window,NULL,hInst,NULL);
	toolx+=84;
*/
	toolx+=40;
	clipnameedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
		,toolx,tooly,150,CONTROLBUTTONY,window,(HMENU)DR_CLIPNAME,hInst,NULL);
	SetWindowText(clipnameedit,clipname);
	toolx=0;
	tooly+=24;
	//progress indicator here
	prginput.y=tooly+8;
	prginput.backloc.x=prginput.x;
	prginput.backloc.y=prginput.y;
	prginput.parent=window;
	prginput.barloc.x=1;
	prginput.barloc.y=161;
	prginput.backbmp=smartrefreshclip;
	progressobj=new progressclass(&prginput);
	tooly+=24;

	audioasl=CreateWindowEx(0,"BUTTON","...",WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		20,tooly,27,24,window,
		(HMENU)DR_AUDIOASL,hInst,NULL);
	mask[DR_AUDIOASL-200].x=20;
	mask[DR_AUDIOASL-200].y=tooly;
	toolx=50;
	hrgn = CreateEllipticRgn(0,0,27,24);
	SetWindowRgn(audioasl,hrgn,TRUE);
	DeleteObject(hrgn);

	audiopathedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
		,toolx,tooly,200,CONTROLBUTTONY,window,(HMENU)DR_AUDIOPATH,hInst,NULL);
	SetWindowText(audiopathedit,audiopath);
	toolx=0;
	tooly+=24;

	//80,tooly,64,CONTROLBUTTONY
	videochk=CreateWindowEx(0,"BUTTON","Video",WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		100,tooly,19,15,window,
		(HMENU)DR_VIDEOCHK,hInst,NULL);
	//SendMessage(videochk,BM_SETCHECK,BST_CHECKED,0);
	mask[DR_VIDEOCHK-200].x=120;
	mask[DR_VIDEOCHK-200].y=tooly;

	//80,tooly,64,CONTROLBUTTONY
	audiochk=CreateWindowEx(0,"BUTTON","Audio",WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		120,tooly,19,15,window,
		(HMENU)DR_AUDIOCHK,hInst,NULL);
	//SendMessage(audiochk,BM_SETCHECK,BST_CHECKED,0);
	mask[DR_AUDIOCHK-200].x=120;
	mask[DR_AUDIOCHK-200].y=tooly;

	tooly+=24;
	videoasl=CreateWindowEx(0,"BUTTON","...",WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		20,tooly,27,24,window,
		(HMENU)DR_VIDEOASL,hInst,NULL);
	mask[DR_VIDEOASL-200].x=20;
	mask[DR_VIDEOASL-200].y=tooly;
	toolx+=50;
	hrgn = CreateEllipticRgn(0,0,27,24);
	SetWindowRgn(videoasl,hrgn,TRUE);
	DeleteObject(hrgn);

	videopathedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
		,toolx,tooly,170,CONTROLBUTTONY,window,(HMENU)DR_VIDEOPATH,hInst,NULL);
	SetWindowText(videopathedit,videopath);
	toolx=0;
	tooly+=40;

	//All these buttons are the extras added for this gui interface
	toolx=270;
	minbt=CreateWindowEx(0,"BUTTON",NULL,WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		toolx,120,37,77,window,
		(HMENU)DR_MINBT,hInst,NULL);
	mask[DR_MINBT-200].x=toolx;
	mask[DR_MINBT-200].y=120;
	toolx+=38;

	hrgn = CreateEllipticRgn(0, 0, 77, 77);
	hrgn2 = CreateRectRgn(0, 0, 77, 77);
	OffsetRgn(hrgn2, 37, 0);
	CombineRgn(hrgn, hrgn, hrgn2, RGN_DIFF);
	SetWindowRgn(minbt,hrgn,TRUE);
	DeleteObject(hrgn);
	DeleteObject(hrgn2);

	closebt=CreateWindowEx(0,"BUTTON",NULL,WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		toolx,120,37,77,window,
		(HMENU)DR_CLOSEBT,hInst,NULL);
	mask[DR_CLOSEBT-200].x=toolx;
	mask[DR_CLOSEBT-200].y=120;

	hrgn = CreateEllipticRgn(0, 0, 77, 77);
	hrgn2 = CreateRectRgn(0, 0, 77, 77);
	OffsetRgn(hrgn2, -37, 0);
	CombineRgn(hrgn, hrgn, hrgn2, RGN_DIFF);
	OffsetRgn(hrgn, -38, 0);
	SetWindowRgn(closebt,hrgn,TRUE);
	DeleteObject(hrgn);
	DeleteObject(hrgn2);

	optionsbt=CreateWindowEx(0,"BUTTON",NULL,WS_VISIBLE|WS_CHILD|BS_OWNERDRAW,
		20,10,77,77,window,
		(HMENU)DR_OPTIONSBT,hInst,NULL);
	mask[DR_OPTIONSBT-200].x=20;
	mask[DR_OPTIONSBT-200].y=10;
	hrgn = CreateEllipticRgn(0,0,77,77);
	SetWindowRgn(optionsbt,hrgn,TRUE);
	DeleteObject(hrgn);
	}
