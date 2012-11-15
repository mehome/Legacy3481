#include "handlemain.h"
#include "PCXconsole.h"

BOOL loadersclass::pcxclass::openpcx(struct imagelist *media) {
	LPBITMAPINFOHEADER lpBitmapInfoHeader;
	char *source;
	char *decodedbits;
	BOOL result;
	long size;

	if (debug) printc("TIFF");
	if (!(source=(char *)load(media->filesource,&size,&pmem))) return(1);
	decodedbits=PCX2raw(source,size,&lpBitmapInfoHeader);
	if (source) dispose((struct memlist *)source,&pmem);
	result=raw2yuv(media,lpBitmapInfoHeader,decodedbits,FALSE);
	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	if (lpBitmapInfoHeader) disposenode(nodeobject,(struct memlist *)lpBitmapInfoHeader);
	return (result);
	}

void loadersclass::pcxclass::closepcx(struct imagelist *media) {
	if (debug) printc("Bitmap");
	if (media->mediabmp) dispose((struct memlist *)media->mediabmp,&pmem);
	media->mediabmp=NULL;
	}


HBITMAP loadersclass::pcxclass::getthumbpcx(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo) {
	void *buffer;
	char *source;
	HDC hdcimage;
	HBITMAP hbm;
	long size;
	char *decodedbits;
	UINT yindex,yindex2=0;
	UINT width,height;

	hdcimage=CreateCompatibleDC(NULL);
	if (!(source=(char *)load(mediaptr->filesource,&size,&pmem))) return(FALSE);
	if ((decodedbits=PCX2raw(source,size,bitmapinfo))==0) return(FALSE);

	mediaptr->thumb=hbm=CreateDIBSection(hdcimage,(LPBITMAPINFO)*bitmapinfo,DIB_RGB_COLORS,&buffer,NULL,NULL);
	//memcpy(buffer,decodedbits,(*bitmapinfo)->biSizeImage);
	height=(*bitmapinfo)->biHeight;
	width=(*bitmapinfo)->biWidth * ((*bitmapinfo)->biBitCount/8);
	for (yindex=height-1;yindex>0;yindex--) {
		memcpy((char *)buffer+width*yindex2,decodedbits+width*yindex,width);
		yindex2++;
		}

	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	if (source) dispose((struct memlist *)source,&pmem);
	if (hdcimage) DeleteDC(hdcimage);
	return (hbm);
	}


char *loadersclass::pcxclass::decompress (char *source,char *dest,DWORD maincount,char *eos,char *eod) {
	__asm {
		mov	edi,dest
		mov	esi,source
		mov	ecx,maincount
doagain:
		mov	al,[esi]
		test	al,128 //are the 2 bits on
		jz	bitsoff
		test	al,64
		jz	bitsoff
		jmp	bitson
bitsoff:
		//2bits are not on
		mov	[edi],al
		inc	edi
		inc	esi
		dec	ecx
		jmp	checkEOF
bitson:
		push	ecx			//save ecx main count
		and	al,03fh		//al = the amount of times to repeat max 63
		xor	ecx,ecx
		mov	cl,al			//ecx = loop of repeat
		/*
		Shouldn't need to cut a repeat
		//check min (eod-edi,ecx)
		mov	edx,eod
		sub	edx,edi
		cmp	ecx,edx
		cmovl	edx,ecx
		mov	ecx,edx		//ecx & edx = copy of repeat
		*/
		mov	edx,ecx
		mov	al,[esi+1]	//al = the char to repeat
		rep	stosb
		pop	ecx			//restore main loop
		sub	ecx,edx
		add	esi,2
checkEOF:
		cmp	esi,eos
		jg		done
		cmp	edi,eod
		jg		done
		cmp	ecx,0
		jne	doagain
done:
		mov	source,esi
		}
	return(source);
	}


void loadersclass::pcxclass::convertpalette (char *source,char *dest,int numofcolors) {
	//copy three bytes into 4
	__asm {
		//__R2G2B2__R1G1B1
		mov	edi,dest
		mov	esi,source
		mov	ecx,numofcolors	//16 or 256 colors
		shr	ecx,1			//2 at a time
loop16:
		movq	mm0,[esi]	//mm0 = G3R3B2G2R2B1G1R1
		movq	mm1,mm0
		psrlq	mm1,24		//mm1 = ______G3R3B2G2R2
		psllq	mm1,40		//mm1 = B2G2R2__________
		psrlq	mm1,8			//mm1 = __B2G2R2________
		psllq	mm0,40		//mm0 = B1G1R1__________
		psrlq	mm0,40		//mm0 = __________B1G1R1
		por	mm0,mm1		//mm0 = __B2G2R2__B1G1R1
		//now to flip R and B
		movq	mm2,mm0
		pslld	mm2,24		//mm2 = R2______R1______
		psrld	mm2,8			//mm2 = __R2______R1____
		movq	mm1,mm0
		psrld	mm1,16		//mm1 = ______B2______B1
		por	mm1,mm2		//mm1 = __R2__B2__R1__B1
		movq	mm2,mm0
		psrld	mm2,8			//mm2 = ____B2G2____B1G1
		pslld	mm2,24		//mm2 = G2______G1______
		psrld mm2,16		//mm2 = ____G2______G1__
		por	mm1,mm2		//mm1 = __R2G2B2__R1G1B1
		movq	[edi],mm1
		add	edi,8
		add	esi,6
		dec	ecx
		jne	loop16
		emms
		}
	}


void loadersclass::pcxclass::consolidatebitplanes(char *scanline,char *dest,int planesize) {
	__asm {
		mov	edi,dest
		mov	esi,scanline
		mov	eax,planesize	//eax = G index
		mov	ecx,eax			
		shr	ecx,3				//8 colors at a time
		mov	edx,eax			//edx = B index
		shl	edx,1
loopplane:
		movq			mm0,[esi]		//mm0 = R8R7R6R5R4R3R2R1
		movq			mm1,[esi+eax]	//mm1 = G8G7G6G5G4G3G2G1
		movq			mm2,[esi+edx]	//mm2 = B8B7B6B5B4B3B2B1
		pxor			mm5,mm5			//mm5 = ________________
		movq			mm3,mm0			//mm3 = R8R7R6R5R4R3R2R1
		movq			mm6,mm2			//mm6 = B8B7B6B5B4B3B2B1
		punpcklbw	mm6,mm1			//mm6 = G4B4G3B3G2B2G1B1
		movq			mm7,mm6
		punpcklbw	mm3,mm5			//mm3 = __R4__R3__R2__R1
		punpcklwd	mm7,mm3			//mm7 = __R2G2B2__R1G1B1
		movq			[edi],mm7		//store 1 and 2
		punpckhwd	mm6,mm3			//mm6 = __R4G4B4__R3G3B3
		movq			[edi+8],mm6		//store 3 and 4
		movq			mm6,mm2			//mm6 = B8B7B6B5B4B3B2B1
		punpckhbw	mm6,mm1			//mm6 = G8B8G7B7G6B6G5B5
		movq			mm7,mm6
		punpckhbw	mm0,mm5			//mm0 = __R8__R7__R6__R5
		punpcklwd	mm7,mm0			//mm7 = __R6G6B6__R5G5B5
		movq			[edi+16],mm7	//store 5 and 6
		punpckhwd	mm6,mm0			//mm6 = __R8G8B8__R7G7B7
		movq			[edi+24],mm6	//store 7 and 8
		add			edi,32
		add			esi,8
		dec			ecx
		jne			loopplane
		emms
		}
	}


char *loadersclass::pcxclass::PCX2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo) {
	//const long pullcolor[2]={0x10001000,0x10001000};
	//Given a pointer to source returns a pointer to dest
	//returns bitmapbits.. and a pointer to a BITMAPINFO struct
	//returns null if error
	char *encodedbits,*destindex,*dest=NULL;
	struct PCXheader *header=(struct PCXheader *)source;
	LPBITMAPINFOHEADER bmi;
	char *bmipalette;
	char *pcxpalette;

	//Convert PCX header to bitmap header
	bmi=(LPBITMAPINFOHEADER)newnode(nodeobject,sizeof(BITMAPINFOHEADER)+1024);
	bmipalette=(char *)bmi+sizeof(BITMAPINFOHEADER);
	memset(bmi,0,sizeof(BITMAPINFOHEADER)+1024);
	bmi->biSize=sizeof(BITMAPINFOHEADER);
	bmi->biWidth=(header->Xmax-header->Xmin)+1;
	bmi->biHeight=(header->Ymax-header->Ymin)+1;
	bmi->biPlanes=1; //We convert to 1
	if (header->NPlanes==3)	bmi->biBitCount=header->BitsPerPixel<<2;
	else bmi->biBitCount=header->BitsPerPixel*header->NPlanes;
	bmi->biSizeImage=((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes*bmi->biHeight/8;
	//figure out palette info
	if (header->Version<5) {
		pcxpalette=(char *)header+16;
		bmi->biClrUsed=16;
		convertpalette(pcxpalette,bmipalette,16);
		}
	else { //version 5 expect 256 colors or 24 bit
		//Test for 256
		if (*(pcxpalette=source+(size-769))==12) {
			//biClrUsed can remain zero
			pcxpalette++;
			convertpalette(pcxpalette,bmipalette,256);
			}
		else { //No palette make sure it is 24bit
			if (!(header->NPlanes==3)) {wsprintf(string,"No palette detected and not 24 bit");goto errorpcx;}
			}
		}
	encodedbits=source+sizeof(struct PCXheader);
	if ((dest=destindex=(char *)mynew(&pmem,bmi->biSizeImage))==0) {wsprintf(string,"No memory available");goto errorpcx;}
	switch (header->NPlanes) { 
		case 1: {
			if (header->Encoding) {
				decompress(encodedbits,dest,bmi->biSizeImage,source+(size-sizeof(struct PCXheader)-768),dest+bmi->biSizeImage);
				}
			//TODO uncompressed support
			else {
				wsprintf(string,"Uncompressed support not yet implemented");
				goto errorpcx;
				}
			break;
			}

		case 3: { //24bit
			char *scanlinebuffer;
			int bytesperline;
			char *sourcesize=source+size;
			char *eod;
			char *desteod=dest+bmi->biSizeImage;
			int planesize=header->BytesPerLine;
			int y;

			bytesperline=header->BytesPerLine*header->NPlanes;
			int destadvance=header->BytesPerLine<<2;
			scanlinebuffer=(char *)newnode(nodeobject,bytesperline);
			eod=scanlinebuffer+bytesperline;

			if (header->Encoding) {
				for (y=0;y<bmi->biHeight;y++) {
					encodedbits=decompress(encodedbits,scanlinebuffer,bytesperline,sourcesize,eod);
					consolidatebitplanes(scanlinebuffer,destindex,planesize);
					destindex+=destadvance;
					if ((destindex>desteod)||(encodedbits>sourcesize)) break;
					}
				}
			//TODO uncompressed support
			else {
				wsprintf(string,"Uncompressed support not yet implemented");
				if (scanlinebuffer) disposenode(nodeobject,(struct memlist *)scanlinebuffer);
				goto errorpcx;
				}
			if (scanlinebuffer) disposenode(nodeobject,(struct memlist *)scanlinebuffer);
			break;
			}
		default: wsprintf(string,"Only 8 and 24 bitsperpixel supported");goto errorpcx;
		} //end switch planes
	*bitmapinfo=bmi;
	return (dest);
errorpcx:
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	*bitmapinfo=NULL;
	if (dest) dispose((struct memlist *)dest,&pmem);
	//printf("%s\n",string);
	printc(string);
	return(0);
	}

