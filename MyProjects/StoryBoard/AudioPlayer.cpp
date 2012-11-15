#include "HandleMain.h"
#include "audio.h"

#define WAVBUFSIZE 4
#define HALFWAVSIZE WAVBUFSIZE>>1

audioclass::playerclass::playerclass() {
	voicehead=voicetail=NULL;
	}
audioclass::playerclass::~playerclass() {
	}

BOOL audioclass::playerclass::streamwav() {
	struct wavvoicelist *voiceindex,*voicenext;
	struct wavinfo *wavobj;
	void *dsbuf1,*dsbuf2;
	DWORD dsbuflen1,dsbuflen2,playpos;
	ULONG halfbuffersize,cursoroffset;
	UINT bytesread;
	LPDIRECTSOUNDBUFFER lpdsb;
	wavcursorstat oldstatus;
	BOOL cursorstatus;
	BOOL streamit;

	if (voiceindex=voicehead) {
		do {
			voicenext=voiceindex->next;
			wavobj=voiceindex->voice;
			lpdsb=wavobj->lpdsb;
			halfbuffersize=wavobj->halfbuffersize;
			oldstatus=wavobj->cursorstatus;
			// first see if voice needs to be streamed this also is necessary to know when its
			//time to remove a voice
			lpdsb->GetCurrentPosition(&playpos,NULL);
			cursorstatus=playpos>halfbuffersize;
			//weed out the offs
			if ((oldstatus==on1)||(oldstatus==on2)) {
				streamit=FALSE;
				if (cursorstatus) {
					if (oldstatus==on2) {
						if (debug) printc("Stream to 1");
						cursoroffset=0;
						wavobj->cursorstatus=on1;
						streamit=TRUE;
						}
					}
				else { //cursor status is on the first set 
					if (oldstatus==on1) {
						if (debug) printc("Stream to 2");
						cursoroffset=halfbuffersize;
						wavobj->cursorstatus=on2;
						streamit=TRUE;
						}
					}

				if (streamit) {

					if FAILED(lpdsb->Lock(cursoroffset,halfbuffersize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)) {
						return initFail("Unable to lock DS buffer");
						}
	
					bytesread=readwave(wavobj,dsbuf1,dsbuflen1);

					if (bytesread<dsbuflen1) {
						// Fill in silence for the rest of the buffer.
						FillMemory((char *)dsbuf1+bytesread, dsbuflen1-bytesread,
							(BYTE)(wavobj->pcm.wBitsPerSample == 8 ? 128 : 0));
						//TODO fill other half of buffer with zero too
						if (debug) printc("signaled off");
						if (oldstatus==on1) wavobj->cursorstatus=off1;
						else wavobj->cursorstatus=off2;
						}

					//unlock immediately
					if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
						return initFail("Unable to unlock DS buffer");
					} //end streamit
				}// end if oldstatus=on1 or on2
			else { //old status signaled for off
				if (((cursorstatus)&&(oldstatus==offc2))||((cursorstatus==0)&&(oldstatus==offc1))) {
					if (debug) printc("removed voice");
					removewavvoice(voiceindex);
					continue;
					}
				if ((cursorstatus)&&(oldstatus==off1)) {
					wavobj->cursorstatus=offc1;
					if FAILED(lpdsb->Lock(0,halfbuffersize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0))
						return initFail("Unable to lock DS buffer");
					FillMemory((char *)dsbuf1, dsbuflen1,
						(BYTE)(wavobj->pcm.wBitsPerSample == 8 ? 128 : 0));
					if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
						return initFail("Unable to unlock DS buffer");
					}
				if ((cursorstatus==0)&&(oldstatus==off2)) {
					wavobj->cursorstatus=offc2;
					if FAILED(lpdsb->Lock(halfbuffersize,halfbuffersize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0))
						return initFail("Unable to lock DS buffer");
					FillMemory((char *)dsbuf1, dsbuflen1,
						(BYTE)(wavobj->pcm.wBitsPerSample == 8 ? 128 : 0));
					if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
						return initFail("Unable to unlock DS buffer");
					}
				}
			} while (voiceindex=voicenext);
		}
	return (TRUE);
	}


void audioclass::playerclass::addwavvoice(struct wavinfo *wavobj) {
	struct wavvoicelist *voiceindex,*voice;
	//link new point node
	voice=(struct wavvoicelist *)newnode(nodeobject,sizeof(struct wavvoicelist));
	EnterCriticalSection(&csglobal);
	voice->voice=wavobj;
	if (voiceindex=voicetail) {
		voice->prev=voiceindex;
		voice->next=NULL;
		voiceindex->next=voice;
		voicetail=voice;
		}
	else { //firstnode
		voicetail=voicehead=voice;
		voice->prev=voice->next=NULL;
		}
	LeaveCriticalSection(&csglobal);
	}


void audioclass::playerclass::removewavvoice(struct wavvoicelist *voice) {
	struct wavinfo *wavobj=voice->voice;
	EnterCriticalSection(&csglobal);
	if (voice->prev) voice->prev->next=voice->next;
	else voicehead=voice->next;
	if (voice->next) voice->next->prev=voice->prev;
	else voicetail=voice->prev;
	LeaveCriticalSection(&csglobal);
	disposenode(nodeobject,(struct memlist *)voice);
	//for now lets have this close the all of the voice contents
	closeWAV(wavobj);
	}


void audioclass::playerclass::closevoices() {
	struct wavvoicelist *voiceindex,*tempindex;
	if (voiceindex=voicehead) {
		do {
			closeWAV(voiceindex->voice);
			tempindex=voiceindex->next;
			disposenode(nodeobject,(struct memlist *)voiceindex);
			} while (voiceindex=tempindex);
		}
	voicehead=voicetail=NULL;
	}

BOOL audioclass::playerclass::initFail(char *reason)  {
	char errormsg[256];

	strcpy(errormsg,"DirectDraw Init FAILED Reason: ");
	strcat(errormsg,reason);
	error(0,errormsg);
	return (FALSE);
	} // End initFail


BOOL audioclass::playerclass::startup() {
	lpDS = NULL;
	lpdsbPrimary=NULL;
	voicehead=voicetail=NULL;

	//HRESULT hr;
	WAVEFORMATEX wfx;
	//DirectSoundinit area
	//set up DirectSound object
	if FAILED(DirectSoundCreate(NULL,&lpDS,NULL))
		return initFail("Unable to create DirectSound object");
	if FAILED(lpDS->SetCooperativeLevel(screen,DSSCL_PRIORITY))
		return initFail("Unable to set CooperativeLevel in DS");
	//Cooperative level of priority allows formating primary buffer
	// Obtain primary buffer
	INIT_DIRECTX_STRUCT(dsbdesc);
	dsbdesc.dwFlags=DSBCAPS_PRIMARYBUFFER;
	if FAILED(lpDS->CreateSoundBuffer(&dsbdesc,&lpdsbPrimary,NULL))
		return initFail("Unable to create DS primary sound buffer");

	// Set primary buffer format
 	memset(&wfx,0,sizeof(WAVEFORMATEX)); 
	wfx.wFormatTag=WAVE_FORMAT_PCM; 
	wfx.nChannels=2; 
	wfx.nSamplesPerSec=48000; 
	wfx.wBitsPerSample=16; 
	wfx.nBlockAlign=wfx.wBitsPerSample/8*wfx.nChannels;
	wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
	if FAILED(lpdsbPrimary->SetFormat(&wfx))
		return initFail("Unable to format DS default standard");
	return(TRUE);
	} //end startup


int audioclass::playerclass::frame2audiobytes(int frame,struct wavinfo *wavobj) {
	int result;
	result=((wavobj->pcm.nAvgBytesPerSec*1000/29970)*frame)&0xFFFFFFFC;
	return ((int)result);
	}


BOOL audioclass::playerclass::openmedia(struct wavinfo *wavobj) {
	struct imagelist *streamptr;
	long cropin,in,cropinresult;
	struct wavinfo *audiomedia;
	LPDIRECTSOUNDBUFFER lpdsb;
	UINT bytesread;

	//Clean up any previous buffer and notification
	if (wavobj->hfile) 
		closeWAV(wavobj);
	//TODO remove voice by including voice in the struct
	if (!(openWAV(wavobj))) return(0);
	wavobj->outbytes=0;
	//Now check and implement cropin
	if (streamptr=controls->streamptr) if (audiomedia=(struct wavinfo *)streamptr->audio) {
		cropin=streamptr->cropin;
		in=audiomedia->in;
		if (audiomedia->frameoffset<cropin) cropinresult=cropin-audiomedia->frameoffset;
		else cropinresult=0;
		lseek(wavobj->hfile,max(frame2audiobytes(cropinresult,wavobj),frame2audiobytes(in,wavobj)),SEEK_CUR);
		//calculate cropout
		if (wavobj->out) wavobj->outbytes=wavobj->totalsize-min(frame2audiobytes(streamptr->cropout-cropin,wavobj),frame2audiobytes(audiomedia->out-in,wavobj));
		}
	

	INIT_DIRECTX_STRUCT(dsbdesc);
	dsbdesc.dwFlags=
		//DSBCAPS_CTRLDEFAULT|		// Have some controls
		//DX7 Has removed this flag must specify individually 
		//how stupid... sigh growing pains
		DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY|
		DSBCAPS_GLOBALFOCUS|		// Allows background playing
		DSBCAPS_STATIC;
	//For now we'll set the buffersize for 4 seconds
	dsbdesc.dwBufferBytes=wavobj->pcm.nAvgBytesPerSec*WAVBUFSIZE;
	//pull format specs
	dsbdesc.lpwfxFormat=&wavobj->pcm;


	if FAILED(lpDS->CreateSoundBuffer(&dsbdesc,&lpdsb,NULL))
		return initFail("Unable to create DS secondary sound buffer");
	wavobj->lpdsb=lpdsb;
	//Set up the notifications

	//Set up the first part of stream before play
	{
	void *dsbuf1,*dsbuf2;
	DWORD dsbuflen1,dsbuflen2;
	wavobj->halfbuffersize=wavobj->pcm.nAvgBytesPerSec*HALFWAVSIZE;

		if FAILED(lpdsb->Lock(0,wavobj->halfbuffersize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0))
			return initFail("Unable to lock DS buffer");

		bytesread=readwave(wavobj,dsbuf1,dsbuflen1);

		if (bytesread<dsbuflen1) {
			wavobj->cursorstatus=off1;
			// Fill in silence for the rest of the buffer.
			FillMemory((char *)dsbuf1+bytesread, dsbuflen1-bytesread,
				(BYTE)(wavobj->pcm.wBitsPerSample == 8 ? 128 : 0));

			}
		else wavobj->cursorstatus=on1;

		//unlock immediately
		if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
			return initFail("Unable to unlock DS buffer");
	} //end filling buffer w/sound
	//check to see if we read entire buffer already
	return(TRUE);
	} //end openmedia

void audioclass::playerclass::playwav(struct wavinfo *wavobj) {
	LPDIRECTSOUNDBUFFER lpdsb=wavobj->lpdsb;
	// Ensure that position is at 0, ready to go
	lpdsb->SetCurrentPosition(0);
	//might as well play it
	lpdsb->Play(0,0,DSBPLAY_LOOPING);
	addwavvoice(wavobj);
	}


UINT audioclass::playerclass::readwave(struct wavinfo *wavobj,void *dsbuf1,DWORD dsbuflen1) {
	UINT bytesread;
	DWORD length;
	length=min(dsbuflen1,wavobj->totalsize-wavobj->outbytes-wavobj->currentbytesread);
	wavobj->currentbytesread+=length;

	EnterCriticalSection(&csglobal);
	bytesread=read(wavobj->hfile,dsbuf1,length);
	LeaveCriticalSection(&csglobal);
	return (bytesread);
	}


void audioclass::playerclass::shutdown() {
	closevoices();
	//The DirectSound Object will auto close all buffers
	if (lpDS) {
		lpDS->Release();
		lpDS = NULL;
		}
	}


BOOL audioclass::playerclass::openWAV(struct wavinfo *wavobj) {
	char string[256];
	int hfile=NULL;
	
	ULONG *checkid=(ULONG *)string;
	ULONG advance;
	BOOL done=false;

	//wsprintf(string,"Loading %s...",filename);printc(string);
	EnterCriticalSection(&csglobal);
	wavobj->hfile=hfile=open(wavobj->filesource,_O_RDONLY|_O_BINARY|_O_SEQUENTIAL);
	LeaveCriticalSection(&csglobal);

	if (hfile==-1) {wsprintf(string,"Unable to open file");goto errorwav;}
	read(hfile,string,12);
	if ((*checkid!=ID_RIFF)||(*(checkid+2)!=ID_WAVE)) {
		wsprintf(string,"Not RIFF WAVE file");
		goto errorwav;
		}
	wavobj->size=*(checkid+1);
	if (debug) {
		wsprintf(string,"RIFF size (total chunks)=%ld",wavobj->size);
		printc(string);
		}
	do {
		if (!(read(hfile,string,8))) {
			wsprintf(string,"Unable to find the data chunk in WAV file");
			goto errorwav;
			}
		advance=*(checkid+1);
		string[4]=0;
		if (debug) {
			wsprintf(string,"%s size,%ld",string,advance);
			printc(string);
			}
		switch (*checkid) {
			case ID_fmt: {
				ULONG pcmsize=sizeof(PCMWAVEFORMAT);
				// Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
				// if there are extra parameters at the end, we'll ignore them
				if (advance<pcmsize) {
					wsprintf(string,"Only uncompressed standard PCM format supported at this time");
					goto errorwav;
					}
				read(hfile,&(wavobj->pcm),pcmsize);
				if (!(wavobj->pcm.wFormatTag==1)) {
					wsprintf(string,"Only uncompressed standard PCM format supported at this time");
					goto errorwav;
					}
				if (advance>pcmsize) lseek(hfile,advance-pcmsize,SEEK_CUR);
				break;
				 } //end case ID_fmt
			case ID_data:
				wavobj->currentbytesread=0;
				wavobj->totalsize=advance;
				done=TRUE;
			break;
			default:
			lseek(hfile,advance,SEEK_CUR);
			}
		} while (!done);
	return(TRUE);
errorwav:
	if (hfile) {
		close(hfile);
		wavobj->hfile=NULL;
		}
	//disposenode(nodeobject,(struct memlist *)wavobj);
	printc(string);
	return(FALSE);
	}


void audioclass::playerclass::closeWAV (struct wavinfo *wavobj) {
	if (wavobj) {
		if (wavobj->lpdsb) {
			wavobj->lpdsb->Release();
			wavobj->lpdsb=NULL;
			}
		if (wavobj->hfile) {
			close(wavobj->hfile);
			wavobj->hfile=NULL;
			}
		}
	}
