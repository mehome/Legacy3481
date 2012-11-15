#include "handlemain.h"
#include "UnLZWstrip.h"


void loadersclass::loaderscommon::makegraypalette (char *dest) {
	__asm {
		mov		edi,dest
		pxor		mm1,mm1	//mm1 = ________________
		pcmpeqb	mm3,mm3	//mm3 = ffffffffffffffff
		psrlw		mm3,15	//mm3 = ___1___1___1___1
		packuswb	mm1,mm3	//mm1 = _1_1_1_1________
		packuswb	mm3,mm3	//mm3 = _1_1_1_1_1_1_1_1
		paddb		mm3,mm3	//mm3 = _2_2_2_2_2_2_2_2
		mov		ecx,256
		shr		ecx,3		//8 colors at a time= 32
loopgrey:
		movq		[edi],mm1
		paddb		mm1,mm3
		movq		[edi+8],mm1
		paddb		mm1,mm3
		movq		[edi+16],mm1
		paddb		mm1,mm3
		movq		[edi+24],mm1
		paddb		mm1,mm3
		add		edi,32
		dec		ecx
		jne		loopgrey
		emms
		}
	}


BOOL loadersclass::loaderscommon::raw2yuv(struct imagelist *media,LPBITMAPINFOHEADER lpBitmapInfoHeader,char *decodedbits,BOOL upsidedown)
	{
	ULONG *YUVdest;
	int x,y,scalex,scaley;

	if (!(media->mediabmp=YUVdest=(ULONG *)mynew(&pmem,691200))) goto error;
	//begin check scaling 
	x=lpBitmapInfoHeader->biWidth;
	y=lpBitmapInfoHeader->biHeight;
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
	scalex=min(scalex,scaley);
	scaley=scalex;
	//end scaleconfiger
	switch (lpBitmapInfoHeader->biBitCount) {
		case 8:
			if (scalex>1) renderrgb8scale(YUVdest,x,y,scalex,scaley,decodedbits,lpBitmapInfoHeader);
			else renderrgb8(YUVdest,x,y,decodedbits,lpBitmapInfoHeader,upsidedown);
			break;
		case 24:
			//printc("24");
			if (scalex>1) renderrgb24scale(YUVdest,x,y,scalex,scaley,decodedbits);
			else renderrgb24(YUVdest,x,y,decodedbits,upsidedown);
			break;
		case 32:
			//printc("32");
			if (scalex>1) renderrgb32scale(YUVdest,x,y,scalex,scaley,decodedbits,upsidedown);
			else renderrgb32(YUVdest,x,y,decodedbits,upsidedown);
			break;
		default:
			printc("Unable to render uncompressed format");
			return(FALSE);
		}

	return (0);
error:
	if (YUVdest) dispose((struct memlist *)YUVdest,&pmem);
	return (1);
	}


BOOL loadersclass::tifclass::opentif(struct imagelist *media) {
	LPBITMAPINFOHEADER lpBitmapInfoHeader;
	char *source;
	char *decodedbits;
	BOOL result;
	long size;

	if (debug) printc("TIFF");
	if (!(source=(char *)load(media->filesource,&size,&pmem))) return(1);
	decodedbits=TIF2raw(source,size,&lpBitmapInfoHeader);
	if (source) dispose((struct memlist *)source,&pmem);
	result=raw2yuv(media,lpBitmapInfoHeader,decodedbits,FALSE);
	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	if (lpBitmapInfoHeader) disposenode(nodeobject,(struct memlist *)lpBitmapInfoHeader);
	return (result);
	}


//TODO check if bmi needs to be disposed
void loadersclass::tifclass::closetif(struct imagelist *media) {
	if (debug) printc("Bitmap");
	if (media->mediabmp) dispose((struct memlist *)media->mediabmp,&pmem);
	media->mediabmp=NULL;
	}


HBITMAP loadersclass::tifclass::getthumbtif(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo) {
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
	if ((decodedbits=TIF2raw(source,size,bitmapinfo))==0) return(FALSE);

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


UWORD loadersclass::tifclass::Getword(UWORD *x) {
	if (amiga) return (flipword(*x));
	else return (*x);
	}


ULONG loadersclass::tifclass::Getlong(ULONG *x) {
	if (amiga) return (fliplong(*x));
	else return (*x);
	}


void loadersclass::tifclass::bgr2rgb(char *dest,ULONG SizeImage) {
	const ULONG onefour=0xFF0000FF;
	__asm {
		mov edi,dest
		mov ecx,SizeImage
again:
		movq		mm0,[edi]		//mm0 = ____B2G2R2B1G1R1
		movq		mm1,mm0			
		movd		mm3,onefour		//mm3 = ________ff____ff
		pand		mm1,mm3			//mm1 = ________R2____R1
		psllq		mm1,16			//mm1 = ____R2____R1____
		movq		mm2,mm0			
		psllq		mm3,16			//mm3 = ____ff____ff____
		pand		mm2,mm3			//mm2 = ____B2____B1____
		psrlq		mm2,16			//mm2 = ________B2____B1
		movq		mm4,mm0
		psrlq		mm3,8				//mm3 = ______ff____ff__
		pcmpeqb	mm5,mm5			//mm5 = ffffffffffffffff
		psllq		mm5,48			//mm5 = ffff____________
		por		mm3,mm5			//mm3 = ffff__ff____ff__
		pand		mm4,mm3			//mm4 = G3R3__G2____G1__
		por		mm4,mm2			//mm4 = G3R3__G2B2__G1B1
		por		mm4,mm1			//mm4 = G3R3R2G2B2R1G1B1
		movq		[edi],mm4
		add		edi,6
		sub		ecx,6
		cmp		ecx,0
		jg			again
		emms
		}
	}

void loadersclass::tifclass::skiptotag(unsigned int *tag,unsigned int desttag,ULONG *IFDcount,char **sourceindex) {
	while (*tag<desttag&&*IFDcount) {
		if (debug) printf("tag %d not supported\n");
		*IFDcount--;
		*sourceindex+=12;
		*tag=(int)Getword((UWORD *)*sourceindex);
		}
	}


void loadersclass::tifclass::advancetag(unsigned int *tag,ULONG *IFDcount,char **sourceindex) {
	(*IFDcount)--;
	*sourceindex+=12;
	*tag=(int)Getword((UWORD *)*sourceindex);
	}


char *loadersclass::tifclass::TIF2raw (char *source, long size, LPBITMAPINFOHEADER *bitmapinfo ) {
	//const long pullcolor[2]={0x10001000,0x10001000};
	//Given a pointer to source returns a pointer to dest
	//returns bitmapbits.. and a pointer to a BITMAPINFO struct
	//returns null if error
	unsigned int tag, i, numstrips=0,bcount=0, striptype, bpp;
	ULONG IFDcount, advance, rowspstrip,SizeImage,predict;
	ULONG width=0, height=0, bps=0, cmprs=0, photoint=0;
	ULONG *stripindex=NULL, *bcountindex=NULL;
	char *currstrip=NULL, *newstrip=NULL, *dest=NULL, *sourceindex=NULL;
	UINT striplength;
	LPBITMAPINFOHEADER bmi=(LPBITMAPINFOHEADER)newnode(nodeobject,sizeof(BITMAPINFOHEADER)+1024);

	bmi->biSize=sizeof(BITMAPINFOHEADER);
	//Check first 2 bytes for order of pointers
	amiga=0;
	if (*((UWORD *)source)==0x4d4d) amiga=1;
	
	//Check 42 for tif signature
	if (!(*(source+2+amiga)==42)) {
		wsprintf(string,"This file is corrupted TIF file");
		goto errortif;
		}

	advance=Getlong((ULONG *)(source+4));
	//Ok here starts the IFD block... the tags are sorted in asending order so we'll optimise
	//by not switching this block, will iterate a link list for each IFD block on multiple
	//images; however I am going to assume that the first IFD block will have all the
	//information needed to process the image so the loop will remain commented out
	//do {
		sourceindex=source+advance;
		IFDcount=Getword((UWORD *)sourceindex);
		sourceindex+=2;
		
		// I am going to assume that no tag is going to be repeated
		// we'll need to research this
		tag=(int)Getword((UWORD *)sourceindex);
		/*
		Type
		1 = BYTE	An 8-bit unsigned integer.
		2 = ASCII 8-bit bytes	that store ASCII codes; the last byte must be null.
		3 = SHORT A 16-bit (2-byte) unsigned integer.
		4 = LONG	A 32-bit (4-byte) unsigned integer.
		5 = RATIONAL	 Two LONG_s:	first numerator; second denominator.
		*/

		//Check of interest 254
		if (tag==254) {
			if (debug) printc("NewSubFileType %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		if (tag==255) {
			if (debug) printc("Subfiletype %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getword((UWORD *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		//Must have 256-259
		if (tag==256) {
			if ( Getword((UWORD *)(sourceindex+2)) == 3 ) width = Getword((UWORD *)(sourceindex+8));
			 else width = Getlong((ULONG *)(sourceindex+8));
			bmi->biWidth=width;
			if (width%4) {
				wsprintf(string,"Error: Tif Image width must be a multiple of 4");
				goto errortif;
				}
			if (debug) printc("ImageWidth %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),width);
			advancetag( &tag,&IFDcount,&sourceindex );
			//For robust reasons.. we need to check type and pull
			//either short or long the actual values
			}
		
		if (tag==257) {
			if ( Getword((UWORD *)(sourceindex+2)) == 3 ) height = Getword((UWORD *)(sourceindex+8));
			 else height = Getlong((ULONG *)(sourceindex+8));
			bmi->biHeight=height;
			if (debug) printc("ImageLength %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),height);
			advancetag( &tag,&IFDcount,&sourceindex );
			//same here this is the number of rows
			}

		if (tag==258) {
			bps = Getword( (UWORD *)(sourceindex+8) );
			if (debug) printc("BitsPerSample %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),bps);
			i = (UWORD)bps;
			advancetag(&tag,&IFDcount,&sourceindex);
			//this should always be 8 for what we are doing if not exit no support
			}
		
		if (tag==259) {
			cmprs = Getword( (UWORD *)(sourceindex+8) );
			if (debug) printc("Compression %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),cmprs);
			advancetag(&tag,&IFDcount,&sourceindex);
			//NOTE: Here's where jpg would be... we should be able to use the library for
			//this case
			}

		skiptotag(&tag,262,&IFDcount,&sourceindex);

		//Must Have 262
		if (tag==262) {
			photoint = Getword((UWORD *)(sourceindex+8));
			if (debug) printc("PhotometricInterpretation %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),photoint);
			//This is like pixel format or colorspace... for now lets focus on GrayScale
			//and RGB palette, RGB... these are values 0-3
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		skiptotag(&tag,270,&IFDcount,&sourceindex);

		//Not of interest 270
		if (tag==270) {
			if (debug) printc("ImageDescription %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			if (debug==2) printc("%s",source+Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		skiptotag(&tag,273,&IFDcount,&sourceindex);

		//Must have 273
		if (tag==273) {
			striptype = Getword((UWORD *)(sourceindex+2));
			numstrips = Getlong((ULONG *)(sourceindex+4));
			if ( striptype == 3 )	stripindex = (ULONG *)(source + Getword((UWORD *)(sourceindex+8)));			
			else stripindex = (ULONG *)(source + Getlong((ULONG *)(sourceindex+8)));			
			if (debug) printc("StripOffsets %d type %d count %ld value %ld", tag, striptype, numstrips, stripindex);
			advancetag(&tag,&IFDcount,&sourceindex);
			//Short or Long
			}

		skiptotag(&tag,277,&IFDcount,&sourceindex);

		//Must have 277-279
		if (tag==277) {
			bpp = Getword((UWORD *)(sourceindex+8)); // bites per pixel.
			if (debug) printc("SamplesPerPixel %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getword((UWORD *)(sourceindex+4)),bpp);
			advancetag(&tag,&IFDcount,&sourceindex);
			bmi->biBitCount=bpp*8;
			//3	for RGB (see 338 extra) 2 for YUV or 16bit... and 1 for 8 bit for palette or gray scale
			//Let's keep whichever native bitsperpixel they use here...
			}
		
		if (tag==278) {
			if ( Getword((UWORD *)(sourceindex+2)) == 3 )	rowspstrip = Getword((UWORD *)(sourceindex+8));			
			 else rowspstrip = Getlong((ULONG *)(sourceindex+8));			
			if (debug) printc("RowsPerStrip %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),rowspstrip);
			advancetag(&tag,&IFDcount,&sourceindex);
			//Short or Long
			}

		if (tag==279) {
			if ( Getword((UWORD *)(sourceindex+2)) == 3 )	bcountindex = (ULONG *)(source + Getword((UWORD *)(sourceindex+8)));			
			 else bcountindex = (ULONG *)(source + Getlong((ULONG *)(sourceindex+8)));
			if (debug) printc("StripByteCounts %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),bcountindex);
			advancetag(&tag,&IFDcount,&sourceindex);
			//Short or Long
			}

		skiptotag(&tag,282,&IFDcount,&sourceindex);

		//Not of interest 282,283
		if (tag==282) {
			if (debug) printc("XResolution %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}
		
		if (tag==283) {
			if (debug) printc("YResolution %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		if (tag==284) {
			if (debug) printc("PlanerConfiguration %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getword((UWORD *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//We should not encounter this... but lets check for planar if planar print not
			//supported at that point I can test that .tif with the routines I wrote for pcx
			//this is a low priority... but we should test and error out for now...
			}

		skiptotag(&tag,296,&IFDcount,&sourceindex);

		//Check of interest 296
		if (tag==296) {
			if (debug) printc("ResolutionUnit %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		skiptotag(&tag,317,&IFDcount,&sourceindex);

		if (tag==317) {
			predict = Getlong((ULONG *)(sourceindex+8));
			if (debug) printc("Predictor %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),predict);
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		//Keep an eye out for this... we may not have any tif's that use palettes yet
		if (tag==320) {
			if (debug) printc("ColorMap %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			}

		//ok 32bit vs 24bit here
		if (tag==338) {
			if (debug) printc("ExtraSamples %d type %d count %ld value %ld",tag,Getword((UWORD *)(sourceindex+2)),Getlong((ULONG *)(sourceindex+4)),Getlong((ULONG *)(sourceindex+8)));
			advancetag(&tag,&IFDcount,&sourceindex);
			//This is how to tell if is is 24 or 32... the extra could be alpha byte when filling
			//out the bitmapinfo struct... check this when setting bitsperpixel
			}

		striplength=width*rowspstrip*bpp;
		SizeImage=width*height*bpp;
		bmi->biPlanes=1;
		bmi->biXPelsPerMeter = 4;
		bmi->biYPelsPerMeter = 3;
		bmi->biSizeImage=SizeImage;
		bmi->biClrImportant = 0;
		bmi->biClrUsed = 0;
		//Seems that 
		//init gray palette for gray scale images
		if ((photoint==0)||(photoint==1)) {
			makegraypalette((char *)bmi+sizeof(BITMAPINFOHEADER));
			}
		//TODO copy color index for palette images

		//Make memory avaible here.
		dest = (char *)mynew(&pmem,SizeImage+striplength);
		newstrip = dest;
		//goto errortif;
		//main loop
		switch (cmprs) {
			case 1:
				for( i = 0; i < numstrips; i++)	{
					if (numstrips==1) currstrip=(char *)stripindex;
					else if ( striptype == 3 )	currstrip = (source + Getword( (UWORD *)(stripindex+i) ));
					else currstrip = (source + Getlong( (ULONG *)(stripindex+i)) );		
					memcpy(newstrip,currstrip,striplength);
					newstrip += striplength;
					} // end for
				break;
			case 5: {
				UnLZWclass unlzw(predict,bpp,width,rowspstrip);
				for( i = 0; i < numstrips; i++)	{
					if (numstrips==1) currstrip=(char *)stripindex;
					else if ( striptype == 3 )	currstrip = (source + Getword( (UWORD *)(stripindex+i) ));
					else currstrip = (source + Getlong( (ULONG *)(stripindex+i)) );		
					unlzw.UnLZWstrip(currstrip,newstrip);
					newstrip += striplength;
					} // end for
				break;
				}
			default:
			wsprintf(string,"unsupported TIFF compression");
			goto errortif;
			}
		//test the RGB flipped
		if ((photoint==2)&&(bpp==3)) bgr2rgb(dest,SizeImage);
		sourceindex+=(IFDcount*12);
	//	} while(advance=getlong((ULONG *)(sourceindex)));
	*bitmapinfo=bmi;
	return (dest);
errortif:
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	*bitmapinfo=NULL;
	if (dest) dispose((struct memlist *)dest,&pmem);
	//printf("%s\n",string);
	printc(string);
	return (0);
	}


