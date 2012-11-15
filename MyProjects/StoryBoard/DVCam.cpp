#include "HandleMain.h"
#include "../include/DVCam.h"

#pragma comment(lib, "../lib/DVCamlibs")

#define CP_CLIPNAME		200
#define CP_VIDEOCHK		204
#define CP_VIDEOPATH		205
#define CP_VIDEOASL		206
#define CP_START			207

BOOL captureclass::DVCamclass::startcapture() {
	char buffer[32];
	int oldfilenumber;

	SetWindowText(startstopbt,"Stop");
	//Set the new incremented Filename
	strcpy(buffer,clipname);
	oldfilenumber=filenumber;
	wsprintf(string,"%s%d",clipname,filenumber);
	SetWindowText(clipnameedit,string);	
	strcpy(clipname,buffer);
	filenumber=++oldfilenumber;
	if (debug) printc("Start Capture");
	//end gui stuff todo put this all above in its own function
	if (dvcamlib) dvcamlib->startcapture();
	return(TRUE);
	}

void captureclass::DVCamclass::stopcapture() {
	if (debug) printc("Stop Capture");
	SetWindowText(startstopbt,"Start");
	//end gui stuff todo put this all above in its own function
	if (dvcamlib) dvcamlib->stopcapture();
	}

int captureclass::DVCamclass::Callback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	char buffer[MAX_PATH];

	switch(uMsg) {

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case CP_VIDEOCHK:
							if (SendMessage(videochk,BM_GETCHECK,0,0)==BST_CHECKED) {
								SendMessage(videochk,BM_SETCHECK,BST_UNCHECKED,0);
								EnableWindow(videopathedit,FALSE);
								EnableWindow(videoasl,FALSE);
								}
							else {
								SendMessage(videochk,BM_SETCHECK,BST_CHECKED,0);
								EnableWindow(videopathedit,TRUE);
								EnableWindow(videoasl,TRUE);
								}
							break;
						case CP_VIDEOASL:
							getdirectory("Select Video Path",videopath);
							SetWindowText(videopathedit,videopath);
							break;
						case CP_START:
							if (controls->mycontrolis&PLAYING) {
								controls->mycontrolis&=(~PLAYING);
								InvalidateRect(controls->controlbuttons[3],NULL,FALSE);
								stopcapture();
								}
							else {
								controls->mycontrolis|=PLAYING;
								InvalidateRect(controls->controlbuttons[3],NULL,FALSE);
								startcapture();
								}
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
							case CP_CLIPNAME:
								GetWindowText(clipnameedit,clipname,32);
								GetWindowText(clipnameedit,fullname,32);
								filenumber=0;
								break;
							case CP_VIDEOPATH:
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
			if (dvcamlib) return dvcamlib->AppCommand(w_ptr,uMsg,wParam,lParam);
			break;
			} //end WM_COMMAND

		//case WM_TIMER:
		//	streamcapture();
		//	return(0);

		case WM_CLOSE:
			windowtoggle(screen,w_ptr,NULL);
			CheckMenuItem(m_ptr,IDM_CAPTURE,MF_BYCOMMAND|MFS_UNCHECKED);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}

captureclass::DVCamclass::DVCamclass() {
	strcpy(videopath,"V:\\VideoMedia");
	strcpy(clipname,"Default");
	filenumber=hfile=totalbyteswritten=0;
	dvcamlib=new DVCamlib();
	//dvcamlib=NULL;
	}

void captureclass::DVCamclass::createwindow(HWND w_ptr) {
	int toolx=0;
	int tooly=24;

	window=CreateWindowEx(0,"OBJGRAYWIN","DV Capture",
		WS_POPUP|WS_OVERLAPPEDWINDOW,235,70,300,180,w_ptr,
		menu=CreateMenu(),hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(long)this);

	AppendMenu(menu,MF_POPUP|MFT_STRING,(UINT)CreatePopupMenu(),"Capture Devices");
	AppendMenu(menu,MF_POPUP|MFT_STRING,(UINT)CreatePopupMenu(),"Options");
	DrawMenuBar(window);

	//TODO to make this visible we'll need controls to call functions to
	//show both windows... as well as reset toaster on toaster capture

	prevwindow=CreateWindowEx(0,"OBJGRAYWIN","DV Preview",
		WS_POPUP|WS_OVERLAPPEDWINDOW,10,10,10,10,w_ptr,
		NULL,hInst,NULL);
	SetWindowLong(prevwindow,GWL_USERDATA,(long)this);

	//since we are not going to resize we'll put the children here
	CreateWindowEx(0,"STATIC","Clip Name:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,80,CONTROLBUTTONY,window,NULL,hInst,NULL);
	toolx+=84;
	clipnameedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD
		,toolx,tooly,200,CONTROLBUTTONY,window,(HMENU)CP_CLIPNAME,hInst,NULL);
	SetWindowText(clipnameedit,clipname);
	toolx=0;
	tooly+=24;

	videochk=CreateWindowEx(0,"BUTTON","Video",WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		toolx,tooly,64,CONTROLBUTTONY,window,
		(HMENU)CP_VIDEOCHK,hInst,NULL);
	SendMessage(videochk,BM_SETCHECK,BST_CHECKED,0);
	toolx+=64;
	videopathedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD
		,toolx,tooly,200,CONTROLBUTTONY,window,(HMENU)CP_VIDEOPATH,hInst,NULL);
	SetWindowText(videopathedit,videopath);
	toolx+=204;
	videoasl=CreateWindowEx(0,"BUTTON","...",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
		toolx,tooly,20,CONTROLBUTTONY,window,
		(HMENU)CP_VIDEOASL,hInst,NULL);
	toolx=0;
	tooly+=40;

	startstopbt=CreateWindowEx(0,"BUTTON","Start",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
		toolx+120,tooly,64,CONTROLBUTTONY+10,window,
		(HMENU)CP_START,hInst,NULL);

	}

BOOL captureclass::DVCamclass::startup() {
	//!(controls->previewoff)
	if (dvcamlib) dvcamlib->startup(preview->prevwindow,window,prevwindow,videopath,fullname,!(preview->previewoff));
	return(TRUE);
	} //end startup

void captureclass::DVCamclass::showdisplay() {
	ShowWindow(window,SW_SHOW);
	//ShowWindow(prevwindow,SW_SHOW);
	CheckMenuItem(m_ptr,IDM_CAPTURE,MF_BYCOMMAND|MFS_CHECKED);
	}

void captureclass::DVCamclass::hidedisplay() {
	ShowWindow(window,SW_HIDE);
	//ShowWindow(prevwindow,SW_HIDE);
	CheckMenuItem(m_ptr,IDM_CAPTURE,MF_BYCOMMAND|MFS_UNCHECKED);
	}

void captureclass::DVCamclass::shutdown() {
	if (dvcamlib) dvcamlib->shutdown();
	if (prevwindow) DestroyWindow(prevwindow);
	}

captureclass::DVCamclass::~DVCamclass() {
	if (dvcamlib) dvcamlib->~DVCamlib();
	}

