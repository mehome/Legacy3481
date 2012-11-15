#include "StdAfx.h"


void BurnInTimeCode::FillInRow(byte *Buffer,unsigned long TimeCode,unsigned row) {
	ULONG *UYVY=(ULONG *)Buffer;
	for (unsigned xcount=8;xcount;xcount--) {
		//Grab the number...
		unsigned long number=0;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		*UYVY++=0x10801080;
		}
	}


bool BurnInTimeCode::BlitTimeCode(
		char *TimeCodeString,		//##:##:##:##
		byte *FieldEvenZero,		//Top Row Even Field... or interleaved (if FieldOddone is NULL)
		byte *FieldOddOne,			//2nd Row Odd Field
		unsigned XOffset,unsigned YOffset,	//Position to place text
		unsigned long BuffXres, //Size of the VideoBuffer
		unsigned long BuffYres
		) {

	unsigned long TimeCode=0x12345678;

	unsigned destwidthbytes=BuffXres<<1;
	long direction=1,xordirection=0;

	//Field 0 then Field 1... using line skip
	unsigned fieldheight=BuffYres>>1;
	unsigned rowwithccfieldoffset;
	unsigned row=YOffset;
	byte *Destination_UYVY;
	unsigned toggleoffset=0;
	
	for (unsigned ycount=0;ycount<20;ycount++) {
		rowwithccfieldoffset=row+toggleoffset;
		Destination_UYVY=FieldEvenZero+(destwidthbytes*rowwithccfieldoffset);

		//Now to fill in this row
		FillInRow(Destination_UYVY+XOffset,TimeCode,ycount);

		row+=xordirection;
		xordirection^=direction;
		toggleoffset^=fieldheight;
		}

	return true;
	}
