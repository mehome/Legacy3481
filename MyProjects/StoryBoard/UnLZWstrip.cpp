#include "handlemain.h"
#include "UnLZWstrip.h"
// Copyright 2000 by Dan Stockelman, Dan's Dealers and Computers (DDC), James Killian, and Exodus.

 //we'll increase the scope of these as needed
#define EOI 257
#define CLEAR 256

void UnLZWclass::Inittable() {
	UINT i;
	tablestart=(char *)newnode(nodeobject,518);
	memset(tablestart,0,518);
	for ( i = 0; i < 256; i++) {
		table[i].string = tablestart+(i<<1);
		*(table[i].string) = (char)i;
		table[i].length=1;
		}

	table[256].string = tablestart+512;
	*(table[256].string) = (char)1;  //all hi to low

	table[257].string = tablestart+514;
	table[257].string[0] = (char)1;
	table[257].string[1] = (char)1;

	memset(table+257,0,4098-257);
	}

void UnLZWclass::Blanktable() {
	memset(table+258,0,4098-258);
	if (pmem) killnode(nodetable,&pmem);
	nodetable=createnode(&pmem,4096,0);
	tablemax=257;
	sizer=9;
	memset(table+257,0,4098-257);
	}

/*
char image[ ][ ];
int row, col;
//take horizontal differences:

for (row = 0; row < nrows; row++)
	for (col = ncols - 1; col >= 1; col--)
	image[row][col] -= image[row][col-1];
*/

void UnLZWclass::Unpredictgrey(char *strip, int bcount, int bpp) {
	UINT row, col , rowindex;

	for (row = 0; row < rowsperstrip; row++) {
		rowindex=row*720;
		for (col = 1; col <width ; col++)
		*(strip+col+rowindex) += *(strip+(col-1)+rowindex);
		}
	}

void UnLZWclass::Unpredictrgb(char *strip, int bcount, int bpp) {
 int j, bpp3;

	bpp3 = bpp * 3;
	for( j = 1; j < bcount; j+=bpp)	{
		strip[j] += strip[j-bpp3];
		strip[j+=bpp] += strip[j-bpp3];
		strip[j+=bpp] += strip[j-bpp3];
		}

	 }


void UnLZWclass::Unpredictrgba(char *strip, int bcount, int bpp) {
 int j, bpp4;

	bpp4 = bpp * 4;
	for( j = 1; j < bcount; j+=bpp)	{
		strip[j] += strip[j-bpp4];
		strip[j+=bpp] += strip[j-bpp4];
		strip[j+=bpp] += strip[j-bpp4];
		strip[j+=bpp] += strip[j-bpp4];
		}

	}


bool UnLZWclass::Isintable(UWORD code) {
	if ( code > tablemax ) return( false );
	return( true );
	}


void UnLZWclass::Addstringtotable() {
	tablemax++;
	table[tablemax].length = out.length;
	table[tablemax].string = (char *)newnode( nodetable, out.length);
	memcpy(table[tablemax].string,out.string,out.length);

	switch (tablemax) {
		case 510:
		case 1022:
		case 2046:
			sizer++;
		}
	if (tablemax>=4094) Blanktable();
	}


UWORD UnLZWclass::Getnextcode(char *currstrip, int *bitoffset) {
	UWORD code=0;
	UBYTE code2;
	int byteindex, unusedbits, numtoshift, posnumtoshift;
	const UWORD masktable[16]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767};

	byteindex=*bitoffset>>3;
	unusedbits=*bitoffset-(byteindex<<3);
	numtoshift=16-sizer-unusedbits;
	*bitoffset += sizer;

	code = *((UWORD *)(currstrip+byteindex));
	code = flipword( code );
	if (numtoshift>=0) {
		code = code>>numtoshift;
		}
	else { //number needs to be shifted to the left
		posnumtoshift=abs(numtoshift);
		code = code<<posnumtoshift;
		code2=*(currstrip+byteindex+2);
		code2=(code2>>(8-posnumtoshift));
		code |= code2;
		}

	code = code & masktable[sizer];

	return(code);
	}


void UnLZWclass::UnLZWstrip(char *currstrip, char *newstripindex) {
	int bitoffset;
	UWORD code, oldcode;
	unsigned int nbcount=0;
	sizer = 9;
	bitoffset=0;

	while ((code = Getnextcode( currstrip, &bitoffset )) != EOI ) {
		if (code == CLEAR) {
			Blanktable();
			/*
			Code = GetNextCode();
			if (Code == EoiCode) break;
			WriteString(StringFromCode(Code));
			OldCode = Code;
			*/
			code = Getnextcode(currstrip, &bitoffset);
			if (code==EOI) break;
			//if (code<=tablemax) {
				memcpy(newstripindex+nbcount,table[code].string,table[code].length);
				nbcount+=table[code].length;
				//}

			oldcode = code;
			} // End if for Clearcode

		else if ( Isintable( code ) ) {
			/*
			if (IsInTable(Code)) {
			WriteString(StringFromCode(Code));
			AddStringToTable(StringFromCode(OldCode)+FirstChar(StringFromCode(Code)));
			OldCode = Code;
			*/
			//Writestring( Stringfromcode( code ), nbcount, newstripindex );
			//if (code<=tablemax) {
				memcpy(newstripindex+nbcount,table[code].string,table[code].length);
				nbcount+=table[code].length;
				//}

			//setting up the first part of outstring
			//if (oldcode<=tablemax) {
				memcpy(out.string,table[oldcode].string,table[oldcode].length);
				out.length= table[oldcode].length;
				//}

			*(out.string+out.length)=*(table[code].string);
			out.length+=1;

			Addstringtotable();
			oldcode = code;
			} // else if

		else {
			/*
			OutString = StringFromCode(OldCode)+FirstChar(StringFromCode(OldCode));
			WriteString(OutString);
			AddStringToTable(OutString);
			OldCode = Code;
			*/
			//setting up the first part of outstring
			//if (oldcode<=tablemax) {
				memcpy(out.string,table[oldcode].string,table[oldcode].length);
				out.length= table[oldcode].length;
				*(out.string+out.length)=*(table[oldcode].string);
				out.length+=1;
				//}
			//if (out.string) {
				memcpy(newstripindex+nbcount,out.string,out.length);
				nbcount+=out.length;
				//}

			Addstringtotable();
			oldcode = code;

			} // else if	
		//if (nbcount > striplength) break;
		} // while

	//Unpredict Gray
	if ( predict == 2 ) Unpredictgrey( newstripindex, striplength, bpp );

	//if (nbcount > striplength) printf("didn't detect EOI\n");
	}

UnLZWclass::UnLZWclass(ULONG predictparm,UINT bppparm,ULONG widthparm,ULONG rpsparm) {
	nodetable=createnode(&pmem,4096,0);
	predict=predictparm;
	bpp=bppparm;
	width=widthparm;
	rowsperstrip=rpsparm;
	striplength=widthparm*rpsparm;
	out.string=(char *)newnode(nodeobject,4100);
	Inittable();
	}

UnLZWclass::~UnLZWclass() {
	disposenode(nodeobject,(struct memlist *)tablestart);
	disposenode(nodeobject,(struct memlist *)out.string);
	if (pmem) killnode(nodetable,&pmem);
	}

