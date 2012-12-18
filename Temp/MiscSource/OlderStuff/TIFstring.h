#ifndef TIF_H
#define TIF_H

#include "CBasic.h"

extern char string[256];

struct PCXheader {
	UBYTE Manufacturer;	//Constant Flag, 10 = ZSoft .pcx 
	UBYTE Version;			//Version information 
								//0 = Version 2.5 of PC Paintbrush 
								//2 = Version 2.8 w/palette information 
								//3 = Version 2.8 w/o palette information 
								//4 = PC Paintbrush for Windows(Plus for
								//		Windows uses Ver 5) 
								//5 = Version 3.0 and > of PC Paintbrush
								//		and PC Paintbrush +, includes
								//		Publisher's Paintbrush . Includes
								//		24-bit .PCX files 
	UBYTE Encoding;			//1 = .PCX run length encoding 
	UBYTE BitsPerPixel;	//Number of bits to represent a pixel
	UWORD Xmin,Ymin;		//		(per Plane) - 1, 2, 4, or 8 
	UWORD Xmax,Ymax;		//Image Dimensions: Xmin,Ymin,Xmax,Ymax
	UWORD HDpi;				//Horizontal Resolution of image in DPI* 
	UWORD VDpi;				//Vertical Resolution of image in DPI* 
	UBYTE Colormap[48];	//Color palette setting, see text 
	UBYTE Reserved64;		//Should be set to 0. 
	UBYTE NPlanes;			//Number of color planes 
	UWORD BytesPerLine;	//Number of bytes to allocate for a scanline
								//		plane.  MUST be an EVEN number.  Do NOT
								//		calculate from Xmax-Xmin. 
	UWORD PaletteInfo;	//How to interpret palette- 1 = Color/BW,
								//   2 = Grayscale (ignored in PB IV/ IV +) 
	UWORD HscreenSize;	//Horizontal screen size in pixels. New field
								//		found only in PB IV/IV Plus 
	UWORD VscreenSize;	//Vertical screen size in pixels. New field
								//		found only in PB IV/IV Plus 
	UBYTE Filler[54];		//Blank to fill out 128 byte header.  Set all
								//   bytes to 0 
	};

#endif /* TIF_H */
