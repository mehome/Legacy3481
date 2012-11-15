#include "StdAfx.h"

#include "RTVlib.h"
#pragma comment(lib, "RTVlib")

const unsigned XWidth=720;
const unsigned YHeight=480;
const unsigned BufferSizeInDWORDS=(XWidth*YHeight)>>1;
const unsigned BufferSizeInBytes=BufferSizeInDWORDS<<2;

unsigned long DestBuffer[BufferSizeInDWORDS];

void main(unsigned argc, char **argv) {
	BurnInTimeCode bc;

	//Some colors  (UYVY)
	//80 10 80 10  =  black
	//80 EB 80 EB  =  White
	//F0 29 6E 29  =  blue

	// fill in a color
	long count=BufferSizeInBytes;
	unsigned long *memoryindex=DestBuffer;
	while (count>0) {
		*memoryindex++=0x296e29f0;
		//*memoryindex++=0x10801080;
		count-=4;
		}

	bc.BlitTimeCode("12:34:56:78",(byte *)DestBuffer);
	unsigned fieldheight=YHeight>>1;
	BuildRTVFile("test.rtv",2,0,XWidth,fieldheight-(fieldheight&1),29.97f);
	WriteRTVFile("test.rtv",DestBuffer);
	CloseRTVFile("test.rtv");
	}
