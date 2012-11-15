#ifndef ILBM2RAW_H
#define ILBM2RAW_H
// All objects headers

//Here are some Amiga definitions to parse the view mode
#define ECS_SPECIFIC
#define MONITOR_ID_MASK			0xFFFF1000
#define DEFAULT_MONITOR_ID		0x00000000
#define NTSC_MONITOR_ID			0x00011000
#define PAL_MONITOR_ID			0x00021000
/* defines used for Modes in IVPargs */
#define GENLOCK_VIDEO	0x0002
#define LACE		0x0004
#define DOUBLESCAN	0x0008
#define SUPERHIRES	0x0020
#define PFBA		0x0040
#define EXTRA_HALFBRITE 0x0080
#define GENLOCK_AUDIO	0x0100
#define DUALPF		0x0400
#define HAM		0x0800
#define EXTENDED_MODE	0x1000
#define VP_HIDE	0x2000
#define SPRITES	0x4000
#define HIRES		0x8000

//Amiga version MakeID(a,b,c,d) ((a)<<24|(b)<<16|(c)<<8|(d))
//PC version
#define MakeID(d,c,b,a) ((a)<<24|(b)<<16|(c)<<8|(d))

#define ID_FORM	MakeID('F','O','R','M')
#define ID_ILBM	MakeID('I','L','B','M')
//BMHD CMAP CAMG ANNO and BODY
#define ID_BMHD	MakeID('B','M','H','D')
#define ID_CMAP	MakeID('C','M','A','P')
#define ID_CAMG	MakeID('C','A','M','G')
#define ID_ANNO	MakeID('A','N','N','O')
#define ID_BODY	MakeID('B','O','D','Y')


// Here are the function prototypes
// End Global function prototypes
#define cmpNone		0
#define cmpByteRun1	1

struct BitmapHeader {
	UWORD w,h; //raster width and height in pixels
	WORD x,y; //pixel position for this image
	UBYTE nPlanes; // # source bitplanes
	UBYTE masking;
	UBYTE compression;
	UBYTE pad1; //unused; ignore on read, write as 0
	UWORD transparentColor; //transparent "color number" (sort of)
	UBYTE xAspect,yAspect; //pixel aspect, a ratio width : height
	WORD pageWidth, pageHeight; //source page size in pixels
	};

struct ColorRegister {
	UBYTE red,green,blue;
	};


#endif //ILBM2RAW_H

