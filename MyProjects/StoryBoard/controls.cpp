#include "HandleMain.h"
int controlsclass::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	if (w_ptr==frameswindow)
		return (handleframes(w_ptr,uMsg,wParam,lParam));
	if (w_ptr==window)
		return (handlecontrols(w_ptr,uMsg,wParam,lParam));
	return(DefWindowProc(w_ptr,uMsg,wParam,lParam));

	}

int controlsclass::handlecontrols(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{
	class storyboardclass *storyboard=getstoryboard();

	switch(uMsg) {

		case WM_HSCROLL:
			nostaticpreview=nostaticxypreview=-1;
			switch LOWORD(wParam) {
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_THUMBTRACK:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
					frameindex=SendMessage(scrub,TBM_GETPOS,0,0);
					updateframesdisplay(storyboard);
					if (streamptr) {
						updatevideoparm=streamptr;
						updatevideorequest++;
						SetEvent(arrayofevents[EVENT_VIDEO]);
						}
					return(0);
				case TB_BOTTOM:
					frameindex=storyboard->framecounter-1;
					updateframesdisplay(storyboard);
					if (streamptr) {
						updatevideoparm=streamptr;
						updatevideorequest++;
						SetEvent(arrayofevents[EVENT_VIDEO]);
						}
					return(0);
				case TB_TOP:
					frameindex=1;
					updateframesdisplay(storyboard);
					if (streamptr) {
						updatevideoparm=streamptr;
						updatevideorequest++;
						SetEvent(arrayofevents[EVENT_VIDEO]);
						}
				return(0);
				}
			break;

		case WM_COMMAND: {
			nostaticpreview=nostaticxypreview=-1;
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
						adjustglow(storyboard,glowpos-1);
						break;
					case IDC_MEDIANEXT:
						adjustglow(storyboard,glowpos+1);
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
								audio->player.closevoices();
								//explicitly define record to be redrawn
								rc.left=(IDC_RECORD-CB_IDCOFFSET)*CONTROLBUTTONX;
								rc.right=rc.left+CONTROLBUTTONX;
								rc.top=0;
								rc.bottom=CONTROLBUTTONY;
								InvalidateRect(window,&rc,FALSE);
								goto skip2;
							case IDC_REWIND:
								playdelay=FALSE;
								SetEvent(arrayofevents[EVENT_PLAY]);
								if (debug) printc("RewindDN");
								mycontrolis|=REWINDING;
								mycontrolis&=(~FASTFORWARDING);
								goto skip2;
							case IDC_PLAY:
								if (debug) printc("Play");
								storyboard->selectimageobj->resetlist(TRUE);
								storyboard->updateimagelist();
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
									else SetEvent(arrayofevents[EVENT_PLAY]);
									}
								break;
							case IDC_FASTFORWARD:
								mycontrolis|=FASTFORWARDING;
								mycontrolis&=(~REWINDING);
								playdelay=FALSE;
								SetEvent(arrayofevents[EVENT_PLAY]);
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
		case WM_NCHITTEST: {
			if (!(storyboard->pointermode==selection)) {
				storyboard->pointermode=selection;
				InvalidateRect(storyboard->tools.toolwindow,NULL,FALSE);
				}
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
			} //end WM_NCHITTEST:

		case WM_CLOSE:
			windowtoggle(screen,window,0);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	} //end handlecontrols


int controlsclass::handleframes(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
	{

	switch(uMsg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(frameswindow,&ps);
			smartrefresh(ps.hdc,framesclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(frameswindow,&ps);
			return (0);
			}
		case WM_CLOSE:
			windowtoggle(screen,frameswindow,NULL);
			return(0L);
		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	}


DWORD controlsclass::playfunc(class storyboardclass *storyboard,LPVOID parm) {
	struct wavvoicelist *audioindex,*nextaudioindex;
	char skipping=0;

	WaitForSingleObject(arrayofevents[EVENT_PLAY],INFINITE);
	if (toaster->rtmeoutput) toaster->resync();
	while (!killplaythread) {
		//SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
		if (mycontrolis&REWINDING) frameindex-=1;
			else frameindex+=1;
		if (frameindex>(long)storyboard->framecounter) frameindex=1;
		if (frameindex<1) frameindex=storyboard->framecounter;
		SendMessage(scrub,TBM_SETPOS,TRUE,frameindex);
		if (storyboard->imagelisthead) { 
			updateframesdisplay(storyboard);
			updatevideoparm=streamptr;
			updatevideorequest++;
			SetEvent(arrayofevents[EVENT_VIDEO]);
			}
		//SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		if (debug==2) {
			wsprintf(string,"fb=%ld",frames_behind);
			printc(string);
			}
		if (playdelay) {
			//wsprintf(string,"framesbehind %d",frames_behind);printc(string);
			Sleep(31L);
			if (frames_behind) while (frames_behind<(-4)) Sleep(0L);
/**/ //To Run on w98
			while (updatevideorequest) {
				if (debug) {
					if (frames_behind>0) {
						if (toaster->rtmeoutput) toaster->resync();
						//updatevideorequest=0;
						if (skipping==0) {
							printc("Skipping Frames..Begin");
							//printc("Skipping Frames");
							skipping=1;
							}
						}
					}
				Sleep(0L);
				}
/**/
			/**/
			if (debug) if (skipping) if (frames_behind<1) {
				printc("Skipping Frames..End");
				skipping=0;
				}
			/**/
			}
		else Sleep(1L);
			//Check audioplayqueue to play audio
		if (playdelay) {
			if (audioindex=audio->playqueuehead) {
				do {
					nextaudioindex=audioindex->next;
					if (audioindex->media==streamptr)
						if (mediatimer+frames_behind>=audioindex->voice->frameoffset-streamptr->cropin) {
							audio->player.playwav(audioindex->voice);
							//wsprintf(string,"Frames Behind=%d",frames_behind);printc(string);
							audio->remvoicefromplay(audioindex);
							}
					} while (audioindex=nextaudioindex);
				}
			}
			//
		//mycontrolis structure should have been a byte
		//so that I can quickly check all conditions
		if ((!(mycontrolis&(PLAYING|FASTFORWARDING|REWINDING)))||(mycontrolis&RECORDING)) {
			WaitForSingleObject(arrayofevents[EVENT_PLAY],INFINITE);
			if (toaster->rtmeoutput) toaster->resync();
			}
		}

	return(0);
	} 


void controlsclass::initframecounter(class storyboardclass *storyboard) {
			frameindex=oldframeindex=mediatimer=1;
			storyboard->oldglowpos=glowpos=0;
			storyboard->scrolloffset=0;
			//if (streamptr==storyboard->imagelisthead) storyboard->oldstreamptr=streamptr=storyboard->imagelisthead->next;
			/*else*/ storyboard->oldstreamptr=streamptr=storyboard->scrollpos=storyboard->imagelisthead;
			}


void controlsclass::adjustframecounter(class storyboardclass *storyboard,float linenumber,long framenum,char add) {
	ULONG framecount;

	if (streamptr)	if (linenumber<streamptr->linenumber) initframecounter(storyboard);
	if(framenum) {
		//It Turns out the framenum and the position shift will not be the
		//same we may need to implement this but for now reset if the
		//glowpos goes past what is being inserted we'll see how this
		//works out later when scrolling gets implemented.
		if (add) {
			storyboard->framecounter+=framenum;
			/*
			if (streamptr)	if (linenumber<streamptr->linenumber) {
				frameindex+=framenum;
				oldframeindex+=framenum;
				storyboard->oldglowpos+=add;
				glowpos+=add;
				}
			*/
			}
		//If we pulled the glow lets reset the frameindex
		/*if (streamptr)	if (linenumber==streamptr->linenumber) {*/
		framecount=storyboard->framecounter;
		SendMessage(scrub,TBM_SETPOS,TRUE,frameindex);
		SendMessage(scrub,TBM_SETRANGE,TRUE,MAKELONG(1,framecount));
		//SendMessage(scrub,TBM_SETTICFREQ,tickfreq,0);
		updateframesdisplay(storyboard);
		if (streamptr) {
			updatevideoparm=streamptr;
			updatevideorequest++;
			SetEvent(arrayofevents[EVENT_VIDEO]);
			}
		}
	}


void controlsclass::updateframecounter(class storyboardclass *storyboard) {
	ULONG framecount;

	initframecounter(storyboard);
	framecount=storyboard->framecounter;
	SendMessage(scrub,TBM_SETPOS,TRUE,frameindex);
	SendMessage(scrub,TBM_SETRANGE,TRUE,MAKELONG(1,framecount));

	updateframesdisplay(storyboard);
	if (streamptr) {
		updatevideoparm=streamptr;
		updatevideorequest++;
		SetEvent(arrayofevents[EVENT_VIDEO]);
		}
	}


void controlsclass::adjustglow(class storyboardclass *storyboard,short item) {
	long mediaindex;
	long lframeindex=0;
	struct imagelist *streamindex=streamptr;

	mediaindex=item-glowpos;
	if (mediaindex>0) {
		for (mediaindex=glowpos;(mediaindex<item)&&(streamindex->next);mediaindex++)  {
			lframeindex+=computeactualframes(streamindex);
			streamindex=streamindex->next;
			}
		++lframeindex-=(short)mediatimer;
		}
	else if (mediaindex<0) {
		for (mediaindex=glowpos;(mediaindex>item)&&(streamindex->prev);mediaindex--)  {
			lframeindex-=computeactualframes(streamindex->prev);
			streamindex=streamindex->prev;
			}
		++lframeindex-=(short)mediatimer;
		}
	//wsprintf(string,"frameindex=%d",lframeindex);printc(string);
	if (lframeindex) {
		frameindex+=lframeindex;
		SendMessage(scrub,TBM_SETPOS,TRUE,frameindex);
		updateframesdisplay(storyboard);
		updatevideoparm=streamptr;
		updatevideorequest++;
		SetEvent(arrayofevents[EVENT_VIDEO]);
		}
	else {
		storyboard->updateglow(glowpos,streamptr);
		updatevideoparm=streamptr;
		updatevideorequest++;
		SetEvent(arrayofevents[EVENT_VIDEO]);
		}
	}


long controlsclass::computeactualframes(struct imagelist *streamptr) {
	//Assuming streamptr is already valid
	long actualframes=streamptr->actualframes;
	if (streamptr->id==id_media) {
		if (streamptr->prev) if (streamptr->prev->id==id_dve) actualframes-=streamptr->prev->duration;
		if (streamptr->next) if (streamptr->next->id==id_dve) actualframes-=streamptr->next->duration;
		if (actualframes<0) actualframes=0;
		}
	return(actualframes);
	}


BOOL controlsclass::updateframesdisplay(class storyboardclass *storyboard) {
	HDC hdc,smartdc;
	HFONT oldfont;
	RECT rc;
	long actualframes;

	//the first section updates the frameswindow
	hdc=GetDC(frameswindow);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,framesclip);
	SetTextColor(smartdc,GetSysColor(COLOR_WINDOWTEXT));
	SetBkColor(smartdc,GetSysColor(COLOR_MENU));
	oldfont=(HFONT)SelectObject(smartdc,mediafont);
	wsprintf(string,"Frames %ld     ",frameindex);
	TextOut (smartdc,10,4,string,strlen(string));
	if (oldframecounter!=(long)storyboard->framecounter) {
		GetClientRect(frameswindow,&rc);
		oldframecounter=storyboard->framecounter;
		wsprintf(string,"Total %ld   ",storyboard->framecounter);
		TextOut (smartdc,rc.right-50,4,string,strlen(string));
		}
	SelectObject(smartdc,oldfont);
	DeleteDC(smartdc);
	ReleaseDC(frameswindow,hdc);
	InvalidateRgn(frameswindow,NULL,FALSE);

	//This section will update the streamptr
	fiadjustment+=frameindex-oldframeindex;
	oldframeindex=frameindex;
	if (!(updatingstreamptr)) {
		updatingstreamptr=TRUE;

		//check to ensure streamptr is valid
		if (streamptr==NULL) {
			streamptr=storyboard->imagelisthead;
			mediatimer=1;
			}

skiptocheckadjustment:

		while (fiadjustment) {
			long difference;
			long tempadj;
				
			tempadj=fiadjustment;
			fiadjustment=0;

			//check for head
			if (frameindex==1)  {
				mediatimer=1;
				storyboard->updateglow(glowpos=0,streamptr=storyboard->imagelisthead);
				continue;
				}
			//here is the actual adjustment
			actualframes=computeactualframes(streamptr);
			if (tempadj>0) {
				difference=actualframes-mediatimer;
				if (difference<tempadj) {
					tempadj-=difference;
					}
				else {
					mediatimer+=tempadj;
					continue;
					}
				while (tempadj) {
					glowpos++;
					actualframes=computeactualframes(streamptr=streamptr->next);
					if (tempadj<=actualframes) {
						mediatimer=tempadj;
						if (streamptr) storyboard->updateglow(glowpos,streamptr);
						else return controlsfail();
						goto skiptocheckadjustment;
						}
					else tempadj-=actualframes;
					}
				}
			else if (tempadj<0) {
				if ((abs(tempadj))<mediatimer) {
					mediatimer+=tempadj;
					continue;
					}
				else {
					tempadj+=mediatimer;
					}
				do {
					--glowpos;
					streamptr=streamptr->prev;
					//If our media is 0 then move over to the DVE before it
					//if (!streamptr->prev) return controlsfail();
					if ((!(computeactualframes(streamptr)||tempadj))&&(streamptr->prev)) storyboard->updateglow(--glowpos,streamptr=streamptr->prev);

					actualframes=computeactualframes(streamptr);
					if ((long)(abs(tempadj))<=actualframes) {
						mediatimer=actualframes+tempadj;
						if (streamptr) storyboard->updateglow(glowpos,streamptr);
						else return controlsfail();

						goto skiptocheckadjustment;
						}
					else tempadj+=actualframes;
					}  while (tempadj);
				} // tempadj<0
			} //end while fiadjustment
		updatingstreamptr=FALSE;
		}
	return (TRUE);
	}


DWORD controlsclass::updatevideoNoCard()
	{
	ULONG *videobuf=toaster->NoToaster;
	//lean and mean
	WaitForSingleObject(arrayofevents[EVENT_VIDEO],INFINITE);
	while (!killvideothread) {
		//For now submit RGB to toaster
		//printc("GO");
		//We'll boost its process priority
		//SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
		while (updatevideorequest) {
			//wsprintf(string,"%lx",videobufid);printc(string);
			if (fiadjustment<10) medialoaders->mediamanager(videobuf,updatevideoparm,mediatimer,mycontrolis&PLAYING);
			else medialoaders->mediamanager(videobuf,updatevideoparm,mediatimer,(mycontrolis&PLAYING)|MMDRAFTMODE);
			//Call Preview update while buffer is still fresh
			preview->updatepreview(videobuf);

			//SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
			//printc("Stop");
			if (debug) if (updatevideorequest>2) printc("Skipping frames");
			updatevideorequest--;
			}
		WaitForSingleObject(arrayofevents[EVENT_VIDEO],INFINITE);
		} //end while not killvideothread
	return (0);
	} // end updatevideo()


DWORD controlsclass::updatevideo() //Toaster Version
	{
	ULONG *videobuf=NULL;
	long videobufid;
	//lean and mean
	WaitForSingleObject(arrayofevents[EVENT_VIDEO],INFINITE);
	while (!killvideothread) {
		//For now submit RGB to toaster
		//printc("GO");
		//We'll boost its process priority
		//SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
		while (updatevideorequest) {
			if (videobuf=toaster->AllocFrame(&videobufid)) {
				//wsprintf(string,"%lx",videobufid);printc(string);
				if (fiadjustment<10) medialoaders->mediamanager(videobuf,updatevideoparm,mediatimer,mycontrolis&PLAYING);
				else medialoaders->mediamanager(videobuf,updatevideoparm,mediatimer,(mycontrolis&PLAYING)|MMDRAFTMODE);
				//Call Preview update while buffer is still fresh
				//if (!previewoff) updatepreview(videobuf);
				if (!preview->previewoff) SetEvent(arrayofevents[EVENT_PREVIEW]);

				toaster->SendFrame(videobufid);
				}
			else printc("Warning! ToasterAllocFrame() Timed out.");
			//if (videobufid>=0x167400) Sleep(32L); //keep it in video sync
			//SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
			//printc("Stop");
			if (debug) if (updatevideorequest>2) printc("Skipping frames");
			if (updatevideorequest>30) updatevideorequest=0;
			updatevideorequest--;
			}
		WaitForSingleObject(arrayofevents[EVENT_VIDEO],INFINITE);
		} //end while not killvideothread
	return (0);
	} // end updatevideo()


void controlsclass::updatevideonow() //Toaster Version
	{
	ULONG *videobuf=NULL;
	long videobufid;
	//wsprintf(string,"Frames behind=%ld",frames_behind);printc(string);
	if (frames_behind>1) {
		if (videobuf=toaster->AllocFrame(&videobufid)) {
			//wsprintf(string,"%lx",videobufid);printc(string);
		if (fiadjustment<10) medialoaders->mediamanager(videobuf,updatevideoparm,mediatimer,mycontrolis&PLAYING);
		else medialoaders->mediamanager(videobuf,updatevideoparm,mediatimer,(mycontrolis&PLAYING)|MMDRAFTMODE);
		//Call Preview update while buffer is still fresh
		if (!preview->previewoff) preview->updatepreview(videobuf);
			toaster->SendFrame(videobufid);
			}
		else printc("Warning! ToasterAllocFrame() Timed out.");
		toaster->resync();
		}
	} // end updatevideo()


controlsclass::controlsclass() {
	mediatimer=frameindex=oldframeindex=1;
	fiadjustment=0;
	streamptr=NULL;
	updatingstreamptr=FALSE;
	updatevideorequest=0;
	mycontrolis=0;
	glowpos=0;
	playdelay=TRUE;
	QueryPerformanceFrequency(&performancefrequency);
	/*
	flc=divclocksbyframerate(performancefrequency,29970);
	mlc=divu64byu32(performancefrequency,1000);
	*/
	frames_behind=0;
	if (toaster->rtmeoutput) toaster->resync();
	}


void controlsclass::createwindow(HWND w_ptr) {
	window=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|
		WS_DLGFRAME|WS_CHILD,235,260,300,270,w_ptr,
		(HMENU)IDC_CONTROLS,hInst,NULL);

	frameswindow=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|
		WS_DLGFRAME|WS_CHILD,235,270,300,300,w_ptr,
		(HMENU)IDC_FRAMES,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	SetWindowLong(frameswindow,GWL_USERDATA,(long)this);
	}


LONG controlsclass::resize() {
	LONG y=size100notheight(screen,window,27,65,100,CONTROLBUTTONY+6);
	y=size100notheight(screen,frameswindow,0,65,27,CONTROLBUTTONY+6);
	configuresize();
	SetWindowPos(scrub,NULL,cbtb_offsetx,0,cbtb_widthx,CONTROLBUTTONY,SWP_NOACTIVATE|SWP_NOZORDER);
	return (y);
	}


void controlsclass::clearframeclip() {
	RECT rc;
	HDC hdc,smartdc;
	if (framesclip) { //clear framesclip for resize
		hdc=GetDC(frameswindow);
		smartdc=CreateCompatibleDC(hdc);
		SelectObject(smartdc,framesclip);
		GetClientRect(frameswindow,&rc);
		FillRect(smartdc,&rc,(HBRUSH)(COLOR_MENU+1));
		DeleteDC(smartdc);
		ReleaseDC(frameswindow,hdc);
		oldframecounter=-1;
		g_updateframedisplay();
		}
	}


BOOL controlsclass::configuresize() {
	RECT rc;

	clearframeclip();
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

	scrub=CreateWindowEx(0,TRACKBAR_CLASS,"",WS_VISIBLE|WS_CHILD|
		TBS_NOTICKS|/*TBS_ENABLESELRANGE|*/TBS_HORZ|TBS_TOP,
		cbtb_offsetx,0,cbtb_widthx,CONTROLBUTTONY,window,
		(HMENU)IDC_SCRUB,hInst,NULL);

	//initialize the scrub
	SendMessage(scrub,TBM_SETRANGE,TRUE,MAKELONG(1L,1L));
	//SendMessage(scrub,TBM_SETTICFREQ,0,0);
	//SendMessage(scrub,TBM_SETLINESIZE,0,1L);
	//SendMessage(scrub,TBM_SETPAGESIZE,0,1L);

	//smartrefresh fot the frameswindow
	framesclip=createclipwin(frameswindow,(HBRUSH)(COLOR_MENU+1));
	return (TRUE);
	}


BOOL controlsclass::controlsfail()  {
	error(0,"The Controls object has detected an internal error\n Please contact technical support");
	return (FALSE);
	} // End initFail


void controlsclass::shutdown() {
	//Lets Cleanup the surface for Preview and Video
	if (window) DestroyWindow(window);
	if (frameswindow) DestroyWindow(frameswindow);
	}


controlsclass::~controlsclass() {
	}

