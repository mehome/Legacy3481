/*
TODO:
Scaling
*/
#include "HandleMain.h"  
#include "loaderiff.h"

typedef loadersclass::iffclass iffloader;

HBITMAP iffloader::getthumbiff(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo) {
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
	char *tempbuffer=NULL,*source;
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
		tempbuffer=ILBM2raw(source,size,&x2,&y2);
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


BOOL iffloader::openiff(struct imagelist *media) {
/**/
	ULONG *YUVdest;
	char *tempbuffer,*source;
	long size=0;
	ULONG x,y;
	UWORD x2,y2;

	if (debug) printc("IFF");
	media->mediaiff=NULL; //TODO: see if this is already init
	if (!(source=(char *)(load(media->filesource,&size,&pmem)))) goto error;
	if (!(tempbuffer=ILBM2raw(source,size,&x2,&y2))) goto error;
	x=(ULONG)x2;y=(ULONG)y2; //optimise make them long
	dispose((struct memlist *)source,&pmem);


	if (!(media->mediaiff=YUVdest=(ULONG *)mynew(&pmem,691200))) goto error;
	renderrgb32(YUVdest,x,y,tempbuffer,0);
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

	if (debug) printc("IFF");
	media->mediaiff=NULL; //TODO: see if this is already init
	if (!(source=(char *)(load(media->filesource,&size,&pmem)))) goto error;
	if (!(tempbuffer=ILBM2raw(source,size,&x2,&y2))) goto error;
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
	if (!(media->mediaiff=YUVdest=(ULONG *)mynew(&pmem,691200))) goto error;
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


void iffloader::closeiff(struct imagelist *media) {
	if (debug) printc("IFF");
	if (media->mediaiff) dispose((struct memlist *)media->mediaiff,&pmem);
	media->mediaiff=NULL;
	}


BOOL iffloader::getframeiff(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage) {
	//long t=691200;
	ULONG *mediabufindex=mediaptr->mediaiff;
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


UWORD iffloader::unpackiffbyterun (char *source,char *dest,char *eof,short xbytes) {
	UBYTE i,j;
	char *index=source;

	while( xbytes>0 && (index<eof)) {
		// ByteRun1 decompression
		// [0..127]   : followed by n+1 bytes of data.
		if (*index>=0) {
			i=*index+1;
			for(j=0; j<i; j++) {
				index++;
				xbytes--;
				if (index<eof) *dest++=*index;
				}
			}
		// [-1..-127] : followed by byte to be repeated (-n)+1 times
		else if (*index<=-1 && *index>=-127) {
			i=(-*index)+1;
			index++;
			for(j=0; j<i; j++) {
				xbytes--;
				if (index<eof) *dest++=*index;
				}
			}
		index++;	// -128 = NOP. also advances for all other cases
		}
	return((UWORD)(index-source));
	} //end unpack


char *iffloader::ILBM2raw (char *source,long size,UWORD *x,UWORD *y) {
	//Given a pointer to source returns a pointer to dest
	//The raw is BGRA data BGRA BGRA BGRA
	//also gives x and y diminsions
	//returns null if error
	long formsize,t;
	ULONG advance,modeid;
	char *scanbuf=NULL;
	char *dest=NULL;
	UBYTE *index,*finalsize;
	struct BitmapHeader *BMHD=NULL;
	struct ColorRegister *CMAP=NULL;
	UWORD maxcolors=0;

	modeid=0;
	index=(UBYTE *)source;
	//First test that this is an iff ILBM
	if ((*((ULONG *)source)!=ID_FORM)||(*(((ULONG *)source)+2)!=ID_ILBM)) {
		wsprintf(string,"Not Iff file");
		goto erroriff;
		}
	//We'll need both the length and the size, they should both
	//be equal but if not use the minimum value.  For example
	//If length is smaller than size this means the file could
	//have been trunicated, and in the other case the iff contains
	//more data and the remainder can be ignored.
	formsize=fliplong(*(((ULONG *)source)+1));
	finalsize=(UBYTE *)source+min(formsize,size-8);
	if ((size-8)<formsize) printc("Warning: IFF file is incomplete the remainder will be padded with zeros");
	t=formsize;
	if (t&1) t++;
	else if (t<(size-8)) {
		printc("Warning: IFF file size is larger than what is specified,");
		printc("this file may be corrupt");
		}
	if (debug) {
		wsprintf(string,"Actual file size=%ld",size);printc(string);
		wsprintf(string,"FORM size (total chunks)=%ld",formsize);printc(string);
		}
	index=(UBYTE *)source+12;
	//Now we can parse the chunks
	while (index<finalsize) {
		advance=fliplong(*(((ULONG *)index)+1));

		switch (*((ULONG *)index)) {
		//Were looking for BMHD CMAP CAMG ANNO and BODY only BMHD and BODY are manditory
			case ID_BMHD:
				BMHD=(struct BitmapHeader *)(index+8);
				*x=flipword(BMHD->w);
				*y=flipword(BMHD->h);
				break;

			case ID_CMAP:
				CMAP=(struct ColorRegister *)(index+8);
				maxcolors=(UWORD)advance/3;//maxcolors shouldn't exceed 5 bit planes
				//maxcolors is 1 based
				break;

			case ID_CAMG:
				//Determine if we need to alter the pixel aspect to 1:1
				//This will apply only to the NTSC/PAL 1.3 modes
				modeid=fliplong(*(((ULONG *)index)+2));
				//Knock bad bits out of old-style CAMG modes before checking availability.
				//(some ILBM CAMG's have these bits set in old 1.3 modes, and should not)
				//If not an extended monitor ID, or if marked as extended but missing
				//upper 16 bits, screen out inappropriate bits now.
				if ((!(modeid&MONITOR_ID_MASK))||
					((modeid&EXTENDED_MODE)&&(!(modeid&0xFFFF0000))))
					modeid&=(~(EXTENDED_MODE|SPRITES|GENLOCK_AUDIO|GENLOCK_VIDEO|VP_HIDE));
				//Check for bogus CAMG like some brushes have, with junk in
				//upper word and extended bit NOT set not set in lower word.
				if ((modeid&0xFFFF0000)&&(!(modeid&EXTENDED_MODE))) {
					modeid=NULL;
					if (*x>=640) modeid|=HIRES;
					if (*y>=400) modeid|=LACE;
					}
				//Now we can assume modeid conforms to the 2.0 32bit standard
				break;

			case ID_ANNO:
				strncpy(string,(char *)(index+8),min(advance,255));
				string[advance]=0;
				if (debug) printc(string);
				break;

			case ID_BODY: {
				char *destindex=NULL;
				char *scanbufindex;
				UBYTE *chunkindex=NULL;
				ULONG scanwidthbytes,scanrow,scalexwidth;
				enum {lo,hi,super} moderesolution; //1=Lo 2=Hi 3=Super
				enum {indexed,ehb,ham6,ham8,iff24} colormode;
				enum hammodes {normal,bham,rham,gham} hammode;
				UWORD xindex=0;
				UBYTE scandepth;
				UBYTE	oldR=0;
				UBYTE	oldG=0;
				UBYTE	oldB=0;
				char scandepthindex;
				char scalex,scaley;

				moderesolution=lo;
				colormode=indexed;
				scalex=scaley=1;
				//first make sure there is at least BMHD
				if (!BMHD) {
					wsprintf(string,"No BitmapHeader information found for this IFF");
					goto erroriff;
					}
				//now check for CMAP none required is for 24bit planes
				//Note we do not support indexing past 8 bits,
				//and so it will error on a 24 with a CMAP
				if (BMHD->nPlanes>8&&CMAP) {
					wsprintf(string,"Only up to 8 indexed bitplanes are supported");
					goto erroriff;
					}
				if (!CMAP) {
					//TODO make a default gray scale palette
					//or have user load a palette
					if (BMHD->nPlanes<24) {
						wsprintf(string,"No palette detected for color indexed bitmap");
						goto erroriff;
						}
					else {
						colormode=iff24;
						 //Turn HAM and Halfbrite off if they are on
						if (modeid&HAM) modeid^=HAM;
						if (modeid&EXTRA_HALFBRITE) modeid^=EXTRA_HALFBRITE;
						}
					}
				else {
					if (debug) {
						wsprintf(string,"%d palette colors defined",maxcolors);
						printc(string);
						}
					//Ensure that there are enough colors for bitplanes
					if (modeid&HAM) {
						colormode=ham6;
						//ensure we have 6 and 16 or 8 and 64
						//if not then disable HAM
						if (BMHD->nPlanes==6) {
							if (maxcolors<16) goto notenoughcolors;
							}
						else {
							colormode=ham8;
							if (BMHD->nPlanes==8) {
								if (maxcolors<64) goto notenoughcolors;
								}
							else {
								printc("Warning: incorrect number of bitplanes to be HAM");
								printc("HAM mode disabled");
								modeid^=(HAM|EXTRA_HALFBRITE);
								}
							}
						}
					else if (modeid&EXTRA_HALFBRITE) { //TODO Halfbrite
						if (BMHD->nPlanes==6) {
							colormode=ehb;
							}
						else {
							printc("Warning: ModeID detected EHB but there are not 6 bitplanes");
							printc("attempting to read as a regular indexed palette iff");
							if (maxcolors<1<<BMHD->nPlanes) goto notenoughcolors;
							}
						}
					else if (maxcolors<1<<BMHD->nPlanes) {
notenoughcolors:
						wsprintf(string,"Not enough palette colors defined for IFF");
						goto erroriff;
						}
					}
				//Now let's print some courtesy specs of image
				if (debug) {
					wsprintf(string,"Source resolution %d x %d x %d",*x,*y,BMHD->nPlanes);
					printc(string);
					}
				if (debug) if (BMHD->masking) printc("Optional masking bitplane detected");
				//CAMG is optional if not defined nothing gets scaled
				if (modeid) {
					if (debug) {
						print("Amiga Screen Mode ID=");
						if (modeid&NTSC_MONITOR_ID) print("NTSC:");
						else if(modeid&PAL_MONITOR_ID) print("PAL:");
						else print("Default:");
						}
					//TODO may wish to scale cross monitor modes
					if (modeid&SUPERHIRES) {
						if (debug) print("Super-High Res ");
						moderesolution=super;
						}
					else if (modeid&HIRES) {
						if (debug) print("High Res ");
						moderesolution=hi;
						}
					else {
						if (debug) print("Low Res ");
						moderesolution=lo;
						}
					if (debug) {
						if (modeid&HAM) print("HAM ");
						else if (modeid&EXTRA_HALFBRITE) print("HalfBrite ");
						if (modeid&LACE) print("Laced ");
						printc(" ");
						}
					//Now to figure out from the modeid if we need to scale images
					//Quad Y
					if ((moderesolution==super)&&(!(modeid&LACE))) {
						if (debug) printc("Scale conversion: Y is quadrupled.");
						scaley=4;
						}
					//Double Y
					else if (((moderesolution==super)&&(modeid&LACE))|| //Super-High Res Laced
						((moderesolution==hi)&&(!(modeid&LACE)))) {//High Res
						if (debug) printc("Scale conversion: Y is doubled.");
						scaley=2;
						}
					//Double X
					else if ((moderesolution==lo)&&(*x<=360)) { //Low Res Laced
						if (debug) printc("Scale conversion: X is doubled.");
						scalex=2;
						if (!(modeid&LACE)) {
							if (debug) printc("Scale conversion: Y is doubled.");
							scaley=2;
							}
						}
					}
				//There may be instances where there is no modeid or pic needs to be
				//scaled anyway.
				if ((*x<=360)&&(*y<=240)&&(scalex==1)&&(scaley==1)) {
					if (debug) printc("Scale conversion: X and Y are doubled.");
					scalex=2;scaley=2;
					}
				scalexwidth=*x*scalex<<2;
				if (debug) {
					wsprintf(string,"Destination resolution %d x %d x %d",*x*scalex,*y*scaley,32);printc(string);
					print("Compression=");
					switch (BMHD->compression) {
						case 0:
							printc("None");
							break;
						case 1:
							printc("byteRun1");
							break;
						default:
							printc("Unknown");
							wsprintf(string,"Unknown Compression detected");
							goto erroriff;
						}
					}
				else if (BMHD->compression>1) {
					wsprintf(string,"Unknown Compression detected");
					goto erroriff;
					}
				//Here's the main conversion loop * * * *

				chunkindex=index+8;
				//Allocate a scanline x depth source buffer
				scanrow=0;
				scanwidthbytes=((*x+15)>>4)<<1;  //Number of bytes including the pad
				scandepth=BMHD->nPlanes;
				if (BMHD->masking==1) scandepth++;
				if (!(scanbuf=(char *)mynew(&pmem,scanwidthbytes*(scandepth)))) {
					wsprintf(string,"Not enough memory available");
					goto erroriff;
					}
				//Allocate the destination RAW BGRA final output
				if (!(dest=destindex=(char *)mynew(&pmem,*x*scalex*(*y)*scaley<<2))) {
					wsprintf(string,"Not enough memory available");
					goto erroriff;
					}
				/*Loop through rows*/ 
				for (scanrow=0;scanrow<*y;scanrow++) {
					//Unpack each plane [mask] to the scanbuf
					scanbufindex=scanbuf;
					for (scandepthindex=0;scandepthindex<scandepth;scandepthindex++) {
						if (BMHD->compression) {
							chunkindex+=unpackiffbyterun((char *)chunkindex,scanbufindex,(char *)finalsize,(short)scanwidthbytes);
							scanbufindex+=scanwidthbytes;
							}
						else for (t=scanwidthbytes;t;t--) *scanbufindex++=*chunkindex++;
						}
					//At this point the scanbuf has scandepth of a given scan line
					//we'll interpret this data depending upon the color mode to extract the pixel
					//This will loop x, and fill a destination scan line
					//well keep scandepth incremented to ignore the mask
					xindex=0;

					for (scanbufindex=scanbuf;scanbufindex<scanbuf+scanwidthbytes;scanbufindex++) {
						UBYTE R,G,B,paletteindex;

						//Loop through the bits of each bytes
						for (t=7;t>=0;t--) {
							switch(colormode) {
								case indexed:
									//indexed mode can pull full value from bitplanes to get a palette value
									paletteindex=0;
									for (scandepthindex=scandepth-1;scandepthindex>=0;scandepthindex--) {
										paletteindex=paletteindex<<1;
										if(*(scanbufindex+scandepthindex*scanwidthbytes)&(1<<t)) paletteindex|=1;
										}
									R=(CMAP+paletteindex)->red;
									G=(CMAP+paletteindex)->green;
									B=(CMAP+paletteindex)->blue;
									break;
								case ehb:
									paletteindex=0;
									for (scandepthindex=scandepth-1;scandepthindex>=0;scandepthindex--) {
										paletteindex=paletteindex<<1;
										if(*(scanbufindex+scandepthindex*scanwidthbytes)&(1<<t)) paletteindex|=1;
										}
									if (paletteindex&32) {
										paletteindex^=32;
										R=(CMAP+paletteindex)->red>>1;
										G=(CMAP+paletteindex)->green>>1;
										B=(CMAP+paletteindex)->blue>>1;
										}
									else {
										R=(CMAP+paletteindex)->red;
										G=(CMAP+paletteindex)->green;
										B=(CMAP+paletteindex)->blue;
										}
									break;
								case ham6:
									paletteindex=0;
									for (scandepthindex=scandepth-1;scandepthindex>=0;scandepthindex--) {
										paletteindex=paletteindex<<1;
										if(*(scanbufindex+scandepthindex*scanwidthbytes)&(1<<t)) paletteindex|=1;
										}
									//take out top 2 bits
									hammode=(hammodes)(paletteindex>>4);
									paletteindex&=0x0f;
									switch (hammode) {
										case normal:
											R=oldR=(CMAP+paletteindex)->red;
											G=oldG=(CMAP+paletteindex)->green;
											B=oldB=(CMAP+paletteindex)->blue;
											break;
										case rham:
											R=oldR=((paletteindex<<4)|paletteindex);
											G=oldG;
											B=oldB;
											break;
										case gham:
											R=oldR;
											G=oldG=((paletteindex<<4)|paletteindex);
											B=oldB;
											break;
										case bham:
											R=oldR;
											G=oldG;
											B=oldB=((paletteindex<<4)|paletteindex);
										}
									break;
								case ham8:
									paletteindex=0;
									for (scandepthindex=scandepth-1;scandepthindex>=0;scandepthindex--) {
										paletteindex=paletteindex<<1;
										if(*(scanbufindex+scandepthindex*scanwidthbytes)&(1<<t)) paletteindex|=1;
										}
									//take out top 2 bits
									hammode=(hammodes)(paletteindex>>6);
									paletteindex&=0x03f;
									switch (hammode) {
										case normal:
											R=oldR=(CMAP+paletteindex)->red;
											G=oldG=(CMAP+paletteindex)->green;
											B=oldB=(CMAP+paletteindex)->blue;
											break;
										case rham:
											R=oldR=(paletteindex<<2)|(paletteindex>>6);
											G=oldG;
											B=oldB;
											break;
										case gham:
											R=oldR;
											G=oldG=(paletteindex<<2)|(paletteindex>>6);
											B=oldB;
											break;
										case bham:
											R=oldR;
											G=oldG;
											B=oldB=(paletteindex<<2)|(paletteindex>>6);
										}
									break;
								case iff24:
									paletteindex=0;
									for (scandepthindex=7;scandepthindex>=0;scandepthindex--) {
										R=R<<1;
										G=G<<1;
										B=B<<1;
										if (*(scanbufindex+scandepthindex*scanwidthbytes)&(1<<t)) R|=1;
										if (*(scanbufindex+(scandepthindex+8)*scanwidthbytes)&(1<<t)) G|=1;
										if (*(scanbufindex+(scandepthindex+16)*scanwidthbytes)&(1<<t)) B|=1;
										}
									break;
								} //end switch colormode
							if (++xindex<=*x) {
								//put RGB onto the dest
								*destindex++=B;
								*destindex++=G;
								*destindex++=R;
								*destindex++=0; //alpha
								if (scalex==2) {
									*destindex++=B;
									*destindex++=G;
									*destindex++=R;
									*destindex++=0; //alpha
									}
								}
							} //end loop bits
						} //loop bytes
					if (scaley==2) {
						transmem((ULONG *)(destindex-scalexwidth),(ULONG *)destindex,scalexwidth);
						destindex+=scalexwidth;
						}
					if (scaley==4) {
						for (t=0;t<3;t++) {
							transmem((ULONG *)(destindex-scalexwidth),(ULONG *)destindex,scalexwidth);
							destindex+=scalexwidth;
							}
						}
					} //end loop rows
				*x*=scalex;
				*y*=scaley;
				break; 
				} //end BODY
			default:
				if (debug) {
					wsprintf(string,"%s is ignored\n",index);
					printc(string);
					}
			} //end switching different chunks
		index+=advance+8;
		if (advance&1) index++;
		} //end while index<finalsize
	//cleanup other allocated memory
	if (scanbuf) dispose((struct memlist *)scanbuf,&pmem);
	return (dest);
erroriff:
	printc(string);
	if (dest) dispose((struct memlist *)dest,&pmem);
	if (scanbuf) dispose((struct memlist *)scanbuf,&pmem);
	return(dest=0);  //elogant I know
	}

