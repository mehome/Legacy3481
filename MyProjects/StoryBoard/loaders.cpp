#include "HandleMain.h"  
//This group contatins the means of loading the media onto the storyboard 
//as well as bringing the images to the controls for display


//* * * This group loads media to controls for display * * *

BOOL loadersclass::mediamanager(ULONG *videobuf,struct imagelist *streamptr,ULONG mediatimer,UBYTE mode) {
	BOOL (loadersclass::*tempframefunc)(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
	struct imagelist *tempmedia,*tempmedia2;

	tempmedia2=NULL;
	tempframefunc=NULL;
	if ((!(streamptr))||(streamptr->id==id_error)) {
		blackvideo(videobuf);
		return (TRUE);
		}
	if (mode&MMDRAFTMODE) {
		//TODO use streamptr->image as a quick blit
		//I do not see how it is possible yet
		return (TRUE);
		}
	playmode=mode&MMSCRUBMODE;
	if ((oldstreamptr!=streamptr)||((!nextmediaptr)&&(streamptr->next))) {
		oldstreamptr=streamptr;
		//Update Resources
		if (streamptr->id==id_media) {
			if (streamptr->prev) {
				if (streamptr->prev->id==id_dve) {
					addtomediatimer=streamptr->prev->duration;
					}
				else addtomediatimer=0;
				}
			else addtomediatimer=0;

			//Update Resources if image is a Media
			if (streamptr==nextmediaptr) {
				closemedia(prevmediaptr);
				//Rotate pointers
				prevmediaptr=mediaptr;
				prevgetframefunc=getframefunc;
				mediaptr=nextmediaptr;
				getframefunc=nextgetframefunc;
				nextmediaptr=NULL;
				nextgetframefunc=NULL;
				//find the next media
				setnextmediaptr(streamptr);
				}
			else if (streamptr==prevmediaptr) {
				closemedia(nextmediaptr);
				//Rotate pointers
				nextmediaptr=mediaptr;
				nextgetframefunc=getframefunc;
				mediaptr=prevmediaptr;
				getframefunc=prevgetframefunc;
				prevmediaptr=NULL;
				prevgetframefunc=NULL;
				//find the prev media
				setprevmediaptr(streamptr);
				}
			else {//This media has not been previously defined or been closed
				//Close will check for Null
				closemedia(prevmediaptr);
				closemedia(mediaptr);
				closemedia(nextmediaptr);
				prevmediaptr=mediaptr=nextmediaptr=NULL;
				setnextmediaptr(streamptr); //find the next media
				setprevmediaptr(streamptr); //find the prev media
				//Open all media's
				getframefunc=NULL;
				mediaptr=streamptr;
				if (mediaptr) openmedia(mediaptr,&getframefunc);
				} //end media not been previously defined
			}  //end streamptr=media

		else { //streamptr=dve
			if (streamptr->prev) {
				if (streamptr->prev->id==id_media) {
					addtomediatimer=streamptr->prev->actualframes+streamptr->prev->cropin;
					addtomediatimer-=streamptr->duration;
					tempmedia=streamptr->prev;
					}
				else goto nomediadetected; 
				}
			else {
nomediadetected:
				addtomediatimer=0;
				if (debug) printc("Warning:Media manager can not find the media assosiated with this Transition");
				tempmedia=NULL; //yes an elogant and efficient way to simplify
				}
			//Search it, Seek and Destroy, ha ha ha ha
			//Search for the existance of the tempmedia to be mediaptr
			if (tempmedia) {
				//The streamptr->prev is valid in this scope
				if (tempmedia!=mediaptr) {
					//move mediaptr contents to be checked in next's conditions
					tempmedia2=mediaptr;
					tempframefunc=getframefunc;
					if (tempmedia==nextmediaptr) {
						//Rotate pointers
						mediaptr=nextmediaptr;
						getframefunc=nextgetframefunc;
						//init next since we are using it
						nextmediaptr=NULL;
						nextgetframefunc=NULL;
						}
					else if (tempmedia==prevmediaptr) {
						//Rotate pointers
						if (nextmediaptr) closemedia(nextmediaptr);
						nextmediaptr=mediaptr;
						nextgetframefunc=getframefunc;
						mediaptr=prevmediaptr;
						getframefunc=prevgetframefunc;
						//init prev since we are using it
						prevmediaptr=NULL;
						prevgetframefunc=NULL;
						}
					else {//This media has not been previously defined or been closed
						//Close will check for Null
						//Open mediaptr
						getframefunc=NULL;
						mediaptr=tempmedia;
						if (mediaptr) openmedia(mediaptr,&getframefunc);
						} //end media not been previously defined
					} //end tempmedia!=mediaptr
				} //end if tempmedia
			else {
				getframefunc=NULL;
				mediaptr=NULL;
				}
			if (streamptr->next) {
				if (streamptr->next->id==id_media) {
					tempmedia=streamptr->next;
					//Search for the nextmedia
					if (tempmedia!=nextmediaptr) {
						//We can close nextmediaptr since we are writing over it
						if (nextmediaptr) closemedia(nextmediaptr);
						nextgetframefunc=NULL; //This may be redundant
						//Check and see if tempmedia2 has outlived its usefulness
						if (tempmedia!=tempmedia2) {
							if (tempmedia2) {
								closemedia(tempmedia2);
								tempframefunc=NULL;
								}

							if (tempmedia==prevmediaptr) {
								nextmediaptr=prevmediaptr;
								nextgetframefunc=prevgetframefunc;
								//close prev since we are using it
								prevmediaptr=NULL;
								prevgetframefunc=NULL;
								} 
							else { //next is not opened yet
								nextgetframefunc=NULL;
								nextmediaptr=tempmedia;
								if (nextmediaptr) openmedia(nextmediaptr,&nextgetframefunc);
								}
							} //end if tempmedia!=tempmedia2
						else { //our nextmedia is out tempmedia2
							nextmediaptr=tempmedia2;
							nextgetframefunc=tempframefunc;
							tempmedia2=NULL;
							tempframefunc=NULL;
							}
						} //end if tempmedia!=nextmediaptr
					} //end if we have a next media
				else goto nonextmediadetected;
				} //end if we have anything next
			else {
nonextmediadetected:
				nextgetframefunc=NULL;
				nextmediaptr=NULL;
				}
			tempmedia=NULL; //This may be redundant
			//Finally prevmediaptr will be closed if dve
			if (prevmediaptr) closemedia(prevmediaptr);
			prevmediaptr=NULL;
			prevgetframefunc=NULL;
			} //end streamptr=dve
		} //end streamptr=media and Update of Media Resources

	//Last section getframe
	if (streamptr->id==id_media) {
		//Here's our basic streamer w/o transitions
		if (!((getframefunc&&mediaptr)&&
			((this->*getframefunc)(videobuf,mediaptr,mediatimer+addtomediatimer+streamptr->cropin,toggleimagestream))))
			blackvideo(videobuf);
		//Test for filters on single stream
		if (mediaptr->mediafilter) filters->filterimage(videobuf,mediaptr,mediatimer);
		return (TRUE);
		}
	else { //DVE implementation;
		//Send Image A B and C over to FX to get calculated
		//TODO see if we really need to check mediaptr condition
		class generalFX *DVEprefs=streamptr->DVEprefs;

		if (DVEprefs->reverse) {
			if (!((getframefunc&&mediaptr)&&
				((this->*getframefunc)(imageb,mediaptr,mediatimer+addtomediatimer,!toggleimagestream))))
				blackvideo(imagea);

			if (!((nextgetframefunc&&nextmediaptr)&&
				((this->*nextgetframefunc)(videobuf,nextmediaptr,mediatimer+nextmediaptr->cropin,toggleimagestream))))
				blackvideo(imageb);
			}
		else {
			if (!((getframefunc&&mediaptr)&&
				((this->*getframefunc)(videobuf,mediaptr,mediatimer+addtomediatimer,!toggleimagestream))))
				blackvideo(imagea);

			if (!((nextgetframefunc&&nextmediaptr)&&
				((this->*nextgetframefunc)(imageb,nextmediaptr,mediatimer+nextmediaptr->cropin,toggleimagestream))))
				blackvideo(imageb);
			}

		if (streamptr->mediafilter) filters->filterimage(videobuf,streamptr,mediatimer);
		//streamptr->DVEprefs->doFX(streamptr,videobuf,imageb,videobuf,mediatimer);
		//Set up the transition threads
		DVEprefs->imagea=DVEprefs->videobuf=videobuf;
		DVEprefs->imageb=imageb;
		DVEprefs->dvetime=mediatimer;
		SetEvent(arrayofevents[EVENT_DVE1]);
		SetEvent(arrayofevents[EVENT_DVE2]);
		WaitForMultipleObjects(2,arrayofevents+EVENT_DVE1F,TRUE,INFINITE);
		return (TRUE);
		}
	}


void loadersclass::blackvideo(ULONG *videobuf) {
	/*
	long t=691200;
	do {
		*videobuf++=0x10801080; //black
		t-=4;
		} while (t);
	*/
	const static long black[2]={0x10801080,0x10801080};
	__asm {
		movq	mm0,black
		mov	ecx,86400
		mov	edi,videobuf
doagain:
		movq	[edi],mm0
		add	edi,8
		dec	ecx
		jne	doagain
		EMMS
		}
	}

void loadersclass::flushusedmedia(struct imagelist *index) {
	//Check and see if this image was opened and close it from the loaders
	if (index==mediaptr) {
		closemedia(index);
		mediaptr=NULL;
		getframefunc=NULL;
		}
	if (index==prevmediaptr) {
		closemedia(index);
		prevmediaptr=NULL;
		prevgetframefunc=NULL;
		}
	if (index==nextmediaptr) {
		closemedia(index);
		nextmediaptr=NULL;
		nextgetframefunc=NULL;
		}
	}

void loadersclass::setnextmediaptr(struct imagelist *streamptr) {
	struct imagelist *findnext=NULL;
	if (streamptr->next) {
		if (streamptr->next->id==id_media) {
			findnext=streamptr->next;
			}
		else if (streamptr->next->next) if (streamptr->next->next->id==id_media) 
			findnext=streamptr->next->next;
		}
	//Open the next media
	if ((nextmediaptr)&&(findnext!=nextmediaptr)) closemedia(nextmediaptr);
	if ((findnext)&&(findnext!=nextmediaptr)) openmedia(findnext,&nextgetframefunc);
	nextmediaptr=findnext;
	}


void loadersclass::setprevmediaptr(struct imagelist *streamptr) {
	struct imagelist *findprev=NULL;
	if (streamptr->prev) {
		if (streamptr->prev->id==id_media) {
			findprev=streamptr->prev;
			}
		else if (streamptr->prev->prev) if (streamptr->prev->prev->id==id_media) 
			findprev=streamptr->prev->prev;
		}
	//Open the prev media
	if ((prevmediaptr)&&(findprev!=prevmediaptr)) closemedia(prevmediaptr);
	if ((findprev)&&(findprev!=prevmediaptr)) openmedia(findprev,&prevgetframefunc);
	prevmediaptr=findprev;
	}


DWORD loadersclass::streamfunc()
	{
	WaitForSingleObject(arrayofevents[EVENT_STREAM],INFINITE);
	while (!killstreamthread) {
		//SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);

		//SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		//printc("Stop");
		if (streamthreadmode) toaster->writertv();
		else rtvobj.streamrtv();
		/*
		if (debug) if (updatevideorequest>2) printc("Skipping frames");
		updatevideorequest=0;
		*/

		WaitForSingleObject(arrayofevents[EVENT_STREAM],INFINITE);
		} //end while not killstreamthread
	return (0);
	} // end streamfunc()


UWORD loadersclass::gettotalframes(struct imagelist *media) {
	UWORD frames;
	switch (media->mediatype) {
		case rtv:
			frames=rtvobj.gettotalrtv(media);
			break;
		case rtv_still:
			frames=0;
			break;
		case avi:
			frames=aviobj.gettotalavi(media);
			break;
		case mpg:
			if (debug) printc("MPEG Not Yet Implemented");
			frames=0;
			break;
		case mov:
			if (debug) printc("Quicktime Not Yet Implemented");
			frames=0;
			break;
		case iff:
		case bmp:
		case tga:
		case pcx:
		case tif:
		case jpg:
			frames=gettotalstills(media);
			break;
		case gif:
			if (debug) printc("Gif Not Yet Implemented");
			frames=0;
			break;
		default:
			printc("Warning: Unknown Media Type");
			frames=0;
		} //end switch of mediatype
	return (frames);
	} //end get total frames


HBITMAP loadersclass::openthumb(struct imagelist *buffer,UWORD *totalframes,LPBITMAPINFOHEADER *bitmapinfo) {
	//BOOL (loadersclass::*getframefunc)(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
	ULONG theframe;
	//This Belongs to Media for Grabbing thumbnails
	//Manually switch through and get HBITMAP

	switch (buffer->mediatype) {
		case avi: {
			HBITMAP hbm=NULL;
			aviobj.beginavi(buffer);
			if (!(aviobj.openavi(buffer,TRUE))) {
				*totalframes=gettotalframes(buffer);
				theframe=*totalframes;
				if (theframe>1) theframe/=2;
				hbm=aviobj.getthumbavi(buffer,theframe,bitmapinfo);
				}
			aviobj.closeavi(buffer,TRUE);
			aviobj.endavi(buffer);
			return(hbm);
			break;
			}
		case bmp:
			*totalframes=gettotalframes(buffer); //does not require media to be open
			//return(buffer->mediabmp=(HBITMAP)LoadImage(hInst,buffer->filesource,IMAGE_BITMAP,VIDEOX,VIDEOY,LR_LOADFROMFILE));
			return(bmpobj.getthumbbmp(buffer,bitmapinfo));
		case pcx:
			*totalframes=gettotalframes(buffer); //does not require media to be open
			return(pcxobj.getthumbpcx(buffer,bitmapinfo));
		case tif:
			*totalframes=gettotalframes(buffer); //does not require media to be open
			return(tifobj.getthumbtif(buffer,bitmapinfo));
		case jpg:
			*totalframes=gettotalframes(buffer); //does not require media to be open
			return(jpgobj.getthumbjpg(buffer,bitmapinfo));
		case iff:
			*totalframes=gettotalframes(buffer);
			return(iffobj.getthumbiff(buffer,bitmapinfo));
		case tga:
			*totalframes=gettotalframes(buffer);
			return(tgaobj.getthumbtga(buffer,bitmapinfo));
		case rtv:
		case rtv_still:
			*totalframes=gettotalframes(buffer);
			theframe=*totalframes;
			if (buffer->mediatype==rtv_still) theframe=0;
			else theframe/=2;
			return(rtvobj.getthumbrtv(buffer,theframe,bitmapinfo));
		}
	return(NULL);
	}

void loadersclass::closethumb(struct imagelist *buffer,LPBITMAPINFOHEADER bitmapinfo) {
	switch (buffer->mediatype) {
		case avi:
			//if (bitmapinfo) disposenode(nodeobject,(struct memlist *)bitmapinfo);
			if (buffer->thumb) DeleteObject(buffer->thumb);
			buffer->thumb=NULL;
			//aviobj.closeavi(buffer,TRUE);
			break;
		case bmp:
		case pcx:
		case tif:
		case jpg:
			if (bitmapinfo) disposenode(nodeobject,(struct memlist *)bitmapinfo);
			if (buffer->thumb) DeleteObject(buffer->thumb);
			buffer->mediabmp=NULL;
			break;
		case iff:
		case tga:
		case rtv:
			if (buffer->thumb) DeleteObject(buffer->thumb);
			buffer->thumb=NULL;
			break;
		}
	}

//This section Reserved for Still Media API's * * *

UWORD loadersclass::gettotalstills(struct imagelist *media) {
	return (120);
	}


BOOL loadersclass::getframestills(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	/*
	long t=691200;
	do {
		*videobuf++=0x1b761bb8; //blue
		t-=4;
		} while (t);

	const static long blue[2]={0x1b761bb8,0x1b761bb8};
	__asm {
		movq	mm0,blue
		mov	ecx,86400
		mov	edi,videobuf
doagain:
		movq	[edi],mm0
		add	edi,8
		dec	ecx
		jne	doagain
		EMMS
		}
	*/
		//long t=691200;
	rtvobj.checkrtvstream(videobuf,mediaptr,framenum,bimage);
	ULONG *mediabufindex=mediaptr->mediabmp;
__asm {
		mov ecx,172800
		mov edi,videobuf
		mov esi,mediabufindex
		rep movsd
		}
	return (TRUE);
	}


//End Still Media Area * * *
//Here are our getframe Hook functions for still frames better than switch but I still would like
//to figure out how to pass the actual function back in openmedia

BOOL loadersclass::getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	rtvobj.checkrtvstream(videobuf,mediaptr,framenum,bimage);
	return (aviobj.getframeavi(videobuf,mediaptr,framenum,bimage));
	}
BOOL loadersclass::getframeiff(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	rtvobj.checkrtvstream(videobuf,mediaptr,framenum,bimage);
	return (iffobj.getframeiff(videobuf,mediaptr,framenum,bimage));
	}
BOOL loadersclass::getframetga(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	rtvobj.checkrtvstream(videobuf,mediaptr,framenum,bimage);
	return (tgaobj.getframetga(videobuf,mediaptr,framenum,bimage));
	}
BOOL loadersclass::getframertvstill(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	rtvobj.checkrtvstream(videobuf,mediaptr,framenum,bimage);
	return (rtvobj.getframertvstill(videobuf,mediaptr,framenum,bimage));
	}
BOOL loadersclass::getframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	rtvobj.checkrtvstream(videobuf,mediaptr,framenum,bimage);
	if (playmode) return (rtvobj.getframertv(videobuf,mediaptr,framenum,bimage));
	else return (rtvobj.getoneframertv(videobuf,mediaptr,framenum,bimage));
	}

//end getframe hook functions


BOOL loadersclass::openmedia(
	struct imagelist *media,
	BOOL (loadersclass::**getframefunc)(ULONG *videobuf,
		struct imagelist *mediaptr,ULONG framenum,BOOL bimage)
	)  {
	//struct audionode *audioindex;
	BOOL error=FALSE;
	//Open findout what the media type is 
	//and call the correct filter
	//We are assuming that mediamanager will provide a valid media
	if (media->filesource) {
		if (debug) {
			wsprintf(string,"Opening Media %s, type=",media->filesource);
			print(string);
			}
		switch (media->mediatype) {
			case rtv:
				*getframefunc=getframertv;
				error=rtvobj.openrtv(media);
				if (debug) printc("RTV");
				break;
			case rtv_still:
				*getframefunc=getframertvstill;
				error=rtvobj.openrtv(media);
				if (debug) printc("RTV still");
				break;
			case avi:
				if (debug) printc("AVI");
				*getframefunc=getframeavi;
				error=aviobj.openavi(media,0);
				break;
			case mpg:
				if (debug) printc("MPEG Not Yet Implemented");
				error=TRUE;
				break;
			case mov:
				if (debug) printc("Quicktime Not Yet Implemented");
				error=TRUE;
				break;
			case iff:
				*getframefunc=getframeiff;
				error=iffobj.openiff(media);
				if (debug) printc("IFF");
				break;
			case bmp:
				*getframefunc=getframestills;
				error=bmpobj.openbmp(media);
				if (debug) printc("BMP");
				break;
			case pcx:
				*getframefunc=getframestills;
				error=pcxobj.openpcx(media);
				if (debug) printc("PCX");
				break;
			case tif:
				*getframefunc=getframestills;
				error=tifobj.opentif(media);
				if (debug) printc("TIF");
				break;
			case jpg:
				*getframefunc=getframestills;
				error=jpgobj.openjpg(media);
				if (debug) printc("JPG");
				break;
			case gif:
				if (debug) printc("Gif Not Yet Implemented");
				error=TRUE;
				break;
			case tga:
				*getframefunc=getframetga;
				error=tgaobj.opentga(media);
				if (debug) printc("TGA");
				break;
			default:
				printc("Warning: Unknown Media Type");
				error=TRUE;
			}
		if (error) {
			printc("Warning Unable to load this File or Filetype");
			*getframefunc=NULL;
			}//end error
		}// end valid file source
	else { //Null detected for file source
		if (debug) printc("No FileSource detected for this Media");
		}
/*
	//here is where we open all audio layers for this media
	if (audioindex=media->audio) {
		do {
			if (((struct wavinfo *)audioindex)->hfile==0) 
				if (audio->player.openmedia((struct wavinfo *)audioindex))
					audio->addvoicetoplay((struct wavinfo *)audioindex,media);
			} while (audioindex=audioindex->next);
		}
*/
	return(error);
	} //end Open Media


void loadersclass::closemedia(struct imagelist *media) {
	//Findout what the media type is 
	//and close the media
	//We may get NULL for media
	if (media) {
		if (debug) {
			wsprintf(string,"Closing Media %s, type=",media->filesource);
			print(string);
			}
		switch (media->mediatype) {
			case rtv:
				if (debug) printc("RTV");
				rtvobj.closertv(media);
				break;
			case rtv_still:
				rtvobj.closertv(media);
				if (debug) printc("RTV still");
				break;
			case avi:
				if (debug) printc("AVI");
				aviobj.closeavi(media,0);
				break;
			case mpg:
				if (debug) printc("MPEG");
				break;
			case mov:
				if (debug) printc("Quicktime");
				break;
			case iff:
				iffobj.closeiff(media);
				if (debug) printc("ILBM");
				break;
			case bmp:
				bmpobj.closebmp(media);
				if (debug) printc("BMP");
				break;
			case pcx:
				pcxobj.closepcx(media);
				if (debug) printc("PCX");
				break;
			case tif:
				tifobj.closetif(media);
				if (debug) printc("TIF");
				break;
			case jpg:
				jpgobj.closejpg(media);
				if (debug) printc("JPEG");
				break;
			case gif:
				if (debug) printc("GIF");
				break;
			case tga:
				tgaobj.closetga(media);
				if (debug) printc("Targa");
				break;
			default:
				printc("Warning: Unknown Media Type");
			} //end enuming through the types
		} //end validating media
	}

void loadersclass::beginmedia(struct imagelist *media) {
	if (media) {
		if (debug) {
			wsprintf(string,"Beginning Media %s, type=",media->filesource);
			print(string);
			}
		switch (media->mediatype) {
			case rtv:
				if (debug) printc("RTV");
				rtvobj.beginrtv(media);
				break;
			case rtv_still:
				if (debug) printc("RTV still");
				rtvobj.beginrtv(media);
				break;
			case avi:
				if (debug) printc("AVI");
				aviobj.beginavi(media);
				break;
			case mpg:
				if (debug) printc("MPEG");
				break;
			case mov:
				if (debug) printc("Quicktime");
				break;
			case iff:
				if (debug) printc("ILBM");
				break;
			case bmp:
				if (debug) printc("BMP");
				break;
			case pcx:
				if (debug) printc("PCX");
				break;
			case tif:
				if (debug) printc("TIF");
				break;
			case jpg:
				if (debug) printc("JPEG");
				break;
			case gif:
				if (debug) printc("GIF");
				break;
			case tga:
				if (debug) printc("Targa");
				break;
			default:
				printc("Warning: Unknown Media Type");
			} //end enuming through the types
		} //end validating media
	}

void loadersclass::endmedia(struct imagelist *media) {
	if (media) {
		if (debug) {
			wsprintf(string,"Ending Media %s, type=",media->filesource);
			print(string);
			}
		switch (media->mediatype) {
			case rtv:
				if (debug) printc("RTV");
				rtvobj.endrtv(media);
				break;
			case rtv_still:
				if (debug) printc("RTV still");
				rtvobj.endrtv(media);
				break;
			case avi:
				if (debug) printc("AVI");
				aviobj.endavi(media);
				break;
			case mpg:
				if (debug) printc("MPEG");
				break;
			case mov:
				if (debug) printc("Quicktime");
				break;
			case iff:
				if (debug) printc("ILBM");
				break;
			case bmp:
				if (debug) printc("BMP");
				break;
			case pcx:
				if (debug) printc("PCX");
				break;
			case tif:
				if (debug) printc("TIF");
				break;
			case jpg:
				if (debug) printc("JPEG");
				break;
			case gif:
				if (debug) printc("GIF");
				break;
			case tga:
				if (debug) printc("Targa");
				break;
			default:
				printc("Warning: Unknown Media Type");
			} //end enuming through the types
		} //end validating media
	}

loadersclass::loadersclass() {
	ULONG align;
	mediaptr=prevmediaptr=nextmediaptr=NULL;
	//the plus 64 is to ensure 512k byte alignment
	if (!(videobuffers=new ULONG[172800*2+128])) error(0,"Not enough Memory Available");
	//To type cast a pointer to 32bit and pull the lower 9 bits will still be 64bit compliant!
	align=(ULONG)videobuffers&511;
	if (align) imagea=(ULONG *)((char *)videobuffers+(512-align));
	imageb=imagea+172800;
	addtomediatimer=0;
	oldstreamptr=NULL;
	playmode=0;
	toggleimagestream=0;
	}

void loadersclass::shutdown() {
	//Make sure all Media Resources from MediaManager are closed
	//Note These check for NULL internally
	//These have to be called prior to the cleanup()
	closemedia(mediaptr);
	closemedia(nextmediaptr);
	closemedia(prevmediaptr);
	mediaptr=nextmediaptr=prevmediaptr=NULL;
	}

loadersclass::~loadersclass() {
	delete [] videobuffers;
	}
