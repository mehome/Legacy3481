#include "HandleMain.h"  

const long RGB1[2]={0xb5817062,0x0000da1d};
const long RGB2[2]={0x7fff1917,0x000041cb};
const long RGB3[2]={0xa1bcedd3,0x00007062};
const long addoffsets[2]={0x00100080,0x00100080};
const long addtoy[2]={0x10001000,0x10001000};

static char scaleybuffer[5760];
void renderrgb16scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,BOOL upsidedown) {
	ULONG highrgb[2];
	ULONG lowyuv;
	ULONG *pixel;
	int xindex,yindex,t,loopscaley,doloopscale,centerx,centery,yloop,ystep;
	int yscaley,xscalex,cropy,cropx;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	UBYTE field;
	ULONG *lpscaleybuffer=(ULONG *)&scaleybuffer;
	ULONG *scalebufindex;
	ULONG bytesline=x<<1;
	ULONG *palette=(ULONG *)decodedbits; //This is only assuming BITMAPINFO type of data

	xscalex=x*scalex;
	yscaley=y*scaley;
	centerx=(720-xscalex)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((xscalex+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-yscaley)>>1;
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
	cropy=max(0,(yscaley-480)/scaley);
	if (xscalex>720) {
		cropx=720/scalex;
		xscalex=720;
		}
	else cropx=x&0xfffffffe;

	doloopscale=min(2,scaley);

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
					}
				}
			}

		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=!field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			for (loopscaley=0;loopscaley<doloopscale;loopscaley++) {
			scalebufindex=lpscaleybuffer;
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=4) {
				if (field) {
					if (scaley>1) {
						if (loopscaley^(!upsidedown)) pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*yindex));
						else pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*(yindex+1)));
						}
					else pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*yindex));
					}
				else {
					if (loopscaley^(!upsidedown)) pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*(yindex-1)));
					else pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*yindex));
					}
				__asm {
					mov			esi,scalebufindex
					mov			edi,videobuf

					mov			eax,pixel
					pxor			mm1,mm1			//mm1 = ________________
					movq			mm0,[eax]		//mm0 = RGB4RGB3RGB2RGB1

					movq			mm2,mm0
					psllw			mm2,11
					psrlw			mm2,8				//mm2 = __B4__B3__B2__B1
					movq			mm3,mm0
					psllw			mm3,6
					psrlw			mm3,8				//mm3 = __G4__G3__G2__G1
					movq			mm4,mm0
					psllw			mm4,1
					psrlw			mm4,8				//mm4 = __R4__R3__R2__R1

					movq			mm5,mm2
					punpckhwd	mm5,mm1			//mm5 = ______B4______B3
					movq			mm6,mm3
					punpckhwd	mm6,mm1			//mm6 = ______G4______G3
					pslld			mm6,8				//mm6 = ____G4______G3__
					por			mm5,mm6			//mm5 = ____G4B4____G3B3
					movq			mm6,mm4
					punpckhwd	mm6,mm1			//mm6 = ______R4______R3
					pslld			mm6,16			//mm6 = __R4______R3____
					por			mm5,mm6			//mm5 = __R4G4B4__R3G3B3
					movq			highrgb,mm5

					movq			mm5,mm2
					punpcklwd	mm5,mm1			//mm5 = ______B2______B1
					movq			mm6,mm3
					punpcklwd	mm6,mm1			//mm6 = ______G2______G1
					pslld			mm6,8				//mm6 = ____G2______G1__
					por			mm5,mm6			//mm5 = ____G2B2____G1B1
					movq			mm6,mm4
					punpcklwd	mm6,mm1			//mm6 = ______R2______R1
					pslld			mm6,16			//mm6 = __R2______R1____
					por			mm5,mm6			//mm5 = __R2G2B2__R1G1B1
					movq			mm0,mm5

														//mm0 = A2R2G2B2A1R1G1B1
					//Convert first 2 pixels to YUV
					movq			mm2,mm0			//mm2 = ________A2R2G2B2
					psrlq			mm2,32

					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			lowyuv,mm0

														//mm0 = A2R2G2B2A1R1G1B1
					//Convert pixels 3&4 to YUV
					movq			mm0,highrgb
					movq			mm2,mm0			//mm2 = ________A2R2G2B2
					psrlq			mm2,32

					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1

					//Combine low and high pixels
					psllq			mm0,32			//mm0 = Y4V4Y3U3________
					movd			mm1,lowyuv		//mm1 = ________Y2V2Y1U1
					por			mm0,mm1			//mm0 = Y4V4Y3V3Y2V2Y1U1
					//Arrange YUV pixels for scaling

					movq			mm2,mm0
					psrld			mm2,24	//mm2 = ______Y4______Y2
					pslld			mm2,8		//mm2 = ____Y4______Y2__
					movq			mm3,mm0
					psrld			mm3,8		//mm3 = __Y4V4Y3__Y2V2Y1
					pslld			mm3,24	//mm3 = Y3______Y1______
					movq			mm4,mm0
					pcmpeqb		mm1,mm1	//mm1 = ffffffffffffffff
					psrld			mm1,8		//mm1 = __ffffff__ffffff
					movq			mm5,mm0
					pand			mm4,mm1	//mm4 = __V4Y3V3__V2Y1U1
					por			mm4,mm3	//mm4 = Y3V4Y3V3Y1V2Y1U1
					pcmpeqb		mm1,mm1	//mm1 = ffffffffffffffff
					psrlw			mm1,8		//mm1 = __ff__ff__ff__ff
					pand			mm5,mm1	//mm5 = __V4__V3__V2__V1
					por			mm5,mm2	//mm5 = __V4Y4V3__V2Y2V1
					pslld			mm2,16	//mm2 = Y4______Y2______
					por			mm5,mm2	//mm5 = Y4V4Y4V3Y2V2Y2V1
					//mm4 first set of pixels
					//mm5 second set of pixels
					}
				//do {
				//fourtimes=scalex>>2;
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel1:
					movd			[edi],mm4
					add			edi,4
					movd			[esi],mm4
					add			esi,4
					dec			ecx
					jne			pixel1
					}
				//} while (fourtimes--);

					//do second pixel now 
				//fourtimes=scalex>>2;
				//do {
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel2:
					movd			[edi],mm5
					add			edi,4
					movd			[esi],mm5
					add			esi,4
					dec			ecx
					jne			pixel2
					}
				//} while (fourtimes--);

				__asm {
					psrlq		mm4,32	//mm4 = ________Y3V4Y3V3
					psrlq		mm5,32	//mm5 = ________Y4V4Y4V3
					}

				//fourtimes=scalex>>2;
				//do {
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel3:
					movd			[edi],mm4
					add			edi,4
					movd			[esi],mm4
					add			esi,4
					dec			ecx
					jne			pixel3
					}
				//} while (fourtimes--);

					//do fourth pixel now 
				//fourtimes=scalex>>2;
				//do {
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel4:
					movd			[edi],mm5
					add			edi,4
					movd			[esi],mm5
					add			esi,4
					dec			ecx
					jne			pixel4
					}
				//} while (fourtimes--);

				__asm {
					emms
					mov			[videobuf],edi
					mov			[scalebufindex],esi
					}
				} //end x loop
			//fill in remainder of x
			if (xscalex+centerx<720) {
				for (xindex=xscalex+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (t=(scaley-1)>>1;t;t--) {
				__asm {
/**/
					mov	edi,videobuf
					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx
					shr	ecx,1
					mov	eax,10801080h
loopecx3:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx3
nocenterx:
					mov	ecx,xscalex
					shr	ecx,1
					mov	esi,lpscaleybuffer
					rep	movsd

					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx2
					mov	eax,xscalex
					add	eax,centerx
					shr	eax,1
					mov	ecx,360
					sub	ecx,eax
/**/					
					//mov	ecx,360
					mov	eax,10801080h
loopecx2:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx2
nocenterx2:
					mov	[videobuf],edi
					}
				}
			} //end loopscaley	
			yindex+=ystep;
			} //end y loop
		if (yscaley<480) {
			for (yindex=(480-yscaley-centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderyuvscale


void renderrgb16(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown) {
	ULONG *pixel;	
	ULONG highrgb[2];
	ULONG lowyuv;
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	ULONG bytesline=x<<1;
	UBYTE field;

	//Center
	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
	centery+=calibratey;
	centery&=0xfffffffe;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	//Crop
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffe;

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
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
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			//for (xindex=0;xindex<cropx;xindex+=4) {
				pixel=(ULONG *)(decodedbits+(bytesline*yindex));
				__asm {
					mov			ecx,cropx
					mov			edi,videobuf
					mov			eax,pixel
loopx:
					pxor			mm1,mm1			//mm1 = ________________
					movq			mm0,[eax]		//mm0 = RGB4RGB3RGB2RGB1

					movq			mm2,mm0
					psllw			mm2,11
					psrlw			mm2,8				//mm2 = __B4__B3__B2__B1
					movq			mm3,mm0
					psllw			mm3,6
					psrlw			mm3,8				//mm3 = __G4__G3__G2__G1
					movq			mm4,mm0
					psllw			mm4,1
					psrlw			mm4,8				//mm4 = __R4__R3__R2__R1

					movq			mm5,mm2
					punpckhwd	mm5,mm1			//mm5 = ______B4______B3
					movq			mm6,mm3
					punpckhwd	mm6,mm1			//mm6 = ______G4______G3
					pslld			mm6,8				//mm6 = ____G4______G3__
					por			mm5,mm6			//mm5 = ____G4B4____G3B3
					movq			mm6,mm4
					punpckhwd	mm6,mm1			//mm6 = ______R4______R3
					pslld			mm6,16			//mm6 = __R4______R3____
					por			mm5,mm6			//mm5 = __R4G4B4__R3G3B3
					movq			highrgb,mm5

					movq			mm5,mm2
					punpcklwd	mm5,mm1			//mm5 = ______B2______B1
					movq			mm6,mm3
					punpcklwd	mm6,mm1			//mm6 = ______G2______G1
					pslld			mm6,8				//mm6 = ____G2______G1__
					por			mm5,mm6			//mm5 = ____G2B2____G1B1
					movq			mm6,mm4
					punpcklwd	mm6,mm1			//mm6 = ______R2______R1
					pslld			mm6,16			//mm6 = __R2______R1____
					por			mm5,mm6			//mm5 = __R2G2B2__R1G1B1
					movq			mm0,mm5

														//mm0 = A2R2G2B2A1R1G1B1
					//Convert first 2 pixels to YUV

					movq			mm2,mm0			//mm2 = ________A2R2G2B2
					psrlq			mm2,32

					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			lowyuv,mm0

														//mm0 = A2R2G2B2A1R1G1B1
					//Convert pixels 3&4 to YUV
					movq			mm0,highrgb
					movq			mm2,mm0			//mm2 = ________A2R2G2B2
					psrlq			mm2,32

					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1

					//Combine low and high pixels
					psllq			mm0,32			//mm0 = Y4V4Y3U3________
					movd			mm1,lowyuv		//mm1 = ________Y2V2Y1U1
					por			mm0,mm1			//mm0 = Y4V4Y3V3Y2V2Y1U1


					movq			[edi],mm0
					add			edi,8
					add			eax,8
					sub			ecx,4
					cmp			ecx,0
					jne			loopx
					mov			[videobuf],edi
					emms
					}

				//} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-(y&0xfffffffe)-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderyuv


void renderyuvscale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,BOOL upsidedown) {
	ULONG *pixel;
	int xindex,yindex,t,loopscaley,doloopscale,centerx,centery,yloop,ystep;
	int yscaley,xscalex,cropy,cropx;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	UBYTE field;
	ULONG *lpscaleybuffer=(ULONG *)&scaleybuffer;
	ULONG *scalebufindex;
	ULONG bytesline=x<<1;
	ULONG *palette=(ULONG *)decodedbits; //This is only assuming BITMAPINFO type of data

	xscalex=x*scalex;
	yscaley=y*scaley;
	centerx=(720-xscalex)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((xscalex+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-yscaley)>>1;
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
	cropy=max(0,(yscaley-480)/scaley);
	if (xscalex>720) {
		cropx=720/scalex;
		xscalex=720;
		}
	else cropx=x&0xfffffffe;

	doloopscale=min(2,scaley);

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
					}
				}
			}

		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=!field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			for (loopscaley=0;loopscaley<doloopscale;loopscaley++) {
			scalebufindex=lpscaleybuffer;
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=4) {
				if (field) {
					if (scaley>1) {
						if (loopscaley^(!upsidedown)) pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*yindex));
						else pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*(yindex+1)));
						}
					else pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*yindex));
					}
				else {
					if (loopscaley^(!upsidedown)) pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*(yindex-1)));
					else pixel=(ULONG *)(decodedbits+(xindex<<1)+(bytesline*yindex));
					}
				__asm {
					mov			esi,scalebufindex
					mov			edi,videobuf

					mov			eax,pixel
					movq			mm0,[eax]
					movq			mm1,addtoy
					psubusb		mm0,mm1
					paddusb		mm0,mm1
					paddusb		mm0,mm1
					psubusb		mm0,mm1	//mm0 = Y4V4Y3V3Y2V2Y1U1
					movq			mm2,mm0
					psrld			mm2,24	//mm2 = ______Y4______Y2
					pslld			mm2,8		//mm2 = ____Y4______Y2__
					movq			mm3,mm0
					psrld			mm3,8		//mm3 = __Y4V4Y3__Y2V2Y1
					pslld			mm3,24	//mm3 = Y3______Y1______
					movq			mm4,mm0
					pcmpeqb		mm1,mm1	//mm1 = ffffffffffffffff
					psrld			mm1,8		//mm1 = __ffffff__ffffff
					movq			mm5,mm0
					pand			mm4,mm1	//mm4 = __V4Y3V3__V2Y1U1
					por			mm4,mm3	//mm4 = Y3V4Y3V3Y1V2Y1U1
					pcmpeqb		mm1,mm1	//mm1 = ffffffffffffffff
					psrlw			mm1,8		//mm1 = __ff__ff__ff__ff
					pand			mm5,mm1	//mm5 = __V4__V3__V2__V1
					por			mm5,mm2	//mm5 = __V4Y4V3__V2Y2V1
					pslld			mm2,16	//mm2 = Y4______Y2______
					por			mm5,mm2	//mm5 = Y4V4Y4V3Y2V2Y2V1
					//mm4 first set of pixels
					//mm5 second set of pixels
					}
				//do {
				//fourtimes=scalex>>2;
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel1:
					movd			[edi],mm4
					add			edi,4
					movd			[esi],mm4
					add			esi,4
					dec			ecx
					jne			pixel1
					}
				//} while (fourtimes--);

					//do second pixel now 
				//fourtimes=scalex>>2;
				//do {
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel2:
					movd			[edi],mm5
					add			edi,4
					movd			[esi],mm5
					add			esi,4
					dec			ecx
					jne			pixel2
					}
				//} while (fourtimes--);

				__asm {
					psrlq		mm4,32	//mm4 = ________Y3V4Y3V3
					psrlq		mm5,32	//mm5 = ________Y4V4Y4V3
					}

				//fourtimes=scalex>>2;
				//do {
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel3:
					movd			[edi],mm4
					add			edi,4
					movd			[esi],mm4
					add			esi,4
					dec			ecx
					jne			pixel3
					}
				//} while (fourtimes--);

					//do fourth pixel now 
				//fourtimes=scalex>>2;
				//do {
				__asm {
					mov			ecx,scalex
					shr			ecx,1
pixel4:
					movd			[edi],mm5
					add			edi,4
					movd			[esi],mm5
					add			esi,4
					dec			ecx
					jne			pixel4
					}
				//} while (fourtimes--);

				__asm {
					emms
					mov			[videobuf],edi
					mov			[scalebufindex],esi
					}
				} //end x loop
			//fill in remainder of x
			if (xscalex+centerx<720) {
				for (xindex=xscalex+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (t=(scaley-1)>>1;t;t--) {
				__asm {
/**/
					mov	edi,videobuf
					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx
					shr	ecx,1
					mov	eax,10801080h
loopecx3:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx3
nocenterx:
					mov	ecx,xscalex
					shr	ecx,1
					mov	esi,lpscaleybuffer
					rep	movsd

					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx2
					mov	eax,xscalex
					add	eax,centerx
					shr	eax,1
					mov	ecx,360
					sub	ecx,eax
/**/					
					//mov	ecx,360
					mov	eax,10801080h
loopecx2:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx2
nocenterx2:
					mov	[videobuf],edi
					}
				}
			} //end loopscaley	
			yindex+=ystep;
			} //end y loop
		if (yscaley<480) {
			for (yindex=(480-yscaley-centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderyuvscale


void renderyuv(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown) {
	ULONG *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	ULONG bytesline=x<<1;
	UBYTE field;

	//Center
	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
	centery+=calibratey;
	centery&=0xfffffffe;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	//Crop
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffe;

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
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
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			//for (xindex=0;xindex<cropx;xindex+=4) {
				pixel=(ULONG *)(decodedbits+(bytesline*yindex));
				__asm {
					mov			ecx,cropx
					mov			edi,videobuf
					mov			eax,pixel
loopx:
					movq			mm0,[eax]
					movq			mm1,addtoy
					psubusb		mm0,mm1
					paddusb		mm0,mm1
					paddusb		mm0,mm1
					psubusb		mm0,mm1
					movq			[edi],mm0
					add			edi,8
					add			eax,8
					sub			ecx,4
					cmp			ecx,0
					jne			loopx
					mov			[videobuf],edi
					emms
					}

				//} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-(y&0xfffffffe)-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderyuv


void renderrgb8scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo) {
	ULONG *pixel;	
	int xindex,yindex,t,fourtimes,loopscaley,doloopscale,centerx,centery;
	int yscaley,xscalex,cropy,cropx;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	UBYTE field;
	ULONG *lpscaleybuffer=(ULONG *)&scaleybuffer;
	ULONG *scalebufindex;
	ULONG bytesline=(x+3)&0xfffffffc;
	ULONG *palette=(ULONG *)((char *)bitmapinfo+bitmapinfo->biSize);

	y&=0xfffffffe;
	int numofcolors=bitmapinfo->biClrUsed;
	if (numofcolors==0) numofcolors=256;
	xscalex=x*scalex;
	yscaley=y*scaley;
	centerx=(720-xscalex)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((xscalex+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-yscaley)>>1;
	centery+=calibratey;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(yscaley-480)/scaley);
	if (xscalex>720) {
		cropx=720/scalex;
		xscalex=720;
		}
	else cropx=x&0xfffffffe;

	doloopscale=min(2,scaley);

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
					}
				}
			}

		for (yindex=y-1-field;yindex>=cropy;yindex-=2) {
			for (loopscaley=0;loopscaley<doloopscale;loopscaley++) {
			scalebufindex=lpscaleybuffer;
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=2) {
				if (field) {
					if (scaley>1) {
						if (loopscaley) pixel=(ULONG *)(decodedbits+xindex+(bytesline*yindex));
						else pixel=(ULONG *)(decodedbits+xindex+(bytesline*(yindex+1)));
						}
					else pixel=(ULONG *)(decodedbits+xindex+(bytesline*yindex));
					}
				else {
					if (loopscaley) pixel=(ULONG *)(decodedbits+xindex+(bytesline*(yindex-1)));
					else pixel=(ULONG *)(decodedbits+xindex+(bytesline*yindex));
					}
				fourtimes=scalex>>2;
				__asm {
					mov			esi,scalebufindex
					mov			edi,videobuf
					}
				do {
				__asm {
					mov			eax,pixel
					xor			edx,edx
					mov			dl,[eax]
					shl			edx,2
					mov			eax,palette
					add			eax,edx

					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					movq			mm2,mm0			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm0			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					}
				} while (fourtimes--);

					//do second pixel now 
				fourtimes=scalex>>2;
				do {
				__asm {
					mov			eax,pixel
					add			eax,1
					xor			edx,edx
					mov			dl,[eax]
					shl			edx,2
					mov			eax,palette
					add			eax,edx

					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					movq			mm2,mm0			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm0			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					emms
					mov			[videobuf],edi
					mov			[scalebufindex],esi
					}
				} while (fourtimes--);
				} //end x loop
			//fill in remainder of x
			if (xscalex+centerx<720) {
				for (xindex=xscalex+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (t=(scaley-1)>>1;t;t--) {
				__asm {
/**/
					mov	edi,videobuf
					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx
					shr	ecx,1
					mov	eax,10801080h
loopecx3:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx3
nocenterx:
					mov	ecx,xscalex
					shr	ecx,1
					mov	esi,lpscaleybuffer
					rep	movsd

					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx2
					mov	eax,xscalex
					add	eax,centerx
					shr	eax,1
					mov	ecx,360
					sub	ecx,eax
/**/					
					//mov	ecx,360
					mov	eax,10801080h
loopecx2:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx2
nocenterx2:
					mov	[videobuf],edi
					}
				}
			} //end loopscaley	
			} //end y loop
		if (yscaley<480) {
			for (yindex=(480-yscaley-centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderrgb8scale



void renderrgb8(ULONG *videobuf,int x,int y,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo,BOOL upsidedown) {
	char *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	ULONG bytesline=(x+3)&0xfffffffc;
	ULONG *palette=(ULONG *)((char *)bitmapinfo+bitmapinfo->biSize);
	UBYTE field;

	int numofcolors=bitmapinfo->biClrUsed;
	if (numofcolors==0) numofcolors=256;
	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
	centery+=calibratey;
	centery&=0xfffffffe;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffe;

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
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
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=2) {
				pixel=(decodedbits+xindex+(bytesline*yindex));
				__asm {
					mov			edi,videobuf
					mov			eax,pixel
					xor			edx,edx
					mov			dl,[eax]
					shl			edx,2
					mov			eax,palette
					add			eax,edx
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff

					mov			eax,pixel
					add			eax,1
					xor			edx,edx
					mov			dl,[eax]
					shl			edx,2
					mov			eax,palette
					add			eax,edx

					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					emms
					mov	[videobuf],edi
					}

				} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-(y&0xfffffffe)-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderrgb8


void renderrgb24scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits) {
	ULONG *pixel;	
	int xindex,yindex,t,fourtimes,loopscaley,doloopscale,centerx,centery;
	int yscaley,xscalex,cropy,cropx;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	UBYTE field;
	ULONG *lpscaleybuffer=(ULONG *)&scaleybuffer;
	ULONG *scalebufindex;
	ULONG bytesline=(x*3+3)&0xfffffffc;

	y&=0xfffffffe;
	xscalex=((x)&0xfffffffc)*scalex;
	yscaley=y*scaley;
	centerx=(720-xscalex)>>1;
	centerx=(centerx+calibratex)&0xfffffffc;
	if ((xscalex+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-yscaley)>>1;
	centery+=calibratey;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(yscaley-480)/scaley);
	if (xscalex>720) {
		cropx=720/scalex;
		xscalex=720;
		}
	else cropx=(x)&0xfffffffc;

	doloopscale=min(2,scaley);

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
					}
				}
			}

		for (yindex=y-1-field;yindex>=cropy;yindex-=2) {
			for (loopscaley=0;loopscaley<doloopscale;loopscaley++) {
			scalebufindex=lpscaleybuffer;
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=2) {
				if (field) {
					if (scaley>1) {
						if (loopscaley) pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*yindex));
						else pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*(yindex+1)));
						}
					else pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*yindex));
					}
				else {
					if (loopscaley) pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*(yindex-1)));
					else pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*yindex));
					}
				fourtimes=scalex>>2;
				__asm {
					mov			esi,scalebufindex
					mov			edi,videobuf
					}
				do {
				__asm {
					mov			eax,pixel
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Bp2_Rp1_Gp1_Bp1
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					movq			mm2,mm0			//mm2 = _____Rp1_Gp1_Bp1
					movq			mm5,mm0			//mm5 = _____Rp1_Gp1_Bp1
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = UYVY
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					}
				} while (fourtimes--);

					//do all second pixel now 
				fourtimes=scalex>>2;
				do {
				__asm {
					mov			eax,pixel
					add			eax,3
					movd			mm0,[eax]
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Bp3_Rp2_Gp2_Bp2
					pand			mm0,mm3			//mm0 = _____Rp2_Gp2_Bp2
					movq			mm4,mm0			//mm4 = _____Rp2_Gp2_Bp2
					movq			mm2,mm0			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm0			//mm5 = _____Rp2_Gp2_Bp2

					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = UYVY
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					emms
					mov	[videobuf],edi
					mov	[scalebufindex],esi
					}
				} while (fourtimes--);
				} //end x loop
			//fill in remainder of x
			if (xscalex+centerx<720) {
				for (xindex=xscalex+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (t=(scaley-1)>>1;t;t--) {
				__asm {
/**/
					mov	edi,videobuf
					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx
					shr	ecx,1
					mov	eax,10801080h
loopecx3:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx3
nocenterx:
					mov	ecx,xscalex
					shr	ecx,1
					mov	esi,lpscaleybuffer
					rep	movsd

					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx2
					mov	eax,xscalex
					add	eax,centerx
					shr	eax,1
					mov	ecx,360
					sub	ecx,eax
/**/					
					//mov	ecx,360
					mov	eax,10801080h
loopecx2:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx2
nocenterx2:
					mov	[videobuf],edi
					}
				}
			} //end loopscaley	
			} //end y loop
		if (yscaley<480) {
			for (yindex=(480-yscaley-centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderrgb24scale


void renderrgb32scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,BOOL upsidedown) {
	ULONG *pixel;	
	int xindex,yindex,t,fourtimes,loopscaley,doloopscale,centerx,centery,yloop,ystep;
	int yscaley,xscalex,cropy,cropx;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	UBYTE field;
	ULONG *lpscaleybuffer=(ULONG *)&scaleybuffer;
	ULONG *scalebufindex;
	ULONG bytesline=x<<2;
/*
	xscalex=x*scalex;
	yscaley=y*scaley;
	centerx=(720-xscalex)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((xscalex+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-yscaley)>>1;
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
*/

	y&=0xfffffffe;
	xscalex=((x)&0xfffffffc)*scalex;
	yscaley=y*scaley;
	centerx=(720-xscalex)>>1;
	centerx=(centerx+calibratex)&0xfffffffc;
	if ((xscalex+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-yscaley)>>1;
	centery+=calibratey;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	cropy=max(0,(yscaley-480)/scaley);
	if (xscalex>720) {
		cropx=720/scalex;
		xscalex=720;
		}
	else cropx=(x)&0xfffffffc;

	doloopscale=min(2,scaley);

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
					}
				}
			}
		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=!field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			for (loopscaley=0;loopscaley<doloopscale;loopscaley++) {
			scalebufindex=lpscaleybuffer;
			if (centerx) {
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=2) {
				if (field) {
					if (scaley>1) {
						if (loopscaley^(!upsidedown)) pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*yindex));
						else pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*(yindex+1)));
						}
					else pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*yindex));
					}
				else {
					if (loopscaley^(!upsidedown)) pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*(yindex-1)));
					else pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*yindex));
					}
				fourtimes=scalex>>2;
				__asm {
					mov			esi,scalebufindex
					mov			edi,videobuf
					}
				do {
				__asm {
					mov			eax,pixel
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					movq			mm2,mm0			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm0			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					}
				} while (fourtimes--);

					//do all second pixel now 
				fourtimes=scalex>>2;
				do {
				__asm {
					mov			eax,pixel
					add			eax,4
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					movq			mm2,mm0			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm0			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					movd			[esi],mm0
					add			esi,4
					emms
					mov			[videobuf],edi
					mov			[scalebufindex],esi
					}
				} while (fourtimes--);
				} //end x loop
			//fill in remainder of x
			if (xscalex+centerx<720) {
				for (xindex=xscalex+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (t=(scaley-1)>>1;t;t--) {
				__asm {
/**/
					mov	edi,videobuf
					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx
					shr	ecx,1
					mov	eax,10801080h
loopecx3:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx3
nocenterx:
					mov	ecx,xscalex
					shr	ecx,1
					mov	esi,lpscaleybuffer
					rep	movsd

					mov	ecx,centerx
					cmp	ecx,0
					je		nocenterx2
					mov	eax,xscalex
					add	eax,centerx
					shr	eax,1
					mov	ecx,360
					sub	ecx,eax
/**/					
					//mov	ecx,360
					mov	eax,10801080h
loopecx2:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx2
nocenterx2:
					mov	[videobuf],edi
					}
				}
			} //end loopscaley
			yindex+=ystep;
			} //end y loop
		if (yscaley<480) {
			for (yindex=(480-yscaley-centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderrgb32scale


void renderrgb24(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown) {
	//note differences: bytesline... the add 3 and * 3 in x loop
	ULONG *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	ULONG bytesline=(x*3+3)&0xfffffffc;
	UBYTE field;

	//Center
	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
/*
	__asm {
		mov	ecx,y
		and	ecx,1
		mov	eax,centery
		and	eax,0fffffffeh
		or		eax,ecx
		mov	[centery],eax
		}
*/
	centery+=calibratey;
	centery&=0xfffffffe;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	//Crop
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffe;

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
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
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=2) {
				pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*yindex));
				__asm {
					mov			edi,videobuf
					mov			eax,pixel
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					add			eax,3
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Bp2_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Bp3_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					emms
					mov			[videobuf],edi
					}

				} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-(y&0xfffffffe)-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderrgb24




void renderrgb32(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown) {
	ULONG *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx,yloop,ystep;
	int calibratex=0;
	int calibratey=0;
	long size=0;
	ULONG bytesline=x<<2;
	UBYTE field;

	//Center
	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
	if ((x+calibratex<1)||(centerx<1)) centerx=0;
	centery=(480-y)>>1;
/*
	__asm {
		mov	ecx,y
		and	ecx,1
		mov	eax,centery
		and	eax,0fffffffeh
		or		eax,ecx
		mov	[centery],eax
		}
*/
	centery+=calibratey;
	centery&=0xfffffffe;
	if ((y+calibratey<1)||(centery<1)) centery=0;
	//Crop
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffe;

	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		if (centery) {
			for (yindex=(centery)>>1;yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx1:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx1
					mov	[videobuf],edi
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
				for (xindex=centerx;xindex;xindex-=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			for (xindex=0;xindex<cropx;xindex+=2) {
				pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*yindex));
				__asm {
					mov			edi,videobuf
					mov			eax,pixel
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					add			eax,4
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1
					punpcklbw	mm2,mm1			//mm2 = _Ap2_Rp2_Gp2_Bp2
					pand			mm2,mm3			//mm2 = _____Rp2_Gp2_Bp2
					movq			mm5,mm2			//mm5 = _____Rp2_Gp2_Bp2
					//28770*B-19071*G-9699*R
					pmaddwd		mm0,RGB1			//mm0 = ____Rp1a____GBp1
					movq			mm6,mm0			//mm6 = ____Rp1a____GBp1
					psllq			mm6,32			//mm6 = ____GBp1________
					psrlq			mm3,16			//mm3 = ________ffffffff
					//16843*R+33030*G+6423*B
					pmaddwd		mm4,RGB2			//mm4 = ____Rp1b____GBp1
					movq			mm7,mm4			//mm7 = ____Rp1b____GBp1
					psrlq			mm7,32			//mm7 = ____________Rp1b
					pand			mm4,mm3			//mm4 = ___________GBp1b
					por			mm6,mm4			//mm6 = ___GBp1a___GBp1b
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm0,mm3			//mm0 = ____Rp1a________
					por			mm0,mm7			//mm0 = ____Rp1a____Rp1b
					paddd			mm0,mm6			//mm0 = ____RGB1____RGB2
					psrad			mm0,16			//mm0 = >>16
					//Regs free mm1,mm3,mm4,mm6,mm7
					//28770*R1-24117*G1-4653*B1
					pmaddwd		mm2,RGB3			//mm2 = ____Rp2c___GBp2c
					movq			mm6,mm2			//mm6 = ____Rp2c___GBp2c
					psllq			mm6,32			//mm6 = ___GBp2c________
					psrlq			mm3,32			//mm3 = ________ffffffff
					//16843*R1+33030*G1+6423*B1
					pmaddwd		mm5,RGB2			//mm5 = ____Rp2d___GBp2d
					movq			mm7,mm5			//mm7 = ____Rp2d___GBp2d
					psrlq			mm7,32			//mm7 = ____________Rp2d
					pand			mm5,mm3			//mm5 = ___________GBp2d
					por			mm6,mm5			//mm6 = ___GBp2c___GBp2d
					psllq			mm3,32			//mm3 = ffffffff________
					pand			mm2,mm3			//mm2 = ____Rp2c________
					por			mm2,mm7			//mm2 = ____Rp2c____Rp2d
					paddd			mm2,mm6			//mm2 = ____RGB3____RGB4
					psrad			mm2,16			//mm2 = >>16
					//Regs free all but mm0 and mm2
					//pack it up oops turn them around
					movq			mm4,mm0
					psrlq			mm0,32
					psllq			mm4,32
					por			mm0,mm4			//mm0 = ____RGB2____RGB1 
					movq			mm3,mm2
					psrlq			mm2,32
					psllq			mm3,32
					por			mm2,mm3			//mm2 = ____RGB4____RGB3
					packssdw		mm0,mm2			//mm0 = RGB4RGB3RGB2RGB1
					//add the final offsets
					paddw			mm0,addoffsets
					packuswb		mm0,mm1			//mm0 = ________Y2V2Y1U1
					movd			[edi],mm0
					add			edi,4
					emms
					mov			[videobuf],edi
					}

				} //end x loop
			//fill in remainder of x
			if (x+centerx<720) {
				for (xindex=x+centerx;xindex<720;xindex+=2) {
					__asm {
						mov	edi,videobuf
						mov	eax,10801080h
						mov	[edi],eax
						add	edi,4
						mov	[videobuf],edi
						}
					}
				}
			yindex+=ystep;
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-(y&0xfffffffe)-centery)>>1);yindex;yindex--) {
				__asm {
					mov	edi,videobuf
					mov	ecx,360
					mov	eax,10801080h
loopecx:
					mov	[edi],eax
					add	edi,4
					dec	ecx
					cmp	ecx,0
					jne	loopecx
					mov	[videobuf],edi
					}
				}
			}
		} //end field loop
	} //end renderrgb32


