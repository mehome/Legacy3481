#include "HandleMain.h"

#define BI_UYVY         mmioFOURCC('U','Y','V','Y')
#define BI_VYUY         mmioFOURCC('V','Y','U','Y') //ATI-Alias for YUYV
#define BI_YUY2         mmioFOURCC('Y','U','Y','2') //MS-Alias for  YUYV
#define BI_YUV2         mmioFOURCC('Y','U','V','2') //MXB-Alias for UYVY

typedef loadersclass::aviclass aviloader;
const long RGB1[2]={0xb5817062,0x0000da1d};
const long RGB2[2]={0x7fff1917,0x000041cb};
const long RGB3[2]={0xa1bcedd3,0x00007062};
const long addoffsets[2]={0x00100080,0x00100080};
const double baseframerate=0.033366700033366;

loadersclass::aviclass::aviclass() {
	/*
	HDC hdc;

	hdc=GetDC(screen);
	avicontainer=CreateCompatibleBitmap(hdc,720,VIDEOY);
	ReleaseDC(screen,hdc);
	*/
	}

loadersclass::aviclass::~aviclass() {
	/*
	if (avicontainer) {
		DeleteObject(avicontainer);
		}
	*/
	}

UWORD loadersclass::aviclass::gettotalavi(struct imagelist *media) {
	struct avihandle *avivar=media->mediaavi;
	UWORD actualframes=(UWORD)AVIStreamLength(avivar->videostream);
	UWORD virtualframes=(UWORD)(actualframes/avivar->framerate);
	//wsprintf(string,"newsize=%d",virtualframes);
	//printc(string);
	return (virtualframes);
	}


HBITMAP loadersclass::aviclass::getthumbavi(struct imagelist *mediaptr,ULONG framenum,LPBITMAPINFOHEADER *bitmapinfo) {
	/*
	HDC hdc,hmem;
	LPBITMAPINFOHEADER lpBitmapInfoHeader = NULL;
 
	// Draw video frame DIB
	lpBitmapInfoHeader=(LPBITMAPINFOHEADER)AVIStreamGetFrame(mediaptr->mediaavi->videoframe,(LONG)framenum);

	hdc=GetDC(screen);
	hmem=CreateCompatibleDC(hdc);
	SelectObject(hmem,avicontainer);

   DrawDibDraw(drawdib,hmem,0,0,720,VIDEOY,
		         lpBitmapInfoHeader,NULL,
		         0,0,-1,-1,0);

	DeleteDC(hmem);
	ReleaseDC(screen,hdc);

	return (avicontainer);
	*/
	void *buffer;
	LPBITMAPINFOHEADER source;
	HDC hdcimage;
	HBITMAP hbm=NULL;
	char *decodedbits;

	hdcimage=CreateCompatibleDC(NULL);
	source=(LPBITMAPINFOHEADER)AVIStreamGetFrame(mediaptr->mediaavi->videoframe,(LONG)(framenum*mediaptr->mediaavi->framerate));
	if (source) {
		source->biSizeImage = ((source->biWidth*source->biBitCount+31)&~0x1f)*source->biPlanes*source->biHeight/8;
		*bitmapinfo=(LPBITMAPINFOHEADER)newnode(nodeobject,source->biSize+1024);
		memcpy(*bitmapinfo,source,source->biSize+1024);
		(*bitmapinfo)->biCompression=0;
		hbm=CreateDIBSection(hdcimage,(LPBITMAPINFO)*bitmapinfo,DIB_RGB_COLORS,&buffer,NULL,NULL);
		if (hbm) {
			decodedbits = (char *)(((DWORD)source)+source->biSize+(source->biClrUsed<<2));
			memcpy(buffer,decodedbits,source->biSizeImage);
			}
		}
	if (hdcimage) DeleteDC(hdcimage);
	return (hbm);
	}


BOOL loadersclass::aviclass::getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	struct avihandle *avivar=mediaptr->mediaavi;
	LPBITMAPINFOHEADER lpBitmapInfoHeader = NULL;
	char *decodedbits;
	//ULONG bytesline;
	long currentframe=(LONG)((framenum-1)*avivar->framerate);
	int x,y,scalex,scaley;
	//int test;

	//Stream audio part
/**/
	if (avivar->lpdsb) {
		if ((!(currentframe==avivar->lastframe))&&(controls->mycontrolis&2)) {
			//Obtain hunk size for this sample to figure how much we are streaming
			LONG audiobeg= AVIStreamSampleToSample(avivar->audiostream, avivar->videostream,currentframe);
			//LONG audioend= AVIStreamSampleToSample(avivar->audiostream, avivar->videostream,currentframe+1);
			if (framenum==(ULONG)mediaptr->cropin+1) {
				avivar->nextaudiobeg=audiobeg;
				}	

			//wsprintf(string,"%ld",currentframe);
			//printc(string);

			if (audiobeg>=avivar->nextaudiobeg) {
				void *dsbuf1,*dsbuf2;
				DWORD dsbuflen1,dsbuflen2;
				LPDIRECTSOUNDBUFFER lpdsb=avivar->lpdsb;
				LONG samplesize;

				AVIStreamRead(avivar->audiostream,avivar->nextaudiobeg,AVISTREAMREAD_CONVENIENT,NULL,NULL,&samplesize,NULL);
				//Set up the first part of stream before play
				if (samplesize) {
			
					if FAILED(lpdsb->Lock(avivar->lastoffset,samplesize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)) {
						printc("Unable to lock DS buffer");
						return (TRUE);
						}

					AVIStreamRead(avivar->audiostream,avivar->nextaudiobeg,AVISTREAMREAD_CONVENIENT,dsbuf1,dsbuflen1,NULL,NULL);
					if (dsbuflen2==0) avivar->lastoffset+=dsbuflen1;
					else {
						avivar->lastoffset=dsbuflen2;
						//printc("we have second avi buffer lock");
						AVIStreamRead(avivar->audiostream,avivar->nextaudiobeg+dsbuflen1,AVISTREAMREAD_CONVENIENT,dsbuf2,dsbuflen2,NULL,NULL);
						}
					if (avivar->lastoffset>=avivar->audiobuffersize) avivar->lastoffset=0;
					//unlock immediately
					if FAILED(lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2)) {
						printc("Unable to unlock DS buffer");
						return (TRUE);
						}
					}
				//all our postincrement advances here
				//avivar->nextaudiobeghalf=avivar->nextaudiobeg;
				//avivar->nextaudiobeghalf+=samplesize>>1;
				avivar->nextaudiobeg+=samplesize;
				} //end filling buffer w/sound on a new frame
			//lets see if this was the first frame if so then play
			} //end if we are at the right point to add a new frame
		//wsprintf(string,"fb=%d",framenum+controls->frames_behind);printc(string);
		if (framenum==(ULONG)mediaptr->cropin+1) {
			avivar->lpdsb->SetCurrentPosition(0);
			avivar->lpdsb->Play(0,0,DSBPLAY_LOOPING);
			}
		if (framenum>=(ULONG)mediaptr->cropout-1) {
			avivar->lpdsb->Play(0,0,0);
			avivar->nextaudiobeg=0;
			avivar->lastoffset=0;
			}
		} // end if we have sound
/**/

	// Draw video frame DIB
	if ((lpBitmapInfoHeader=(LPBITMAPINFOHEADER)AVIStreamGetFrame(mediaptr->mediaavi->videoframe,currentframe))==0) {
		wsprintf(string,"Unable to read frame %d",framenum);printc(string);
		return(FALSE);
		}
	decodedbits = (char *)(((DWORD)lpBitmapInfoHeader)+lpBitmapInfoHeader->biSize);

	x=lpBitmapInfoHeader->biWidth;y=lpBitmapInfoHeader->biHeight;
	//bytesline = ((x*lpBitmapInfoHeader->biBitCount+31)&~0x1f)*lpBitmapInfoHeader->biPlanes/8;
	scalex=avivar->scalex;scaley=avivar->scaley;
	x=avivar->x;y=avivar->y;

/**/
	switch (lpBitmapInfoHeader->biBitCount) {
		case 8:
			if (lpBitmapInfoHeader->biClrUsed==0) decodedbits+=1024;
			else decodedbits+=lpBitmapInfoHeader->biClrUsed<<2;
			if (avivar->scalex>1) renderrgb8scale(videobuf,x,y,scalex,scaley,decodedbits,lpBitmapInfoHeader);
			else renderrgb8(videobuf,x,y,decodedbits,lpBitmapInfoHeader,(BOOL)avivar->upsidedown);
			break;
		case 16:
			//TODO check compression for reverse value for YUV
			if (lpBitmapInfoHeader->biCompression) {
				if (avivar->scalex>1) renderyuvscale(videobuf,x,y,scalex,scaley,decodedbits,(BOOL)avivar->upsidedown);
				else renderyuv(videobuf,x,y,decodedbits,(BOOL)avivar->upsidedown);
				}
			else {
				printc("16 bit format not yet implemented");
				return(FALSE);
				}
			break;
		case 24:
			//printc("24");
			if (avivar->scalex>1) renderrgb24scale(videobuf,x,y,scalex,scaley,decodedbits);
			else renderrgb24(videobuf,x,y,decodedbits,(BOOL)avivar->upsidedown);
			break;
		case 32:
			//printc("32");
			if (avivar->scalex>1) renderrgb32scale(videobuf,x,y,scalex,scaley,decodedbits,(BOOL)avivar->upsidedown);
			else renderrgb32(videobuf,x,y,decodedbits,(BOOL)avivar->upsidedown);
			break;
		default:
			printc("Unable to render uncompressed format");
			return(FALSE);
		}
/**/
/*
	long t=691200;
	do {
		*videobuf++=0x30b9306d; //red
		t-=4;
		} while (t);
*/

/*
	{
	HDC hdc,hmem;static char dest[1382400];

	ULONG *YUVdest=videobuf;
	long size=0;
	int xindex,yindex;
	int field;
	ULONG *pixel;	

	hdc=GetDC(screen);
	hmem=CreateCompatibleDC(hdc);
	SelectObject(hmem,avicontainer);
	DrawDibDraw(drawdib,hmem,0,0,720,VIDEOY,lpBitmapInfoHeader,NULL,0,0,-1,-1,0);
	GetBitmapBits(avicontainer,1382400,dest);

	DeleteDC(hmem);
	ReleaseDC(screen,hdc);

	//Scale the image into 720x480 using crop
	//Now we have 720x480x32 RGB now we convert to YUV and separate fields
	//Intentionally close these vars in a local scope to pull registers
	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
	for (yindex=field;yindex<480;yindex+=2) {
		for (xindex=0;xindex<720;xindex+=2) {

			pixel=((ULONG *)(dest+(xindex<<2)+2880*yindex));

			__asm {
				mov			eax,pixel
				movd			mm0,[eax]
				pxor			mm1,mm1			//mm1 = ________________
				pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
				add			eax,4
				psrlq			mm3,16			//mm3 = ____ffffffffffff
				punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
				movd			mm2,[eax]
				pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
				movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
				punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
				pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
				movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
				//28770*B-19071*G-9699*R
				pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
				movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
				psllq			mm6,32			//mm6 = ____GBp1________
				psrlq			mm3,16			//mm3 = ________ffffffff
				//16843*R+33030*G+6423*B
				pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
				movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
				psrlq			mm7,32			//mm7 = ____________Rp1b
				pand			mm4,mm3			//mm4 = ___________GBp1b
				por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
				psllq			mm3,32			//mm3 = ffffffff________
				pand			mm0,mm3			//mm0 = ____Rp1a________
				por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
				paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
				psrad			mm0,16			//mm0 = >>16
				//Regs free mm1,mm3,mm4,mm6,mm7
				//28770*R1-24117*G1-4653*B1
				pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
				movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
				psllq			mm6,32			//mm6 = ___GBp2c________
				psrlq			mm3,32			//mm3 = ________ffffffff
				//16843*R1+33030*G1+6423*B1
				pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
				movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
				psrlq			mm7,32			//mm7 = ____________Rp2d
				pand			mm5,mm3			//mm5 = ___________GBp2d
				por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
				psllq			mm3,32			//mm3 = ffffffff________
				pand			mm2,mm3			//mm2 = ____Rp2c________
				por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
				paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
				psrad			mm2,16			//mm2 = >>16
				//Regs free all but mm0 and mm2
				//pack it up oops turn them around
				movq			mm4,mm0
				psrlq			mm0,32
				psllq			mm4,32
				por			mm0,mm4			//mm0 = ____RGB2____RGB1 
				movq			mm3,mm2
				psrlq			mm2,32
				psllq			mm3,32
				por			mm2,mm3			//mm2 = ____RGB4____RGB3
				packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
				//add the final offsets
				paddw			mm0,addoffsets
				packuswb		mm0,mm1			//mm0 = UYVY
				movd			[edi],mm0
				add			edi,4
				emms
				}
			} //end YUV conversion scope
		} //end Y
		} //end field
	}
*/
	avivar->lastframe=currentframe;
	return(TRUE);
	} //end getframeavi


BOOL loadersclass::aviclass::openavi(struct imagelist *media,BOOL thumb)  {
	struct avihandle *avivar;
	LONG i;
	DWORD sourcecompression;
	BITMAPINFOHEADER *bmi,*bmif;
	AVIFILEINFO	pFileInfo;
	PAVISTREAM pAviStream;
	AVISTREAMINFO StreamInfo;
	HRESULT rc;

	//Unlike some of the other filters using the VFW api we need more
	//than 4 bytes to open an avi so we create a structure of all the
	//vars necessary and alloc mem and our final pointer will point
	//to this structure... this way all filters will always only need
	//just 4 bytes to return to the imagelist structure which will
	//union all the different types of pointer returns per filter.
	if (thumb) {
		media->mediaavi=avivar=(struct avihandle *)newnode(nodeobject,sizeof(struct avihandle));
		memset(avivar,0,sizeof(struct avihandle));
		avivar->filesource=media->filesource;
		}
	else avivar=media->mediaavi;

	if (debug) printc("AVI");
	rc=SendMessage(screen,WM_APP,100,(long)avivar);
	if (avivar->avifile==0) rc=AVIFileOpen(&(avivar->avifile),media->filesource,OF_READ,NULL);
	if (rc) {
		printc("Warning: unable to open AVI");
		print("Reason: ");
		switch (rc) {
			case AVIERR_BADFORMAT:
				printc("File is not recognised or corrupt");
				break;
			case AVIERR_FILEOPEN:
				printc("A disk error occurred while opening the file");
				break;
			case AVIERR_FILEREAD:
				printc("A disk error occurred while reading the file");
				break;
			case AVIERR_MEMORY:
				printc("Not enough memory available");
				break;
			case REGDB_E_CLASSNOTREG:
				printc("The handler can not be found in the registry");
				break;
			default:
				printc("Unknown");
			}
		return (TRUE);
		}
	// get input AVI file info
	pFileInfo.dwStreams = 0L;
	rc=AVIFileInfo(avivar->avifile,&pFileInfo,sizeof(AVIFILEINFO));
	// find video stream
	for (i=0;i<(LONG)pFileInfo.dwStreams;i++) { 
		rc=AVIFileGetStream(avivar->avifile,&pAviStream,0L,i);
		if (rc) break;
      rc=AVIStreamInfo(pAviStream,&StreamInfo,sizeof(AVISTREAMINFO)); 
      if (rc) continue;
			if (StreamInfo.fccType==streamtypeVIDEO) {
			avivar->videostream = pAviStream;
			if (thumb) {
				if ((StreamInfo.dwScale==100)&&(StreamInfo.dwRate==2997)) avivar->framerate=1;
				else {
					if (!(StreamInfo.dwScale==0.0))	avivar->framerate=StreamInfo.dwRate*baseframerate/StreamInfo.dwScale;
					else avivar->framerate=1;
					}
				}
			//rc=AVIStreamAddRef(pAviStream);
			}
		else if (StreamInfo.fccType==streamtypeAUDIO) {
			LPDIRECTSOUNDBUFFER lpdsb;
			DSBUFFERDESC dsbdesc;
			LPDIRECTSOUND lpDS=audio->player.lpDS;

			avivar->audiostream = pAviStream;
			//rc=AVIStreamAddRef(pAviStream);

			if ((lpDS)&&(thumb==0)) {

				//since we have an audio stream lets create the secondary buffer
				INIT_DIRECTX_STRUCT(dsbdesc);
				dsbdesc.dwFlags=
					//DSBCAPS_CTRLDEFAULT|		// Have some controls
					//DX7 Has removed this flag must specify individually 
					//how stupid... sigh growing pains
					DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY|
					DSBCAPS_GLOBALFOCUS|		// Allows background playing
					DSBCAPS_STATIC;

				//TODO need to find a method to obtain the largest chunksize 
				//if dwSuggestedBufferSize is NULL
				if ((dsbdesc.dwBufferBytes=avivar->audiobuffersize=StreamInfo.dwSuggestedBufferSize<<2)==0) {
					printc("default buffersize not yet supported; Audio Fail");
					goto nosuggestedbufsize;
					}
				avivar->totalaudiosize=StreamInfo.dwLength;
				//pull format specs
				dsbdesc.lpwfxFormat=avivar->waveinfo;


				if FAILED(lpDS->CreateSoundBuffer(&dsbdesc,&lpdsb,NULL)) {
					printc("Unable to create DS secondary sound buffer");
					return (TRUE);
					}
				avivar->lpdsb=lpdsb;

				} // end if lpDS
nosuggestedbufsize:;

			} // end if audio stream
		} //end for search of video stream
/**/
		{
		LONG cnt = sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD);
		bmi = (BITMAPINFOHEADER *)newnode(nodeobject,cnt);
		rc = AVIStreamReadFormat(avivar->videostream,0,(void *)bmi,&cnt);
		if (rc) {
			wsprintf(string,"AVIStreamReadFormat vids: 0x%08lx\n",rc);
			printc(string);
			return(TRUE);
			}
		if (!thumb) bmi->biCompression = avivar->biCompression;
		// recalculate the image size
		bmi->biSize=40;
		bmi->biBitCount=16;
		bmi->biSizeImage = ((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes*bmi->biHeight/8;
		//bytesline = ((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes/8;
			//Check for scaleing
		sourcecompression=bmi->biCompression;
		//end scaleconfiger
		}
/**/
	if ((thumb)||(avivar->biCompression==0)) bmif=0;
	else bmif=bmi;
	if (avivar->videostream) {
		if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,bmif))) {
			printc("Warning:Unrecognized AVI Codec");
			return(TRUE);
			}
		}
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	avivar->lastframe=-1;
	return(0);
	} //end open AVI


void loadersclass::aviclass::closeavi(struct imagelist *media,BOOL thumb) {
	HRESULT rc;
	struct avihandle *avivar=media->mediaavi;
	if (debug) printc("AVI");

	// release resources allocated during video decompression
	if (avivar->videoframe) {
		rc=AVIStreamGetFrameClose(avivar->videoframe);
		avivar->videoframe=NULL;
		}
	// release AVI streams
	if (avivar->videostream) {
		rc=AVIStreamRelease(avivar->videostream);
		avivar->videostream=NULL;
		}

	if (avivar->audiostream) {
		rc=AVIStreamRelease(avivar->audiostream);
		avivar->audiostream=NULL;
		avivar->nextaudiobeg=0;
		avivar->lastoffset=0;
		if (avivar->lpdsb) {
			avivar->lpdsb->Stop();
			avivar->lpdsb->Release();
			avivar->lpdsb=NULL;
			}
		}
	// close AVI file
	if (avivar->avifile) rc=AVIFileRelease(avivar->avifile);
	avivar->avifile=NULL;
	if (thumb) {
		//Finally dispose our avi Node
		disposenode(nodeobject,(struct memlist *)avivar);
		media->mediaavi=NULL;
		}
	}


BOOL loadersclass::aviclass::beginavi(struct imagelist *media)  {
	struct avihandle *avivar;
	LONG i,sizewfmt;
	DWORD sourcecompression;
	BITMAPINFOHEADER *bmi;
	AVIFILEINFO	pFileInfo;
	PAVISTREAM pAviStream;
	AVISTREAMINFO StreamInfo;
	HRESULT rc;
	BOOL isyuv;

	//Unlike some of the other filters using the VFW api we need more
	//than 4 bytes to open an avi so we create a structure of all the
	//vars necessary and alloc mem and our final pointer will point
	//to this structure... this way all filters will always only need
	//just 4 bytes to return to the imagelist structure which will
	//union all the different types of pointer returns per filter.
	media->mediaavi=avivar=(struct avihandle *)newnode(nodeobject,sizeof(struct avihandle));
	memset(avivar,0,sizeof(struct avihandle));

	if (debug) printc("AVI");
	avivar->filesource=media->filesource;
	rc=SendMessage(screen,WM_APP,100,(long)avivar);
	if (avivar->avifile==0) rc=AVIFileOpen(&(avivar->avifile),media->filesource,OF_READ,NULL);
	if (rc) {
		printc("Warning: unable to open AVI");
		print("Reason: ");
		switch (rc) {
			case AVIERR_BADFORMAT:
				printc("File is not recognised or corrupt");
				break;
			case AVIERR_FILEOPEN:
				printc("A disk error occurred while opening the file");
				break;
			case AVIERR_FILEREAD:
				printc("A disk error occurred while reading the file");
				break;
			case AVIERR_MEMORY:
				printc("Not enough memory available");
				break;
			case REGDB_E_CLASSNOTREG:
				printc("The handler can not be found in the registry");
				break;
			default:
				printc("Unknown");
			}
		return (TRUE);
		}
	// get input AVI file info
	pFileInfo.dwStreams = 0L;
	rc=AVIFileInfo(avivar->avifile,&pFileInfo,sizeof(AVIFILEINFO));
	// find video stream
	for (i=0;i<(LONG)pFileInfo.dwStreams;i++) { 
		rc=AVIFileGetStream(avivar->avifile,&pAviStream,0L,i);
		if (rc) break;
      rc=AVIStreamInfo(pAviStream,&StreamInfo,sizeof(AVISTREAMINFO)); 
      if (rc) continue;
			if (StreamInfo.fccType==streamtypeVIDEO) {
			avivar->videostream = pAviStream;
			if ((StreamInfo.dwScale==100)&&(StreamInfo.dwRate==2997)) avivar->framerate=1;
			else {
				if (!(StreamInfo.dwScale==0.0))	avivar->framerate=StreamInfo.dwRate*baseframerate/StreamInfo.dwScale;
				else avivar->framerate=1;
				}
			//rc=AVIStreamAddRef(pAviStream);
			}
		else if (StreamInfo.fccType==streamtypeAUDIO) {
			avivar->audiostream = pAviStream;
			//rc=AVIStreamAddRef(pAviStream);
			//Grab Audio waveformat info
			AVIStreamReadFormat(pAviStream,0,NULL,&sizewfmt);
			avivar->waveinfo=(WAVEFORMATEX *)newnode(nodeobject,sizewfmt);
			AVIStreamReadFormat(pAviStream,0,avivar->waveinfo,&sizewfmt);
			}
		} //end for search of video stream

		{
		int x,y,scalex,scaley;
		LONG cnt = sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD);
		bmi = (BITMAPINFOHEADER *)newnode(nodeobject,cnt);
		rc = AVIStreamReadFormat(avivar->videostream,0,(void *)bmi,&cnt);
		if (rc) {
			wsprintf(string,"AVIStreamReadFormat vids: 0x%08lx\n",rc);
			printc(string);
			return(TRUE);
			}
		//bmi->biCompression = 0; // we want it in raw form, uncompressed
		// recalculate the image size
		bmi->biSize=40;
		bmi->biBitCount=16;
		bmi->biSizeImage = ((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes*bmi->biHeight/8;
		//bytesline = ((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes/8;
			//Check for scaleing
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
		isyuv=TRUE;
		sourcecompression=bmi->biCompression;
		//end scaleconfiger
		}

	if (avivar->videostream) {
		bmi->biCompression=BI_UYVY;
		if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,bmi))) {
			bmi->biCompression=BI_YUV2;
			if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,bmi))) {
				bmi->biCompression=BI_VYUY;
				if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,bmi))) {
					bmi->biCompression=BI_YUY2;
					if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,bmi))) {
						isyuv=FALSE;
						//if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,(LPBITMAPINFOHEADER)AVIGETFRAMEF_BESTDISPLAYFMT))) {
						if (!(avivar->videoframe=AVIStreamGetFrameOpen(avivar->videostream,NULL))) {
							printc("Warning:Unrecognized AVI Codec");
							return(TRUE);
							}
						}
					}
				}
			}
		}
	if (isyuv) {
		if (sourcecompression==0x47504a4d) avivar->upsidedown=0;
		}
	else bmi->biCompression=0;

	avivar->biCompression=bmi->biCompression;
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	//now to close the avi

	// release resources allocated during video decompression
	if (avivar->videoframe) {
		rc=AVIStreamGetFrameClose(avivar->videoframe);
		avivar->videoframe=NULL;
		}
	// release AVI streams
	if (avivar->videostream) {
		rc=AVIStreamRelease(avivar->videostream);
		avivar->videostream=NULL;
		}
	if (avivar->audiostream) {
		rc=AVIStreamRelease(avivar->audiostream);
		avivar->audiostream=NULL;
		}
	// close AVI file
	if (avivar->avifile) rc=AVIFileRelease(avivar->avifile);
	avivar->avifile=NULL;
	return (0);
	}


void loadersclass::aviclass::endavi(struct imagelist *media)  {
	struct avihandle *avivar=media->mediaavi;
	if (avivar->waveinfo) {
		disposenode(nodeobject,(struct memlist *)avivar->waveinfo);
		avivar->waveinfo=NULL;
		}
	//Finally dispose our avi Node
	disposenode(nodeobject,(struct memlist *)avivar);
	media->mediaavi=NULL;
	}
