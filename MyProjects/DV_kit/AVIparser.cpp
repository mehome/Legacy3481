#include "handlemain.h"
#include "AVIparser.h"
#include "DVstructs.h"


long *avihandle::makeKFtable(AVIstream *video,AVIstream *audio) {
	/*
	This function creates a array of keyframes where the audio chunk procedes a video chunk.
	For optimum performance and since DV (our most used form) is one for one... I've decided
	to have every single video frame represented... the frames that don't contain audio will 
	have a value of -1 so the maximum number of frames is a signed long.  We should be able
	to safely assume that there will be no more than one audio chunk per video frame in this
	algorithm.

	I've decided to integrate the open and creation and destruction of the cindex which will
	make a complete pass over the avi because typically at this time it is common to only
	work with one stream of video and audio.  For future expansion this algorithm may have
	to make a function which can handle multiple streams and make multiple KF tables.  However
	even if that is the case this interface will be able to create one table for an avi which
	has multiple streams... the parent function can choose which streams to work with.
	*/
	long *KFtable;
	ULONG vindex,aindex;
	DWORDLONG qwVideoBase,qwAudioBase;
	DWORD dwVideoOffset,dwAudioOffset;
	DWORD dwVideoSize,dwAudioSize;
	DWORD audiomax=audio->cindexsptr->dwDuration;

	KFtable=(long *)mynew(&pmem,totalframes*4);
	aindex=0;
	getframeoffset(aindex,audio,&qwAudioBase,&dwAudioOffset,&dwAudioSize);
	for (vindex=0;vindex<totalframes;vindex++) {
		getframeoffset(vindex,video,&qwVideoBase,&dwVideoOffset,&dwVideoSize);
		if (((qwAudioBase+dwAudioOffset)<(qwVideoBase+dwVideoOffset))&&(aindex<audiomax)) {
			KFtable[vindex]=aindex;
			aindex++;
			if (aindex<audiomax-1) getframeoffset(aindex,audio,&qwAudioBase,&dwAudioOffset,&dwAudioSize);
			}
		else KFtable[vindex]=-1;
		}
	//Checksum if all the audio chunks were not used then we'll spmyRead them out evenly
	//This will be acceptable for raw converstion... for StoryBoard we'll need to redo our
	//algorithm to ensure the chunk time of audio (not seek position) is keyed on the correct
	//frame
	if (aindex<audiomax-1) {
		int interval=totalframes/audiomax;
		aindex=0;
		if (debug) printc ("Audio chunks after video Detected.");
		for (vindex=0;vindex<totalframes;vindex++) {
			if ((!(vindex%interval))&&(aindex<audiomax)) {
				KFtable[vindex]=aindex;
				aindex++;
				}
			else KFtable[vindex]=-1;
			}
		}
	return (KFtable);
	}


ULONG avihandle::getframeoffset(ULONG framenum,
	struct AVIstream *stream,
	DWORDLONG *qwframeoffset,
	DWORD *dwOffset,       // 32 bit offset to data (points to data, not riff header)
	DWORD *dwSize         // 31 bit size of data (does not include size of riff header) (high bit is deltaframe bit)
	) {
	/*
	For now we'll only support the superindex method to find frame, since this is the only
	method which surpasses the 2gig with a 64bit offset... however we'll need to add the
	old index method which will be refined to support a non-indexed avi... both will conform
	to a simple index table which can be assumed to be 32 bit.
	*/
	/*
	we have in the clist of how many frames each index has and take the difference of this
	and the frame num... to get the proper index and offset of it.
	Note: duration is a zero based value to subtract it needs to be 1 based
	*/
	AVISTDINDEX *cindex;
	struct cindexlist *clist;

	clist=stream->cindexsptr;
	while ((framenum>=clist->dwDuration)&&(clist)) {
		framenum-=(clist->dwDuration);
		if (clist->next) clist=clist->next;
		else {
			getcindex();
			clist=clist->next;
			}
		}

	if (!clist) {
		//This should never happen
		//printerror("Internal frame overflow error trunicating to first frame");
		printf("Internal frame overflow error trunicating to first frame");
		clist=AVIstreamhead->cindexsptr;
		framenum=1;
		}

	cindex=clist->cindexptr;
	*qwframeoffset=cindex->qwBaseOffset;
	*dwOffset=cindex->aIndex[framenum].dwOffset;
	*dwSize=(cindex->aIndex[framenum].dwSize)&0x7fffffff;
	return(framenum);
	}


void avihandle::idx1toindx(char *source,ULONG size) {
	//This function is designed to strip out and convert the first stream only
	AVIstream *slist;
	DWORD totalstreams=avih->dwStreams;
	AVISTDINDEX **cindex=(AVISTDINDEX **)newnode(nodeobject,sizeof(APTR)*totalstreams);
	AVISTDINDEX_ENTRY **dest=(AVISTDINDEX_ENTRY **)newnode(nodeobject,sizeof(APTR)*totalstreams);
	ULONG *count=(ULONG *)newnode(nodeobject,sizeof(APTR)*totalstreams);
	char *eof=source+size;
	DWORD t=0;
	int totalsize;

	totalframes=avih->dwTotalFrames;
	totalsize=sizeof(struct _avistdindex)+(totalframes*8); 
	//There is no way to determine the number of entries so we'll use totalframes for all streams
	//So far all totalframes have been large enough... we can't check for overflow, but we'll 
	//need to be aware in case there is in which case we will have to precount the chunks to
	//allocate the correct memory size.

	for (t=0;t<totalstreams;t++) {
		addstream(&slist);
		slist->cindexsptr=(struct cindexlist *)newnode(nodeobject,sizeof(struct cindexlist));
		slist->cindexsptr->dwDuration=totalframes;
		slist->cindexsptr->next=NULL;
		cindex[t]=slist->cindexsptr->cindexptr=(AVISTDINDEX *)mynew(&pmem,totalsize);
		//set up cindexptr header (avistdindex)
		//DWORDLONG qwBaseOffset;     // base offset this is all we need
		cindex[t]->qwBaseOffset=movipos;
		//Now to convert the old entries into the new
		dest[t]=(AVISTDINDEX_ENTRY *)((char *)cindex[t]+32); //32 is a hack for the header area
		count[t]=0;
		}
	
	//struct _avioldindex_entry {
	//	DWORD dwChunkId;	only pulling 00?? streams
	//	DWORD dwFlags;		ignored
	//	DWORD dwOffset;	offset of riff chunk header for the data
	//	DWORD dwSize;		size of the data (excluding riff header size)
	//	}
	//typedef struct _avistdindex_entry {
	//	DWORD dwOffset;	// 32 bit offset to data (points to data, not riff header)
	//	DWORD dwSize;		// 31 bit size of data (does not include size of riff header), bit 31 is deltaframe bit
	//	} AVISTDINDEX_ENTRY;

	{ //fill in the chunk data
		long *keyframe=(long *)mynew(&pmem,totalframes*4);
		struct _avioldindex {
			DWORD dwChunkId;	//only pulling 00?? streams
			DWORD dwFlags;		//used to detect Keyframes
			DWORD dwOffset;	//offset of riff chunk header for the data
			DWORD dwSize;		//size of the data (excluding riff header size)
			} *sourceindex=(struct _avioldindex *)source;
		DWORD dwChunkId,checkoffset=0;
		ULONG idvalue;
		ULONG movposoffset=0;
		DWORD *lastvalidoffset=(ULONG *)newnode(nodeobject,sizeof(APTR)*totalstreams);
		DWORD *lastvalidsize=(ULONG *)newnode(nodeobject,sizeof(APTR)*totalstreams);
		t=0;
		//Several AVI's used absolute offset instead of relative offsets based from movi
		// we'll check the first offset to determine if this is the case
		do {
			dwChunkId=(sourceindex->dwChunkId&0x0000ffff);
			idvalue=atol((char *)&dwChunkId);
			if ((dwChunkId==0x00003030)||((idvalue<totalstreams)&&(idvalue!=0)))
				checkoffset=sourceindex->dwOffset;
			sourceindex+=1;
			} while ((eof>=(char *)sourceindex)&&(checkoffset==0));
		printf("First offset=%lx  movipos=%lx\n",checkoffset,(DWORD)movipos);
		if (checkoffset>=((DWORD)movipos-4)) movposoffset=(DWORD)movipos-8;

		sourceindex=(struct _avioldindex *)source;
		while ((eof>=(char *)sourceindex)) {
			dwChunkId=(sourceindex->dwChunkId&0x0000ffff);
			idvalue=atol((char *)&dwChunkId);
			//Basically if we pull an ID value in the stream range... note zero has to be checked
			//a special way
			if ((dwChunkId==0x00003030)||((idvalue<totalstreams)&&(idvalue!=0))) {
				if (dest[idvalue]->dwSize=sourceindex->dwSize) {
					lastvalidsize[idvalue]=sourceindex->dwSize;
					dest[idvalue]->dwOffset=lastvalidoffset[idvalue]=sourceindex->dwOffset-movposoffset;
					}
				else {
					dest[idvalue]->dwOffset=lastvalidoffset[idvalue];
					dest[idvalue]->dwSize=lastvalidsize[idvalue];
					}
				dest[idvalue]+=1;
				count[idvalue]+=1;
				if (idvalue==0) {
					//KeyFrame implementation
					if (sourceindex->dwFlags&0x10) {
						keyframe[totalKF]=t;
						totalKF++;
						}
					t+=1;
					}
				}
			sourceindex+=1;
			}
		if (lastvalidsize) disposenode(nodeobject,(struct memlist *)lastvalidsize);
		if (lastvalidoffset) disposenode(nodeobject,(struct memlist *)lastvalidoffset);
		//Copy keyframe numbers over
		KFnumber=(long *)mynew(&pmem,totalKF*4);
		memcpy (KFnumber,keyframe,totalKF*4);
		dispose((struct memlist *)keyframe,&pmem);
		}
	slist=AVIstreamhead;
	for (t=0;t<totalstreams;t++) {
		slist->cindexsptr->dwDuration=count[t];
		slist=slist->next;
		}
	//once again we assume that the video stream is the first
	totalframes=count[0];

	if (count) disposenode(nodeobject,(struct memlist *)count);
	if (dest) disposenode(nodeobject,(struct memlist *)dest);
	if (cindex) disposenode(nodeobject,(struct memlist *)cindex);
	}  //end idx1toindx


BOOL avihandle::getcindex() {
	AVIMETAINDEX chunkinfo;
	struct AVIstream *slist;
	struct cindexlist *clist;
	struct superindexentries *sindex;
	int totalsize;
	UINT indexadvance;
	if (slist=AVIstreamhead) {
		do {
			if (slist->sindex) sindex=slist->sindex;
			else sindex=slist->sindexptr; //first node
			if (slist->currententry<slist->nEntriesInUse) {
				//make a simple link list of cindexptrs for overflow control we monitor the 
				//entries per index chunk and skip over this to force the cindex to null
				clist=(struct cindexlist *)newnode(nodeobject,sizeof(struct cindexlist));
				clist->next=NULL;
				if (slist->cprev) slist->cprev->next=clist;
				else slist->cindexsptr=clist;
				slist->cprev=clist;

				mySeek64(hfile,sindex->qwOffset,SEEK_SET);
				myRead(hfile,&chunkinfo,sizeof(AVIMETAINDEX));
				clist->dwDuration=chunkinfo.nEntriesInUse;
				indexadvance=sindex->dwDuration*8;
				totalsize=sizeof(AVIMETAINDEX)+(indexadvance);
				clist->cindexptr=(AVISTDINDEX *)mynew(&pmem,totalsize);
				memcpy(clist->cindexptr,&chunkinfo,sizeof(AVIMETAINDEX));
				myRead(hfile,((char *)clist->cindexptr)+sizeof(AVIMETAINDEX),indexadvance);
				slist->sindex=(struct superindexentries *)((char *)sindex +sizeof(struct superindexentries));
				slist->currententry++;
				}
			} while (slist=slist->next);
		clist->next=NULL;
		}
	else return (FALSE);
	return (TRUE);
	}


BOOL avihandle::openavi() {
	//hfile=open(filesource,_O_RDONLY|_O_BINARY);
	if ((hfile=OpenReadSeq(filesource))==(void *)-1) return (FALSE);

	if (!idx1) getcindex();
	//sindexptr will exist if we have an indx chunk
	//cindexsptr will exist if we have an idx1 chunk, and nothing will need to occur here 
	//since it was taken care of during the construct phase
	else if (!AVIstreamhead) {
		printf("Note:AVI contains no index\n");
		printf("(only indexed avi's are supported at this time)\n");
		return (FALSE);
		}
	//cindexptr=mynew(&pmem,sizeof(AVISTDINDEX)+(*));
	return(TRUE);
	} // end openavi


void avihandle::closeavi() {
	//sindexptr determines the scope of cindexptr and if idx1 we retain memory until endavi
	if (!idx1) {
		struct AVIstream *slist;
		struct cindexlist *cnext,*cindexsptr;
		if (slist=AVIstreamhead) {
			do {
				if (cindexsptr=slist->cindexsptr) {
					do {
						cnext=cindexsptr->next;
						dispose((struct memlist *)cindexsptr->cindexptr,&pmem);
						disposenode(nodeobject,(struct memlist *)cindexsptr);
						} while (cindexsptr=cnext);
					slist->cindexsptr=NULL;
					}
				slist->sindex=NULL;
				slist->cprev=NULL;
				slist->currententry=0;
				} while (slist=slist->next);
			}
		}
	if (hfile) {
		myClose(hfile);
		hfile=NULL;
		}
	}


BOOL avihandle::handleLISTodml(ULONG *outskipped,ULONG size) {
	ULONG advance;
	ULONG *checkid=(ULONG *)string;
	BOOL done=false;

	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		*outskipped+=advance+8;
		string[4]=0;
		printf("%s size,%lx\n",string,advance);
		switch (*checkid) {
			case ID_dmlh:
				myRead(hfile,&totalframes,4);
				mySeek(hfile,advance-4,SEEK_CUR);
				break;
			default:
			mySeek(hfile,advance,SEEK_CUR);
			}
		if (*outskipped>=size) done=1;
		} while (!done);
	return (0);
	}


void avihandle::addstream(AVIstream **slist) {
	//link new superlist index in a list
	*slist=(struct AVIstream *)newnode(nodeobject,sizeof(struct AVIstream));
	memset(*slist,0,sizeof(struct AVIstream));
	if (sprev) sprev->next=*slist;
	else AVIstreamhead=*slist;
	sprev=*slist;
	}


BOOL avihandle::handleLISTstrl(ULONG *outskipped,ULONG size) {
	ULONG advance;
	ULONG *checkid=(ULONG *)string;
	BOOL done=false;
	const static BITMAPINFO type1format={
		sizeof(BITMAPINFOHEADER), 
		720,		//LONG   biWidth; 
		480,		//LONG   biHeight; 
		1,		//WORD   biPlanes; 
		16,		//WORD   biBitCount 
		ID_dvsd,	//DWORD  biCompression; 
		120000,	//DWORD  biSizeImage; 
		0,		//LONG   biXPelsPerMeter; 
		0,		//LONG   biYPelsPerMeter; 
		NULL,	//DWORD  biClrUsed; 
		NULL		//DWORD  biClrImportant; 
		};

	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		*outskipped+=advance+8;
		string[4]=0;
		printf("%s size,%lx\n",string,advance);
		switch (*checkid) {
			case ID_strh: {
				struct avistreamlist *strhold=strh;
				strh=(struct avistreamlist *)newnode(avitempmem,sizeof(struct avistreamlist));
				//link it to old
				strh->next=NULL;
				myRead(hfile,&((*strh).avistreamnode),sizeof(struct AVIStreamHeader));
				if (strhold) strhold->next=strh;
				else {
					strhhead=strh;
					//Test for type1
					if (strhhead->avistreamnode.fccType==ID_iavs) {
						hasaudio=hasvideo=TRUE;
						type1=TRUE;
						//Manually supply the stream format info since DV does not provide this we'll need to
						//Determine the correct video format... for now assume NTSC
						videobuffersize=strh->avistreamnode.dwSuggestedBufferSize;
						memcpy(bmi,&type1format,sizeof(BITMAPINFOHEADER));
						bmisize=0x28;
						}
					else type1=FALSE;
					}
				if (advance-sizeof(AVIStreamHeader)) mySeek(hfile,advance-sizeof(AVIStreamHeader),SEEK_CUR);
				break;
				}
			case ID_strd:
				mySeek(hfile,advance,SEEK_CUR);
				break;
			case ID_strf:
				strh->strf=(char *)newnode(avitempmem,advance);
				//link it to old
				myRead(hfile,strh->strf,advance);
				if (strh->avistreamnode.fccType==ID_auds) {
					hasaudio=TRUE;
					memcpy(&waveinfo,strh->strf,sizeof(WAVEFORMATEX));
					if (!(audiobuffersize=strh->avistreamnode.dwSuggestedBufferSize)) {
						if (debug) printc("have to manually configure audiobuffer size");
						}

					}
				if (strh->avistreamnode.fccType==ID_vids) {
					hasvideo=TRUE;
					memcpy(bmi,strh->strf,bmisize=advance);
					vcmhandler=strh->avistreamnode.fccHandler;
					videobuffersize=strh->avistreamnode.dwSuggestedBufferSize;
					}
				//mySeek(hfile,advance,SEEK_CUR);
				break;
			case ID_indx: {
				int result;
				union {
					char *metaindex;
					AVISUPERINDEX *superindex;
					AVISTDINDEX *chunkindex;
					};
				metaindex=NULL;
				metaindex=(char *)mynew(&pmem,advance+8);
				result=myRead(hfile,&(superindex->wLongsPerEntry),advance);
				indextype=superindex->bIndexType; //may need to change this
				if (indextype==AVI_INDEX_OF_INDEXES) {
					AVIstream *slist;
					addstream(&slist);
					slist->nEntriesInUse=superindex->nEntriesInUse;
					DWORD totalsize=sizeof(struct superindexentries) * slist->nEntriesInUse;

					//copy index entries for open
					slist->sindexptr=(struct superindexentries *)newnode(nodeobject,totalsize);
					memcpy(slist->sindexptr,superindex->aIndex,totalsize);
					}
				//todo else
				if (metaindex) dispose((struct memlist *)metaindex,&pmem);
				break;
				}
			default:
			mySeek(hfile,advance,SEEK_CUR);
			}
		if (*outskipped>=size) done=1;
		} while (!done);
	return (0);
	}


BOOL avihandle::handleLISThdrl(ULONG *outskipped,ULONG size) {
	ULONG advance;
	ULONG *checkid=(ULONG *)string;
	BOOL done=false;

	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		*outskipped+=advance+8;
		string[4]=0;
		printf("%s size,%lx\n",string,advance);
		switch (*checkid) {
			case ID_avih:
				myRead(hfile,&(avih->dwMicroSecPerFrame),sizeof(struct AVIMainHeader));
				//Width and Height may not be included here
				mySeek(hfile,advance-sizeof(struct AVIMainHeader),SEEK_CUR);
				break;
			case ID_LIST: {
				ULONG skipped=4;
				//Get list type
				myRead(hfile,string,4);
				string[4]=0;
				printf("---- List Type=%s\n",string);
				//Expecting strl
				switch(*checkid) {
					case ID_strl:
						if (handleLISTstrl(&skipped,advance)) return(1);
						break;
					case ID_odml:  //NEW extension
						if (handleLISTodml(&skipped,advance)) return(1);
						break;
					default:
					wsprintf(string,"Warning:Unsupported list type");
					}
				mySeek(hfile,advance-skipped,SEEK_CUR);
				break;
				}
			default:
			mySeek(hfile,advance,SEEK_CUR);
			}
		if (*outskipped>=size) done=1;
		} while (!done);
	return (0);
	}


long avihandle::getidvalue(DWORD *dwChunkId) {
	long idvalue;

	*dwChunkId&=0x0000ffff;
	idvalue=atol((char *)dwChunkId);
	if ((*dwChunkId==0x00003030)||(((ULONG)idvalue<avih->dwStreams)&&(idvalue!=0))) return(idvalue);
	else return(-1);
	}

BOOL avihandle::handleLISTrec(ULONG *outskipped,ULONG size) {
	ULONG advance;
	ULONG *checkid=(ULONG *)string;
	long idvalue;
	BOOL done=false;

	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		*outskipped+=advance+8;
		string[4]=0;
		//printf("%s size,%lx\n",string,advance);
		if ((idvalue=getidvalue((DWORD *)string))!=-1) 
			printf("Stream %d pos %lx length %lx\n",idvalue,myTell(hfile)-(DWORD)movipos,advance);		
		mySeek(hfile,advance,SEEK_CUR);

		if (*outskipped>=size) done=1;
		} while (!done);
	return (0);
	}


BOOL avihandle::handleLISTmovi(ULONG *outskipped,ULONG size) {
	ULONG advance;
	ULONG *checkid=(ULONG *)string;
	long idvalue;
	BOOL done=false;

	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		*outskipped+=advance+8;
		string[4]=0;
		//printf("%s size,%lx\n",string,advance);
		switch (*checkid) {
			case ID_LIST: {
				ULONG skipped=4;

				//Get list type
				myRead(hfile,string,4);
				string[4]=0;
				//printf("---- List Type=%s\n",string);
				//Check for rec chunks
				if (*checkid==ID_rec) if (handleLISTrec(&skipped,advance)) return(1);
				mySeek(hfile,advance-skipped,SEEK_CUR);
				break;
				}
			default:
			if ((idvalue=getidvalue((DWORD *)string))!=-1) 
				printf("Stream %d pos %lx length %lx\n",idvalue,myTell(hfile)-(DWORD)movipos,advance);
			mySeek(hfile,advance,SEEK_CUR);
			}

		if (*outskipped>=size) done=1;
		} while (!done);
	return (0);
	}


BOOL avihandle::handleRIFF(ULONG *outskipped,ULONG size) {
	ULONG advance;
	ULONG *checkid=(ULONG *)string;
	BOOL done=false;

	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		*outskipped+=advance+8;
		string[4]=0;
		printf("%s size,%lx\n",string,advance);
		switch (*checkid) {
			case ID_LIST: {
				ULONG skipped=4;
				//Get list type
				myRead(hfile,string,4);
				string[4]=0;
				printf("---- List Type=%s\n",string);
				//Expecting hdrl or movi
				switch(*checkid) {
					case ID_hdrl:
						if (handleLISThdrl(&skipped,advance)) return(1);
						break;
					case ID_movi:
						//used for idx1 backward compatability < 2GB
						movipos=myTell64(hfile)+4;
						//This call is suspended until we implement avi's which don't have idx1
						//if (!AVIstreamhead) if (handleLISTmovi(&skipped,advance)) return(1);
						break;
					default:
					wsprintf(string,"Error:Unable to define AVI list type");
					return(1);
					}
				mySeek(hfile,advance-skipped,SEEK_CUR);
				break;
				}
			case ID_idx1: {
				char *source;
				//Superindex will exist or be created as long as we have the indx chunk
				if (AVIstreamhead)	mySeek(hfile,advance,SEEK_CUR);
				else {
					idx1=TRUE;
					source=(char *)mynew(&pmem,advance);
					myRead(hfile,source,advance);
					idx1toindx(source,advance);
					dispose((struct memlist *)source,&pmem);
					}
				done=1;
				break;
				}
			default:
			mySeek(hfile,advance,SEEK_CUR);
			}
		if (*outskipped>=size) done=1;
		} while (!done);
	return (0);
	}


avihandle::avihandle (char *filesourceparm) {
	ULONG *checkid=(ULONG *)string;
	ULONG advance;
	BOOL done=false;
	BOOL Did1AlmyReady=0;

	filesource=filesourceparm;
	//if (debug) printc("AVI");
	//hfile=open(filesource,_O_RDONLY|_O_BINARY);
	hfile=OpenReadSeq(filesource);

	avitempmem=createnode(&pmem,2048,0);

	avih=(struct AVIMainHeader *)newnode(avitempmem,sizeof(struct AVIMainHeader));
	bmi=(BITMAPINFOHEADER *)newnode(nodeobject,sizeof(BITMAPINFOHEADER)+256*4);
	strh=strhhead=NULL;
	totalframes=0;
	AVIstreamhead=sprev=NULL;
	surroundsound=0;
	idx1=FALSE;
	KFtable=KFnumber=NULL;
	totalKF=0;
	hasaudio=hasvideo=FALSE;
	audiobuffersize=0;
	vcmhandler=0;
	//This group of vars are for the parent app to support multiple instances
/**/
	hic=NULL;
	audiopcm=audiopcm2=NULL;
	outbmi=NULL;
	invideobuf=outvideobuf=NULL;
	//audiostuff
	lastoffset=0;
	lpdsb=NULL;
	lpDsNotify=NULL;
	avivarnode=NULL;
	pcmlisthead=pcmlisttail=NULL;
	lastnotify=4;
/**/
	//static WAVEFORMATEX wfx={WAVE_FORMAT_PCM,2,48000,192000,4,16,0};
	//Set our waveinfo with the following defaults
	waveinfo.wFormatTag=WAVE_FORMAT_PCM; 
	waveinfo.nChannels=2; //always 2 because thats all directsound wavs can support 
	waveinfo.nSamplesPerSec=48000;
	waveinfo.nAvgBytesPerSec=192000; 
	waveinfo.nBlockAlign=4; 
	waveinfo.wBitsPerSample=16; 
	waveinfo.cbSize=0; 

	//parse all info which is always constant throughout the life of media
	if (hfile==(void *)-1) {wsprintf(string,"Unable to open file");goto erroravi;}
	do {
		if (!(myRead(hfile,string,8))) {
			printf("Reached end of avi\n");
			break;
			}
		advance=*(checkid+1);
		string[4]=0;
		printf("%s size,%lx\n",string,advance);
		switch (*checkid) {
			case ID_RIFF: {
				ULONG skipped=4;
				//Get RIFF type
				myRead(hfile,string,4);
				string[4]=0;
				printf("---- RIFF Type=%s\n",string);
				//Expecting AVI or AVIX
				switch(*checkid) {
					case ID_AVI:
					case ID_AVIX:
						if (handleRIFF(&skipped,advance)) goto erroravi;
						break;
					default:
					printf("Error:RIFF type AVI or AVIX expected");
					goto erroravi;
					}
				mySeek(hfile,advance-skipped,SEEK_CUR);
				Did1AlmyReady=TRUE;
				break;
				}
			default:
			if (Did1AlmyReady) {
				printf("Warning:excess data beyond RIFF Chunk not defined as RIFF:AVIX\n");
				done=1;
				}
			else {
				wsprintf(string,"RIFF header not found\n");
				goto erroravi;
				}
			}
		} while (!done);
	// * * * Now to parse AVI * * *

	//open media
	myClose(hfile);
	openavi();

	//TODO handle CDVC and check for other fourcc's which are DV
	if ((strhhead->avistreamnode.fccHandler==ID_dvsd)||(strhhead->avistreamnode.fccHandler==ID_DVSD)) {
		//For DV only we need to actually go in and myRead the first frame to detect the video and
		//audio formats... We are going under the assumption that these formats will remain the same
		//throughout the entire duration of the media... 
		//class avihandle *avivar=media->mediaavi;
		BYTE framebuf[DV_FR_MAX_SIZE];
		struct cindexlist *clist;
		AVISTDINDEX *cindex;
		DWORDLONG qwframeoffset;
		DWORD dwOffset;       // 32 bit offset to data (points to data, not riff header)
		DWORD dwSize;         // 31 bit size of data (does not include size of riff header) (high bit is deltaframe bit)
		//unsigned char ac_dv_adta[DV_AC_ADTA_SIZE];
		struct tr_dv_aux r_dv_aux;

		//get frame 0 contents assuming stream 00 is video
		clist=AVIstreamhead->cindexsptr;

		cindex=clist->cindexptr;
		qwframeoffset=cindex->qwBaseOffset;
		dwOffset=cindex->aIndex[0].dwOffset;
		dwSize=cindex->aIndex[0].dwSize;
	
		mySeek64(hfile,qwframeoffset+dwOffset,SEEK_SET);
		myRead(hfile,framebuf,dwSize);
		//call the analyze routine and parse
		analyze_fr0(&r_dv_aux,framebuf);  // phase 1: collect data from AAUX/ADTA blocks into local buffer
		switch (r_dv_aux.aaux[0].sample_rate) {
			// 0=48kHz 1=44.1kHz 2=32kHz
			case 0:
				waveinfo.nSamplesPerSec=48000;
				break;
			case 1:
				waveinfo.nSamplesPerSec=44100;
				break;
			case 2:
				waveinfo.nSamplesPerSec=32000;
				break;
			}
		// we are going to assume all are converted to 16bit
		//if  (r_dv_aux.aaux[0].channel_num==2) waveinfo.nChannels=1;
		//Mikes codec will always have 2 channels even for mono
		waveinfo.nBlockAlign=waveinfo.nChannels*waveinfo.wBitsPerSample/8;
		waveinfo.nAvgBytesPerSec=waveinfo.nBlockAlign*waveinfo.nSamplesPerSec;
		surroundsound=r_dv_aux.aaux[0].channel_num;
		audiobuffersize=DV_AC_ADTA_SIZE;
		}
	if (AVIstreamhead->next) {
		if ((!type1)&&(!surroundsound)) KFtable=makeKFtable(AVIstreamhead,AVIstreamhead->next);

		if (audiobuffersize==0) {
			//I'm betting that only the older version avi's which do not use the new odml
			//extensions may have not set the audiobuffersize... this algorithm will only parse
			//the first group of cindex entries (which will be all for idx1) and find the largest
			//audio chunk among these values
			struct cindexlist *cindexsptr=AVIstreamhead->next->cindexsptr;
			DWORD t=0,duration=cindexsptr->dwDuration,chunksize;
			AVISTDINDEX *cindex=cindexsptr->cindexptr;

			for (t=0;t<duration;t++) {
				if ((chunksize=cindex->aIndex[t].dwSize)>audiobuffersize) audiobuffersize=chunksize;
				}
			}
		else audiobuffersize=max(audiobuffersize,avih->dwSuggestedBufferSize);
		}
	videobuffersize=max(videobuffersize,avih->dwSuggestedBufferSize);
	//myClose media
	closeavi();
	//myClose all external Resources
	if (pmem) killnode(avitempmem,&pmem);

	if (hfile) {
		myClose(hfile);
		hfile=NULL;
		}
	error=0;
	goto noerror;
erroravi:
	error=1;
	if (hfile) {
		myClose(hfile);
		hfile=NULL;
		}
	//printerror(string);
	printf(string);
noerror:;
	}


avihandle::~avihandle () {
	struct AVIstream *snext;
	struct cindexlist *cindexsptr,*cnext;

	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	if (KFtable) dispose((struct memlist *)KFtable,&pmem);
	if (KFnumber) dispose((struct memlist *)KFnumber,&pmem);
	if (AVIstreamhead) {
		do {

			//This is here for idx1... all other cases should have set cindexsptr to NULL
			if (cindexsptr=AVIstreamhead->cindexsptr) {
				do {
					cnext=cindexsptr->next;
					dispose((struct memlist *)cindexsptr->cindexptr,&pmem);
					disposenode(nodeobject,(struct memlist *)cindexsptr);
					} while (cindexsptr=cnext);
				}

			snext=AVIstreamhead->next;
			if (AVIstreamhead->sindexptr) disposenode(nodeobject,(struct memlist *)AVIstreamhead->sindexptr);
			disposenode(nodeobject,(struct memlist *)AVIstreamhead);
			} while (AVIstreamhead=snext);
		}
	}

