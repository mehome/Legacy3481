#include "handlemain.h"
#include "../include/ijl.h"

#pragma comment(lib, "../lib/ijl11")


BOOL loadersclass::jpgclass::openjpg(struct imagelist *media)
	{
	char* source;
	LPBITMAPINFOHEADER lpBitmapInfoHeader;
	char *decodedbits;
	long size;
	BOOL result;

	if (debug) printc("Bitmap");
	source=(char *)load(media->filesource,&size,&pmem);
	decodedbits=JPG2raw(source,size,&lpBitmapInfoHeader);
	result=raw2yuv(media,lpBitmapInfoHeader,decodedbits,TRUE);

	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	if (lpBitmapInfoHeader) disposenode(nodeobject,(struct memlist *)lpBitmapInfoHeader);
	if (source) dispose((struct memlist *)source,&pmem);
	return (result);
	}


void loadersclass::jpgclass::closejpg(struct imagelist *media) {
	if (debug) printc("Jpg");
	if (media->mediabmp) dispose((struct memlist *)media->mediabmp,&pmem);
	media->mediabmp=NULL;
	}


HBITMAP loadersclass::jpgclass::getthumbjpg(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo) {
	void *buffer;
	char *source;
	HDC hdcimage;
	HBITMAP hbm=NULL;
	long size;
	char *decodedbits;

	hdcimage=CreateCompatibleDC(NULL);
	source=(char *)load(mediaptr->filesource,&size,&pmem);
	if (source) {
		if ((decodedbits=JPG2raw(source,size,bitmapinfo))==0) return(FALSE);

		if (mediaptr->thumb=hbm=CreateDIBSection(hdcimage,(LPBITMAPINFO)*bitmapinfo,DIB_RGB_COLORS,&buffer,NULL,NULL))
			memcpy(buffer,decodedbits,(*bitmapinfo)->biSizeImage);
		if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
		if (source) dispose((struct memlist *)source,&pmem);
		if (hdcimage) DeleteDC(hdcimage);
		}
	return (hbm);
	}


char *loadersclass::jpgclass::JPG2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo) {
	//Given a pointer to source returns a pointer to dest
	//returns bitmapbits.. and a pointer to a BITMAPINFO struct
	//returns null if error
	JPEG_CORE_PROPERTIES jcprops;
	IJLERR status = IJL_INVALID_ENCODER;
	LPBITMAPINFOHEADER bmi=NULL;
	DWORD width,nchannels,padbytes,wholeimagesize;
	int height;
	char *dest;

	//////////////////////////////////////////////////////////////////
	//Initialize the IJL.
	ijlInit(&jcprops);

	jcprops.JPGBytes =(UBYTE *)source;
	jcprops.JPGSizeBytes = size;

	//////////////////////////////////////////////////////////////////
	//Get the image parameters.
	status = ijlRead(&jcprops,IJL_JBUFF_READPARAMS);
	if (status < IJL_OK)	{strcpy(string,"Unable to read JPG");goto errorjpg;}

	//////////////////////////////////////////////////////////////////
	//Set up the local data structures.
	width = jcprops.JPGWidth;
	height = jcprops.JPGHeight;

	nchannels = 3;
	padbytes = ((((width*3) + 3) >> 2) * 4) - (width * 3);
	wholeimagesize = (padbytes + (width * nchannels)) * height;

	//Allocate the dest bitmap structure.
	if ((dest=(char *)mynew(&pmem,wholeimagesize))==0) {
		wsprintf(string,"Error:Not enough memory available");
		goto errorjpg;
		}

	//////////////////////////////////////////////////////////////////
	//Configure the IJL to read the JPEG data into the DIB.
	jcprops.DIBColor = IJL_BGR;  //<---- YUV! TODO!!
	jcprops.DIBChannels = nchannels;
	jcprops.DIBWidth = width;
	jcprops.DIBHeight = -height;
	jcprops.DIBPadBytes = padbytes;
	jcprops.DIBBytes =(UBYTE *) dest;

	//////////////////////////////////////////////////////////////////
	//Read the data from the input buffer.
	status = ijlRead(&jcprops, IJL_JBUFF_READENTROPY);

	//////////////////////////////////////////////////////////////////
	//Free the IJL.
	ijlFree(&jcprops);
	if (status < IJL_OK) {strcpy(string,"Unable to read JPG");goto errorjpg;}
	//Set up our Bitmapinfo
	bmi=(LPBITMAPINFOHEADER)newnode(nodeobject,sizeof(BITMAPINFOHEADER)+1024);
	memset(bmi,0,sizeof(BITMAPINFOHEADER)+1024);
	bmi->biSize=sizeof(BITMAPINFOHEADER);
	bmi->biWidth=width;
	bmi->biHeight=height;
	bmi->biPlanes=1;
	bmi->biBitCount=24;
	bmi->biSizeImage=wholeimagesize;

	*bitmapinfo=bmi;
	return (dest);

errorjpg:
   ijlFree(&jcprops);
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	*bitmapinfo=NULL;
	if (dest) dispose((struct memlist *)dest,&pmem);
	//printf("%s\n",string);
	printc(string);
	return(0);
	}

