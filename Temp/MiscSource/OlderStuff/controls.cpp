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


DWORD controlsclass::updatepreview(ULONG *videobuf) {
	HRESULT hr;
	DDSURFACEDESC ddsd;
	DWORD dwWidth,dwHeight;
	ULONG lPitch;
	ULONG *psurf;
	ULONG dwBytesInRow;
	ULONG	widthexcess;
	//COLORREF color;

	if (lpDDSPrimary==NULL)	return (DWORD)E_FAIL; //TODO put on thread maker
	// make sure this surface is restored.
	//lpDDSOverlay->Restore();

	INIT_DIRECTX_STRUCT(ddsd);
	// Lock down the surface so we can modify it's contents.
	if (!(lpDDSOverlay->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)==DD_OK)) return ((DWORD)E_FAIL);
	psurf=(ULONG *)ddsd.lpSurface;

	dwWidth=ddsd.dwWidth>>1;
	dwHeight=ddsd.dwHeight>>1;   //TEMP stuff here
	lPitch=ddsd.lPitch;
	dwBytesInRow=ddsd.dwWidth<<1;
	widthexcess=720-dwBytesInRow;

/*Here is the UYVY version*/
	__asm	{
		mov		edi, psurf
		 mov		ecx, dwHeight
		mov		esi, videobuf
loopy:
		mov		ebx,dwWidth
		shr		ebx,2
loopx:
		movq		mm0, [esi]
		 dec		ebx
		movq		mm1, [esi+8]
		movq		[edi], mm0
		movq		[edi+1408], mm0
		 add		esi, 16
		movq		[edi+8], mm1
		movq		[edi+1416], mm1
		add		edi, 16
		cmp		ebx,0
		jne		loopx
		add		edi,dwBytesInRow
		add		esi,32	//This may change
		dec		ecx
		jne		loopy
		emms
		}
/**/

/* This is the inverted YUY2 version of copy
	__asm {
	//for(y=0; y<dwHeight; y++) {
		mov		esi,videobuf
		mov		edi,psurf
		mov		ecx,dwHeight
		//for(x=0; x<dwWidth; x+=2) *psurf++=*videobuf++;
loopy:
		mov		ebx,dwWidth
loopx:
		mov		eax,[esi]
		mov		[edi+1],al
		mov		[edi+1409],al
		shr		eax,8
		mov		[edi+2],al
		mov		[edi+1410],al
		shr		eax,8
		mov		[edi+3],al
		mov		[edi+1411],al
		shr		eax,8
		mov		[edi],al
		mov		[edi+1408],al
		add		esi,4
		add		edi,4
		dec		ebx
		cmp		ebx,0
		jne		loopx
		//psurf+=(lPitch-dwBytesInRow);
		add		edi,dwBytesInRow
		add		esi,32	//This may change
		dec		ecx
		jne		loopy
		//}
		}
*/

	lpDDSOverlay->Unlock(NULL);
	hr=lpDDSOverlay->UpdateOverlay(&rs,lpDDSPrimary,&rd,dwupdateflags,&ovfx);
	return (hr);
	}


DWORD controlsclass::updatepreviewthread() { 
	HRESULT hr;
	DDSURFACEDESC ddsd;
	DWORD dwWidth,dwHeight;
	ULONG lPitch;
	ULONG *psurf;
	ULONG dwBytesInRow;
	ULONG	widthexcess;
	ULONG *videobuf;

	//COLORREF color;
	WaitForSingleObject(arrayofevents[EVENT_PREVIEW],INFINITE);
	while (!killpreviewthread) {

		if (lpDDSPrimary==NULL)	return (DWORD)E_FAIL; //TODO put on thread maker
		// make sure this surface is restored.
		//lpDDSOverlay->Restore();

		INIT_DIRECTX_STRUCT(ddsd);
		// Lock down the surface so we can modify it's contents.
		if (!(lpDDSOverlay->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)==DD_OK)) return ((DWORD)E_FAIL);
		psurf=(ULONG *)ddsd.lpSurface;

		dwWidth=ddsd.dwWidth>>1;
		dwHeight=ddsd.dwHeight>>1;   //TEMP stuff here
		lPitch=ddsd.lPitch;
		dwBytesInRow=ddsd.dwWidth<<1;
		widthexcess=720-dwBytesInRow;
		
		if (videobuf=(ULONG *)toaster->videobuf) {
			/*Here is the UYVY version*/
			__asm	{
				mov		edi, psurf
				 mov		ecx, dwHeight
				mov		esi, videobuf
loopy:
				mov		ebx,dwWidth
				shr		ebx,2
loopx:
				movq		mm0, [esi]
				 dec		ebx
				movq		mm1, [esi+8]
				movq		[edi], mm0
				movq		[edi+1408], mm0
				 add		esi, 16
				movq		[edi+8], mm1
				movq		[edi+1416], mm1
				add		edi, 16
				cmp		ebx,0
				jne		loopx
				add		edi,dwBytesInRow
				add		esi,32	//This may change
				dec		ecx
				jne		loopy
				emms
				}
			}
		lpDDSOverlay->Unlock(NULL);
		hr=lpDDSOverlay->UpdateOverlay(&rs,lpDDSPrimary,&rd,dwupdateflags,&ovfx);
		WaitForSingleObject(arrayofevents[EVENT_PREVIEW],INFINITE);
		}
	return (hr);
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
			updatepreview(videobuf);

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
				if (!previewoff) SetEvent(arrayofevents[EVENT_PREVIEW]);

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
		if (!previewoff) updatepreview(videobuf);
			toaster->SendFrame(videobufid);
			}
		else printc("Warning! ToasterAllocFrame() Timed out.");
		toaster->resync();
		}
	} // end updatevideo()


controlsclass::controlsclass() {
	UINT isitchecked;
	//printc("import construct called");
	lpDD=NULL;
	lpDD4=NULL;
	lpDDSPrimary=lpDDSOverlay=NULL;
	lpClipper=NULL;
	lpDDPal=NULL;
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
	isitchecked=GetMenuState(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND);
	previewoff=((isitchecked&MFS_CHECKED)==0);
	if (toaster->rtmeoutput) toaster->resync();
	}


void controlsclass::createwindow(HWND w_ptr) {
	window=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|
		WS_DLGFRAME|WS_CHILD,235,260,300,270,w_ptr,
		(HMENU)IDC_CONTROLS,hInst,NULL);

	prevwindow=CreateWindowEx(0,"STATIC","",WS_VISIBLE|SS_BLACKRECT|
		WS_CHILD,200,260,235,300,w_ptr,
		(HMENU)IDC_PREVIEW,hInst,NULL);

	frameswindow=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|
		WS_DLGFRAME|WS_CHILD,235,270,300,300,w_ptr,
		(HMENU)IDC_FRAMES,hInst,NULL);

	SetWindowLong(window,GWL_USERDATA,(long)this);
	SetWindowLong(frameswindow,GWL_USERDATA,(long)this);
	}


LONG controlsclass::resize() {
	LONG y=size100notheight(screen,window,27,65,100,CONTROLBUTTONY+6);
	y=size100notheight(screen,frameswindow,0,65,27,CONTROLBUTTONY+6);
	size100noty(screen,prevwindow,0,y,27,100);
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
	ULONG usex,usey;

	clearframeclip();

	GetClientRect(window,&rc);
	cbtb_offsetx=CONTROLBUTTONX*CB_NUMOFBUTTONS;
	cbtb_widthx=(rc.right-rc.left)-cbtb_offsetx;
	//We may later decide to make the buttons expandable
	//Finally lets grab the actual preview window specs for blitting;
	//Done here for speed
	GetWindowRect(prevwindow,&rc);
	prevx=rc.left;
	prevy=rc.top;
	prevheight=rc.bottom-rc.top; 
	GetClientRect(prevwindow,&rc);
	prevwidth=rc.right;
	//calculate the preview windows width and height
	//to see if we need black bars...
	usex=(prevheight*4)/3;
	usey=(prevwidth*3)/4;
	if (usex<prevwidth) {
		//calculate the horx bars
		prevx+=(prevwidth-usex)/2;
		prevwidth=usex;
		}
	else if (usey<prevheight) {
		//calculate the vert bars
		prevy+=(prevheight-usey)/2;
		prevheight=usey;
		}
	//if neither bars are implemented they must be equal
	//**************************************************************
	//             Direct Draw Init II Setting up the rectangles
	//Grab some alignment vars
	DDCAPS capsDrv;
	unsigned int ustretchfactor1000,udestsizealign,usourcesizealign;

	if (!lpDD4) setuppreview();
	INIT_DIRECTX_STRUCT(capsDrv);
	if (!(lpDD4->GetCaps(&capsDrv,NULL)==DD_OK))  return (0);
	ustretchfactor1000=capsDrv.dwMinOverlayStretch>1000 ? capsDrv.dwMinOverlayStretch : 1000;
	udestsizealign=capsDrv.dwAlignSizeDest;
	usourcesizealign=capsDrv.dwAlignSizeSrc;
	//Set up the source rectangle
	rs.top=0;//prevy;
	rs.left=0;//prevx;
	rs.right=VIDEOX;//prevwidth;
	rs.bottom=VIDEOY;//prevheight;
	//Now to apply the alignment
	if (capsDrv.dwCaps&DDCAPS_ALIGNSIZESRC&&usourcesizealign)
		rs.right-=rs.right % usourcesizealign;
	//Set up the Destination rectangle
	rd.left=prevx;
	rd.top=prevy;
	rd.right=((prevwidth+prevx)*ustretchfactor1000+999)/1000;
	rd.bottom=(prevheight+prevy)*ustretchfactor1000/1000;
	//And now the last bit of alignment
	if (capsDrv.dwCaps&DDCAPS_ALIGNSIZEDEST&&udestsizealign)
		rs.right=(int)((rd.right+udestsizealign-1)/udestsizealign)*udestsizealign;

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
	//Preset Flags that will be sent to UpdateOverlay
	if (!previewoff) dwupdateflags=DDOVER_SHOW|DDOVER_DDFX;
	else dwupdateflags=DDOVER_HIDE|DDOVER_DDFX;
	return (TRUE);
	}


BOOL controlsclass::setuppreview() {
	//**************************************************************
	//             Direct Draw Init
	//Lets Create the surface for Preview and Monitor
	//create the main DirectDraw object
	//DDPIXELFORMAT YUY2format={sizeof(DDPIXELFORMAT),DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0};  // YUY2
	//UYVY below need to enum types
	DDPIXELFORMAT YUY2format={sizeof(DDPIXELFORMAT),DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0}; // UYVY
	DDSURFACEDESC ddsd,ddsdOverlay;
	DDCAPS capsDrv;
	char *errormsg;
	//HRESULT ddrval;
	
	errormsg="Uknown"; //default errormsg for this section
	if (!(DirectDrawCreate(NULL,&lpDD,NULL)==DD_OK)) {return initFail(errormsg);}
	if (!(lpDD->QueryInterface(IID_IDirectDraw2, (void **)&lpDD4)==DD_OK)) {return initFail("Need newer version");}

	lpDD->Release();lpDD=NULL; //We don't need this anymore

	//For Normal operations we no longer need to provide an HWND
	if (!(lpDD4->SetCooperativeLevel(NULL,DDSCL_NORMAL)==DD_OK)) {return initFail(errormsg);}

	// Create the primary surface
	INIT_DIRECTX_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS;
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
	if (!(lpDD4->CreateSurface(&ddsd,&lpDDSPrimary,NULL)==DD_OK))  {return initFail(errormsg);}

	// create a clipper for the primary surface
	if (!(lpDD4->CreateClipper(0,&lpClipper,NULL)==DD_OK))  {return initFail(errormsg);}
	if (!(lpClipper->SetHWnd(0,prevwindow)==DD_OK))  {return initFail(errormsg);}
	if (!(lpDDSPrimary->SetClipper(lpClipper)==DD_OK))  {return initFail(errormsg);}

	//Now to check to see if the PC supports overlays, Yes Storyboard will now only
	//work on machines that have overlay I'm not about to have to convert to RGB
	//At least for this first release.
	errormsg=NO_OVERLAY_HARDWARE;
	// Get driver capabilities to determine Overlay support.
	INIT_DIRECTX_STRUCT(capsDrv);
	if (!(lpDD4->GetCaps(&capsDrv,NULL)==DD_OK))  {return initFail(errormsg);}
	// Does the driver support overlays in the current mode? 
	// (Currently the DirectDraw emulation layer does not support overlays.
	// Overlay related APIs will fail without hardware support).  
	if (!(capsDrv.dwCaps & DDCAPS_OVERLAY)) {return initFail(errormsg);}

	//Now to create the YUV overlay surface
	errormsg=UNABLE_TO_CREATE_OVERLAY;
	//Set up the overlay struct
	INIT_DIRECTX_STRUCT(ddsdOverlay);
	ddsdOverlay.dwBackBufferCount=0;
	ddsdOverlay.ddsCaps.dwCaps=DDSCAPS_OVERLAY|DDSCAPS_VIDEOMEMORY;
	ddsdOverlay.dwFlags= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
	ddsdOverlay.dwWidth=VIDEOX;
	ddsdOverlay.dwHeight=VIDEOY;
    ddsdOverlay.dwBackBufferCount=1;
	//We want raw frame, YUV422 surface, U0Y0V0Y1 U0Y0V0Y1 U0Y0V0Y1
	//Note PC lobyte hibyte makes it appear 0xYVYU reversed
	//It appears Matrox only supports YUY2 oh well
	ddsdOverlay.ddpfPixelFormat=YUY2format;
	// Try to create the overlay surface
	if (!(lpDD4->CreateSurface(&ddsdOverlay,&lpDDSOverlay,NULL)==DD_OK))  {return initFail(errormsg);}
	// Create an overlay FX structure so we can specify a source color key.
	// This information is ignored if the DDOVER_SRCKEYOVERRIDE flag isn't set.
	INIT_DIRECTX_STRUCT(ovfx);
	ovfx.dckSrcColorkey.dwColorSpaceLowValue=0; // Specify black as the color key
	ovfx.dckSrcColorkey.dwColorSpaceHighValue=0;

	return (TRUE);
	}


BOOL controlsclass::initFail(char *reason)  {
	char errormsg[256];

	strcpy(errormsg,"DirectDraw Init FAILED Reason: ");
	strcat(errormsg,reason);
	error(0,errormsg);
	return (FALSE);
	} // End initFail


BOOL controlsclass::controlsfail()  {
	error(0,"The Controls object has detected an internal error\n Please contact technical support");
	return (FALSE);
	} // End initFail


void controlsclass::shutdown() {
	//Lets Cleanup the surface for Preview and Video
	if (lpDD4)  {
		if(lpDDSOverlay)  {
			lpDDSOverlay->UpdateOverlay(NULL, lpDDSPrimary, NULL, DDOVER_HIDE, NULL);
			lpDDSOverlay->Release();
			lpDDSOverlay=NULL;
			}
		if(lpDDSPrimary)  {
			lpDDSPrimary->Release();
			lpDDSPrimary=NULL;
			}
		lpDD4->Release();
		lpDD4=NULL;
		}
	if (lpDD) {
		lpDD->Release();
		lpDD = NULL;
		}
	//End DirectX surface stuff
	if (window) DestroyWindow(window);
	if (prevwindow) DestroyWindow(prevwindow);
	if (frameswindow) DestroyWindow(frameswindow);
	}


controlsclass::~controlsclass() {
	}

