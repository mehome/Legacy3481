#include "HandleMain.h"

#define CP_CLIPNAME		200
#define CP_AUDIOCHK		201
#define CP_AUDIOPATH		202
#define CP_AUDIOASL		203
#define CP_VIDEOCHK		204
#define CP_VIDEOPATH		205
#define CP_VIDEOASL		206
#define CP_START			207

static WAVEFORMATEX wfx={WAVE_FORMAT_PCM,2,44100,176400,4,16,0};
// wFormatTag, nChannels, nSamplesPerSec, mAvgBytesPerSec,
// nBlockAlign, wBitsPerSample, cbSize

//Here is the general capture class implementation
//Eventually this should be separated from the toaster capture

captureclass::captureclass() {
	ULONG align;
	long t;
	DWORD SectorsPerCluster,BytesPerSector,NumberOfFreeClusters,TotalNumberOfClusters;

	//TODO pull the correct value in prefs
	vcaptureptr=(class capcommon *)(new captoaster());
	capturing=0;

	//This is from rtv contruct
	//TODO pull default video drive from prefs
	if (!(GetDiskFreeSpace("v:",&SectorsPerCluster,&BytesPerSector,
		&NumberOfFreeClusters,&TotalNumberOfClusters)))
		BytesPerSector=512;
 
	//the plus 64 is to ensure 512k byte alignment
	if (!(videobuffers=new ULONG[172800*RTVMAXCACHE+BytesPerSector/4])) error(0,"Not enough Memory Available");
	//To type cast a pointer to 32bit and pull the lower 9 bits will still be 64bit compliant!
	align=(ULONG)videobuffers&(BytesPerSector-1); //assuming BPS is a power of 2
	if (align) rtvcache[0]=(ULONG *)((char *)videobuffers+(BytesPerSector-align));
	else rtvcache[0]=videobuffers;
	for (t=1;t<RTVMAXCACHE;t++) rtvcache[t]=*rtvcache+t*172800;
	}

BOOL captureclass::choosedevice(int capdevice) {
	shutdown();
	if (vcaptureptr) delete vcaptureptr;
	switch (capdevice) {
		case 0:
			vcaptureptr=(class capcommon *)(new captoaster());
			break;
		case 1:
			vcaptureptr=(class capcommon *)(new DVCamclass());
			break;
		default: return (FALSE);
		}
	createwindow(screen);
	startup();
	return (TRUE);
	}

captureclass::~captureclass() {
	if (vcaptureptr) delete vcaptureptr;
	delete [] videobuffers;
	}

BOOL captureclass::startup() {
	if (vcaptureptr) return(vcaptureptr->startup());
	else return(0);
	}

void captureclass::showdisplay() {
	if (vcaptureptr) vcaptureptr->showdisplay();
	}

void captureclass::hidedisplay() {
	if (vcaptureptr) vcaptureptr->hidedisplay();
	}

void captureclass::shutdown() {
	if (vcaptureptr) vcaptureptr->shutdown();
	if (vcaptureptr) if (vcaptureptr->window) DestroyWindow(vcaptureptr->window);
	}

void captureclass::createwindow(HWND w_ptr) {
	if (vcaptureptr) vcaptureptr->createwindow(w_ptr);
	}

BOOL captureclass::startcapture() {
	if (vcaptureptr) {
		capturing=TRUE;
		return(vcaptureptr->startcapture());
		}
	else return(0);
	}

void captureclass::stopcapture() {
	if ((vcaptureptr)&&(capturing)) {
		vcaptureptr->stopcapture();
		capturing=FALSE;
		}
	}

void captureclass::captoaster::streamcapture() {
	DWORD capturepos;
	void *lockedbuffer,*dummybuffer;
	DWORD buflen,dummybuflen;

	if FAILED(pDSCB->GetCurrentPosition(&capturepos,NULL)) printerror("Couldn't read cursorpos");
	//wsprintf(string,"currentpos = %ld %ld",capturepos,NULL);printc(string);
	if (capturepos<352800) {
		if (cursorstatus==on2) {
			if (debug) printc("Get 2nd data Audio");
			cursorstatus=on1;
			//lock and load
			pDSCB->Lock(352800,352800,&lockedbuffer,&buflen,&dummybuffer,&dummybuflen,0);
			write(hfile,lockedbuffer,buflen);
			totalbyteswritten+=buflen;
			pDSCB->Unlock(lockedbuffer,buflen,dummybuffer,dummybuflen);
			}
		}
	else {
		if (cursorstatus==on1) {
			if (debug) printc("Get 1st data Audio");
			cursorstatus=on2;
			//lock and load
			pDSCB->Lock(0,352800,&lockedbuffer,&buflen,&dummybuffer,&dummybuflen,0);
			write(hfile,lockedbuffer,buflen);
			totalbyteswritten+=buflen;
			pDSCB->Unlock(lockedbuffer,buflen,dummybuffer,dummybuflen);
			}
		}
	}


void captureclass::captoaster::stopcapture() {
	void *lockedbuffer,*dummybuffer;
	DWORD buflen,dummybuflen;
	DWORD readpos,startoffset,totalsize;
	SetWindowText(startstopbt,"Start");
	//Audio
	if ((SendMessage(audiochk,BM_GETCHECK,0,0)==BST_CHECKED)&&(pDSCB)) {
		KillTimer(window,IDM_CAPTURE);
		if FAILED(pDSCB->Stop()) printerror("Warning: capture sound failed");
		//Now patch in the last bit of sound captured here
		pDSCB->GetCurrentPosition(NULL,&readpos);
		if (readpos>=352800) startoffset=352800;
		else startoffset=0;
		pDSCB->Lock(startoffset,readpos,&lockedbuffer,&buflen,&dummybuffer,&dummybuflen,0);
		write(hfile,lockedbuffer,buflen);
		totalbyteswritten+=buflen;
		pDSCB->Unlock(lockedbuffer,buflen,dummybuffer,dummybuflen);
		//Finally update the header for total data and file size
		lseek(hfile,4,SEEK_SET);
		totalsize=totalbyteswritten+0x24;
		write(hfile,&totalsize,4);
		lseek(hfile,0x20,SEEK_CUR);
		write(hfile,&totalbyteswritten,4);
		if (hfile) close(hfile);
		hfile=NULL;
		//must release the capture buffer to reset it
		if (pDSCB) pDSCB->Release();
		pDSCB=NULL;
		}
	if (SendMessage(videochk,BM_GETCHECK,0,0)==BST_CHECKED) {
		toaster->stopcapture();
		}
	}


BOOL captureclass::captoaster::startcapture() {
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
	//For sync reasons the first mediagram callback will start the audio capture
	//Video
	//configure videofilename
	GetWindowText(clipnameedit,string,MAX_PATH);
	wsprintf(toaster->toastercapobj->videofilename,"%s%s.rtv",videopath,string);
	if (SendMessage(videochk,BM_GETCHECK,0,0)==BST_CHECKED) {
		toaster->startcapture();
		}
	else startaudiocapture(); //since toaster mediagram starts the audio otherwise
	return (TRUE);
	}


BOOL captureclass::captoaster::startaudiocapture() {
	const static char header[20]={'R','I','F','F',' ',' ',' ',' ','W','A','V','E','f','m','t',' ',16,0,0,0};
	//Audio
	if (SendMessage(audiochk,BM_GETCHECK,0,0)==BST_CHECKED) {
		//A real lame way to reset the capture cursor :P
	 	if FAILED(lpDSC->CreateCaptureBuffer(&dscbd,&pDSCB,NULL))
			return initFail("Unable to create DS capture sound buffer");
		//configure audiofilename
		GetWindowText(clipnameedit,string,MAX_PATH);
		wsprintf(audiofilename,"%s%s.wav",audiopath,string);
		//start to fill in header info
		hfile=open(audiofilename,_O_WRONLY|_O_CREAT|_O_TRUNC|_O_BINARY|_O_SEQUENTIAL,_S_IREAD | _S_IWRITE);
		write(hfile,&header,20);
		write(hfile,&wfx,16);
		write(hfile,"data    ",8);
		cursorstatus=on1;
		totalbyteswritten=0;
		SetTimer(window,IDM_CAPTURE,1000,0);
		if FAILED(pDSCB->Start(DSCBSTART_LOOPING)) printerror("Warning: unable to capture sound");
		}
	return (TRUE);
	}


int captureclass::captoaster::Callback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	char buffer[MAX_PATH];

	switch(uMsg) {

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case CP_AUDIOCHK:
							if (SendMessage(audiochk,BM_GETCHECK,0,0)==BST_CHECKED) {
								SendMessage(audiochk,BM_SETCHECK,BST_UNCHECKED,0);
								EnableWindow(audiopathedit,FALSE);
								EnableWindow(audioasl,FALSE);
								}
							else {
								SendMessage(audiochk,BM_SETCHECK,BST_CHECKED,0);
								EnableWindow(audiopathedit,TRUE);
								EnableWindow(audioasl,TRUE);
								}
							break;
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
						case CP_AUDIOASL:
							getdirectory("Select Audio Path",audiopath);
							SetWindowText(audiopathedit,audiopath);
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
								filenumber=0;
								break;
							case CP_AUDIOPATH:
								GetWindowText(audiopathedit,audiopath,MAX_PATH);
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
			break;
			} //end WM_COMMAND

		case WM_TIMER:
			streamcapture();
			return(0);

		case WM_CLOSE:
			windowtoggle(screen,window,IDM_CAPTURE);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}


captureclass::captoaster::captoaster() {
	strcpy(audiopath,"C:\\VideoMedia\\");
	strcpy(videopath,"V:\\VideoMedia\\");
	strcpy(clipname,"Default");
	filenumber=hfile=totalbyteswritten=0;
	}

void captureclass::captoaster::createwindow(HWND w_ptr) {
	int toolx=0;
	int tooly=24;

	window=CreateWindowEx(0,"OBJGRAYWIN","Capture",
		WS_POPUP|WS_OVERLAPPEDWINDOW,310,70,300,180,w_ptr,
		NULL,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(long)this);

	//since we are not going to resize we'll put the children here
	CreateWindowEx(0,"STATIC","Clip Name:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,80,CONTROLBUTTONY,window,NULL,hInst,NULL);
	toolx+=84;
	clipnameedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
		,toolx,tooly,200,CONTROLBUTTONY,window,(HMENU)CP_CLIPNAME,hInst,NULL);
	SetWindowText(clipnameedit,clipname);
	toolx=0;
	tooly+=24;

	audiochk=CreateWindowEx(0,"BUTTON","Audio",WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		toolx,tooly,64,CONTROLBUTTONY,window,
		(HMENU)CP_AUDIOCHK,hInst,NULL);
	SendMessage(audiochk,BM_SETCHECK,BST_CHECKED,0);
	toolx+=64;
	audiopathedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
		,toolx,tooly,200,CONTROLBUTTONY,window,(HMENU)CP_AUDIOPATH,hInst,NULL);
	SetWindowText(audiopathedit,audiopath);
	toolx+=204;
	audioasl=CreateWindowEx(0,"BUTTON","...",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
		toolx,tooly,20,CONTROLBUTTONY,window,
		(HMENU)CP_AUDIOASL,hInst,NULL);
	toolx=0;
	tooly+=24;

	videochk=CreateWindowEx(0,"BUTTON","Video",WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		toolx,tooly,64,CONTROLBUTTONY,window,
		(HMENU)CP_VIDEOCHK,hInst,NULL);
	SendMessage(videochk,BM_SETCHECK,BST_CHECKED,0);
	toolx+=64;
	videopathedit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL
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


BOOL captureclass::captoaster::startup() {
	lpDSC = NULL;
	DSCCAPS dscaps;
	//HRESULT hr;
	//DirectSoundinit area
	if FAILED(DirectSoundCaptureCreate(NULL,&lpDSC,NULL))
		return initFail("Unable to create DS Capture Object");
	INIT_DIRECTX_STRUCT(dscaps);
	dscaps.dwFlags=NULL;
	dscaps.dwFormats=WAVE_FORMAT_4S16;
	dscaps.dwChannels=2;
	if FAILED(lpDSC->GetCaps(&dscaps))
		return initFail("Unable to get the capture caps");

	// Create capture buffer
	INIT_DIRECTX_STRUCT(dscbd);
	dscbd.dwFlags=0;
	dscbd.dwBufferBytes=wfx.nAvgBytesPerSec*4; //4 seconds
	dscbd.dwReserved=0;
	dscbd.lpwfxFormat=&wfx;
	pDSCB=NULL;
	return(TRUE);
	} //end startup


void captureclass::captoaster::showdisplay() {
	ShowWindow(window,SW_SHOW);
	CheckMenuItem(m_ptr,IDM_CAPTURE,MF_BYCOMMAND|MFS_CHECKED);
	if (toaster->rtmeoutput) toaster->startupcapture();
	}


void captureclass::captoaster::hidedisplay() {
	ShowWindow(window,SW_HIDE);
	CheckMenuItem(m_ptr,IDM_CAPTURE,MF_BYCOMMAND|MFS_UNCHECKED);
	}


void captureclass::captoaster::shutdown() {
	if (lpDSC) {
		lpDSC->Release();
		lpDSC = NULL;
		}
	}


BOOL captureclass::captoaster::initFail(char *reason)  {
	printerror("Warning: DirectSound Init FAILED Reason: ");
	printc(reason);
	printc("Capture is not available");
	lpDSC = NULL;
	return (FALSE);
	} // End initFail


captureclass::captoaster::~captoaster() {
	}

DWORD captureclass::streamfunc()
	{
	WaitForSingleObject(arrayofevents[EVENT_STREAM],INFINITE);
	while (!killstreamthread) {
		if (streamthreadmode) toaster->writertv();
		else dv2rtv->dv2rtvmain();
		WaitForSingleObject(arrayofevents[EVENT_STREAM],INFINITE);
		} //end while not killstreamthread
	return (0);
	} // end streamfunc()
