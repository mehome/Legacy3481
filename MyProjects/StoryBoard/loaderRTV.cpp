#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include "HandleMain.h"
#include "../include/newtek/RTVlib.h"

#pragma comment(lib, "../lib/I386/RTVlib")

typedef loadersclass::rtvclass rtvloader;
static struct nodevars *streamrequestnode=createnode(&pmem,1024,0);

UWORD rtvloader::gettotalrtv(struct imagelist *media) {
	UWORD number;
	if ((number=(UWORD)(ReadRTVNoFrames(media->filesource)))==1) {
		number=120;
		media->mediatype=rtv_still;
		}
	else number--;
	return (number);
	}


HBITMAP rtvloader::getthumbrtv(struct imagelist *mediaptr,ULONG framenum,LPBITMAPINFOHEADER *bitmapinfo) {
	const static BITMAPINFO bmi={
		sizeof(BITMAPINFOHEADER), 
		720,		//LONG   biWidth; 
		480,		//LONG   biHeight; 
		1,		//WORD   biPlanes; 
		32,		//WORD   biBitCount 
		BI_RGB,	//DWORD  biCompression; 
		NULL,	//DWORD  biSizeImage; 
		4,		//LONG   biXPelsPerMeter; 
		3,		//LONG   biYPelsPerMeter; 
		NULL,	//DWORD  biClrUsed; 
		NULL		//DWORD  biClrImportant; 
		};
	void *buffer;
	HDC hdcimage;
	HBITMAP hbm;

	*bitmapinfo=(LPBITMAPINFOHEADER)&bmi;
	hdcimage=CreateCompatibleDC(NULL);
	mediaptr->thumb=hbm=CreateDIBSection(hdcimage,&bmi,DIB_RGB_COLORS,&buffer,NULL,NULL);
	ReadRTVFileBGRA(mediaptr->filesource,framenum,buffer);
	CloseRTVFile(mediaptr->filesource);

	if (hdcimage) DeleteDC(hdcimage);
	return (hbm);
	}

char **createfilelist(char *filename,unsigned *items) {
	struct stringlist {
		struct stringlist *next;
		char *string;
		} *stringlisthead,*stringlisttemp,*stringlistindex;
	char **dest;
	long size;
	char *filebuffer=(char *)load(filename,&size,&pmem);
	char *fileindex=filebuffer;
	char *fileindexend=fileindex+size;
	*items=0;

	if (!filebuffer) return (NULL);
	*items=1;
	stringlisthead=stringlistindex=(stringlist *)newnode(nodeobject,sizeof(struct stringlist));
	fileindex+=linput(fileindex,string);
	stringlisthead->string=(char *)newnode(nodeobject,strlen(string)+1);
	strcpy(stringlisthead->string,string);
	while (fileindex<fileindexend) {
		fileindex+=linput(fileindex,string);
		stringlistindex->next=(stringlist *)newnode(nodeobject,sizeof(struct stringlist));
		stringlistindex=stringlistindex->next;
		stringlistindex->string=(char *)newnode(nodeobject,strlen(string)+1);
		strcpy(stringlistindex->string,string);
		(*items)++;
		}
	stringlistindex->next=NULL;
	stringlistindex=stringlisthead;
	dest=(char **)newnode(nodeobject,sizeof(APTR) * *items);
	for (unsigned i=0;i<*items;i++) {
		dest[i]=stringlistindex->string;
		stringlisttemp=stringlistindex;
		stringlistindex=stringlistindex->next;
		disposenode(nodeobject,(struct memlist *)stringlisttemp);
		}
	dispose((struct memlist *)filebuffer,&pmem);
	return (dest);
	}

rtvloader::rtvclass() {
	m_AudioPaths=createfilelist("RTVAudioPaths.ini",&m_items);
	ULONG align;
	long t;
	DWORD SectorsPerCluster,BytesPerSector,NumberOfFreeClusters,TotalNumberOfClusters;

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
	//init the id's
	for (t=0;t<3;t++) {
		rtvidcache[t]=0;
		}
	streamreqhead=streamreqtail=NULL;
	updatestreamrequest=0;
	nextmedia=NULL;
	}


rtvloader::~rtvclass() {
	delete [] videobuffers;
	}


void loadersclass::rtvclass::beginrtv(struct imagelist *media) {
	static ULONG rtvidcounter=1;
	int hfile;

	media->mediartv=(struct rtvhandle *)newnode(nodeobject,sizeof(struct rtvhandle));
	media->mediartv->rtvcacheid=rtvidcounter++;
	//See if we have an audio file for this rtv
	if (!project->isstartedbyproject) {
		//Convert path to c...wav
		strcpy(string,media->filesource);
		//TODO make prefs audio device specifier
		string[0]='C';
		string[strlen(string)-3]=0;
		strcat(string,"wav");
		EnterCriticalSection(&csglobal);
		if (hfile=open(string,_O_RDONLY|_O_BINARY)>0) {
			if (debug) printc("RTV has audio");
			close(hfile);
			audio->addwavtomedia(media,string);
			//we can assume that the audio member has only this one
			//audio to assign for rtvaudio
			media->rtvaudio=media->audio;
			}
		else { //Check other paths for Audio
			unsigned t;
			for (unsigned i=0;i<m_items;i++) {
				strcpy(string,m_AudioPaths[i]);
				//Ensure our last char has a backslash
				if (!(string[(t=strlen(string))-1]=='\\')) {
					string[t++]='\\';
					string[t]=0;
					}
				strcat(string,media->text);
				string[strlen(string)-3]=0;
				strcat(string,"wav");
				if (hfile=open(string,_O_RDONLY|_O_BINARY)>0) {
					if (debug) printc("RTV has audio");
					close(hfile);
					audio->addwavtomedia(media,string);
					//we can assume that the audio member has only this one
					//audio to assign for rtvaudio
					media->rtvaudio=media->audio;
					break;
					}
				}
			}
		LeaveCriticalSection(&csglobal);
		} 
	}


BOOL loadersclass::rtvclass::openrtv(struct imagelist *media)
	{
	ULONG *mediabufindex;
	struct imagelist *prevmedia;
	//struct streammsg *streamreqindex;
	long t;
	BOOL withdve;

	media->mediartv->mediartvstill=NULL;
	if (media->mediatype==rtv_still) {
		//if (debug) printc("RTV still");
		media->mediartv->rtvcacheid=0;
		if (media->mediartv->mediartvstill=mediabufindex=new ULONG[172800])
			if (!(ReadRTVFile(media->filesource,0,(void *)mediabufindex))) return(1);
		}
	else {
		//search for an empty id slot to fill
		t=0;
		while ((t<3)&&(rtvidcache[t])) t++;
		if (t>=3) error(0,"Overflow error: RtvIdCache overflow");
		else {
			rtvidcache[t]=media->mediartv->rtvcacheid;
			media->mediartv->cacheslot=t;
			//Calculate when to start next stream if possible
			nextmedia=media;
			if (prevmedia=media->prev) {
				if (prevmedia->id==id_dve) {
					withdve=TRUE;
					prevmedia=prevmedia->prev;
					if (prevmedia->id==id_dve) prevmedia=NULL;
					}
				else withdve=FALSE;
				if (prevmedia) {
					singlestreamlength=prestartnexttime=prevmedia->actualframes;
					if (withdve) singlestreamlength=prestartnexttime-=prevmedia->next->duration;

					//subtract the beginning dve if exists
					if (prevmedia->prev) 
						if (prevmedia->prev->id==id_dve) 
							singlestreamlength-=prevmedia->prev->duration;

					if (prestartnexttime>RTVCACHEUNIT) {
						prestartnexttime-=(RTVCACHEUNIT);
						}
					else prestartnexttime=0; //hmmmm
					prestartnexttime&=RTVNEGRATE;
					if (debug==2) {
						wsprintf(string,"length=%d nexttime=%d",singlestreamlength,prestartnexttime);printc(string);
						}
					}
				}
			else prestartnexttime=0;
			//Signal to obtain the first 4 frames
			} //t is valid
		} //rtv not still
	return (0);
	}


void rtvloader::closertv(struct imagelist *media) {
	long t;

	if (media->mediatype==rtv_still) {
		if (media->mediartv) {
			if (media->mediartv->mediartvstill) delete [] media->mediartv->mediartvstill;
			media->mediartv->mediartvstill=NULL;
			}
		}
	else {
		//search for id and clear it to zero
		t=0;
		while ((t<3)&&(rtvidcache[t]!=media->mediartv->rtvcacheid)) t++;
		if (t>=3) printc("Warning: RtvIdCache is corrupt");
		else {
			rtvidcache[t]=0;
			}
		}
	if (media->filesource) CloseRTVFile(media->filesource); //Either case
	}


void rtvloader::endrtv(struct imagelist *media) {
	if (media->mediatype==rtv) {
		}
	if (media->mediartv) {
		disposenode(nodeobject,(struct memlist *)media->mediartv);
		media->mediartv=NULL;
		}
	}


BOOL rtvloader::getframertvstill(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	//long t=691200;
	ULONG *mediabufindex=mediaptr->mediartv->mediartvstill;
	/*
	do {
		*videobuf++=*mediabufindex++;
		t-=4;
		} while (t);
	*/
__asm {
		mov ecx,172800
		mov edi,videobuf
		mov esi,mediabufindex
		rep movsd
		}
	return(TRUE);
	}


BOOL rtvloader::getoneframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	if (!(ReadRTVFile(mediaptr->filesource,framenum-1,(void *)videobuf))) return (FALSE);
	return(TRUE);
	}


BOOL loadersclass::rtvclass::checkrtvstream(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	struct streammsg *streamreqindex;
	long nextcachevalue;
	int cropin;
	int cachelength;
	int morecacheoffset;
	int imageboffset=((!bimage)*(RTVCACHEHALF>>1));

	if (nextmedia) {
		cropin=(int)nextmedia->cropin;
		//See if we need to start next set of medias
		if ((prestartnexttime)&&(mediaptr->id==id_media)&&(!(mediaptr==nextmedia))&&(nextmedia->mediatype==rtv)) {
			//subtract the prev dve from framenum
			//if (mediaptr->prev) if (mediaptr->prev->id==id_dve) framenum-=mediaptr->prev->duration;
			if ((framenum>(unsigned)prestartnexttime)&&(framenum<=(unsigned)prestartnexttime+RTVCACHEUNIT)) {
				if (!(((controls->frameindex-1)+imageboffset)&RTVREFRESHRATE)) {
					
					nextcachevalue=(cropin&RTVNEGRATE)-RTVCACHEHALF;

					if (singlestreamlength<32) cachelength=RTVCACHEUNIT;
					else {
						cachelength=RTVCACHEHALF;
						morecacheoffset=(((int)framenum-1)&RTVNEGRATE)-(prestartnexttime&RTVNEGRATE);
						nextcachevalue+=morecacheoffset;
						}

					//if ((controls->frameindex-1)&RTVREFRESHRATE>>1) nextcachevalue-=RTVCACHEHALF;
					if (updatestreamrequest<2) {
						updatestreamrequest++;
						//push the media request onto a message queue for streamer
						streamreqindex=streamreqtail;
						EnterCriticalSection(&csglobal);
						streamreqtail=(struct streammsg *)newnode(streamrequestnode,sizeof(struct streammsg));
						if (!streamreqhead) streamreqhead=streamreqtail;
						if (streamreqindex) streamreqindex->next=streamreqtail;
						streamreqtail->next=NULL;	
						streamreqtail->media=nextmedia;
						streamreqtail->rtvframecache=nextcachevalue;
						streamreqtail->cachelength=cachelength;
						LeaveCriticalSection(&csglobal);
						SetEvent(arrayofevents[EVENT_STREAM]);
						}
					else if (debug) printc("Stream is unable to keep up");
					}
				} //end if frameindex
			} //end if > startnexttime
		} //end if nextmedia
	return(TRUE);
	}


BOOL loadersclass::rtvclass::getframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	long t,timeout=0;
	long cacheframe;
	ULONG *source;
	struct streammsg *streamreqindex;

	//framenum-=1; //1 to 0 base conversion
	t=mediaptr->mediartv->cacheslot;
	cacheframe=(long)framenum&RTVNEGRATE;
	if (!((controls->frameindex+(bimage*(RTVCACHEHALF>>1))-1)&RTVREFRESHRATE)) {
	//if (!((framenum-1)&RTVREFRESHRATE)) {
		if (updatestreamrequest<2) {
			updatestreamrequest++;
			//push the media request onto a message queue for streamer
			streamreqindex=streamreqtail;
			EnterCriticalSection(&csglobal);
			streamreqtail=(struct streammsg *)newnode(streamrequestnode,sizeof(struct streammsg));
			if (!streamreqhead) streamreqhead=streamreqtail;
			if (streamreqindex) streamreqindex->next=streamreqtail;
			streamreqtail->next=NULL;
			streamreqtail->media=mediaptr;
			streamreqtail->rtvframecache=cacheframe;
			streamreqtail->cachelength=RTVCACHEHALF;
			LeaveCriticalSection(&csglobal);
			SetEvent(arrayofevents[EVENT_STREAM]);
			}
		else if (debug) printc("Stream is unable to keep up");
		}
	//we'll wait for confirmation
	/**/
	//Sleep(1);
	while ((updatestreamrequest)&&(timeout<100)) {
		if ((debug)&&(timeout>0)) {
			wsprintf (string,"Waiting for stream %ld",timeout);
			printc(string);
			}
		timeout++;
		Sleep(1);
		}
	/**/
	//Move a frame from framebuf to video toaster
	source=rtvcache[(framenum%RTVCACHEUNIT)+t*RTVCACHEUNIT];
	__asm	{
		mov		edi, videobuf
		 mov		ecx, 691200
		mov		esi, source
		shr		ecx,4	
doagain:
		movq		mm0, [esi]
		 dec		ecx
		movq		mm1, [esi+8]
		movq		[edi], mm0
		 add		esi, 16
		movq		[edi+8], mm1
		add		edi, 16
		 cmp		ecx, 0
		jne		doagain
		emms
		}
	return(TRUE);
	} //end getframertv


void loadersclass::rtvclass::streamrtv() {
	static char *readerror="Warning: RTV read error detected";
	long t,high,index/*,cropin*/;
	long cacheframe;
	int cachelength;
	struct imagelist *mediaptr;
	struct streammsg *streamtemp;
	//int hfile;
	//pull which media that needs updating
	while (streamreqhead) {
		mediaptr=streamreqhead->media;
		t=mediaptr->mediartv->cacheslot;
		cacheframe=streamreqhead->rtvframecache;
		cachelength=streamreqhead->cachelength;
		EnterCriticalSection(&csglobal);
		streamtemp=streamreqhead;
		streamreqhead=streamreqhead->next;
		if (streamreqtail==streamtemp) streamreqtail=NULL;
		disposenode(streamrequestnode,(struct memlist *)streamtemp);
		updatestreamrequest--;
		LeaveCriticalSection(&csglobal);

/*		
		if (cacheframe==-1) {
			cropin=(long)mediaptr->cropin;
			if (cropin&RTVCACHEHALF) high=RTVCACHEHALF;
				else high=0;

			//open the first unit of frames
			for (index=0;index<RTVCACHEHALF;index++) {
				if (index<=mediaptr->totalframes) {
					EnterCriticalSection(&csglobal);
					if (!(ReadRTVFile(mediaptr->filesource,cropin+index,(void *)rtvcache[high+index+t*RTVCACHEUNIT]))) 
						printc(readerror);
					LeaveCriticalSection(&csglobal);
					}
				else break;
				}
			} //end first four frames

		else { //regular frames
*/
			if (cacheframe&RTVCACHEHALF) high=0;
				else high=RTVCACHEHALF;
			for (index=0;index<cachelength;index++) {
				if (cacheframe+index>mediaptr->totalframes) break;
				else	{
					EnterCriticalSection(&csglobal);
					if (!(ReadRTVFile(mediaptr->filesource,cacheframe+index+RTVCACHEHALF,(void *)rtvcache[high+index+t*RTVCACHEUNIT]))) 
						printc(readerror);
					LeaveCriticalSection(&csglobal);
					}
				}
			//} //end regular frames
		} //end while there are more requests
	} //end streamrtv()

