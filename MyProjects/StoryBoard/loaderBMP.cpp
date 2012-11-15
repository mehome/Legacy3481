#include "HandleMain.h"  

typedef loadersclass::bmpclass bmploader;

BOOL loadersclass::bmpclass::openbmp(struct imagelist *media)
	{
	LPBITMAPFILEHEADER source;
	LPBITMAPINFOHEADER lpBitmapInfoHeader;
	char *decodedbits;
	long size;
	BOOL result;

	if (debug) printc("Bitmap");
	if (!(source=(LPBITMAPFILEHEADER)load(media->filesource,&size,&pmem))) return (1);
	lpBitmapInfoHeader=(LPBITMAPINFOHEADER)((char *)source+sizeof(BITMAPFILEHEADER));
	decodedbits=(char *)source+source->bfOffBits;
	result=raw2yuv(media,lpBitmapInfoHeader,decodedbits,TRUE);
	if (source) dispose((struct memlist *)source,&pmem);
	return (result);
	}


void bmploader::closebmp(struct imagelist *media) {
	if (debug) printc("Bitmap");
	if (media->mediabmp) dispose((struct memlist *)media->mediabmp,&pmem);
	media->mediabmp=NULL;
	}

/*
BOOL loadersclass::bmpclass::getframebmp(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	//long t=691200;
	ULONG *mediabufindex=mediaptr->mediabmp;
__asm {
		mov ecx,172800
		mov edi,videobuf
		mov esi,mediabufindex
		rep movsd
		}
	return(TRUE);
	}
*/

HBITMAP loadersclass::bmpclass::getthumbbmp(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo) {
	void *buffer;
	LPBITMAPFILEHEADER source;
	HDC hdcimage;
	HBITMAP hbm;
	long size;
	BITMAPINFOHEADER *bmi;

	hdcimage=CreateCompatibleDC(NULL);
	if (!(source=(LPBITMAPFILEHEADER)load(mediaptr->filesource,&size,&pmem))) return(FALSE);
	bmi=(LPBITMAPINFOHEADER)((char *)source+sizeof(BITMAPFILEHEADER));
	bmi->biSizeImage = ((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes*bmi->biHeight/8;
	*bitmapinfo=(LPBITMAPINFOHEADER)newnode(nodeobject,bmi->biSize+1024);
	memcpy(*bitmapinfo,bmi,bmi->biSize+1024);
	mediaptr->thumb=hbm=CreateDIBSection(hdcimage,(LPBITMAPINFO)*bitmapinfo,DIB_RGB_COLORS,&buffer,NULL,NULL);
	if (hbm) memcpy(buffer,(char *)source+source->bfOffBits,bmi->biSizeImage);
	if (source) dispose((struct memlist *)source,&pmem);
	if (hdcimage) DeleteDC(hdcimage);
	return (hbm);
	}
