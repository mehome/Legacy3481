/*
void renderrgb24(ULONG *videobuf,int x,int y,char *decodedbits) {
	ULONG *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx;
	int calibratex=-24;
	int calibratey=0;
	long size=0;
	ULONG bytesline=x*3;
	UBYTE field;

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
		for (yindex=y-1-field;yindex>=cropy;yindex-=2) {
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
				pixel=(ULONG *)(decodedbits+(xindex*3)+(bytesline*yindex));
				__asm {
					mov			edi,videobuf
					mov			eax,pixel
					movd			mm0,[eax]
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					add			eax,4
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Bp2_Rp1_Gp1_Bp1
					movq			mm5,mm0			//mm5 = _Bp2_Rp1_Gp1_Bp1
					psrlq			mm5,48			//mm5 = _____________Bp2
					movd			mm2,[eax]
					pand			mm0,mm3			//mm0 = _____Rp1_Gp1_Bp1
					movq			mm4,mm0			//mm4 = _____Rp1_Gp1_Bp1

					punpcklbw	mm2,mm1			//mm2 = _Gp3_Bp3_Rp2_Gp2
					psllq			mm2,16			//mm2 = _Bp3_Rp2_Gp2____
					por			mm2,mm5			//mm2 = _Bp3_Rp2_Gp2_Bp2

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
					packuswb		mm0,mm1			//mm0 = UYVY
					movd			[edi],mm0
					add			edi,4

					//unroll second set 

					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					movd			mm0,[eax]
					add			eax,4
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Gp3_Bp3_Rp2_Gp2
					psrlq			mm0,32			//mm0 = _________Gp3_Bp3
					movd			mm2,[eax]

					punpcklbw	mm2,mm1			//mm2 = _Rp4_Gp4_Bp4_Rp3

					movq			mm5,mm2			//mm5 = _Rp4_Gp4_Bp4_Rp3
					psllq			mm5,48			//mm5 = _Rp3____________
					psrlq			mm5,16			//mm5 = _____Rp3________
					por			mm0,mm5			//mm0 = _____Rp3_Gp3_Bp3
					movq			mm4,mm0			//mm4 = _____Rp3_Gp3_Bp3

					psrl			mm2,16			//mm2 = _____Rp4_Gp4_Bp4
					movq			mm5,mm2			//mm2 = _____Rp4_Gp4_Bp4
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
					packuswb		mm0,mm1			//mm0 = __Y2__V2__Y1__U1
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
			} //end y loop
		if (y<480) {
			for (yindex=(480-(y&0xfffffffe)-(centery))>>1;yindex;yindex--) {
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
*/

/*
alphaFX::alphaFX(struct imagelist *mediaptr) {
	static UBYTE alphabuffer[1382400]; //hmmm 720 x 480 x 4
	HDC hdc;
	HBITMAP hbm;
	BITMAP              bm;         // bitmap structure 
	BITMAPINFOHEADER    bi;         // bitmap header 
	UBYTE *dest;
	long t;
		alphablendtype=tempabtype=afx_medium;
		//extract the bitmap raw data into global moveable memory
		hbm=(HBITMAP)LoadImage(hInst,mediaptr->filesource,IMAGE_BITMAP,VIDEOX,VIDEOY,LR_LOADFROMFILE);
		//begin locking the alpha image
		//init the mediaalpha for error trapping
		mediaalpha=NULL;

		if (!GetObject(hbm,sizeof(bm),(LPSTR)&bm)) goto errorintrans;

		bi.biSize = sizeof(BITMAPINFOHEADER); 
		bi.biWidth = bm.bmWidth; 
		bi.biHeight = bm.bmHeight; 
		bi.biPlanes = 1; 
		bi.biBitCount = 32; 
		bi.biCompression = BI_RGB; 
		bi.biSizeImage = 0; 
		bi.biXPelsPerMeter = 0; 
		bi.biYPelsPerMeter = 0; 
		bi.biClrUsed = 0; 
		bi.biClrImportant = 0;

		hdc=GetDC(screen);

		if (GetDIBits(hdc,hbm,0,(UINT)bi.biHeight,alphabuffer,(LPBITMAPINFO)&bi,
			DIB_RGB_COLORS)==0) goto errorintrans;

		//GetBitmapBits(hbm,1382400,alphabuffer);
		DeleteObject(hbm);
		ReleaseDC(screen,hdc);

		//end locking the alpha image
		//Now to convert the bitmap to ntsc order
		//RGBA 720x480 to A 720x240 odd 720x240 even
		//flipping x and y
		mediaalpha=GlobalAlloc(GHND,345600); //Number of byte sized pixels
		dest=(UBYTE *)GlobalLock(mediaalpha);
		t=172800;
		while (t) {
			*dest++=alphabuffer[((t/720)*(bm.bmWidthBytes*2))+((720-t%720)*4)];
			t--;
			}
		t=172800;
		while (t) {
			*dest++=alphabuffer[(((t/720)+1)*(bm.bmWidthBytes*2))+((720-t%720)*4)];
			t--;
			}
		GlobalUnlock(mediaalpha);
errorintrans:;
	}
*/
char *PCX2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo) {
	//const long pullcolor[2]={0x10001000,0x10001000};
	//Given a pointer to source returns a pointer to dest
	//returns bitmapbits.. and a pointer to a BITMAPINFO struct
	//returns null if error
	char *encodedbits,*dest=NULL;
	struct PCXheader *header=(struct PCXheader *)source;
	LPBITMAPINFOHEADER bmi=NULL;
	char *bmipalette,*pcxpalette;
	int bmpandpal256=sizeof(BITMAPINFOHEADER)+1024;
	//Convert PCX header to bitmap header
	bmi=(LPBITMAPINFOHEADER)newnode(nodeobject,bmpandpal256);
	bmipalette=(char *)bmi+sizeof(BITMAPINFOHEADER);
	memchr(bmi,0,bmpandpal256);
	bmi->biSize=sizeof(BITMAPINFOHEADER);
	bmi->biWidth=(header->Xmax-header->Xmin)+1;
	bmi->biHeight=(header->Ymax-header->Ymin)+1;
	bmi->biPlanes=1; //We convert to 1
	bmi->biBitCount=header->BitsPerPixel*header->NPlanes;
	bmi->biSizeImage=((bmi->biWidth*bmi->biBitCount+31)&~0x1f)*bmi->biPlanes*bmi->biHeight/8;
	//figure out palette info
	if (header->Version<5) {
		pcxpalette=(char *)header+16;
		bmi->biClrUsed=16;
		convertpalette(pcxpalette,bmipalette,16);
		}
	else { //version 5 expect 256 colors or 24 bit
		//Test for 256
		if (*(pcxpalette=source+(size-769))==12) {
			//biClrUsed can remain zero
			pcxpalette++;
			convertpalette(pcxpalette,bmipalette,256);
			}
		else { //No palette make sure it is 24bit
			if (!(header->NPlanes==3)) {wsprintf(string,"No palette detected and not 24 bit");goto errorpcx;}
			}
		}
	if (header->Encoding) {
		if (header->NPlanes==1) {
			if ((*bitmapinfo=(LPBITMAPINFOHEADER)mynew(&pmem,bmi->biSizeImage+bmpandpal256))==0) {wsprintf(string,"No memory available");goto errorpcx;}
			dest=(char *)*bitmapinfo+bmpandpal256;
			encodedbits=source+sizeof(struct PCXheader);
			decompress(encodedbits,dest,bmi->biSizeImage,source+(size-sizeof(struct PCXheader)-768),dest+bmi->biSizeImage);
			memcpy(*bitmapinfo,bmi,bmpandpal256);
			}
		else {
			wsprintf(string,"Multiple planes not yet supported");
			goto errorpcx;
			}
		}
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	return (dest);
errorpcx:
	if (bmi) disposenode(nodeobject,(struct memlist *)bmi);
	*bitmapinfo=NULL;
	if (dest) dispose((struct memlist *)dest,&pmem);
	printf("%s\n",string);
	return(0);
	}



void renderrgb32top(ULONG *videobuf,int x,int y,char *decodedbits) {
	ULONG *pixel;	
	int xindex,yindex,centerx,centery,cropy,cropx;
	int calibratex=-24;
	int calibratey=0;
	long size=0;
	ULONG bytesline=x<<2;
	UBYTE field;

	centerx=(720-x)>>1;
	centerx=(centerx+calibratex)&0xfffffffe;
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
	cropy=min(480,y);
	cropx=min(720,x);

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
		for (yindex=field;yindex<cropy;yindex+=2) {
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
			} //end y loop
		if (y+centery<480) {
			for (yindex=((480-y-centery)>>1);yindex;yindex--) {
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
	} //end renderrgb32top


				movd			mm0,[esi]			//mm0=________s3s2s1s0
				 pcmpeqb		mm1,mm1				//mm1=FFFFFFFFFFFFFFFF
				movq			mm2,[eax]			//mm2=y3v2y2u2y1v0y0u0
				 movq			mm4,mm0				//mm4=copy of negative
				movq			mm3,[ebx]			//mm3=Y3V2Y2U2Y1V0Y0U0
				 add			ebx,8
				pandn			mm4,mm1				//mm4=________n3n2n1n0
				 pcmpgtb		mm1,mm1				//mm1=0000000000000000
				punpcklbw	mm4,mm0				//mm4=s3n3s2n2s1n1s0n0
				movq			mm5,mm4				//mm5=copy of mm4
				punpcklbw	mm4,mm1				//mm4=__s1__n1__s0__n0
				punpckhbw	mm5,mm1				//mm5=__s3__n3__s2__n2
				punpckhbw	mm3,mm2				//mm3=y3Y3v2V2y2Y2u2U2
				movq			mm6,mm3				//mm6=copy of mm3
				punpckhbw	mm3,mm1				//mm3=__y3__Y3__v2__V2
				pmaddwd		mm3,mm5				//mm3=______DY______Dv
				punpcklbw	mm6,mm1				//mm6=__y2__Y2__u2__U2
				movq			mm1,mm5				//mm1=copy of mm5
				psllq			mm5,32				//mm5=__s2__n2________
				pand			mm1,keeplod			//mm1=__________s2__n2
				por			mm1,mm5				//mm5=__s2__n2__s2__n2
				pmaddwd		mm6,mm1				//mm6=______Dy______Dv
		goto skipfirstdump;
startnextdump:
		// first check to see if there is a next media
		while (mediaptr->next) {
			mediaptr=mediaptr->next;
			if (mediaptr->id==id_media) break;
			}
		if (mediaptr) {
			//We can assume media manager has opened this media
			t=mediaptr->mediartv->cacheslot;
			high4=(long)mediaptr->cropin;
			//open the first four frames
			if (!(ReadRTVFile(mediaptr->filesource,high4,(void *)rtvcache[t*8]))) printc(readerror);
			if (1<=mediaptr->totalframes)
				if (!(ReadRTVFile(mediaptr->filesource,high4+1,(void *)rtvcache[1+t*8]))) printc(readerror);
			if (2<=mediaptr->totalframes)
				if (!(ReadRTVFile(mediaptr->filesource,high4+2,(void *)rtvcache[2+t*8]))) printc(readerror);
			if (3<=mediaptr->totalframes)
				if (!(ReadRTVFile(mediaptr->filesource,high4+3,(void *)rtvcache[3+t*8]))) printc(readerror);
			}
skipfirstdump:;
			/*
			if (!(ReadRTVFile(mediaptr->filesource,cacheframe,(void *)rtvcache[t*4]))) return (FALSE);
			if (cacheframe+1<=mediaptr->totalframes)
				if (!(ReadRTVFile(mediaptr->filesource,cacheframe+1,(void *)rtvcache[1+t*4]))) return (FALSE);
			if (cacheframe+2<=mediaptr->totalframes)
				if (!(ReadRTVFile(mediaptr->filesource,cacheframe+2,(void *)rtvcache[2+t*4]))) return (FALSE);
			if (cacheframe+3<=mediaptr->totalframes)
				if (!(ReadRTVFile(mediaptr->filesource,cacheframe+3,(void *)rtvcache[3+t*4]))) return (FALSE);
			//memcpy(videobuf,rtvcache[(framenum%4)+t*4],691200);
			*/
/*
		wV=(UBYTE)(aV-((aV-bV)/(float)total)*current);
		wZ=(UBYTE)(aZ-((aZ-bZ)/(float)total)*current);
		wU=(UBYTE)(aU-((aU-bU)/(float)total)*current);
		wY=(UBYTE)(aY-((aY-bY)/(float)total)*current);
*/
BOOL rtvloader::getframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum) {
	long t;
	long cacheframe;
	ULONG *source;
	//if (!(ReadRTVFile(mediaptr->filesource,framenum-1,(void *)videobuf))) return (FALSE);
	//First see if we have it cached already
	t=0;
	while ((t<3)&&(rtvidcache[t]!=mediaptr->mediartv->rtvcacheid)) t++;

	if (t<3) {
		//check for frame update
		cacheframe=framenum&0xFFFFFFFC;
		if (cacheframe!=rtvframecache[t]) {
			//put the LONG aligned rtvs in cache
			rtvframecache[t]=cacheframe; //multiple of 4
			//0updatestreamrequest++;
			SetEvent(arrayofevents[EVENT_STREAM]);
			}

		//Move a frame from framebuf to video toaster
		source=rtvcache[(framenum%8)+t*8];
		__asm	{
			mov		edi, videobuf
			 mov		ecx, 691200
			mov		esi, source
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
		} //end if t<3
	else printc("Warning: rtv can't find cached media, memory may be lost");
	return(TRUE);
	} //end getframertv

	goto skiperror;
errorintrans:
	t=691200;
	do {
		*videobuf++=0x10801080; //black
		t-=4;
		} while (t);
skiperror:;

	//Set up video port here, not manditory to open so error is only warning
	if (!(lpDD4->QueryInterface(IID_IDDVideoPortContainer,(void **)&videocontainer)==DD_OK)) {
		printc ("Unable to detect video container");
		goto skipvideoinit;
		}
	if (!(videocontainer->EnumVideoPorts(NULL,NULL,NULL,enumvideo)==DD_OK)) {
		printc ("Unable to Enumerate the Video Port(s)");
		goto skipvideoinit;
		}
	//TODO we'll have user select videoport before doing the rest of this
	//This will eventually need to be moved to where it can be executed upon
	//selection in the menu

	//This first call will find out how many entries this video port has
	//I have no idea why it would be more than one
	if (!(videocontainer->GetVideoPortConnectInfo(videocaps->dwVideoPortID,&videoentries,NULL)==DD_OK)) {
		printc ("Unable to get the connecting information for this Video Port");
		goto skipvideoinit;
		}
	//We will call it again this time to fill an array of connectinfo structures
	//but first allocate memory for these structures
	connectinfo=(LPDDVIDEOPORTCONNECT)newnode(nodeobject,sizeof(DDVIDEOPORTCONNECT)*videoentries);
	if (!(videocontainer->GetVideoPortConnectInfo(videocaps->dwVideoPortID,&videoentries,connectinfo)==DD_OK)) {
		printc ("Unable to get the connecting information for this Video Port");
		goto skipvideoinit;
		}
	//If Further research indicates a reason for more connection types then
	//When opening the videoport, the VideoPortType member would be the arrayed
	//offset but for now I am going to assume it is always 1
	connectinfo->dwSize=sizeof(DDVIDEOPORTCONNECT);
	//TODO We'll eventually have prefs for this Video Port Description structure
	//Since I can not find an appropriate method for optimal frame size
	videoportdesc=(LPDDVIDEOPORTDESC)newnode(nodeobject,sizeof(DDVIDEOPORTDESC));
	//Fill this up Note: Trial and Error to get this right!
	videoportdesc->dwSize=sizeof(DDVIDEOPORTDESC);
	videoportdesc->dwFieldWidth=704;
	videoportdesc->dwVBIWidth=704;
	videoportdesc->dwFieldHeight=480;
	videoportdesc->dwMicrosecondsPerField=17;
	//We'll need to find out how to properly set this member
	videoportdesc->dwMaxPixelsPerSecond=4194304;
	videoportdesc->dwVideoPortID=videocaps->dwVideoPortID;
	videoportdesc->dwReserved1=0;
	videoportdesc->VideoPortType=*connectinfo;
	videoportdesc->dwReserved2=videoportdesc->dwReserved3=0;
	//finally open the videoport
	ddrval=videocontainer->CreateVideoPort(NULL,videoportdesc,&videoport,NULL);
	if (ddrval!=DD_OK)  {

		switch (ddrval) {
			printc ("Unable to open the video port");
			printc ("Reason: ");
			case DDERR_CURRENTLYNOTAVAIL:
				printc("Video Port is not Available");
				break;
			case DDERR_INVALIDOBJECT:
				printc("Invalid Object Detected");
				break;
			case DDERR_INVALIDPARAMS:
				printc("Parameters are Invalid");
				break;
			case DDERR_NOCOOPERATIVELEVELSET:
				printc("No cooperative level has been set");
				break;
			case DDERR_OUTOFCAPS:
				printc("There is another task which has not released the Video Port");
				break;
			case DDERR_OUTOFMEMORY:
				printc("DirectDraw does not have enough memory to perform the operation");
				break;
			default:
			printc ("Uknown");
			}
		}

skipvideoinit:

//Here is some other Videoport stuff
	videocontainer=NULL;

		LPDDVIDEOPORTCONTAINER	videocontainer;	// Video Container
		LPDDVIDEOPORTCAPS			videocaps;			// TODO this will need to be in a list later
		DWORD							videoentries;		// Used with GetConnectInfo
		LPDDVIDEOPORTCONNECT		connectinfo;		// Used with GetConnectInfo
		LPDDVIDEOPORTDESC			videoportdesc;	// To Create the VideoPort
		LPDIRECTDRAWVIDEOPORT	videoport;

//enumvideo

HRESULT WINAPI enumvideo(LPDDVIDEOPORTCAPS lpDDVideoPortCaps,LPVOID lpContext) {
	controls.videocaps=lpDDVideoPortCaps;
	return (DDENUMRET_OK);
	}


//closing stuff
		if(videoport)  {
			videoport->Release();
			videoport=NULL;
			}
		if(videocontainer)  {
			videocontainer->Release();
			videocontainer=NULL;
			}

//Here We have the original Media/DVE syntax error checking idea that was
//Ok, but to delete medias would be too costly I realised that after
//finishing the insertion part... So now this is Rejected, a new design
//Which implements filters now replaces this.

							//Make sure that if our drag image is dve that it is
							//is inserted between two media's to correctly time
							//displace both media's
							if (dragthis->dragimage->id==id_dve) {
								if (index)
									if (index->id==id_media)
										if (index->prev)
											if (index->prev->id==id_media) {
												//First will autoclip the DVE if necessary
												if (dragthis->dragimage->duration > index->prev->actualframes)
													dragthis->dragimage->duration=index->prev->actualframes;
												if (dragthis->dragimage->duration > index->actualframes)
													dragthis->dragimage->duration=index->actualframes;
												//next subtract framecounter and actual frames by the duration of the DVE
												index->prev->actualframes-=dragthis->dragimage->duration;
												index->actualframes-=dragthis->dragimage->duration;
												}
											else {
												printc("Multiple Transitions are not supported in this version");
												goto dvesyntaxerror;
												}
										else {
											printc("DVE's must be inserted between two media images.");
											goto dvesyntaxerror;
											}
									else {
										printc("Multiple Transitions are not supported in this version");
										goto dvesyntaxerror;
										}
								else {
									printc("DVE's must be inserted between two media images.");
dvesyntaxerror:
									disposenode(nodeobject,(struct memlist *)dragthis->dragimage);
									goto dontlinkdve;
									}
								} //end dve syntax checking

//next excerpt this is for the first node detection
							if (dragthis->dragimage->id==id_dve) {
								printc("DVE's must be inserted between two media images.");
								disposenode(nodeobject,(struct memlist *)dragthis->dragimage);
								goto dontlinkdve;
								}

dontlinkdve:  //right before olditem=-1 after adjustframecounter

//This detected a DVE append to just print error... and delete
	||(dragthis->dragimage->id==id_dve)

								if ((dragthis->dragorigin==IDC_MEDIAWINDOW)&&(dragthis->dragimage->id==id_dve))
								printc("Please insert DVE between two Media's.");

// end the crude syntax checking of Media/DVE

	case WM_DRAWITEM: {
			HBITMAP hbmppicture;
			HDC hdcMem,hbmpold;
			LPDRAWITEMSTRUCT lpdis;
			RECT rcBitmap;
			UBYTE whichbutton;
	
			//printc("WM_DRAWITEM");
			lpdis=(LPDRAWITEMSTRUCT) lParam;
			if (lpdis->itemID==-1) return(TRUE);
			switch (lpdis->CtlID) {
				case IDC_STOP:
					whichbutton=0;
					break;
				case IDC_REWIND:
					whichbutton=1;
					break;
				case IDC_PLAY:
					whichbutton=2;
					break;
				case IDC_FASTFORWARD:
					whichbutton=3;
					break;
				case IDC_RECORD:
					whichbutton=4;
					break;
				default:
					whichbutton=0;
					break;
				}
			switch (lpdis->itemAction) {
				case ODA_SELECT:
					//printc("ODA_SELECT");
				case ODA_DRAWENTIRE:
					//printc("ODA_DRAWENTIRE");
					hbmppicture=(HBITMAP)imagebmp_ptr[whichbutton];
					hdcMem=CreateCompatibleDC(lpdis->hDC); 
					hbmpold=(HDC)SelectObject(hdcMem,hbmppicture); 
 						BitBlt(lpdis->hDC, 
							 lpdis->rcItem.left, lpdis->rcItem.top,
							 lpdis->rcItem.right - lpdis->rcItem.left, 
							 lpdis->rcItem.bottom - lpdis->rcItem.top,
							 hdcMem,0,0,SRCCOPY); 
	
					SelectObject(hdcMem, hbmpold); 
					DeleteDC(hdcMem); 
					
					/* Is the item selected? */ 
 					if (lpdis->itemState & ODS_SELECTED) { 
						/*Set RECT coordinates to surround only the bitmap*/ 
						rcBitmap.left = lpdis->rcItem.left;
						rcBitmap.top = lpdis->rcItem.top; 
						rcBitmap.right = lpdis->rcItem.left + CONTROLBUTTONX;
						rcBitmap.bottom = lpdis->rcItem.top + CONTROLBUTTONY; 
						/*Draw a rectangle around bitmap to indicate the selection.*/
						DrawFocusRect(lpdis->hDC, &rcBitmap);
						} 
					break; 

				case ODA_FOCUS: 
				   /* 
					Do not process focus changes. The focus caret 
					(outline rectangle) indicates the selection. 
					The Which one? (IDOK) button indicates the final 
					selection. 
					*/
					break;
				}
			return (TRUE); 
			}


storysourceclass::handlestorysource(
	class dragthisclass *dragthis,
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{

	switch(uMsg) {
		case WM_NCHITTEST: {
			if (!(dragthis->dragtoggle)) {
				//Draw our Focus insert here if the drag is on
				struct tagPOINT mouseloc;
				RECT frameglow;
				short newitem=-1;
				UWORD xcoord,y,ycoord;
				UWORD t=0;

				GetCursorPos(&mouseloc);
				GetWindowRect(window,&frameglow);
				// x=4 y=22 if windowframe is on
				xcoord=mouseloc.x-(short)frameglow.left;
				ycoord=mouseloc.y-(short)frameglow.top;
		
				newitem=(short)(y=(ycoord-MD_OFFSETY)/(MD_YSIZE));
				//Check to see if there is an image in here
				imageindex=imageptr;
				while ((imageindex)&&(t<newitem)) {
					t++;
					imageindex=imageindex->next;
					}

				if (
					(y>=numrows)||(ycoord<MD_OFFSETY)||(!imageindex)||
					(xcoord<MD_OFFSETX)||
					(xcoord>(MD_XSIZE)+MD_OFFSETX)
					) newitem=-1;
				//wsprintf(string,"i%d,oi%d,y%d,yc%d",newitem,olditem,y,ycoord);printc(string);
				if (!(newitem==olditem)) {
					if (!(olditem==-1)) makeglow(oldy);
					if (!(newitem==-1)) makeglow (y);
					}
				olditem=newitem;oldy=y;
				} // end if not dragtoggle
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
			} //end case WM_NCHITTEST

		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(window,&ps);
			smartrefresh(ps.hdc,smartrefreshclip,
				ps.rcPaint.left,ps.rcPaint.top,
				ps.rcPaint.right-ps.rcPaint.left,
				ps.rcPaint.bottom-ps.rcPaint.top);
			EndPaint(window,&ps);
			return (0);
			}

		case WM_LBUTTONDOWN: {
			struct tagPOINT mouseloc;
			RECT frameglow;
			UWORD xcoord,y,ycoord;

			GetCursorPos(&mouseloc);
			GetWindowRect(window,&frameglow);

			xcoord=mouseloc.x-(short)frameglow.left;
			ycoord=mouseloc.y-(short)frameglow.top;
			y=(ycoord-MD_OFFSETY)/(MD_YSIZE);
			if ((y<numrows)||(ycoord<MD_OFFSETY)||(imageindex)||
				(xcoord>=MD_OFFSETX)||
				(xcoord<=(MD_XSIZE)+MD_OFFSETX))  {
				//turn off the glow
				if (!(olditem==-1)) {
					makeglow(oldy);
					olditem=-1;
					}
				dragthis->updateimage(duplicateimage(imageindex));
				}
			return (0);
			}  // end LButton Down

/*
		case WM_LBUTTONDOWN: {
			struct tagPOINT mouseloc;
			RECT frameglow;
			UWORD xcoord,y,ycoord;

			GetCursorPos(&mouseloc);
			GetWindowRect(window,&frameglow);

			xcoord=mouseloc.x-(short)frameglow.left;
			ycoord=mouseloc.y-(short)frameglow.top;
			y=(ycoord-MD_OFFSETY)/(MD_YSIZE);
			if ((y<numrows)||(ycoord<MD_OFFSETY)||(imageindex)||
				(xcoord>=MD_OFFSETX)||
				(xcoord<=(MD_XSIZE)+MD_OFFSETX))  {
				//turn off the glow
				if (!(olditem==-1)) {
					makeglow(oldy);
					olditem=-1;
					}
				dragthis->updateimage(duplicateimage(imageindex));
				}
			return (0);
			}  // end LButton Down
*/	

		case WM_CLOSE:
			windowtoggle(screen,window,0);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


void storysourceclass::makeglow(UWORD y) {
	HDC hdc,smartdc;
	RECT frameglow;
	hdc=GetDC(window);
	smartdc=CreateCompatibleDC(hdc);
	SelectObject(smartdc,smartrefreshclip);
	frameglow.left = MD_OFFSETX;
	frameglow.top = (y*(MD_YSIZE))+MD_OFFSETY; 
	frameglow.right = MD_OFFSETX+MD_XSIZE;
	frameglow.bottom = (y*(MD_YSIZE))+MD_OFFSETY + HALFYBITMAP; 
	DrawFocusRect(smartdc,&frameglow);
	InvalidateRect(window,&frameglow,FALSE);
	DeleteDC(smartdc);
	ReleaseDC(window,hdc);
	}

/*
Oldsize100 stuff
	RECT cs,ww,ws,rc,windowrc;
	LONG offsetx,offsety;

	GetWindowRect(window,&ww);
	GetClientRect(screen,&cs);

	//next find the offset for x and y
	offsety=rc.bottom-cs.bottom;
	offsetx=rc.right-cs.right;
	// To avoid small values we'll sub 200 for each
	//Finally figure the window position

	windowrc.left=(ww.left-offsetx)-200;
	windowrc.top=(ww.top-offsety)-200;
	windowrc.bottom=(ww.bottom-ww.top)-200;
	windowrc.right=(ww.right-ww.left)-200;

	wsprintf(string,"top%d,bottom%d,left%d,right%d",windowrc.top,
		windowrc.bottom,windowrc.left,windowrc.right);printc(string);
	wsprintf(string,"top%d,bottom%d,left%d,right%d",rc.top,
		rc.bottom,rc.left,rc.right);printc(string);
	wsprintf(string,"top%d,bottom%d,left%d,right%d",windowrc.top,
		windowrc.bottom,windowrc.left,windowrc.right);printc(string);
*/

