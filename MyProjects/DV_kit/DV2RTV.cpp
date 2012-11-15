#include "handlemain.h"
#include "AVIparser.h"
#include "madventry.h"
#include "newtek/RTVlib.h"
#include "storyboard/RGBvideorender.h"
#include "progress.h"
#include "DVstructs.h"
#pragma comment(lib, "DVSplitter")
#pragma comment(lib, "mcdvd_32")
#pragma comment(lib, "RTVlib")
#pragma comment(lib, "vfw32")

#define BI_UYVY         mmioFOURCC('U','Y','V','Y')
#define BI_VYUY         mmioFOURCC('V','Y','U','Y') //ATI-Alias for YUYV
#define BI_YUY2         mmioFOURCC('Y','U','Y','2') //MS-Alias for  YUYV
#define BI_YUV2         mmioFOURCC('Y','U','V','2') //MXB-Alias for UYVY

dv2rtvclass::dv2rtvclass() {
/*
	char *inifile=0;
	audiopath[0]=0;
	videopath[0]=0;
	//For now all default paths are done here
	inifile=
*/
	if (audiopath[0]==0) strcpy(audiopath,"C:\\VideoMedia\\");
	if (videopath[0]==0) strcpy(videopath,"V:\\VideoMedia\\");
	strcpy(clipname,"Default");
	playing=0;
	sourceavi[0]=0;
	dragtoggle=FALSE;
	audioon=videoon=TRUE;
	audiopcm=audiopcm2=NULL;
	hic=NULL;
	outbmi=NULL;
	invideobuf=outvideobuf=NULL;
	}


dv2rtvclass::~dv2rtvclass() {
	if (smartrefreshclip) DeleteObject(smartrefreshclip);
	delete progressobj;
	}


BOOL dv2rtvclass::openavi (struct imagelist *media) {
	class avihandle *avivar=media->mediaavi;
	DWORD oldsize,oldsizeimage;
	WORD oldbitcount;

	//parse all info to open resources to frame read
	if (!avivar->openavi()) goto erroravi;
	if (avivar->audiobuffersize) {
		audiopcm=(BYTE *)mynew(&pmem,avivar->audiobuffersize);
		if (avivar->surroundsound) audiopcm2=(BYTE *)mynew(&pmem,avivar->audiobuffersize);
		}
	if ((!avivar->surroundsound)&&(!avivar->type1)) {
		DWORD dwFormatSize=avivar->bmisize;
		outbmi = (BITMAPINFOHEADER *)newnode(nodeobject,dwFormatSize+(256*4));
		memcpy(outbmi,avivar->bmi,dwFormatSize);
		if (avivar->bmi->biCompression) {
			if (!avivar->vcmhandler) avivar->vcmhandler=avivar->bmi->biCompression;
			hic=ICOpen(ID_VIDC,avivar->vcmhandler,ICMODE_DECOMPRESS);
			ICDecompressGetFormat(hic, avivar->bmi, outbmi);

			//this statement here will be used to test for YUV
			oldsize=outbmi->biSize;
			oldbitcount=outbmi->biBitCount;
			oldsizeimage=outbmi->biSizeImage;
			outbmi->biSize=40;
			outbmi->biBitCount=16;
			outbmi->biSizeImage = ((outbmi->biWidth*outbmi->biBitCount+31)&~0x1f)*outbmi->biPlanes*outbmi->biHeight/8;

			outbmi->biCompression=BI_UYVY;
			avivar->upsidedown=FALSE;
			if (ICDecompressQuery(hic,avivar->bmi,outbmi)== ICERR_OK) goto YUVpassed;
			outbmi->biCompression=0;
			//restore back to original settinggs test for native format
			outbmi->biSize=oldsize;
			outbmi->biBitCount=oldbitcount;
			outbmi->biSizeImage=oldsizeimage;
			avivar->upsidedown=TRUE;

YUVpassed:
			if (ICDecompressBegin(hic, avivar->bmi, outbmi) != ICERR_OK) {
				wsprintf(string,"compression format not supported");
				goto erroravi;	
				}
			invideobuf=(char *)mynew(&pmem,avivar->videobuffersize);
			outvideobuf=(char *)mynew(&pmem,outbmi->biSizeImage);
			} //end if compression
		else outvideobuf=(char *)mynew(&pmem,max(avivar->bmi->biSizeImage,(DWORD)(avivar->bmi->biWidth*avivar->bmi->biHeight*(avivar->bmi->biBitCount>>3))));
		} //end if type1 or surround
	return (0);
erroravi:
	printf("%s\n",string);
	return(1);
	}


BOOL dv2rtvclass::renderframe (ULONG *videobuf,class avihandle *avivar) {
	int x,y,scalex,scaley;
			
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
			printerror("Unable to render uncompressed format");
			return(FALSE);
		}
	return (TRUE);
	}


BOOL dv2rtvclass::getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum) {
	AVIstream *audiostream;
	class avihandle *avivar=mediaptr->mediaavi;
	DWORDLONG qwframeoffset,audiobase;
	DWORD dwOffset,audiooffset;		 // 32 bit offset to data (points to data, not riff header)
	DWORD dwSize;			// 31 bit size of data (does not include size of riff header) (high bit is deltaframe bit)
	avivar->getframeoffset(framenum,avivar->AVIstreamhead,&qwframeoffset,&dwOffset,&dwSize);
	//Step 2 Decode Frame and Audio
	if (avivar->surroundsound||avivar->type1) {
		//Everything in this scope is exclusive for Type 1 DV decoding
		//Ok to interface with DV I'll need the entire chunk in memory may need to move this part
		static BYTE framebuf[DV_FR_MAX_SIZE];
		static char tempvideobuf[691200];
		//int y,line=720*2;

		mySeek64(avivar->hfile,qwframeoffset+dwOffset,SEEK_SET);
		myRead(avivar->hfile,framebuf,dwSize);

		DecompressBuffer_DV (
			framebuf,  // source buffer (dv frame)
			dwSize,  // source buffer length
			(PBYTE)tempvideobuf,  // dest buffer (uncompressed frame)
			(LONG)720*2,  // byte size of dest buffer line this may differ from actual pixel size of line
			720,  // pixel width for dest buffer (may differ from DV)
			480,  // pixel height for dest buffer (may differ from DV)
			0,  // currently not used on DV (0)
			FOURCC_UYVY,  // format for decoded frame FOURCC_BGR3,FOURCC_UYVY,...
			0,  // currently not used on DV (0)
			NULL  // currently not used on DV (NULL)
			);
		//need to de-interleave the lines
		renderyuv(videobuf,720,479,tempvideobuf+(720*2),0);
		//Now to interface w/ mikes DV splitter
		//save("dvframe.dv",dwSize,(char *)framebuf);

		if (avivar->surroundsound) {
			pcmsize=decodeAudio(framebuf,audiopcm,1);
			pcmsize=decodeAudio(framebuf,audiopcm2,2);
			}
		else pcmsize=decodeAudio(framebuf,audiopcm,0);
		//Ok this value of pcm size is in samples for now we'll assume we have 16 bit samples (2 bytes)
		//and have 2 channels (stereo) we'll need to calculate bytes per sample later for now it is 4
		pcmsize*=avivar->waveinfo.nBlockAlign;

		}
	//pull audio from audio stream
	else {
		long audioframe;
		//assume conditions not type1 and surround
		if(audiostream=avivar->AVIstreamhead->next) {
			audioframe=avivar->KFtable[framenum];
			if (audioframe!=-1) {
				avivar->getframeoffset(audioframe,audiostream,&audiobase,&audiooffset,(ULONG *)&pcmsize);
				mySeek64(avivar->hfile,audiobase+audiooffset,SEEK_SET);
				myRead(avivar->hfile,audiopcm,pcmsize);
				}
			else pcmsize=0;
			}
		//VCM section
		mySeek64(avivar->hfile,qwframeoffset+dwOffset,SEEK_SET);
		if (avivar->bmi->biCompression) {
			myRead(avivar->hfile,invideobuf,dwSize);
			if (ICDecompress(hic,0,avivar->bmi,invideobuf,outbmi,outvideobuf) == ICERR_OK) {
				if (!(renderframe(videobuf,avivar))) goto redframe;
				}
			else goto redframe;
			} //end if compressed
		else { //uncompressed
			myRead(avivar->hfile,outvideobuf,dwSize);
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
	}


void dv2rtvclass::closeavi (struct imagelist *media) {
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
	media->mediaavi->closeavi();
	}


BOOL dv2rtvclass::beginavi (struct imagelist *media) {
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


void dv2rtvclass::endavi(struct imagelist *media)  {
	if (media->mediaavi) delete media->mediaavi;
	}

struct savevars *dv2rtvclass::BuildWavFile (char *filename,WAVEFORMATEX *pcmdata,ULONG heap) {
	const static int riffheader[5]={0x46464952,0,0x45564157,0x20746d66,16};
	const static int datastring[2]={0x61746164,0};
	struct savevars *saveobj=createsave(&pmem,filename,heap);
	transmemsave(saveobj,(char *)riffheader,20);
	transmemsave(saveobj,(char *)pcmdata,16);
	transmemsave(saveobj,(char *)datastring,8);
	return (saveobj);
	}

BOOL dv2rtvclass::WriteWavFile (struct savevars *saveobj,char *dest,int size) {
	transmemsave(saveobj,dest,size);
	return(0);
	}

void dv2rtvclass::CloseWavFile (struct savevars *saveobj,char *filename,ULONG datasize) {
	ULONG totalsize=datasize+20+16;
	int hfile;
	killsave(saveobj,&pmem);
	//write the total size in the riff header and the data size in the data header
	hfile=open(filename,_O_WRONLY|_O_CREAT|_O_BINARY|_O_SEQUENTIAL,_S_IREAD | _S_IWRITE);
	lseek(hfile,4,SEEK_SET);
	write(hfile,&totalsize,4);
	lseek(hfile,20+16+4,SEEK_SET);
	write(hfile,&datasize,4);
	close(hfile);
	}


void dv2rtvclass::dv2rtvmain() {
	//char dest=NULL;
	//UWORD x,y;
	static char backup[691200];
	char destvideo[MAX_PATH];
	char destaudio[MAX_PATH];
	char destaudio2[MAX_PATH];
	ULONG *videobuf;
	long videobufid;
	struct imagelist media;
	struct savevars *saveobj,*saveobj2;
	ULONG totaldatasize=0;
	int t,totalframes;
	UINT frameindex=0;
	BOOL frameshigh=0;
	//bool audio=(SendMessage(audiochk,BM_GETCHECK,0,0)==BST_CHECKED);
	//bool video=(SendMessage(videochk,BM_GETCHECK,0,0)==BST_CHECKED);
	BOOL audio=audioon;
	BOOL video=videoon;

	//Disable video manipulation
	EnableMenuItem(m_ptr,IDM_VIDEO,MF_GRAYED);
	if (sourceavi[0]==0) {
		printerror("Error.");
		printerror("Please select an AVI DV source first.");
		goto error;
		}
	playing=TRUE;
	SetWindowText(startstopbt,"Stop");
	wsprintf(destvideo,"%s%s.rtv",videopath,clipname);
	wsprintf(destaudio,"%s%s.wav",audiopath,clipname);
	GetWindowText(sourcefileedit,sourceavi,MAX_PATH);
	if (!(toaster->rtmeoutput)) videobuf=toaster->NoToaster;
	if (toaster->videooff) videobuf=(ULONG *)toaster->videobuf;

	//printf("Main\n");
	//do {
		media.mediaavi=NULL;
		//media.filesource="lighter.avi";
		media.filesource=sourceavi;
		//media.filesource="c:\\temp\\dv.avi";
		if (!(beginavi(&media))) {
			//Disable audio or video if not included in stream
			if (media.mediaavi->hasaudio==0) {
				audio=0;
				if (debug) printc("No audio stream detected");
				}
			if (media.mediaavi->hasvideo==0) {
				video=0;
				if (debug) printc("No video stream detected");
				}
			if (!openavi(&media)) {
				//In this section we'll write our RTV and Wav file
				totalframes=media.mediaavi->totalframes;
				//totalframes=1;
				if (audio) {
					if (media.mediaavi->surroundsound) {
						wsprintf(destaudio,"%s%s.wav",audiopath,clipname);
						wsprintf(destaudio2,"%s%s2.wav",audiopath,clipname);
						saveobj=BuildWavFile (destaudio,&(media.mediaavi->waveinfo),48000);
						saveobj2=BuildWavFile (destaudio2,&(media.mediaavi->waveinfo),48000);
						}
					else saveobj=BuildWavFile (destaudio,&(media.mediaavi->waveinfo),48000);
					}
				if (video) BuildRTVFile(destvideo,2,0,720,240,29.97f);

				for (t=0;t<totalframes;t++) {

					progressobj->updateimage(t,totalframes);
					if ((toaster->rtmeoutput)&&(!toaster->videooff)) {
						videobuf=toaster->AllocFrame(&videobufid);
						if (videobuf==0) videobuf=(ULONG *)&backup;
						}

					getframeavi((ULONG *)videobuf,&media,t);
					if ((audio)&&(pcmsize)) {
						if (media.mediaavi->surroundsound) WriteWavFile (saveobj2,(char *)audiopcm2,pcmsize);
						WriteWavFile (saveobj,(char *)audiopcm,pcmsize);
						}
					if (video) {
						//Buffer it
						memcpy(((char *)(capture->rtvcache[frameindex])),videobuf,691200);
						frameindex++;
						if (frameindex==RTVCACHEHALF) {
							//printc("Writelow");
							frameshigh=0;
							writertv(frameshigh,destvideo);
							}
						if (frameindex>=RTVCACHEUNIT) {
							frameindex=0;
							//printc("Writehigh");
							frameshigh=1;
							writertv(frameshigh,destvideo);
							}

						//WriteRTVFile(destvideo,videobuf);
						}
					//paste the video frame to toaster
					if (!preview->previewoff) {
						SetEvent(arrayofevents[EVENT_PREVIEW]);
						//preview->updatepreview(videobuf);
						//don't need this anymore since we have progress indicator
						//else printc("Frame# %ld of %ld completed.",t,totalframes-1);
						}
					if ((toaster->rtmeoutput)&&(videobufid)&&(!toaster->videooff)) toaster->SendFrame(videobufid);

					totaldatasize+=pcmsize;
					if (!playing) break;
					}

				if (audio) {
					if (media.mediaavi->surroundsound) CloseWavFile (saveobj2,destaudio2,totaldatasize);
					CloseWavFile (saveobj,destaudio,totaldatasize);
					}
				if (video) {
					UINT t,start;

					if (frameindex>RTVCACHEHALF) start=RTVCACHEHALF;
					else start=0;
/**/
					for (t=start;t<frameindex;t++) {
						WriteRTVFile(destvideo,capture->rtvcache[t]);
						}
					CloseRTVFile(destvideo);
/**/
					}
				} //end if open avi was successful
			closeavi(&media);
			}
		endavi(&media);

	playing=FALSE;
	EnableMenuItem(m_ptr,IDM_VIDEO,MF_ENABLED);
	SetWindowText(startstopbt,"Start");
	progressobj->updateimage(0,totalframes);
	//printc("done!");
error:;
	} //end dv2rtvmain

void dv2rtvclass::writertv(BOOL frameshigh,char *destvideo) {
	int t,start;
	if (debug) printc("writing video %d",frameshigh);
	start=frameshigh*RTVCACHEHALF;
/**/
	for (t=start;t<start+RTVCACHEHALF;t++) {
		//print("%lx,",medialoaders->rtvobj.rtvcache[t]);
		WriteRTVFile(destvideo,capture->rtvcache[t]);
		}
/**/
	}
