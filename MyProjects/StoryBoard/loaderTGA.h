#ifndef TGA2RAW_H
#define TGA2RAW_H

//These are the bit values defined in the Header/imagespec/imagedesc
#define tgaID_TOP		1<<5
#define tgaID_RIGHT	1<<4
#define tgaID_ALPHA	(1<<4)-1

enum tgaimagetypes {
		tga_noimage,		//0	-	No image data included.
		tga_nocomcmp,		//1	-	Uncompressed, color-mapped images.
		tga_nocomtc,		//2	-	Uncompressed, RGB images.
		tga_nocombw,		//3	-	Uncompressed, black and white images.
		tga_rlecmp=9,		//9	-	Runlength encoded color-mapped images.
		tga_rlergb,			//10	-	Runlength encoded RGB images.
		tga_rlebw,			//11	-	Compressed, black and white images.
		tga_rlehdcmp=32,	//32	-	Compressed color-mapped data, using Huffman, Delta,
								//				and runlength encoding.
		tga_rlehd2cmp		//33	-	Compressed color-mapped data, using Huffman, Delta,
								//				and runlength encoding.  4-pass quadtree-type process.
		};

struct TGAheader {
	UBYTE idlength,colormaptype,imagetype;
	struct cmapspec {
		UBYTE cmapentry[2],cmaplength[2],bitsperentry;
		} cmapspecs;
	struct imagespec {
		UWORD x,y,width,height;
		UBYTE depth,imagedesc;
		}imagespecs;
	};

struct TGAfooter {
	ULONG extoffset;
	ULONG devoffset;
	UBYTE signature[17];
	UBYTE pad[3];  //Not really in TGA format
	};

#endif //TGA2RAW_H

