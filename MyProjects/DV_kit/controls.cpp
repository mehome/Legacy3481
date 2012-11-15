#include "HandleMain.h"

int controlsclass::Callback(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{

	switch(uMsg) {


		case WM_COMMAND: {
			//nostaticpreview=nostaticxypreview=-1;
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			if (notifycode==BN_CLICKED) {
				//Handle our button up
				switch (buttonid) {
					case IDC_REWIND:
						mycontrolis&=(~REWINDING);
						//toaster->resync();
						playdelay=TRUE;
						if (debug) printc("Rewind up");
						break;
					case IDC_FASTFORWARD:
						mycontrolis&=(~FASTFORWARDING);
						//toaster->resync();
						playdelay=TRUE;
						if (debug) printc("Fast Forward up");
						break;
					case IDC_MEDIAPREV:
						break;
					case IDC_MEDIANEXT:
						break;
					default: goto skip1;
					}
				InvalidateRect(window,NULL,FALSE);
				return(0);
			} //end id bn clicked
			skip1:
			break;
			}

		case WM_DRAWITEM: {
			RECT rc;
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
							hdcMem,((lpdis->CtlID)-CB_IDCOFFSET)*SKINGRIDX+1,SKINGRIDY+1,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						//Handle our Button Down
						switch (lpdis->CtlID) {
							case IDC_STOP:
								if (debug) printc("Stop");
								mycontrolis&=(~(PLAYING|REWINDING|FASTFORWARDING));
								//explicitly define record to be redrawn
								rc.left=(IDC_RECORD-CB_IDCOFFSET)*CONTROLBUTTONX;
								rc.right=rc.left+CONTROLBUTTONX;
								rc.top=0;
								rc.bottom=CONTROLBUTTONY;
								InvalidateRect(window,&rc,FALSE);
								goto skip2;
							case IDC_REWIND:
								playdelay=FALSE;
								if (debug) printc("RewindDN");
								mycontrolis|=REWINDING;
								mycontrolis&=(~FASTFORWARDING);
								goto skip2;
							case IDC_PLAY:
								if (debug) printc("Play");
								if (mycontrolis&PLAYING) {
									mycontrolis&=(~(PLAYING|REWINDING|FASTFORWARDING));
									if (mycontrolis&RECORDING) {
										capture->stopcapture();
										}
									}
								else {
									mycontrolis|=PLAYING;
									playdelay=TRUE;
									if (mycontrolis&RECORDING) {
										capture->startcapture();
										}
									}
								break;
							case IDC_FASTFORWARD:
								mycontrolis|=FASTFORWARDING;
								mycontrolis&=(~REWINDING);
								playdelay=FALSE;
								if (debug) printc("Fast ForwardDN");
								goto skip2;
							case IDC_RECORD:
								if (!(mycontrolis&RECORDING)) {
									mycontrolis|=RECORDING;
									capture->showdisplay();
									if (debug) printc("Record");
									//Disable rew ff etc
									EnableWindow(controlbuttons[1],FALSE);
									EnableWindow(controlbuttons[2],FALSE);
									EnableWindow(controlbuttons[4],FALSE);
									EnableWindow(controlbuttons[5],FALSE);
									}
								else {
skip2:
									capture->stopcapture();
									mycontrolis&=(~PLAYING);
									capture->hidedisplay();
									//Enable rew ff etc
									EnableWindow(controlbuttons[1],TRUE);
									EnableWindow(controlbuttons[2],TRUE);
									EnableWindow(controlbuttons[4],TRUE);
									EnableWindow(controlbuttons[5],TRUE);
									if (toaster->rtmeoutput && toaster->probe) {
										toaster->shutdowncapture();
										toaster->probe=NULL;
										}

									mycontrolis&=(~RECORDING);
									//explicitly define play to be redrawn
									rc.left=(IDC_PLAY-CB_IDCOFFSET)*CONTROLBUTTONX;
									rc.right=rc.left+CONTROLBUTTONX;
									InvalidateRect(window,&rc,FALSE);
									}
								break;
							}
						} //end if selected
					else { //Not selected
						//printc("ODA_DRAWENTIRE");
						if (((lpdis->CtlID==IDC_PLAY)&&(mycontrolis&PLAYING))||
							((lpdis->CtlID==IDC_RECORD)&&(mycontrolis&RECORDING)))
							keeplit=SKINGRIDY+1;
						else keeplit=1;
						hdcMem=CreateCompatibleDC(lpdis->hDC); 
						hbmpold=(HDC)SelectObject(hdcMem,skinbmp);
 						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top,
							lpdis->rcItem.right - lpdis->rcItem.left,
							lpdis->rcItem.bottom - lpdis->rcItem.top,
						hdcMem,((lpdis->CtlID)-CB_IDCOFFSET)*SKINGRIDX+1,keeplit,SRCCOPY); 
	
						SelectObject(hdcMem, hbmpold); 
						DeleteDC(hdcMem);
						} // end not selected
					break; 
				} // end switch item action
			return (TRUE); 
			}

		case WM_CLOSE:
			windowtoggle(screen,window,IDM_CONTROLS);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	} //end handlecontrols


controlsclass::controlsclass() {
	mycontrolis=0;
	playdelay=TRUE;
	}


void controlsclass::createwindow(HWND w_ptr) {
	window=CreateWindowEx(WS_EX_TOOLWINDOW,"OBJGRAYWIN","Capture Controls",//WS_VISIBLE|
		WS_DLGFRAME|WS_POPUP|WS_OVERLAPPEDWINDOW,400,260,150,CONTROLBUTTONY+6+22,w_ptr,
		/*(HMENU)IDC_CONTROLS*/NULL,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	}


LONG controlsclass::resize() {
	//LONG y=size100notheight(screen,window,27,65,100,CONTROLBUTTONY+6);
	configuresize();
	return (NULL);
	}


BOOL controlsclass::configuresize() {
	RECT rc;

	GetClientRect(window,&rc);
	cbtb_offsetx=CONTROLBUTTONX*CB_NUMOFBUTTONS;
	cbtb_widthx=(rc.right-rc.left)-cbtb_offsetx;
	//We may later decide to make the buttons expandable
	return (TRUE);
	}


BOOL controlsclass::startup() {
	UBYTE t;

	char *presetinfo[7]={
		"STP",
		"RE",
		"RW",
		"PLA",
		"FF",
		"FE",
		"REC"};

	HMENU idc_controls[7] = {
		(HMENU)IDC_STOP,
		(HMENU)IDC_MEDIAPREV,
		(HMENU)IDC_REWIND,
		(HMENU)IDC_PLAY,
		(HMENU)IDC_FASTFORWARD,
		(HMENU)IDC_MEDIANEXT,
		(HMENU)IDC_RECORD};

	for (t=0;t<CB_NUMOFBUTTONS;t++) {

		controlbuttons[t]=CreateWindowEx(0,"BUTTON",presetinfo[t],WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
			t*CONTROLBUTTONX,0,CONTROLBUTTONX,CONTROLBUTTONY,window,
			idc_controls[t],hInst,NULL);
		}

	return (TRUE);
	}


BOOL controlsclass::controlsfail()  {
	error(0,"The Controls object has detected an internal error\n Please contact technical support");
	return (FALSE);
	} // End initFail


void controlsclass::shutdown() {
	if (window) DestroyWindow(window);
	}


controlsclass::~controlsclass() {
	}

