#include "HandleMain.h"  
#include "loadertga.h"

typedef loadersclass::tgaclass tgaloader;

HBITMAP loadersclass::tgaclass::getthumbtga(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo) {
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
	char *tempbuffer,*source;
	HDC hdcimage;
	HBITMAP hbm=NULL;
	long size=0;
	ULONG x,y,yindex,yindex2,ymin,xmin;
	ULONG xcenter=0;
	ULONG ycenter=0;
	UWORD x2,y2;

	*bitmapinfo=(LPBITMAPINFOHEADER)&bmi;  //Temp until we set up a direct send
	source=(char *)(load(mediaptr->filesource,&size,&pmem));
	if (source) {
		tempbuffer=TGA2raw(source,size,&x2,&y2);
		x=(ULONG)x2;y=(ULONG)y2;
		dispose((struct memlist *)source,&pmem);
		if (tempbuffer) {
			hdcimage=CreateCompatibleDC(NULL);
			mediaptr->thumb=hbm=CreateDIBSection(hdcimage,&bmi,DIB_RGB_COLORS,&buffer,NULL,NULL);

			if (x<704) max(xcenter=((720-x)>>1)-8,0);
			if (y<480) ycenter=(480-y)>>1;
			//Now to put the tempbuffer contents into the buffer upsidedown
			yindex2=0;
			ymin=min(480,y);
			xmin=min(720<<2,x<<2);
			for (yindex=ymin-1;yindex>0;yindex--) {
				//transmem((ULONG *)tempbuffer+x*yindex,(ULONG *)buffer+xcenter+720*(yindex2+ycenter),xmin);
				memcpy((char *)((ULONG *)buffer+xcenter+720*(yindex2+ycenter)),(char *)((ULONG *)tempbuffer+x*yindex),xmin);
				yindex2++;
				}
			if (hdcimage) DeleteDC(hdcimage);
			} //end if tempbuffer
		} //end if source
	//We are done with source and tempbuffer so dispose them
	if (tempbuffer) dispose((struct memlist *)tempbuffer,&pmem);
	return (hbm);
	}


BOOL loadersclass::tgaclass::opentga(struct imagelist *media) {
/**/
	//ULONG *YUVdest;
	char *tempbuffer,*source;
	long size=0;
	ULONG x,y;
	UWORD x2,y2;

	if (debug) printc("TGA");
	media->mediaiff=NULL; //TODO: see if this is already init
	if (!(source=(char *)(load(media->filesource,&size,&pmem)))) goto error;
	if (!(tempbuffer=TGA2raw(source,size,&x2,&y2))) goto error;
	x=(ULONG)x2;y=(ULONG)y2; //optimise make them long
	dispose((struct memlist *)source,&pmem);

	/*
	if (!(media->mediaiff=YUVdest=(ULONG *)mynew(&pmem,691200))) goto error;
	renderrgb32(YUVdest,x,y,tempbuffer,0);
	*/
	//Ok here's the scaled version
	{
	BITMAPINFOHEADER bmi={
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
	bmi.biWidth=x;
	bmi.biHeight=y;
	raw2yuv(media,&bmi,tempbuffer,0);
	}
	dispose((struct memlist *)tempbuffer,&pmem);
/**/
	/*
	ULONG *YUVdest;
	char *tempbuffer,*source,*dest;
	long size=0;
	ULONG *destindex;
	ULONG xindex,yindex;
	ULONG x,y,xmin,ymin,xcenter,ycenter;
	UWORD x2,y2;

	if (debug) printc("tga");
	media->mediatga=NULL; //TODO: see if this is already init
	if (!(source=(char *)(load(media->filesource,&size,&pmem)))) goto error;
	if (!(tempbuffer=TGA2raw(source,size,&x2,&y2))) goto error;
	x=(ULONG)x2;y=(ULONG)y2; //optimise make them long
	dispose((struct memlist *)source,&pmem);
	//Scale the image into 720x480 using crop
	//TODO do stetchblt option later
	if (!(dest=(char *)mynew(&pmem,1382400))) goto error;
	destindex=(ULONG *)dest;
	xcenter=ycenter=0;
	if (x<672) xcenter=((720-x)>>1)-24;
	if (y<480) ycenter=(480-y)>>1;
	xmin=min(x,720);ymin=min(y,480);
	destindex+=720*ycenter;
	for (yindex=0;yindex<ymin;yindex++) {
		destindex+=xcenter;
		for (xindex=0;xindex<xmin;xindex++) {
			*destindex++=*((ULONG *)tempbuffer+xindex+x*yindex);
			}
		if (x<720) for (xindex=x+xcenter;xindex<720;xindex++) *destindex++=0;
		}
	dispose((struct memlist *)tempbuffer,&pmem);
	//Now we have 720x480x32 RGB now we convert to YUV and separate fields
	//Intentionally close these vars in a local scope to pull registers
	if (!(media->mediatga=YUVdest=(ULONG *)mynew(&pmem,691200))) goto error;
	for (yindex=0;yindex<480;yindex+=2) {
		for (xindex=0;xindex<720;xindex+=2) {
			unsigned long R,G,B,R1,G1,B1,UYVY;
			R=G=B=*((ULONG *)(dest+(xindex<<2)+2880*yindex));	
			R1=G1=B1=*((ULONG *)(dest+((xindex+1)<<2)+2880*yindex));
			B=B&0x0ff;
			G=G>>8&0x0ff;
			R=R>>16&0x0ff;
			B1=B1&0x0ff;
			G1=G1>>8&0x0ff;
			R1=R1>>16&0x0ff;

			UYVY=(((28770*B-19071*G-9699*R)>>16)+128)+\
			 (((16843*R+33030*G+6423*B)>>8&65280)+4096)+\
			 (((28770*R1-24117*G1-4653*B1)&16711680)+8388608)+\
			 (((16843*R1+33030*G1+6423*B1)<<8&4278190080)+268435456);
			*YUVdest++=UYVY;
			} //end YUV conversion scope
		}
	for (yindex=1;yindex<480;yindex+=2) {
		for (xindex=0;xindex<720;xindex+=2) {
			unsigned long R,G,B,R1,G1,B1,UYVY;
			R=G=B=*((ULONG *)(dest+(xindex<<2)+2880*yindex));	
			R1=G1=B1=*((ULONG *)(dest+((xindex+1)<<2)+2880*yindex));
			B=B&0x0ff;
			G=G>>8&0x0ff;
			R=R>>16&0x0ff;
			B1=B1&0x0ff;
			G1=G1>>8&0x0ff;
			R1=R1>>16&0x0ff;

			UYVY=(((28770*B-19071*G-9699*R)>>16)+128)+\
			 (((16843*R+33030*G+6423*B)>>8&65280)+4096)+\
			 (((28770*R1-24117*G1-4653*B1)&16711680)+8388608)+\
			 (((16843*R1+33030*G1+6423*B1)<<8&4278190080)+268435456);
			*YUVdest++=UYVY;
			} //end YUV conversion scope
		}
	if (dest) dispose((struct memlist *)dest,&pmem);
	*/
	return (0);
error:
	//We are done with source and tempbuffer so dispose them
	//if (dest) dispose((struct memlist *)dest,&pmem);
	return (1);
	}


void tgaloader::closetga(struct imagelist *media) {
	if (debug) printc("tga");
	if (media->mediatga) dispose((struct memlist *)media->mediatga,&pmem);
	media->mediatga=NULL;
	}


BOOL tgaloader::getframetga(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	//long t=691200;
	ULONG *mediabufindex=mediaptr->mediatga;
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


void tgaloader::unpack (char *source,char *dest,char *eof,char *deof,UBYTE depth) {
	ULONG i,j;
	char *index=(char *)source;

	while ((index<eof)&&(dest<deof)) {
		// ByteRun1 decompression
		// [0..127]   : followed by n+1 bytes of data.
		if (!((*index)&128)) {
			i=(*index+1)*depth;
			index++;
			for(j=0; j<i; j++) {
				if ((index<eof)&&(dest<deof)) *dest++=*index++;
				}
			}
		// [-1..-127] : followed by byte to be repeated (-n)+1 times
		else {
			i=(((*index)&127)+1)*depth;
			index++;
			for(j=0; j<i; j++) {
				if (dest<deof) *dest++=*(index+(j%depth));
				}
			index+=depth;
			}
		}
	} //end unpack


char *tgaloader::TGA2raw (char *source,long size,UWORD *x,UWORD *y) {
	//struct TGAfooter *lpTGAfooter;
	struct TGAheader *lpTGAheader;
	char *ImageID=NULL;
	UBYTE *imagedata;
	char *scanbuf=NULL;
	char *dest=NULL;
	int oldtga=TRUE;
	long advance=0;
	long depth;
	ULONG bytesperpixel;
	enum {rgb,cmap,bw} colormode=rgb;
	BOOL rle=FALSE;
	//Given a pointer to source returns a pointer to dest
	//The raw is BGRA data BGRA BGRA BGRA
	//also gives x and y diminsions
	//returns null if error

	//First determine if TGA is the old or new format by examining the last
	//26 bytes for TRUEVISION-XFILE -This is cheezy for performance but since
	//we're dumping the entire thing to ram it will not matter
	/*
	lpTGAfooter=(struct TGAfooter *)(source+size-26);
	oldtga=strncmp((char *)lpTGAfooter->signature,"TRUEVISION-XFILE",16);
	if (!oldtga) printf("%s\n",lpTGAfooter->signature);
	*/

	lpTGAheader=(struct TGAheader *)source;
	*x=lpTGAheader->imagespecs.width;
	*y=lpTGAheader->imagespecs.height;
	depth=lpTGAheader->imagespecs.depth;
	if (debug) {
		wsprintf(string,"Diminsions %d x %d x %d",*x,*y,depth);
		printc(string);
		}

	//Now to see if there is an Image ID field
	advance=lpTGAheader->idlength;
	if (advance) ImageID=source+sizeof(struct TGAheader);
	advance+=sizeof(struct TGAheader);
	//Now to see if there is a Colormap field
	if (lpTGAheader->colormaptype) {
		advance+= //Heres that stupid non word aligned member
			(lpTGAheader->cmapspecs.cmaplength[0]+lpTGAheader->cmapspecs.cmaplength[1]*256) *
			lpTGAheader->cmapspecs.bitsperentry;
		}
	imagedata=(UBYTE *)source+advance;
	//bytesperpixel=((lpTGAheader->imagespecs.depth+(lpTGAheader->imagespecs.imagedesc&tgaID_ALPHA))>>3);
	bytesperpixel=lpTGAheader->imagespecs.depth>>3;
	if (debug) {
		wsprintf(string,"Alpha Depth=%ld",lpTGAheader->imagespecs.imagedesc&tgaID_ALPHA);
		printc(string);
		print("Image type=");
		}
	switch ((tgaimagetypes)lpTGAheader->imagetype) {
		case tga_noimage: 
			wsprintf(string,"No image data included");
			goto errortga;
		case tga_nocomcmp: 
			if (debug) printc("Uncompressed, color-mapped images");
			rle=FALSE;
			colormode=cmap;
			break;
		case tga_nocomtc:
			if (debug) printc("Uncompressed, RGB images");
			rle=FALSE;
			colormode=rgb;
			break;
		case tga_nocombw:
			if (debug) printc("Uncompressed, black and white images");
			rle=FALSE;
			colormode=bw;
			break;
		case tga_rlecmp:
			if (debug) printc("Runlength encoded color-mapped images");
			rle=TRUE;
			colormode=cmap;
			break;
		case tga_rlergb:
			if (debug) printc("Runlength encoded RGB images");
			rle=TRUE;
			colormode=rgb;
			break;
		case tga_rlebw:
			if (debug) printc("Compressed, black and white images");
			rle=TRUE;
			colormode=rgb;
			break;
		case tga_rlehdcmp:
			printc("Compressed color-mapped data, using Huffman, Delta, and runlength encoding.");
			wsprintf(string,"TGA Huffman Delta compression not supported");
			goto errortga;
		case tga_rlehd2cmp:
			printc("Compressed color-mapped data, using Huffman, Delta,	and runlength encoding.  4-pass quadtree-type process.");
			wsprintf(string,"TGA Huffman Delta compression not supported");
			goto errortga;
		default: 
			wsprintf(string,"Unknown image type");
			goto errortga;
		}
	//Check for conflicting information
	switch (colormode) {
		case rgb:
			if (!((depth==16)||(depth==24)||(depth==32))) printc("Warning: conflicting pixel mode may be corrupt");
			break;
		case cmap:
			//TODO parse this
			break;
		case bw:
			if (depth>8) printc("Warning: conflicting pixel mode may be corrupt");
			break;
		}
	//Here's the main conversion loop * * * *
	{
		char *destindex=NULL;
		char *scanbufindex,*scanrow;
		long direction,row;
		ULONG scanwidthbytes;
		UWORD xindex=0;
		ULONG scandepth;
		ULONG scansize;
		ULONG ycount;
		UWORD top;

		//Allocate a scanline x depth source buffer
		scanrow=0;
		scandepth=bytesperpixel;
		scanwidthbytes=*x*bytesperpixel;  //Number of bytes including the pad
		//Allocate the destination RAW BGRA final output
		if (rle) {
			scansize=*x*(*y)*scandepth;
			if (!(scanbuf=(char *)mynew(&pmem,scansize))) {
				wsprintf(string,"Not enough memory available");
				goto errortga;
				}
			unpack((char *)imagedata,scanbuf,(char *)(imagedata+size),(char *)(scanbuf+scansize),(UBYTE)scandepth);
			}
		else scanbuf=(char *)imagedata;

		if (!(dest=destindex=(char *)mynew(&pmem,*x*(*y)<<2))) {
			wsprintf(string,"Not enough memory available");
			goto errortga;
			}
		/*Loop through rows*/ 
		scanbufindex=scanbuf;
		if (lpTGAheader->imagespecs.imagedesc&tgaID_TOP) {
			top=0;
			direction=1;
			}
		else {
			top=(*y)-1;
			direction=-1;
			}
		for (ycount=0,row=top;ycount<*y;row+=direction,ycount++) {
			//At this point the scanbuf points to uncompressed pixels
			//we'll interpret this data depending upon the color mode to extract the pixel
			//This will loop x, and fill a destination scan line
			xindex=0;
			scanrow=scanbuf+row*scanwidthbytes;
			for (scanbufindex=scanrow;scanbufindex<scanrow+scanwidthbytes;scanbufindex+=scandepth) {
				UBYTE R,G,B,A;
	
				switch(colormode) {
					case rgb:
						switch (depth) {
							case 16:
								//ARRRRRGG+1 GGGBBBBB
								A=*scanbufindex+1;
								R=(A&124)<<1; //1111100
								B=*scanbufindex;
								G=((A&3)<<6)|((B&224)>>2);
								B=B<<3;
								A=A&128;
							case 24:
								B=*scanbufindex;
								G=*(scanbufindex+1);
								R=*(scanbufindex+2);
								A=128;
								break;
							case 32:
								B=*scanbufindex;
								G=*(scanbufindex+1);
								R=*(scanbufindex+2);
								A=*(scanbufindex+3);
								break;
							}
						break;
					case cmap:
						R=B=G=0;
						break;
					case bw:
						R=G=B=*scanbufindex;
						break;
					} //end switch colormode

				if (++xindex<=*x) {
					//put RGB onto the dest
					*destindex++=B;
					*destindex++=G;
					*destindex++=R;
					*destindex++=A; //alpha
					}
				} //loop bytes
			} //end loop rows
		} //end main loop block
	//cleanup other allocated memory
	if ((scanbuf)&&(rle)) dispose((struct memlist *)scanbuf,&pmem);
	return (dest);
errortga:
	printc(string);
	if (dest) dispose((struct memlist *)dest,&pmem);
	if (scanbuf) dispose((struct memlist *)scanbuf,&pmem);
	return(dest=0);  //elogant I know
	}
