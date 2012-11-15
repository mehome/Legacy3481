#include "HandleMain.h"  

#define afx_SOFT		200
#define afx_MEDIUM	201
#define afx_HARD		202
#define afx_OK			203
#define afx_CANCEL	204
#define afx_BtoW		205
#define afx_REVERSE	206

void TRAN_Gradient (UBYTE *pFrame1,UBYTE *pFrame2,UBYTE *pAlpha,DWORD dwPos,DWORD dwLen,BOOL reverse,BOOL i)
	{
	const static ULONG sigbit[2]={0x80808080,0x80808080};
	/*
	DWORD i_,FrameMul_;
	i_=2;

	while (i_--)
		{
	*/
	DWORD FrameMul_;
	if (i) {
		pFrame1 += 345600;
		pFrame2 += 345600;
		pAlpha  += 172800;
		dwPos++;
		}

		if (reverse) FrameMul_=(dwPos<<8)/dwLen; 
		else FrameMul_=((dwLen-dwPos)<<8)/dwLen;
		//FrameMul_ = (FrameMul_ > 255) ? 255 : FrameMul_;
		__asm
			{
			mov         ecx,[FrameMul_]
			mov			edx,255
			cmp         ecx,255
			cmovg			ecx,edx
			mov			[FrameMul_],ecx
			mov			ecx,21600
			movq          mm4, [FrameMul_]    // MM4 = ______________Fm
			mov           eax, pFrame1
			 punpcklbw    mm4, mm4            // MM4 = ____________FmFm
			mov           ebx, pFrame2
			 punpcklwd    mm4, mm4            // MM4 = ________FmFmFmFm
			mov           esi, pAlpha
			 punpckldq    mm4, mm4            // MM4 = FmFmFmFmFmFmFmFm
			movq			mm5,sigbit
			paddb		mm4,mm5
l1:		movq          mm0, [esi]          // MM0 = A7A6A5A4A3A2A1A0
			 pcmpeqb      mm6, mm6            // MM6 = FFFFFFFFFFFFFFFF
			movq          mm2, [eax]          // MM2 = y3v2y2u2y1v0y0u0
			 psrld        mm6, 24             // MM6 = 000000FF000000FF
			movq          mm3, [ebx]          // MM3 = Y3V2Y2U2Y1V0Y0U0
			 movq         mm1, mm0            // MM1 = A7A6A5A4A3A2A1A0
			punpcklbw     mm0, mm0            // MM0 = A3A3A2A2A1A1A0A0
			 movq         mm7, mm6            // MM7 = 000000FF000000FF
			punpckhbw     mm1, mm1            // MM1 = A7A7A6A6A5A5A4A4
			 pand         mm6, mm0            // MM6 = ______A2______A0
			pand          mm7, mm1            // MM7 = ______A6______A4
			 pslld        mm0, 8              // MM0 = A3A2A2__A1A0A0__
			pslld         mm1, 8              // MM1 = A7A6A6__A5A4A4__
			 por          mm0, mm6            // MM0 = A3A2A2A2A1A0A0A0
			movq          mm6, [eax+8]        // MM6 = y7v6y6u6y5v4y4u4
			 por          mm1, mm7            // MM1 = A7A6A6A6A5A4A4A4
			movq          mm7, [ebx+8]        // MM7 = Y7V6Y6U6Y5V4Y4U4
			paddb			mm0,mm5
			paddb			mm1,mm5
			 pcmpgtb      mm0, mm4            // MM0 = FF for Src2, 00 for Src1
			pcmpgtb       mm1, mm4            // MM1 = FF for Src2, 00 for Src1
			 pand         mm3, mm0            // MM3 = Only Dest Src2 Pixels (0-3)
			pand          mm7, mm1            // MM7 = Only Dest Src2 Pixels (4-7)
			 pandn        mm0, mm2            // MM0 = Only Dest Src1 Pixels (0-3)
			pandn         mm1, mm6            // MM1 = Only Dest Src1 Pixels (4-7)
			 por          mm0, mm3            // MM0 = Y3V2Y2U2Y1V0Y0U0
			por           mm1, mm7            // MM1 = Y7V6Y6U6Y5V4Y4U4
			 add          ebx, 16
			movq          [eax], mm0          // Save 0-3.
			 add          esi, 8
			movq          [eax+8], mm1        // Save 4-7.
			 dec          ecx
			add           eax, 16
			 cmp          ecx, 0
			jnz           l1
			}
     __asm emms
	}  


void TRAN_Blend (UBYTE *pFrame1,UBYTE *pFrame2,UBYTE *pAlpha,DWORD dwPos,DWORD dwLen,BOOL reverse,BOOL i)
	{
	//const long goffset[2]={0x00560056,0x00560056};
	static ULONG fm[2];
/*
	DWORD i_,FrameMul_;
	i_=2;
*/
	DWORD FrameMul_;
	if (i) {
		pFrame1 += 345600;
		pFrame2 += 345600;
		pAlpha  += 172800;
		dwPos++;
		}
	
	if (reverse) FrameMul_=((dwLen-dwPos)<<8)/dwLen;
		else FrameMul_=(dwPos<<8)/dwLen;
		//FrameMul_=(FrameMul_>255) ? 255 : FrameMul_;

		__asm {
			mov         ecx,[FrameMul_]
			mov			edx,255
			cmp         ecx,255
			cmovg			ecx,edx
			mov			[FrameMul_],ecx
			mov			ecx,43200
			movq			mm4,[FrameMul_]	//mm4=______________Fm
			mov			eax,pFrame1
			 punpcklbw	mm4,mm4				//mm4=____________FmFm
			mov			ebx,pFrame2
			 punpcklwd	mm4,mm4				//mm4=________FmFmFmFm
			mov			esi,pAlpha
			lea			edx,fm
			pxor			mm7,mm7				//MM7 = ________________
			 punpcklbw	mm4,mm7				//MM4 = __Fm__Fm__Fm__Fm
			 psllw		mm4,1
			 movq			[edx],mm4
blendloop:
			movd			mm0,[esi]			//MM0 = ________s3s2s1s0
			punpcklbw	mm0,mm7				//MM0 = __s3__s2__s1__s0
			//Start medium calculation
		//g=gradient+86;
		//paddsw		mm0,goffset

		//gradient=(255-h)*(255-g)-(h*g);

		movq			mm2,[edx]		//mm2=framemul (or h)
		//movq			mm3,w255
			pcmpeqb		mm3,mm3				//MM3 = FFFFFFFFFFFFFFFF
			psrlw			mm3,8					//mm3 = 00ff00ff00ff00ff
			movq			mm1,mm3
		psubsw		mm3,mm0
		//movq			mm1,w255
		psubsw		mm1,mm2
		movq			mm4,mm1
		pmullw		mm1,mm3	//mm1=((255-h)*(255-g))
		pmulhw		mm4,mm3
		movq			mm3,mm0
		pmullw		mm0,mm2  //h*g
		pmulhw		mm3,mm2
		movq			mm2,mm1
		movq			mm5,mm0
		punpcklwd	mm1,mm4	//mm1mm2=the first ()
		punpckhwd	mm2,mm4
		punpcklwd	mm0,mm3	//mm0mm5=the second ()
		punpckhwd	mm5,mm3
		psubd			mm1,mm0  //mm1mm2=new gradient
		psubd			mm2,mm5
		packssdw		mm1,mm2	//mm1=new gradient
		//if (gradient>255) gradient=0;

		movq			mm0,mm1
			pcmpeqb		mm3,mm3				//MM3 = FFFFFFFFFFFFFFFF
			psrlw			mm3,8					//mm3 = 00ff00ff00ff00ff
		pcmpgtw		mm1,mm3
		pandn			mm1,mm0

		//gradient=(gradient=gradient>=0?gradient:-gradient)>>7;

		movq			mm0,mm1	//mm0= the gradient
		movq			mm3,mm1	//
			pxor			mm5,mm5			//mm5 = ________________
		pcmpgtw		mm1,mm5	//mm1= the mask of positive numbers
		movq			mm2,mm0
		pand			mm2,mm1	//mm2= all the positive numbers
		movq			mm4,mm2
			pcmpeqb		mm5,mm5				//MM5 = FFFFFFFFFFFFFFFF

		pandn			mm4,mm5	//mm4= the mask of negative numbers
		pand			mm3,mm4	//mm3= all the negative numbers
		pandn			mm3,mm5	//mm3= ones complement
			pcmpeqb		mm5,mm5				//MM5 = FFFFFFFFFFFFFFFF
			psrlw			mm5,15				//mm5 = __01__01__01__01

		paddsw		mm3,mm5	//mm3= twos complement now positive
		por			mm2,mm4	//fill the positive with neg mask
		pand			mm3,mm2	//mm2= all positive numbers
		psraw			mm3,7		//divide by 128

		//if (gradient>255) gradient=255;

		movq			mm0,mm3
			pcmpeqb		mm5,mm5				//MM5 = FFFFFFFFFFFFFFFF
			psrlw			mm5,8					//mm5 = 00ff00ff00ff00ff
		pcmpgtw		mm0,mm5 
		por			mm3,mm0
		pand			mm3,mm5 
		movq			mm0,mm3

			//end medium calculation
			packuswb		mm0,mm0
			punpcklbw	mm0,mm0				//MM0 = s3s3s2s2s1s1s0s0
			add			esi,4
			movq			mm3,[eax]			//MM3 = y3v2y2u2y1v0y0u0
			movq			mm2,mm3				//MM2 = y3v2y2u2y1v0y0u0
			movq			mm4,[ebx]			//MM4 = Y3V2Y2U2Y1V0Y0U0
			add			ebx,8
			pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF
			psrlw			mm6,8					//MM6 = __FF__FF__FF__FF
			pxor			mm0,mm6				//MM0 = s3n3s2n2s1n1s0n0
			punpcklbw	mm2,mm4				//MM2 = Y1y1V0v0Y0y0U0u0
			movq			mm5,mm2				//MM5 = Y1y1V0v0Y0y0U0u0
			punpcklbw	mm5,mm7				//MM5 = __Y0__y0__U0__u0
			punpckhbw	mm2,mm7				//MM2 = __Y1__y1__V0__v0
			movq			mm1,mm0				//MM1 = s3n3s2n2s1n1s0n0
			punpckhbw	mm0,mm7				//MM0 = __s3__n3__s2__n2
			punpcklbw	mm1,mm7				//MM1 = __s1__n1__s0__n0
			movq			mm6,mm1				//MM6 = __s1__n1__s0__n0
			punpckldq	mm1,mm1				//MM1 = __s0__n0__s0__n0
			pmaddwd		mm1,mm5				//MM1 = ______Y0______U0
			psrld			mm1,8					//MM1 = ______Y0______U0
			pmaddwd		mm2,mm6				//MM2 = ______Y1______V0
			psrld			mm2,8					//MM2 = ______Y1______V0
			punpckhbw	mm3,mm4				//MM3 = Y3y3V2v2Y2y2U2u2
			movq			mm5,mm3				//MM5 = Y3y3V2v2Y2y2U2u2
			punpcklbw	mm5,mm7				//MM5 = __Y2__y2__U2__u2
			punpckhbw	mm3,mm7				//MM3 = __Y3__y3__V2__v2
			packssdw		mm1,mm2				//MM1 = __Y1__V0__Y0__U0

			pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF
			psrlw			mm6,15				//mm6 = __01__01__01__01
			paddsw		mm1,mm6

			movq			mm4,mm0				//MM4 = __s3__n3__s2__n2
			punpckldq	mm0,mm0				//MM0 = __s2__n2__s2__n2
			pmaddwd		mm5,mm0				//MM5 = ______Y2______U2
			psrld			mm5,8					//MM5 = ______Y2______U2
			pmaddwd		mm3,mm4				//MM3 = ______Y3______V2
			psrld			mm3,8					//MM3 = ______Y3______V2
			packssdw		mm5,mm3				//MM5 = __Y3__V2__Y2__U2

			paddsw		mm5,mm6

			packuswb		mm1,mm5				//MM1 = Y3V2Y2U2Y1V0Y0U0
			movq			[eax],mm1			//Save 0-3.
			 dec			ecx
			add			eax, 8
			 cmp			ecx, 0
			jnz			blendloop
			}
	__asm emms
	} //end Tran_Blend


void TRAN_softBlend (UBYTE *pFrame1,UBYTE *pFrame2,UBYTE *pAlpha,DWORD dwPos,DWORD dwLen,BOOL reverse,BOOL i)
	{
	//const static ULONG one[2]={0x01010101,0x01010101};
	static ULONG fm[2];/*
	DWORD i_,FrameMul_;
	i_=2;

	while (i_--) {
	*/
	DWORD FrameMul_;
	if (i) {
		pFrame1 += 345600;
		pFrame2 += 345600;
		pAlpha  += 172800;
		dwPos++;
		}

		if (reverse) FrameMul_=((dwLen-dwPos)<<8)/dwLen;
		else FrameMul_=(dwPos<<8)/dwLen;
		//FrameMul_=(FrameMul_>255) ? 255 : FrameMul_;

		__asm {
			mov         ecx,[FrameMul_]
			mov			edx,255
			cmp         ecx,255
			cmovg			ecx,edx
			mov			[FrameMul_],ecx
			mov			ecx,43200
			movq			mm4,[FrameMul_]	//mm4=______________Fm
			mov			eax,pFrame1
			 punpcklbw	mm4,mm4				//mm4=____________FmFm
			mov			ebx,pFrame2
			 punpcklwd	mm4,mm4				//mm4=________FmFmFmFm
			mov			esi,pAlpha
			lea			edx,fm
			pxor			mm7,mm7				//MM7 = ________________
			 punpcklbw	mm4,mm7				//MM4 = __Fm__Fm__Fm__Fm
			 movq			[edx],mm4
blendloop:
			movd			mm0,[esi]			//MM0 = ________s3s2s1s0
			punpcklbw	mm0,mm7				//MM0 = __s3__s2__s1__s0
			movq			mm4,[edx]			//mm4=framemul
			paddsw		mm0,mm4				//add framemul
			pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF
			psrlw			mm6,8					//MM6 = __FF__FF__FF__FF
			pxor			mm4,mm6				//MM4 = __nf__nf__nf__nf
			psrlw			mm6,7					//mm6 = __01__01__01__01
			paddsw		mm4,mm6				// add one to nf (two's complement)
			psubusw		mm0,mm4				//sub framemul
			packuswb		mm0,mm0
			punpcklbw	mm0,mm0				//MM0 = s3s3s2s2s1s1s0s0
			add			esi,4
			movq			mm3,[eax]			//MM3 = y3v2y2u2y1v0y0u0
			movq			mm2,mm3				//MM2 = y3v2y2u2y1v0y0u0
			movq			mm4,[ebx]			//MM4 = Y3V2Y2U2Y1V0Y0U0
			add			ebx,8
			pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF
			psrlw			mm6,8					//MM6 = __FF__FF__FF__FF
			pxor			mm0,mm6				//MM0 = s3n3s2n2s1n1s0n0
			punpcklbw	mm2,mm4				//MM2 = Y1y1V0v0Y0y0U0u0
			movq			mm5,mm2				//MM5 = Y1y1V0v0Y0y0U0u0
			punpcklbw	mm5,mm7				//MM5 = __Y0__y0__U0__u0
			punpckhbw	mm2,mm7				//MM2 = __Y1__y1__V0__v0
			movq			mm1,mm0				//MM1 = s3n3s2n2s1n1s0n0
			punpckhbw	mm0,mm7				//MM0 = __s3__n3__s2__n2
			punpcklbw	mm1,mm7				//MM1 = __s1__n1__s0__n0
			movq			mm6,mm1				//MM6 = __s1__n1__s0__n0
			punpckldq	mm1,mm1				//MM1 = __s0__n0__s0__n0
			pmaddwd		mm1,mm5				//MM1 = ______Y0______U0
			psrld			mm1,8					//MM1 = ______Y0______U0
			pmaddwd		mm2,mm6				//MM2 = ______Y1______V0
			psrld			mm2,8					//MM2 = ______Y1______V0
			punpckhbw	mm3,mm4				//MM3 = Y3y3V2v2Y2y2U2u2
			movq			mm5,mm3				//MM5 = Y3y3V2v2Y2y2U2u2
			punpcklbw	mm5,mm7				//MM5 = __Y2__y2__U2__u2
			punpckhbw	mm3,mm7				//MM3 = __Y3__y3__V2__v2
			packssdw		mm1,mm2				//MM1 = __Y1__V0__Y0__U0

			pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF
			psrlw			mm6,15				//mm6 = __01__01__01__01
			paddsw		mm1,mm6

			movq			mm4,mm0				//MM4 = __s3__n3__s2__n2
			punpckldq	mm0,mm0				//MM0 = __s2__n2__s2__n2
			pmaddwd		mm5,mm0				//MM5 = ______Y2______U2
			psrld			mm5,8					//MM5 = ______Y2______U2
			pmaddwd		mm3,mm4				//MM3 = ______Y3______V2
			psrld			mm3,8					//MM3 = ______Y3______V2
			packssdw		mm5,mm3				//MM5 = __Y3__V2__Y2__U2

			paddsw		mm5,mm6

			packuswb		mm1,mm5				//MM1 = Y3V2Y2U2Y1V0Y0U0
			//paddusb		mm1,one
			movq			[eax],mm1			//Save 0-3.
			 dec			ecx
			add			eax, 8
			 cmp			ecx, 0
			jnz			blendloop
			}
	__asm emms
	} //end Tran_softBlend


void TRAN_Dissolve (UBYTE *pFrame1,UBYTE *pFrame2,UBYTE *pAlpha,DWORD dwPos,DWORD dwLen)
	{
	static ULONG fm[2];
	DWORD i_,FrameMul_;
	i_=2;

	while (i_--) {
		FrameMul_=(dwPos<<8)/dwLen;
		//FrameMul_=(FrameMul_>255) ? 255 : FrameMul_;

		__asm {
			mov         ecx,[FrameMul_]
			mov			edx,255
			cmp         ecx,255
			cmovg			ecx,edx
			mov			[FrameMul_],ecx
			mov			ecx,43200
			movq			mm4,[FrameMul_]	//mm4=______________Fm
			mov			eax,pFrame1
			 punpcklbw	mm4,mm4				//mm4=____________FmFm
			mov			ebx,pFrame2
			 punpcklwd	mm4,mm4				//mm4=________FmFmFmFm
			lea			edx,fm
			 punpckldq	mm4,mm4				//MM4 = FmFmFmFmFmFmFmFm
			 movq			[edx],mm4
blendloop:
			movq			mm0,[edx]			//mm4=framemul
			pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF
			add			esi,4
			movq			mm3,[eax]			//MM3 = y3v2y2u2y1v0y0u0
			movq			mm2,mm3				//MM2 = y3v2y2u2y1v0y0u0
			movq			mm4,[ebx]			//MM4 = Y3V2Y2U2Y1V0Y0U0
			pxor			mm7,mm7				//MM7 = ________________
			add			ebx,8
			psrlw			mm6,8					//MM6 = __FF__FF__FF__FF
			pxor			mm0,mm6				//MM0 = s3n3s2n2s1n1s0n0
			punpcklbw	mm2,mm4				//MM2 = Y1y1V0v0Y0y0U0u0
			movq			mm5,mm2				//MM5 = Y1y1V0v0Y0y0U0u0
			punpcklbw	mm5,mm7				//MM5 = __Y0__y0__U0__u0
			punpckhbw	mm2,mm7				//MM2 = __Y1__y1__V0__v0
			movq			mm1,mm0				//MM1 = s3n3s2n2s1n1s0n0
			punpckhbw	mm0,mm7				//MM0 = __s3__n3__s2__n2
			punpcklbw	mm1,mm7				//MM1 = __s1__n1__s0__n0
			movq			mm6,mm1				//MM6 = __s1__n1__s0__n0
			punpckldq	mm1,mm1				//MM1 = __s0__n0__s0__n0
			pmaddwd		mm1,mm5				//MM1 = ______Y0______U0
			psrld			mm1,8					//MM1 = ______Y0______U0
			pmaddwd		mm2,mm6				//MM2 = ______Y1______V0
			psrld			mm2,8					//MM2 = ______Y1______V0
			punpckhbw	mm3,mm4				//MM3 = Y3y3V2v2Y2y2U2u2
			movq			mm5,mm3				//MM5 = Y3y3V2v2Y2y2U2u2
			punpcklbw	mm5,mm7				//MM5 = __Y2__y2__U2__u2
			punpckhbw	mm3,mm7				//MM3 = __Y3__y3__V2__v2
			packssdw		mm1,mm2				//MM1 = __Y1__V0__Y0__U0
			movq			mm4,mm0				//MM4 = __s3__n3__s2__n2
			punpckldq	mm0,mm0				//MM0 = __s2__n2__s2__n2
			pmaddwd		mm5,mm0				//MM5 = ______Y2______U2
			psrld			mm5,8					//MM5 = ______Y2______U2
			pmaddwd		mm3,mm4				//MM3 = ______Y3______V2
			psrld			mm3,8					//MM3 = ______Y3______V2
			packssdw		mm5,mm3				//MM5 = __Y3__V2__Y2__U2
			packuswb		mm1,mm5				//MM1 = Y3V2Y2U2Y1V0Y0U0
			movq			[eax],mm1			//Save 0-3.
			 dec			ecx
			add			eax, 8
			 cmp			ecx, 0
			jnz			blendloop
			}
		pFrame1 += 345600;
		pFrame2 += 345600;
		pAlpha  += 172800;
		dwPos++;
		}
	__asm emms
	} //End Tran_Dissolve


void renderalpha8(ULONG *alphabuf,int x,int y,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo,BOOL upsidedown) {
	const long div3[2]={0x55555555,0x55555555};
	char *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=-24;
	int calibratey=0;
	long size=0;
	ULONG bytesline=(x+3)&0xfffffffc;
	ULONG *palette=(ULONG *)((char *)bitmapinfo+bitmapinfo->biSize);
	UBYTE field;

	int numofcolors=bitmapinfo->biClrUsed;
	if (numofcolors==0) numofcolors=256;
	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffc;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
	__asm {
		mov	ecx,y
		and	ecx,1
		mov	eax,centery
		and	eax,0fffffffeh
		or		eax,ecx
		mov	[centery],eax
		}
	centery+=calibratey;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffc;

	__asm mov edi,alphabuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,alphabuf
					mov	ecx,180
					mov	eax,0
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[alphabuf],edi
					}
				}
			}
		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=4) {
					__asm {
						mov	edi,alphabuf
						mov	eax,0
						mov	[edi],eax
						add	edi,4
						mov	[alphabuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=4) {
				pixel=(decodedbits+xindex+(bytesline*yindex));
				__asm {
					mov			edi,alphabuf
					mov			eax,pixel
					xor			edx,edx
					mov			dl,[eax]
					shl			edx,2
					mov			eax,palette
					add			eax,edx
					movd			mm0,[eax]		//mm0 = ________A1R1G1B1
					mov			eax,pixel
					xor			edx,edx
					mov			dl,[eax+1]
					shl			edx,2
					mov			eax,palette
					add			eax,edx
					movd			mm1,[eax]		//mm1 = ________A2R2G2B2
					punpcklbw	mm0,mm1			//mm0 = A2A1R2R1G2G1B2B1
					mov			eax,pixel
					movq			mm1,mm0			//mm1 = copy of mm0
					xor			edx,edx
					mov			dl,[eax+2]
					shl			edx,2
					mov			eax,palette
					add			eax,edx
					movd			mm2,[eax]		//mm2 = ________A3R3G3B3
					mov			eax,pixel
					xor			edx,edx
					mov			dl,[eax+3]
					shl			edx,2
					mov			eax,palette
					add			eax,edx
					movd			mm3,[eax]		//mm3 = ________A4R4G4B4
					punpcklbw	mm2,mm3			//mm2 = A4A3R4R3G4G3B4B3
					punpcklwd	mm0,mm2			//mm0 = G4G3G2G1B4B3B2B1
					punpckhwd	mm1,mm2			//mm1 = A4A3A2A1R4R3R2R1
					//average the pixels

					movq			mm2,mm0			//mm2 = copy of mm0
					pxor			mm3,mm3			//mm3 = ________________
					movq			mm4,div3			//mm4 = 1/3 * 65536

					punpcklbw	mm2,mm3			//mm2 = __B4__B3__B2__B1
					punpckhbw	mm0,mm3			//mm0 = __G4__G3__G2__G1
					paddw			mm0,mm2			//mm0 = _BG4_BG3_BG2_BG1
					punpcklbw	mm1,mm3			//mm1 = __R4__R3__R2__R1
					paddw			mm0,mm1			//mm0 = RGB4RGB3RGB2RGB1
					movq			mm2,mm0
					
					pmullw		mm0,mm4			//mm0 = _A4l_A3l_A2l_A1l
					movq			mm1,mm0			//mm1 = copy of mm0
					pmulhw		mm2,mm4			//mm2 = _A4h_A3h_A2h_A1h
					punpcklwd	mm0,mm2			//mm0 = ______A2______A1
					punpckhwd	mm1,mm2			//mm1 = ______A4______A3
					psrad			mm0,16
					psrad			mm1,16
					// pack it up
					packssdw		mm0,mm1			//mm0 = __A4__A3__A2__A1

					packuswb		mm0,mm3			//mm0 = ________A4A3A2A1
					movd			[edi],mm0
					add			edi,4
					emms
					mov	[alphabuf],edi
					}

				} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=4) {
					__asm {
						mov	edi,alphabuf
						mov	eax,0
						mov	[edi],eax
						add	edi,4
						mov	[alphabuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-y-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,alphabuf
					mov	ecx,180
					mov	eax,0
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[alphabuf],edi
					}
				}
			}
		} //end field loop
	} //end renderalpha8


void renderalpha32(ULONG *alphabuf,int x,int y,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo,BOOL upsidedown) {
	const long div3[2]={0x55555555,0x55555555};
	char *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=-24;
	int calibratey=0;
	long size=0;
	ULONG bytesline=x<<2;
	UBYTE field;

	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffc;  //centerx has to be 4 byte aligned in alpha mode
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
	__asm {
		mov	ecx,y
		and	ecx,1
		mov	eax,centery
		and	eax,0fffffffeh
		or		eax,ecx
		mov	[centery],eax
		}
	centery+=calibratey;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffc;

	__asm mov edi,alphabuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,alphabuf
					mov	ecx,180
					mov	eax,0
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[alphabuf],edi
					}
				}
			}
		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=4) {
					__asm {
						mov	edi,alphabuf
						mov	eax,0
						mov	[edi],eax
						add	edi,4
						mov	[alphabuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=4) {
				pixel=(decodedbits+(xindex<<2)+(bytesline*yindex));
				__asm {
					mov			edi,alphabuf
					mov			eax,pixel
					movd			mm0,[eax]		//mm0 = ________A1R1G1B1
					movd			mm1,[eax+4]		//mm1 = ________A2R2G2B2
					punpcklbw	mm0,mm1			//mm0 = A2A1R2R1G2G1B2B1
					mov			eax,pixel
					movq			mm1,mm0			//mm1 = copy of mm0
					movd			mm2,[eax+8]		//mm2 = ________A3R3G3B3
					movd			mm3,[eax+12]	//mm3 = ________A4R4G4B4
					punpcklbw	mm2,mm3			//mm2 = A4A3R4R3G4G3B4B3
					punpcklwd	mm0,mm2			//mm0 = G4G3G2G1B4B3B2B1
					punpckhwd	mm1,mm2			//mm1 = A4A3A2A1R4R3R2R1
					//average the pixels

					movq			mm2,mm0			//mm2 = copy of mm0
					pxor			mm3,mm3			//mm3 = ________________
					movq			mm4,div3			//mm4 = 1/3 * 65536

					punpcklbw	mm2,mm3			//mm2 = __B4__B3__B2__B1
					punpckhbw	mm0,mm3			//mm0 = __G4__G3__G2__G1
					paddw			mm0,mm2			//mm0 = _BG4_BG3_BG2_BG1
					punpcklbw	mm1,mm3			//mm1 = __R4__R3__R2__R1
					paddw			mm0,mm1			//mm0 = RGB4RGB3RGB2RGB1
					movq			mm2,mm0
					
					pmullw		mm0,mm4			//mm0 = _A4l_A3l_A2l_A1l
					movq			mm1,mm0			//mm1 = copy of mm0
					pmulhw		mm2,mm4			//mm2 = _A4h_A3h_A2h_A1h
					punpcklwd	mm0,mm2			//mm0 = ______A2______A1
					punpckhwd	mm1,mm2			//mm1 = ______A4______A3
					psrad			mm0,16
					psrad			mm1,16
					// pack it up
					packssdw		mm0,mm1			//mm0 = __A4__A3__A2__A1

					packuswb		mm0,mm3			//mm0 = ________A4A3A2A1
					movd			[edi],mm0
					add			edi,4
					emms
					mov	[alphabuf],edi
					}

				} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=4) {
					__asm {
						mov	edi,alphabuf
						mov	eax,0
						mov	[edi],eax
						add	edi,4
						mov	[alphabuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-y-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,alphabuf
					mov	ecx,180
					mov	eax,0
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[alphabuf],edi
					}
				}
			}
		} //end field loop
	} //end renderalpha32


void renderalpha24(ULONG *alphabuf,int x,int y,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo,BOOL upsidedown) {
	const long div3[2]={0x55555555,0x55555555};
	char *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=-24;
	int calibratey=0;
	long size=0;
	ULONG bytesline=(x*3+3)&0xfffffffc;
	UBYTE field;

	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffc;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
	__asm {
		mov	ecx,y
		and	ecx,1
		mov	eax,centery
		and	eax,0fffffffeh
		or		eax,ecx
		mov	[centery],eax
		}
	centery+=calibratey;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffc;

	__asm mov edi,alphabuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,alphabuf
					mov	ecx,180
					mov	eax,0
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[alphabuf],edi
					}
				}
			}
		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=4) {
					__asm {
						mov	edi,alphabuf
						mov	eax,0
						mov	[edi],eax
						add	edi,4
						mov	[alphabuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=4) {
				pixel=(decodedbits+(xindex*3)+(bytesline*yindex));
				__asm {
					mov			edi,alphabuf
					mov			eax,pixel
					movd			mm0,[eax]		//mm0 = ________A1R1G1B1
					movd			mm1,[eax+3]		//mm1 = ________A2R2G2B2
					punpcklbw	mm0,mm1			//mm0 = A2A1R2R1G2G1B2B1
					mov			eax,pixel
					movq			mm1,mm0			//mm1 = copy of mm0
					movd			mm2,[eax+6]		//mm2 = ________A3R3G3B3
					movd			mm3,[eax+9]	//mm3 = ________A4R4G4B4
					punpcklbw	mm2,mm3			//mm2 = A4A3R4R3G4G3B4B3
					punpcklwd	mm0,mm2			//mm0 = G4G3G2G1B4B3B2B1
					punpckhwd	mm1,mm2			//mm1 = A4A3A2A1R4R3R2R1
					//average the pixels

					movq			mm2,mm0			//mm2 = copy of mm0
					pxor			mm3,mm3			//mm3 = ________________
					movq			mm4,div3			//mm4 = 1/3 * 65536

					punpcklbw	mm2,mm3			//mm2 = __B4__B3__B2__B1
					punpckhbw	mm0,mm3			//mm0 = __G4__G3__G2__G1
					paddw			mm0,mm2			//mm0 = _BG4_BG3_BG2_BG1
					punpcklbw	mm1,mm3			//mm1 = __R4__R3__R2__R1
					paddw			mm0,mm1			//mm0 = RGB4RGB3RGB2RGB1
					movq			mm2,mm0
					
					pmullw		mm0,mm4			//mm0 = _A4l_A3l_A2l_A1l
					movq			mm1,mm0			//mm1 = copy of mm0
					pmulhw		mm2,mm4			//mm2 = _A4h_A3h_A2h_A1h
					punpcklwd	mm0,mm2			//mm0 = ______A2______A1
					punpckhwd	mm1,mm2			//mm1 = ______A4______A3
					psrad			mm0,16
					psrad			mm1,16
					// pack it up
					packssdw		mm0,mm1			//mm0 = __A4__A3__A2__A1

					packuswb		mm0,mm3			//mm0 = ________A4A3A2A1
					movd			[edi],mm0
					add			edi,4
					emms
					mov	[alphabuf],edi
					}

				} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=4) {
					__asm {
						mov	edi,alphabuf
						mov	eax,0
						mov	[edi],eax
						add	edi,4
						mov	[alphabuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-y-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,alphabuf
					mov	ecx,180
					mov	eax,0
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[alphabuf],edi
					}
				}
			}
		} //end field loop
	} //end renderalpha24

generalFX::~generalFX() {
	}

//Alpha Effects
int alphaFX::Callback(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			LRESULT chkstate;

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case afx_SOFT:
							SendMessage(softRD,BM_SETCHECK,BST_CHECKED,0);
							SendMessage(mediumRD,BM_SETCHECK,BST_UNCHECKED,0);
							SendMessage(hardRD,BM_SETCHECK,BST_UNCHECKED,0);
							tempabtype=afx_soft;
							break;
						case afx_MEDIUM:
							SendMessage(mediumRD,BM_SETCHECK,BST_CHECKED,0);
							SendMessage(softRD,BM_SETCHECK,BST_UNCHECKED,0);
							SendMessage(hardRD,BM_SETCHECK,BST_UNCHECKED,0);
							tempabtype=afx_medium;
							break;
						case afx_HARD:
							SendMessage(hardRD,BM_SETCHECK,BST_CHECKED,0);
							SendMessage(softRD,BM_SETCHECK,BST_UNCHECKED,0);
							SendMessage(mediumRD,BM_SETCHECK,BST_UNCHECKED,0);
							tempabtype=afx_hard;
							break;
						case afx_REVERSE:
							chkstate=SendMessage(reverseCHK,BM_GETCHECK,0,0);
							if (chkstate==BST_CHECKED) {
								SendMessage(reverseCHK,BM_SETCHECK,BST_UNCHECKED,0);
								tempreverse=FALSE;
								}
							else {
								SendMessage(reverseCHK,BM_SETCHECK,BST_CHECKED,0);
								tempreverse=TRUE;
								}
							break;
						case afx_OK:
							alphablendtype=tempabtype;
							reverse=tempreverse;
							DestroyWindow(window);
							break;
						case afx_CANCEL:
							DestroyWindow(window);
							break;
						default: goto noprocessCOMMAND;
						}
					return(0);
					} //end id bn clicked
				default: goto noprocessCOMMAND;
				} //End switch notifycode
noprocessCOMMAND:
			break;
			} //end WM_COMMAND

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


alphaFX::alphaFX(struct imagelist *mediaptr) {
	static BITMAPINFOHEADER bmi={
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

	union {
		LPBITMAPFILEHEADER sourcebmp;
		char *source;
		};
	LPBITMAPINFOHEADER lpBitmapInfoHeader;
	char *decodedbits;
	ULONG *dest;
	int x,y;
	long size;
	BOOL upsidedown;

	//TODO set default to preference
	alphablendtype=tempabtype=afx_medium;
	reverse=tempreverse=FALSE;

	//open source establish lpBitmapInfoHeader and decodedbits
	switch (mediaptr->mediatype) {
		case bmp:
			sourcebmp=(LPBITMAPFILEHEADER)load(mediaptr->filesource,&size,&pmem);
			lpBitmapInfoHeader=(LPBITMAPINFOHEADER)((char *)sourcebmp+sizeof(BITMAPFILEHEADER));
			decodedbits=(char *)sourcebmp+sourcebmp->bfOffBits;
			upsidedown=TRUE;
			break;
		case pcx:
			source=(char *)load(mediaptr->filesource,&size,&pmem);
			decodedbits=medialoaders->pcxobj.PCX2raw(source,size,&lpBitmapInfoHeader);
			upsidedown=FALSE;
			break;
		case tif:
			source=(char *)load(mediaptr->filesource,&size,&pmem);
			decodedbits=medialoaders->tifobj.TIF2raw(source,size,&lpBitmapInfoHeader);
			upsidedown=FALSE;
			break;
		case jpg:
			source=(char *)load(mediaptr->filesource,&size,&pmem);
			decodedbits=medialoaders->jpgobj.JPG2raw(source,size,&lpBitmapInfoHeader);
			upsidedown=TRUE;
			break;
		case iff:
			source=(char *)load(mediaptr->filesource,&size,&pmem);
			x=y=0;
			decodedbits=medialoaders->iffobj.ILBM2raw(source,size,(UWORD *)&x,(UWORD *)&y);
			bmi.biWidth=x;
			bmi.biHeight=y;
			bmi.biBitCount=32;
			lpBitmapInfoHeader=&bmi;
			upsidedown=FALSE;
			break;
		case tga:
			source=(char *)load(mediaptr->filesource,&size,&pmem);
			x=y=0;
			decodedbits=medialoaders->tgaobj.TGA2raw(source,size,(UWORD *)&x,(UWORD *)&y);
			bmi.biWidth=x;
			bmi.biHeight=y;
			bmi.biBitCount=32;
			lpBitmapInfoHeader=&bmi;
			upsidedown=FALSE;
			break;
		}

	mediaalpha=GlobalAlloc(GHND,345600); //Number of byte sized pixels
	dest=(ULONG *)GlobalLock(mediaalpha);
	//Now to convert the bitmap to ntsc order
	//from BGRA 720x480, or BGR or 256
	//to A 720x240 odd 720x240 even
	x=lpBitmapInfoHeader->biWidth;
	y=lpBitmapInfoHeader->biHeight;

	switch (lpBitmapInfoHeader->biBitCount) {
		case 8:
			renderalpha8(dest,x,y,decodedbits,lpBitmapInfoHeader,upsidedown);
			break;
		case 24:
			renderalpha24(dest,x,y,decodedbits,lpBitmapInfoHeader,upsidedown);
			break;
		case 32:
			renderalpha32(dest,x,y,decodedbits,lpBitmapInfoHeader,upsidedown);
			break;
		default: 
			wsprintf(string,"Alpha conversion of bitcount %d not supported");
			printc(string);
		}

	GlobalUnlock(mediaalpha);
	//Close source
	switch (mediaptr->mediatype) {
		case tga:
		case iff:
		case bmp:
		case jpg:
			if (source) dispose((struct memlist *)source,&pmem);
			break;
		case pcx:
		case tif:
			if (source) dispose((struct memlist *)source,&pmem);
			if (lpBitmapInfoHeader) disposenode(nodeobject,(struct memlist *)lpBitmapInfoHeader);
			break;
		}

	}


alphaFX::~alphaFX() {
	if (mediaalpha) GlobalFree(mediaalpha);
	mediaalpha=NULL;
	}

void alphaFX::doFX(struct imagelist *dve,ULONG *imagea,ULONG *imageb,ULONG *videobuf,ULONG dvetime,BOOL field) {
	UBYTE *alpha=(UBYTE *)GlobalLock(mediaalpha);
	switch (alphablendtype) {
		case afx_soft:
			TRAN_softBlend ((UBYTE *)videobuf,(UBYTE *)imageb,alpha,(dvetime-1)<<1,dve->duration<<1,reverse,field);
			break;
		case afx_medium:
	/**/
			TRAN_Blend ((UBYTE *)videobuf,(UBYTE *)imageb,alpha,(dvetime-1)<<1,dve->duration<<1,reverse,field);
	/**/
	/*
		__asm	{
		mov		edi, videobuf
		 mov		ecx, 691200
		mov		esi, imageb
		shr		ecx,4	
doagain:
		movq		mm0, [esi]
		 dec		ecx
		movq		mm1, [esi+8]
		movq		[edi], mm0
		 add		esi, 16
		movq		[edi+8], mm1
		add		edi, 16
		 cmp		ecx, 0
		jne		doagain
		emms
		}
	*/
			break;
		case afx_hard:
			TRAN_Gradient ((UBYTE *)videobuf,(UBYTE *)imageb,alpha,(dvetime-1)<<1,dve->duration<<1,reverse,field);
			break;
		}

	GlobalUnlock(mediaalpha);
	}

void alphaFX::openprefs() {
	struct tagPOINT mouseloc;
	int toolx=0;
	int tooly=0;
	GetCursorPos(&mouseloc);

	window=CreateWindowEx(WS_EX_DLGMODALFRAME,"OBJGRAYWIN",NULL,
		WS_POPUP|WS_VISIBLE,mouseloc.x-100,mouseloc.y-50,200,132,screen,
		NULL,hInst,NULL);
	CreateWindowEx(0,"BUTTON","DVE preferences:",WS_VISIBLE|WS_CHILD|BS_GROUPBOX,
		5,tooly,185,84,window,NULL,hInst,NULL);
	tooly+=CONTROLBUTTONY;
	toolx=10;

	softRD=CreateWindowEx(0,"BUTTON","Soft",WS_VISIBLE|WS_CHILD|BS_RADIOBUTTON,
		toolx,tooly,64,CONTROLBUTTONY,window,(HMENU)afx_SOFT,hInst,NULL);
	toolx+=90;

	reverseCHK=CreateWindowEx(0,"BUTTON","Reverse",WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		toolx,tooly,80,CONTROLBUTTONY,window,(HMENU)afx_REVERSE,hInst,NULL);
	tooly+=CONTROLBUTTONY;
	toolx=10;

	mediumRD=CreateWindowEx(0,"BUTTON","Medium",WS_VISIBLE|WS_CHILD|BS_RADIOBUTTON,
		toolx,tooly,72,CONTROLBUTTONY,window,(HMENU)afx_MEDIUM,hInst,NULL);
	tooly+=CONTROLBUTTONY;

	hardRD=CreateWindowEx(0,"BUTTON","Hard",WS_VISIBLE|WS_CHILD|BS_RADIOBUTTON,
		toolx,tooly,64,CONTROLBUTTONY,window,(HMENU)afx_HARD,hInst,NULL);
	tooly+=32;
	toolx=32;

	//check the right alphablend type
	switch (alphablendtype) {
		case afx_soft:
			SendMessage(softRD,BM_SETCHECK,BST_CHECKED,0);
			break;
		case afx_medium:
			SendMessage(mediumRD,BM_SETCHECK,BST_CHECKED,0);
			break;
		case afx_hard:
			SendMessage(hardRD,BM_SETCHECK,BST_CHECKED,0);
			break;
			}

	//check the right direction of transition
	if (reverse) SendMessage(reverseCHK,BM_SETCHECK,BST_CHECKED,0);

	CreateWindowEx(0,"BUTTON","Ok",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER|BS_CENTER,
		toolx,tooly,64,24,window,(HMENU)afx_OK,hInst,NULL);
	toolx+=72;
	CreateWindowEx(0,"BUTTON","Cancel",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER|BS_CENTER,
		toolx,tooly,64,24,window,(HMENU)afx_CANCEL,hInst,NULL);


	SetWindowLong(window,GWL_USERDATA,(long)this);
	}

