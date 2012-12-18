#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h> //added to compile for NT
#include "TIFconsole.h"

struct memlist *pmem=NULL;
struct nodevars *nodeobject;
char string[256];
char *source;
long size;
BOOL amiga=NULL;

ULONG fliplong(ULONG x) {
	__asm {
		mov	eax,x
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		mov	x,edx
		}
	return (x);
	}

UWORD flipword(UWORD x) {
	__asm {
		mov	ax,x
		mov	dl,al
		shr	eax,8
		shl	edx,8
		mov	dl,al
		mov	x,dx
		}
	return (x);
	}

UWORD getword(UWORD *x) {
	if (amiga) return (flipword(*x));
	else return (*x);
	}

ULONG getlong(ULONG *x) {
	if (amiga) return (fliplong(*x));
	else return (*x);
	}

void skiptotag(int *tag,int desttag,ULONG *IFDcount,char **sourceindex) {
	while (*tag<desttag&&*IFDcount) {
		printf("tag %d not supported\n");
		*IFDcount--;
		*sourceindex+=12;
		*tag=(int)getword((UWORD *)*sourceindex);
		}
	}

void advancetag(int *tag,ULONG *IFDcount,char **sourceindex) {
	(*IFDcount)--;
	*sourceindex+=12;
	*tag=(int)getword((UWORD *)*sourceindex);
	}

char *TIF2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo) {
	//const long pullcolor[2]={0x10001000,0x10001000};
	//Given a pointer to source returns a pointer to dest
	//returns bitmapbits.. and a pointer to a BITMAPINFO struct
	//returns null if error
	/*
	char *encodedbits,*destindex,*dest=NULL;
	struct PCXheader *header=(struct PCXheader *)source;
	LPBITMAPINFOHEADER bmi;
	char *bmipalette;
	char *pcxpalette;

	*bitmapinfo=bmi;
	return (dest);
errortif:
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	*bitmapinfo=NULL;
	if (dest) dispose((struct memlist *)dest,&pmem);
	*/
	int tag;
	ULONG IFDcount,advance;
	char *sourceindex;
	//Check first 2 bytes for order of pointers
	if (*((UWORD *)source)==0x4d4d) amiga=1;
	//Check 42 for tif signature
	if (!(*(source+2+amiga)==42)) {
		wsprintf(string,"This file is corrupted TIF file");
		goto errortif;
		}
	advance=getlong((ULONG *)(source+4));
	//Ok here starts the IFD block... the tags are sorted in asending order so we'll optimise
	//by not switching this block, will iterate a link list for each IFD block on multiple
	//images; however I am going to assume that the first IFD block will have all the
	//information needed to process the image so the loop will remain commented out
	//do {
		sourceindex=source+advance;
		IFDcount=getword((UWORD *)sourceindex);
		sourceindex+=2;
		// I am going to assume that no tag is going to be repeated
		// we'll need to research this
		tag=(int)getword((UWORD *)sourceindex);
		/*
		Type
		1 = BYTE  An 8-bit unsigned integer.
		2 = ASCII 8-bit bytes  that store ASCII codes; the last byte must be null.
		3 = SHORT A 16-bit (2-byte) unsigned integer.
		4 = LONG  A 32-bit (4-byte) unsigned integer.
		5 = RATIONAL   Two LONG_s:  first numerator; second denominator.
		*/

		//Check of interest 254
		if (tag==254) {
			printf("NewSubFileType %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		if (tag==255) {
			printf("Subfiletype %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		//Must have 256-259
		if (tag==256) {
			printf("ImageWidth %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//For robust reasons.. we need to check type and pull 
			//either short or long the actual values
			}
		if (tag==257) {
			printf("ImageLength %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//same here this is the number of rows
			}

		if (tag==258) {
			printf("BitsPerSample %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//this should always be 8 for what we are doing if not exit no support
			}
		if (tag==259) {
			printf("Compression %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//LZW that is yours Dan it appears most of these are using LZW
			//I've already made several versions of the packed bits compression
			//after we get LZW and everything else working we'll do the rest when appropriate.
			//NOTE: Here's where jpg would be... we should be able to use the library for 
			//this case
			}


		skiptotag(&tag,262,&IFDcount,&sourceindex);

		//Must Have 262
		if (tag==262) {
			printf("PhotometricInterpretation %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			//This is like pixel format or colorspace... for now lets focus on GrayScale
			//and RGB palette, RGB... these are values 0-3
			advancetag(&tag,&IFDcount,&sourceindex);
			}


		skiptotag(&tag,270,&IFDcount,&sourceindex);

		//Not of interest 270
		if (tag==270) {
			printf("ImageDescription %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			//printf("\n%s\n\n",source+getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		skiptotag(&tag,273,&IFDcount,&sourceindex);

		//Must have 273
		if (tag==273) {
			printf("StripOffsets %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//Short or Long
			}

		skiptotag(&tag,277,&IFDcount,&sourceindex);

		//Must have 277-279
		if (tag==277) {
			printf("SamplesPerPixel %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//3  for RGB (see 338 extra) 2 for YUV or 16bit... and 1 for 8 bit for palette or gray scale
			//Let's keep whichever native bitsperpixel they use here...
			}
		if (tag==278) {
			printf("RowsPerPixel %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//Short or Long
			}
		if (tag==279) {
			printf("StripByteCounts %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//Short or Long
			}

		skiptotag(&tag,282,&IFDcount,&sourceindex);

		//Not of interest 282,283
		if (tag==282) {
			printf("XResolution %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}
		if (tag==283) {
			printf("YResolution %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		if (tag==284) {
			printf("PlanerConfiguration %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//We should not encounter this... but lets check for planar if planar print not
			//supported at that point I can test that .tif with the routines I wrote for pcx
			//this is a low priority... but we should test and error out for now...
			}

		skiptotag(&tag,296,&IFDcount,&sourceindex);

		//Check of interest 296
		if (tag==296) {
			printf("ResolutionUnit %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		skiptotag(&tag,317,&IFDcount,&sourceindex);

		if (tag==317) {
			printf("Predictor %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		//Keep an eye out for this... we may not have any tif's that use palettes yet
		if (tag==320) {
			printf("ColorMap %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}
		//ok 32bit vs 24bit here
		if (tag==338) {
			printf("ExtraSamples %d type %d count %ld value %ld\n",tag,getword((UWORD *)(sourceindex+2)),getlong((ULONG *)(sourceindex+4)),getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//This is how to tell if is is 24 or 32... the extra could be alpha byte when filling
			//out the bitmapinfo struct... check this when setting bitsperpixel
			}

		sourceindex+=(IFDcount*12);
	//	} while(advance=getlong((ULONG *)(sourceindex)));
	return (0);
errortif:
	printf("%s\n",string);
	//printc(string);
	return (0);
	}


void writebmp(char *filename,LPBITMAPINFOHEADER bmi,char *bitmapbits) {
	ULONG palettesize,offsetbits,filesize,biandpal;
	char *source,*sourceindex;
	//first figure out what size the file will be
	//We need to figure out if there is a palette and if so how many colors
	if (bmi->biBitCount<=16) {
		if (bmi->biClrUsed) palettesize=bmi->biClrUsed<<2;
		else palettesize=1024;
		}
	else palettesize=0;
	//fill in the bitmapfileheader struct
	biandpal=sizeof(BITMAPINFOHEADER)+palettesize;
	offsetbits=sizeof(BITMAPFILEHEADER)+biandpal;
	filesize=offsetbits+bmi->biSizeImage;
	source=sourceindex=(char *)mynew(&pmem,filesize);
	((LPBITMAPFILEHEADER)source)->bfType=0x4d42;
	((LPBITMAPFILEHEADER)source)->bfSize=filesize;
	((LPBITMAPFILEHEADER)source)->bfReserved1=0;
	((LPBITMAPFILEHEADER)source)->bfReserved2=0;
	((LPBITMAPFILEHEADER)source)->bfOffBits=offsetbits;
	//now to copy the other fields
	sourceindex+=sizeof(BITMAPFILEHEADER);
	memcpy(sourceindex,bmi,biandpal);
	sourceindex+=biandpal;
	memcpy(sourceindex,bitmapbits,bmi->biSizeImage);
	save(filename,filesize,source);
	}


void init() {
	nodeobject=createnode(&pmem,65536,0);
	//printf("What filename? ");
	//scanf("%s",&string);
	wsprintf(string,"tiff\\coolwipe\\patterns\\BDOORS1.tif");
	source=NULL;
	source=(char *)load(string,&size,&pmem);
	} //end init


void cleanup() {
	if (pmem) killnode(nodeobject,&pmem);
	if (pmem) disposeall(&pmem);
	exit(TRUE);
	}


void main() {
	LPBITMAPINFOHEADER bitmapinfo;
	char *bitmapbits=NULL;

	//printf("Main\n");
	//do {
		init();
		if (source) {
			bitmapbits=TIF2raw(source,size,&bitmapinfo);
			//writebmp("Test.bmp",bitmapinfo,bitmapbits);
			}
		else printf("File not found\n");

		//printf("\nDo Another (y/n)? ");
		//scanf("%s",&string);
		//} while (!(stricmp(string,"y")));

	cleanup();
	} //end main

