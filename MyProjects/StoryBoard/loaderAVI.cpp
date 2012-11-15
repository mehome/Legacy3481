#include "HandleMain.h"
#include "../DV_kit/AVIparser.h"
#include "../include/madventry.h"
#include "../DV_kit/DVstructs.h"
#pragma comment(lib, "../lib/DVSplitter")
#pragma comment(lib, "../lib/mcdvd_32")

#define BI_UYVY         mmioFOURCC('U','Y','V','Y')
#define BI_VYUY         mmioFOURCC('V','Y','U','Y') //ATI-Alias for YUYV
#define BI_YUY2         mmioFOURCC('Y','U','Y','2') //MS-Alias for  YUYV
#define BI_YUV2         mmioFOURCC('Y','U','V','2') //MXB-Alias for UYVY

/*
const long RGB1[2]={0xb5817062,0x0000da1d};
const long RGB2[2]={0x7fff1917,0x000041cb};
const long RGB3[2]={0xa1bcedd3,0x00007062};
const long addoffsets[2]={0x00100080,0x00100080};
const double baseframerate=0.033366700033366;
*/

loadersclass::aviclass::aviclass() {
	avivarlisthead=NULL;
	}

loadersclass::aviclass::~aviclass() {
	}

UWORD loadersclass::aviclass::gettotalavi(struct imagelist *media) {
	class avihandle *avivar=media->mediaavi;
	UWORD actualframes=(UWORD)avivar->totalframes;
	//UWORD virtualframes=(UWORD)(actualframes/avivar->framerate);
	//wsprintf(string,"newsize=%d",virtualframes);
	//printc(string);
	return (actualframes);
	}


HBITMAP loadersclass::aviclass::getthumbavi(struct imagelist *mediaptr,ULONG framenum,LPBITMAPINFOHEADER *bitmapinfo) {
	void *buffer;
	class avihandle *avivar=mediaptr->mediaavi;
	HANDLE hfile=avivar->hfile;
	BITMAPINFOHEADER *outbmi=avivar->outbmi;
	char *invideobuf=avivar->invideobuf;
	char *outvideobuf=avivar->outvideobuf;
	BYTE *audiopcm=avivar->audiopcm;
	BYTE *audiopcm2=avivar->audiopcm2;
	HIC hic=avivar->hic;
	//LPBITMAPINFOHEADER source;
	DWORDLONG qwframeoffset;
	DWORD dwOffset;		 // 32 bit offset to data (points to data, not riff header)
	DWORD dwSize;			// 31 bit size of data (does not include size of riff header) (high bit is deltaframe bit)
	HDC hdcimage;
	HBITMAP hbm=NULL;
	//char *decodedbits;

	hdcimage=CreateCompatibleDC(NULL);

	*bitmapinfo=(LPBITMAPINFOHEADER)outbmi; 

	mediaptr->thumb=hbm=CreateDIBSection(hdcimage,(LPBITMAPINFO)*bitmapinfo,DIB_RGB_COLORS,&buffer,NULL,NULL);
	if (hbm) {
		if (avivar->KFnumber) framenum=avivar->KFnumber[(avivar->totalKF)>>1];
		avivar->getframeoffset(framenum,avivar->AVIstreamhead,&qwframeoffset,&dwOffset,&dwSize);
		mySeek64(hfile,qwframeoffset+dwOffset,SEEK_SET);
		if (avivar->bmi->biCompression) {
			myRead(hfile,invideobuf,dwSize);
			if (ICDecompress(hic,0,avivar->bmi,invideobuf,outbmi,buffer) != ICERR_OK) {
				memset(buffer,128,outbmi->biSizeImage);
				}
			}
		else myRead(hfile,buffer,dwSize);
		}
	if (hdcimage) DeleteDC(hdcimage);
	return (hbm);
	}


BOOL loadersclass::aviclass::renderframe (ULONG *videobuf,class avihandle *avivar) {
	int x,y,scalex,scaley;
	BITMAPINFOHEADER *outbmi=avivar->outbmi;
	char *outvideobuf=avivar->outvideobuf;
			
	scalex=avivar->scalex;scaley=avivar->scaley;
	x=avivar->x;y=avivar->y;
			
	switch (outbmi->biBitCount) {
		case 8:
			if (avivar->scalex>1) renderrgb8scale(videobuf,x,y,scalex,scaley,outvideobuf,outbmi);
			else renderrgb8(videobuf,x,y,outvideobuf,outbmi,(BOOL)avivar->upsidedown);
			break;
		case 16:
			//TODO check compression for reverse value for YUV
			if (outbmi->biCompression) {
				if (avivar->scalex>1) renderyuvscale(videobuf,x,y,scalex,scaley,outvideobuf,(BOOL)avivar->upsidedown);
				else renderyuv(videobuf,x,y-1,outvideobuf+(x*2),(BOOL)avivar->upsidedown);
				}
			else {
				if (avivar->scalex>1) renderrgb16scale(videobuf,x,y,scalex,scaley,outvideobuf,(BOOL)avivar->upsidedown);
				else renderrgb16(videobuf,x,y,outvideobuf,(BOOL)avivar->upsidedown);
				}
			break;
		case 24:
			//printc("24");
			if (avivar->scalex>1) renderrgb24scale(videobuf,x,y,scalex,scaley,outvideobuf);
			else renderrgb24(videobuf,x,y,outvideobuf,(BOOL)avivar->upsidedown);
			break;
		case 32:
			//printc("32");
			if (avivar->scalex>1) renderrgb32scale(videobuf,x,y,scalex,scaley,outvideobuf,(BOOL)avivar->upsidedown);
			else renderrgb32(videobuf,x,y,outvideobuf,(BOOL)avivar->upsidedown);
			break;
		default:
			printc("Unable to render uncompressed format");
			return(FALSE);
		}
	return (TRUE);
	} //end renderframe


void loadersclass::aviclass::streamaudio(int notifypos) {
	struct avivarlist *avivarindex=avivarlisthead;
	//print("npos=%d ",notifypos);
	if (avivarindex) {
		do {
			class avihandle *avivar=avivarindex->avivar;
			void *dsbuf1,*dsbuf2;
			DWORD dsbuflen1,dsbuflen2;
			LPDIRECTSOUNDBUFFER lpdsb=avivar->lpdsb;
			BYTE *audiopcm;
			ULONG samplesize,totalsamples=0;
			DWORD currentpos,bytesneeded;
			DWORD audiobuffersize=avivar->audiobuffersize;
			DWORD lastoffset=avivar->lastoffset;
			DWORD playstatus;
			//TODO make sure this avi is ready to be streamed

		if (avivar->lpdsb) {
			//figure out how many bytes are needed to satify the notification area
			avivar->lastnotify=notifypos;
			switch (notifypos) {
				//case 1: 
					//bytesneeded=audiobuffersize+(audiobuffersize>>1)-lastoffset;
					//break;
				case 1: 
					bytesneeded=(audiobuffersize<<1)+(audiobuffersize>>1)-lastoffset;
					break;
				case 2: 
					bytesneeded=(audiobuffersize*3)+(audiobuffersize>>1)-lastoffset;
					break;
				case 3: 
					if (lastoffset>(audiobuffersize>>1)) bytesneeded=avivar->totalaudiosize-lastoffset+(audiobuffersize>>1);
					else bytesneeded=(audiobuffersize>>1)-lastoffset;
					break;
				case 4: 
					if (lastoffset>audiobuffersize+(audiobuffersize>>1)) bytesneeded=avivar->totalaudiosize-lastoffset+audiobuffersize+(audiobuffersize>>1);
					else bytesneeded=audiobuffersize+(audiobuffersize>>1)-lastoffset;
					break;
				default:
					bytesneeded=(audiobuffersize<<1);
				}
			//lpdsb->GetCurrentPosition(&currentpos,NULL);
			//print("lo=%lx %lx bn=%lx ",avivar->lastoffset,currentpos,bytesneeded);
			do {
				if (avivar->pcmlisttail) samplesize=avivar->pcmlisttail->samplesize;
				else {
					BOOL stopit=0;

					if ((avivar->type1)||(audiobuffersize<8000)) {
						lpdsb->Stop();
						break;
						}

					if FAILED(lpdsb->GetCurrentPosition(&currentpos,NULL)) break;
					//printc("%lx",currentpos);

					lpdsb->GetStatus(&playstatus);
					if (!((playstatus&(DSBSTATUS_PLAYING|DSBSTATUS_LOOPING)==(DSBSTATUS_PLAYING|DSBSTATUS_LOOPING)))) break;
					if (currentpos>=avivar->lastoffset) {
						stopit=TRUE;
						if (currentpos-avivar->lastoffset>avivar->audiobuffersize) stopit=FALSE;
						}
					if (stopit) {
						lpdsb->Stop();
						break;
						}
					Sleep(1);
					continue;
					}
				totalsamples+=samplesize;

				//skip if this audio is not read to stream
				//printc("ss %lx",samplesize);
				if FAILED(lpdsb->Lock(avivar->lastoffset,samplesize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)) {
					printc("Unable to lock DS buffer");
					continue;
					}

				audiopcm=avivar->pcmlisttail->audiopcm;
				memcpy (dsbuf1,audiopcm,min(dsbuflen1,samplesize));
				if (dsbuf2) {
					if (samplesize>dsbuflen1) memcpy(dsbuf2,audiopcm+dsbuflen1,min(samplesize-dsbuflen1,dsbuflen2));
					avivar->lastoffset=dsbuflen2;
					}
				else avivar->lastoffset+=dsbuflen1;
				//remove this pcm from the queue
				{
				struct pcmlist *pcmlisttemp;

				EnterCriticalSection(&csglobal);
				pcmlisttemp=avivar->pcmlisttail;
				avivar->pcmlisttail=avivar->pcmlisttail->prev;
				if (avivar->pcmlisttail) avivar->pcmlisttail->next=NULL;
				else avivar->pcmlisthead=NULL;
				//dispose tail
				if (pcmlisttemp->audiopcm) dispose ((struct memlist *)pcmlisttemp->audiopcm,&pmem);
				disposenode(nodeobject,(struct memlist *)pcmlisttemp);
				LeaveCriticalSection(&csglobal);
				}

				if (avivar->lastoffset>=avivar->totalaudiosize) 
					avivar->lastoffset=0;
				//unlock immediately
				if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2)) {
					printc("Unable to unlock DS buffer");
					continue;
					}
				} while(totalsamples<bytesneeded);
			if (totalsamples>=bytesneeded) {
				avivar->lpdsb->GetStatus(&playstatus);
				if (!(playstatus&DSBSTATUS_PLAYING)) avivar->lpdsb->Play(0,0,DSBPLAY_LOOPING);
				}
			} // end if we have sound

	
			} while(avivarindex=avivarindex->next);
		}
	}


BOOL loadersclass::aviclass::queueaudio(struct imagelist *mediaptr,ULONG framenum,ULONG samplesize) {
	class avihandle *avivar=mediaptr->mediaavi;
	struct pcmlist *pcmnode;
	DWORD playstatus;

	if (((avivar->audiobuffersize<8000)||(controls->mycontrolis&2))&&(avivar->lpdsb)) {
		//copy pcm to a node and link to queue
		pcmnode=(struct pcmlist *)newnode(nodeobject,sizeof(struct pcmlist));
		pcmnode->audiopcm=(BYTE *)mynew(&pmem,samplesize);
		memcpy(pcmnode->audiopcm,avivar->audiopcm,samplesize);
		pcmnode->samplesize=samplesize;

		EnterCriticalSection(&csglobal);
		pcmnode->prev=NULL;
		pcmnode->next=avivar->pcmlisthead;
		if (avivar->pcmlisthead) avivar->pcmlisthead->prev=pcmnode;
		else avivar->pcmlisttail=pcmnode;
		avivar->pcmlisthead=pcmnode;
		LeaveCriticalSection(&csglobal);

		avivar->lpdsb->GetStatus(&playstatus);
		if (!(playstatus&DSBSTATUS_PLAYING)) streamaudio(avivar->lastnotify);
		}
	return(0);
	} //end stream audio 


BOOL loadersclass::aviclass::getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	AVIstream *audiostream;
	class avihandle *avivar=mediaptr->mediaavi;
	BITMAPINFOHEADER *outbmi=avivar->outbmi;
	char *invideobuf=avivar->invideobuf;
	char *outvideobuf=avivar->outvideobuf;
	BYTE *audiopcm=avivar->audiopcm;
	BYTE *audiopcm2=avivar->audiopcm2;
	HIC hic=avivar->hic;

	DWORDLONG qwframeoffset,audiobase;
	DWORD dwOffset,audiooffset;		 // 32 bit offset to data (points to data, not riff header)
	DWORD dwSize;			// 31 bit size of data (does not include size of riff header) (high bit is deltaframe bit)
	HANDLE hfile=avivar->hfile;
	int pcmsize;
	BOOL surroundsound=avivar->surroundsound;

	//framenum--; //frame is 1 based our parser is zero based
	avivar->getframeoffset(--framenum,avivar->AVIstreamhead,&qwframeoffset,&dwOffset,&dwSize);
	//Step 2 Decode Frame and Audio
	if (surroundsound||avivar->type1) {
		//Everything in this scope is exclusive for Type 1 DV decoding
		//Ok to interface with DV I'll need the entire chunk in memory may need to move this part
		//int y,line=720*2;

		mySeek64(hfile,qwframeoffset+dwOffset,SEEK_SET);
		myRead(hfile,invideobuf,dwSize);

		DecompressBuffer_DV (
			(PBYTE)invideobuf,  // source buffer (dv frame)
			dwSize,  // source buffer length
			(PBYTE)outvideobuf,  // dest buffer (uncompressed frame)
			(LONG)720*2,  // byte size of dest buffer line this may differ from actual pixel size of line
			720,  // pixel width for dest buffer (may differ from DV)
			480,  // pixel height for dest buffer (may differ from DV)
			0,  // currently not used on DV (0)
			FOURCC_UYVY,  // format for decoded frame FOURCC_BGR3,FOURCC_UYVY,...
			0,  // currently not used on DV (0)
			NULL  // currently not used on DV (NULL)
			);
		//need to de-interleave the lines
		renderyuv(videobuf,720,479,outvideobuf+(720*2),0);
		//Now to interface w/ mikes DV splitter
		//save("dvframe.dv",dwSize,(char *)framebuf);

		if (surroundsound) {
			pcmsize=decodeAudio((PBYTE)invideobuf,audiopcm,1);
			pcmsize=decodeAudio((PBYTE)invideobuf,audiopcm2,2);
			}
		else pcmsize=decodeAudio((PBYTE)invideobuf,audiopcm,0);
		//Ok this value of pcm size is in samples for now we'll assume we have 16 bit samples (2 bytes)
		//and have 2 channels (stereo) we'll need to calculate bytes per sample later for now it is 4
		pcmsize*=avivar->waveinfo.nBlockAlign;
		queueaudio(mediaptr,framenum,pcmsize);
		}
	//pull audio from audio stream
	else {
		long audioframe;
		//assume conditions not type1 and surround
		if(audiostream=avivar->AVIstreamhead->next) {
			audioframe=avivar->KFtable[framenum];
			if (audioframe!=-1) {
				avivar->getframeoffset(audioframe,audiostream,&audiobase,&audiooffset,(ULONG *)&pcmsize);
				mySeek64(hfile,audiobase+audiooffset,SEEK_SET);
				myRead(hfile,audiopcm,pcmsize);
				queueaudio(mediaptr,framenum,pcmsize);
				}
			else pcmsize=0;
			}
		//VCM section
		mySeek64(hfile,qwframeoffset+dwOffset,SEEK_SET);
		if (avivar->bmi->biCompression) {
			myRead(hfile,invideobuf,dwSize);
			if (ICDecompress(hic,0,avivar->bmi,invideobuf,outbmi,outvideobuf) == ICERR_OK) {
				if (!(renderframe(videobuf,avivar))) goto redframe;
				}
			else goto redframe;
			} //end if compressed
		else { //uncompressed
			myRead(hfile,outvideobuf,dwSize);
			if (!(renderframe(videobuf,avivar))) goto redframe;
			}
		}
	return(1);
redframe:	{
		unsigned long *videobufindex=(unsigned long *)videobuf;
		long t=691200;
		do {
			*videobufindex++=0x30b9306d; //red
			t-=4;
			} while (t);
		}
	return(0);
	} //end getframeavi


BOOL loadersclass::aviclass::openaudio(class avihandle *avivar) {
	DSBPOSITIONNOTIFY PosNotify[4];
	LPDIRECTSOUNDNOTIFY lpDsNotify;
	LPDIRECTSOUNDBUFFER lpdsb;
	DSBUFFERDESC dsbdesc;
	LPDIRECTSOUND lpDS=audio->player.lpDS;
	DWORD audiobuffersize=avivar->audiobuffersize;

	//Audio Implementation
	if (lpDS) { 
		//since we have an audio stream lets create the secondary buffer
		INIT_DIRECTX_STRUCT(dsbdesc);
		dsbdesc.dwFlags=
			//DSBCAPS_CTRLDEFAULT|		// Have some controls
			//DX7 Has removed this flag must specify individually 
			//how stupid... sigh growing pains
			DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 |
			DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY|
			DSBCAPS_GLOBALFOCUS|		// Allows background playing
			DSBCAPS_STATIC;

		dsbdesc.dwBufferBytes=avivar->totalaudiosize=audiobuffersize<<2;
		//pull format specs
		dsbdesc.lpwfxFormat=&avivar->waveinfo;

		if FAILED(lpDS->CreateSoundBuffer(&dsbdesc,&lpdsb,NULL)) {
			wsprintf(string,"Unable to create DS secondary sound buffer");
			goto erroraudio;
			}
		avivar->lpdsb=lpdsb;
		//set up notify
		if FAILED(lpdsb->QueryInterface(IID_IDirectSoundNotify,(LPVOID *)&lpDsNotify)) {
			wsprintf(string,"Unable to create DS secondary sound notify");
			goto erroraudio;
			}
		avivar->lpDsNotify=lpDsNotify;
		//Now to set positions in half of each sample size

		PosNotify[0].dwOffset = audiobuffersize>>1;
		PosNotify[0].hEventNotify = arrayofevents[EVENT_AVIAUD1];
		PosNotify[1].dwOffset     = audiobuffersize+(audiobuffersize>>1);
		PosNotify[1].hEventNotify = arrayofevents[EVENT_AVIAUD2];
		PosNotify[2].dwOffset = (audiobuffersize<<1)+(audiobuffersize>>1);
		PosNotify[2].hEventNotify = arrayofevents[EVENT_AVIAUD3];
		PosNotify[3].dwOffset     = (audiobuffersize*3)+(audiobuffersize>>1);
		PosNotify[3].hEventNotify = arrayofevents[EVENT_AVIAUD4];

		if (FAILED(lpDsNotify->SetNotificationPositions(4,PosNotify))) {
			wsprintf(string,"Unable to set notify positions");
			goto erroraudio;
			}

		avivar->lpdsb->SetCurrentPosition(0);
		//All Direct Sound stuff complete
		{ //Now to add this media to the list of medias which are actively open
			struct avivarlist *avivarnode=avivar->avivarnode=(struct avivarlist *)newnode(nodeobject,sizeof(struct avivarlist));

			if (avivarlisthead) {
				avivarlisttail->next=avivarnode;
				avivarnode->prev=avivarlisttail;
				avivarnode->next=NULL;
				avivarlisttail=avivarnode;
				}
			else {
				avivarlisthead=avivarlisttail=avivarnode;
				avivarnode->avivar=avivar;
				avivarnode->next=avivarnode->prev=NULL;
				}
			} //end addind avivar
 		} // end if lpDS
	 //end Audio Implementation
	return (0);
erroraudio:
	printc("%s\n",string);
	if (avivar->lpDsNotify) {
		avivar->lpDsNotify->Release();
		avivar->lpDsNotify=NULL;
		}
	if (avivar->lpdsb) {
		avivar->lpdsb->Release();
		avivar->lpdsb=NULL;
		}
	return (1);
	}


BOOL loadersclass::aviclass::openavi(struct imagelist *media,BOOL thumb)  {
	class avihandle *avivar=media->mediaavi;
	BITMAPINFOHEADER *outbmi;
	DWORD oldsize,oldsizeimage;
	WORD oldbitcount;

	//parse all info to open resources to frame read
	if (!avivar->openavi()) goto erroravi;
	if (avivar->audiobuffersize) {
		avivar->audiopcm=(BYTE *)mynew(&pmem,avivar->audiobuffersize);
		if (avivar->surroundsound) avivar->audiopcm2=(BYTE *)mynew(&pmem,avivar->audiobuffersize);
		if (thumb==0) openaudio(avivar);
		} // end if audiobuffersize

	if (((!avivar->surroundsound)&&(!avivar->type1))||(thumb)) {
		DWORD dwFormatSize=avivar->bmisize+(256*4);
		avivar->outbmi=outbmi = (BITMAPINFOHEADER *)newnode(nodeobject,dwFormatSize+(256*4));
		memcpy(outbmi,avivar->bmi,dwFormatSize);
		if (avivar->bmi->biCompression) {
			if (!avivar->vcmhandler) avivar->vcmhandler=avivar->bmi->biCompression;
			if (!(avivar->hic=ICOpen(ID_VIDC,avivar->vcmhandler,ICMODE_DECOMPRESS)))
				//it is possible for DIVX to run with div4
				if (avivar->vcmhandler==ID_div4) {
					avivar->vcmhandler=ID_DIVX;
					if (!(avivar->hic=ICOpen(ID_VIDC,avivar->vcmhandler,ICMODE_DECOMPRESS)))
						goto erroravi; //No hope at this point
					}
			ICDecompressGetFormat(avivar->hic, avivar->bmi, outbmi);

			if (thumb) goto YUVpassed;
			//this statement here will be used to test for YUV
			oldsize=outbmi->biSize;
			oldbitcount=outbmi->biBitCount;
			oldsizeimage=outbmi->biSizeImage;
			outbmi->biSize=40;
			outbmi->biBitCount=16;
			outbmi->biSizeImage = ((outbmi->biWidth*outbmi->biBitCount+31)&~0x1f)*outbmi->biPlanes*outbmi->biHeight/8;

			outbmi->biCompression=BI_UYVY;
			if (!(avivar->vcmhandler==ID_cvid)) avivar->upsidedown=FALSE;
			if (ICDecompressQuery(avivar->hic,avivar->bmi,outbmi)== ICERR_OK) goto YUVpassed;
			outbmi->biCompression=0;
			//restore back to original settinggs test for native format
			outbmi->biSize=oldsize;
			outbmi->biBitCount=oldbitcount;
			outbmi->biSizeImage=oldsizeimage;
			avivar->upsidedown=TRUE;

YUVpassed:
			if (ICDecompressBegin(avivar->hic, avivar->bmi, outbmi) != ICERR_OK) {
				wsprintf(string,"compression format not supported");
				goto erroravi;	
				}
			avivar->invideobuf=(char *)mynew(&pmem,avivar->videobuffersize);
			avivar->outvideobuf=(char *)mynew(&pmem,outbmi->biSizeImage);
			} //end if compression
		else avivar->outvideobuf=(char *)mynew(&pmem,max(avivar->bmi->biSizeImage,(DWORD)(avivar->bmi->biWidth*avivar->bmi->biHeight*(avivar->bmi->biBitCount>>3))));
		} //end if type1 or surround
	else {
		avivar->invideobuf=(char *)mynew(&pmem,DV_FR_MAX_SIZE);
		avivar->outvideobuf=(char *)mynew(&pmem,691200);
		}
	return (0);
erroravi:
	printc("%s\n",string);
	return(1);
	} //end open AVI


void loadersclass::aviclass::closeavi(struct imagelist *media,BOOL thumb) {
	class avihandle *avivar=media->mediaavi;
	BITMAPINFOHEADER *outbmi=avivar->outbmi;
	HIC hic=avivar->hic;
	char *invideobuf=avivar->invideobuf;
	char *outvideobuf=avivar->outvideobuf;
	BYTE *audiopcm=avivar->audiopcm;
	BYTE *audiopcm2=avivar->audiopcm2;

	if (invideobuf) {
		dispose((struct memlist *)invideobuf,&pmem);
		invideobuf=NULL;
		}
	if (outvideobuf) {
		dispose((struct memlist *)outvideobuf,&pmem);
		outvideobuf=NULL;
		}
	if (outbmi) {
		disposenode(nodeobject,(struct memlist *)outbmi);
		outbmi=NULL;
		}
	if (hic) {
		ICDecompressEnd(hic);
		ICClose(hic);
		hic=NULL;
		}
	if (audiopcm) {
		dispose((struct memlist *)audiopcm,&pmem);
		audiopcm=NULL;
		}
	if (audiopcm2) {
		dispose((struct memlist *)audiopcm2,&pmem);
		audiopcm2=NULL;
		}
	if (avivar->lpDsNotify) {
		avivar->lpDsNotify->Release();
		avivar->lpDsNotify=NULL;
		}
	if (avivar->lpdsb) {
		avivar->lpdsb->Stop();
		avivar->lpdsb->Release();
		avivar->lpdsb=NULL;
		}
	//Make sure pcm queue is clear
	if (avivar->pcmlisthead) {
		struct pcmlist *pcmnext,*pcmindex=avivar->pcmlisthead;
		do {
			pcmnext=pcmindex->next;
			if (pcmindex->audiopcm) dispose((struct memlist *)pcmindex->audiopcm,&pmem);
			disposenode(nodeobject,(struct memlist *)pcmindex);
			} while (pcmindex=pcmnext);
		avivar->pcmlisthead=avivar->pcmlisttail=NULL;
		}
	//Remove our media from the avivarlist
	if (avivar->avivarnode) {
		struct avivarlist *avivarnode=avivar->avivarnode;

		if (avivarnode->prev) avivarnode->prev->next=avivarnode->next;
		else avivarlisthead=NULL;
		if (avivarnode->next) avivarnode->next->prev=avivarnode->prev;
		else avivarlisttail=avivarlisthead;
		disposenode(nodeobject,(struct memlist *)avivarnode);
		avivar->avivarnode=NULL;
		}

	media->mediaavi->closeavi();
	}


BOOL loadersclass::aviclass::beginavi(struct imagelist *media)  {
	int x,y,scalex,scaley;
	class avihandle *avivar;
	avivar=media->mediaavi=new avihandle(media->filesource);
	BITMAPINFOHEADER *bmi=avivar->bmi;
	if (avivar->error) goto erroravi;

	x=bmi->biWidth;
	y=bmi->biHeight;
	scalex=1;
	if (x<=360) {
		scalex=2;
		if (x<=180) scalex=4;
		}
	scaley=1;
	if (y<=240) {
		scaley=2;
		if (y<=120) scaley=4;
		}
	avivar->scalex=min(scalex,scaley);
	avivar->scaley=avivar->scalex;
	avivar->x=x;avivar->y=y;
	avivar->upsidedown=TRUE;

	return (0);
erroravi:
	return(1);
	}


void loadersclass::aviclass::endavi(struct imagelist *media)  {
	if (media->mediaavi) delete media->mediaavi;
	}
