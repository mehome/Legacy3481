#include "HandleMain.h"  
#define FT_MTEDIT		200
#define FT_MTSCRUB	201


/* * *       FilterDest implementation * * */

int filtersclass::Callback(
	HWND w_ptr,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) 
	{
	class storyboardclass *storyboard=getstoryboard();
	switch(uMsg) {

		case WM_LBUTTONUP: {
			struct filternode *filterptr;
			if (w_ptr==dragthis->dragwindow) {
				if (controls->streamptr) {
					//printc("Filters WM_LBUTTONUP");
					//ensure that were dragging something in
					if (debug) {
						wsprintf(string,"Stream=%lx %s",controls->streamptr,controls->streamptr->text);
						printc(string);
						}
					//TODO parse filter type here and call the appropriate filter
					if (strcmpi(dragthis->dragimage->text,"titler.tga")) {
						filterptr=(struct filternode *)CGobj.initfilterCG(dragthis->dragimage,controls->streamptr);
						filterptr->vfilterptr=(void *)&CGobj;
						}
					else {
						filterptr=(struct filternode *)titlerCGobj.initfiltertitlerCG(dragthis->dragimage,controls->streamptr);
						filterptr->vfilterptr=(void *)&titlerCGobj;
						}
					//update our gui
					updateimagelist();
					//and finally update the video
					controls->updatevideoparm=controls->streamptr;
					controls->updatevideorequest++;
					SetEvent(arrayofevents[EVENT_VIDEO]);
					} // end if streamptr
				} //end if we are dragging a filter in
			return (0);
			}  // end LButton Up

		case WM_LBUTTONDOWN: {
			//printc("Filters WM_LBUTTONDOWN");
			return (0);
			}  // end LButton Down

		case WM_SIZE:
			//TODO configure size
			return (0);

		case WM_CLOSE:
			windowtoggle(screen,window,IDM_FILTERS);
			return(0L);

		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);
	}


filtersclass::filtersclass() {
	windowlist=NULL;
	}


void filtersclass::createwindow(HWND w_ptr) {
	window=CreateWindowEx(0,"OBJWIN","Media Filters Window",
		WS_POPUP|WS_OVERLAPPEDWINDOW,280,100,310,350,w_ptr,
		NULL,hInst,NULL);
	SetWindowLong(window,GWL_USERDATA,(long)this);
	//titlerCGobj.createwindow(w_ptr);
	}


void filtersclass::resize() {
	//size100(screen,window,35,20,72,80);
	}


void filtersclass::resizefilter(HWND filterwindow,ULONG y,ULONG height) {
	size100notyorh(window,filterwindow,0,y,100,height);
	}


void filtersclass::updateimagelist() {
	ULONG count,ysize;
	struct imagelist *streamptr;
	filternode *filterindex;
	struct filterswindowlist *CGwindowprev;
	struct filterswindowlist *CGwindowindex;
	struct filterswindowlist *CGwindowtemp;
	class filterscommon *vfilterptr;
	ULONG mediatimer;

	if (IsWindowVisible(window)) {
		//init
		count=0;ysize=CONTROLBUTTONY;
		streamptr=controls->streamptr;
		filterindex=NULL;
		CGwindowprev=NULL;
		CGwindowindex=windowlist;
		mediatimer=controls->mediatimer;

		if (streamptr) {
			wsprintf(string,"Filters - %s",streamptr->text);
			SetWindowText(window,string);
			miniscrubobj->initminiscrub();
			filterindex=streamptr->mediafilter;
			}
		//Hide all windows for each filtertype created
		if (CGwindowindex) {
			do ShowWindow(CGwindowindex->window,SW_HIDE); while (CGwindowindex=CGwindowindex->next);
			}
		if (filterindex) {
			//Init the transparent lists
			do {
				vfilterptr=(class filterscommon *)filterindex->vfilterptr;
				vfilterptr->windowprev=NULL;
				vfilterptr->windowindex=vfilterptr->windowhead;
				} while (filterindex=filterindex->next);
			CGwindowindex=windowlist;
			filterindex=streamptr->mediafilter;
			do {
				vfilterptr=(class filterscommon *)filterindex->vfilterptr;
				//Since we can't init the windowindex to windowhead at the start of the loop
				//We'll have to do it here
				if (count==0) {
					if (vfilterptr->windowhead) {
						windowlist=(struct filterswindowlist *)(vfilterptr->windowhead)->ptr;
						windowlist->prev=NULL;
						}
					}
				//See if we have any windows created already
				if (vfilterptr->windowindex) {
					CGwindowtemp=(struct filterswindowlist *)(vfilterptr->windowindex)->ptr;
					if (CGwindowindex!=CGwindowtemp) {
						//give the new type the link
						CGwindowindex=CGwindowtemp;
						//Now link to previous if exist
						CGwindowindex->prev=CGwindowprev;
						CGwindowindex->next=NULL;
						if (CGwindowprev) {
							CGwindowprev->next=CGwindowindex;
							}
						CGwindowindex->next=NULL;
						}
					SetWindowPos(CGwindowindex->window,NULL,0,ysize,NULL,NULL,SWP_NOSIZE|SWP_NOZORDER);
					ShowWindow(CGwindowindex->window,SW_SHOW);
					CGwindowprev=CGwindowindex;
					CGwindowindex=CGwindowindex->next;
					//transparent pointer advance
					vfilterptr->windowprev=vfilterptr->windowindex;
					vfilterptr->windowindex=(vfilterptr->windowindex)->next;
					}
				else {
					CGwindowindex=(struct filterswindowlist *)newnode(nodeobject,vfilterptr->windowlistsize);
					vfilterptr->windowindex=(struct listofptrs *)newnode(nodeobject,sizeof(struct listofptrs));
					(vfilterptr->windowindex)->ptr=(void *)CGwindowindex;
					//check the heads
					if (vfilterptr->windowhead==NULL) {
						(vfilterptr->windowhead)=vfilterptr->windowindex;
						vfilterptr->windowprev=NULL;
						}
					if (windowlist==NULL) windowlist=CGwindowindex;

					//Now link to previous if exist
					CGwindowindex->prev=CGwindowprev;
					CGwindowindex->next=NULL;
					if (CGwindowprev) {
						CGwindowprev->next=CGwindowindex;
						}
					CGwindowprev=CGwindowindex;
					CGwindowindex->window=vfilterptr->getnewgui(CGwindowindex,window,count);
					SetWindowPos(CGwindowindex->window,NULL,0,ysize,NULL,NULL,SWP_NOSIZE|SWP_NOZORDER);
					CGwindowindex=NULL;
					//now to assign our transparent pointers
					//link to prev if exists
					(vfilterptr->windowindex)->prev=vfilterptr->windowprev;
					(vfilterptr->windowindex)->next=NULL;
					if (vfilterptr->windowprev) {
						(vfilterptr->windowprev)->next=vfilterptr->windowindex;
						}
					vfilterptr->windowprev=vfilterptr->windowindex;
					vfilterptr->windowindex=NULL;
					}
				vfilterptr->initcontrols(CGwindowprev,filterindex);
				ysize+=CGwindowprev->y;
				//end case CG
				//End switch filtertype
				//Set controls to window
				count++;
				} while (filterindex=filterindex->next);
			}
		} //end if window is visible
	} //end updateimagelist


void filtersclass::startup() {
	struct miniscrubinput miniparms={0,0,window,hInst};
	miniscrubobj=new miniscrub(&miniparms);
	//titlerCGobj.startup();
	}


void filtersclass::closefilters(struct imagelist *media) {
	class filterscommon *vfilterptr;
	filternode *filterindex=NULL;
	if (media) {
		filterindex=media->mediafilter;
		}
	if (filterindex) {
		do {
			vfilterptr=(class filterscommon *)filterindex->vfilterptr;
			vfilterptr->closefilter(filterindex);
			} while (filterindex=filterindex->next);
		}
	media->mediafilter=NULL;
	}


void filtersclass::shutdown() {
	delete miniscrubobj;
	}


filtersclass::~filtersclass() {
	}

/*
void Filter_Blend (UBYTE *pFrame1,UBYTE *pFrame2,long xparm,long yparm,
	short *xrange,short *yrange,UBYTE *pAlpha,DWORD thickparm,UBYTE *thickrange,
	long dwWidth,DWORD dwHeight,DWORD dwPos,DWORD dwLen,BOOL bField)
	{
	//const static ULONG one[2]={0x01010101,0x01010101};
	ULONG ystart,ystartB,ymin;
	ULONG endofx;
	ULONG scalefieldoffset=dwWidth*(dwHeight>>1);
	UBYTE *pField2=pFrame1+345600;
	UBYTE *sourceField2;
	UBYTE *pAlpha2;
	UBYTE *pAlphaB=pAlpha;
	UBYTE *pFrame2B=pFrame2;

	static ULONG fm[2];
	DWORD i_, y_,thickness,thick2,xexcess=0;
	DWORD xexcessB=0;
	DWORD ytotal=0;
	DWORD ytotalB=0;
	long xoffset,yoffset,x2,y2;
	long dwWidthB=dwWidth;

	//if (!bField)//
	dwHeight>>=1;
	//i_=(bField == 0) ? 2 : 1;
	i_=2;

	//This may be questionable for 64bit compliance but is
	//a heak of a lot faster
	if ((int)thickrange&nostaticpreview) {
		thickness=*(thickrange+dwPos);
		thick2=*(thickrange+dwPos+1);
		}
	else thickness=thick2=thickparm;

	if ((int)xrange&nostaticxypreview) {
		xoffset=xrange[dwPos];
		x2=xrange[dwPos+1];
		}
	else xoffset=x2=xparm;

	if ((int)yrange&nostaticxypreview) {
		yoffset=yrange[dwPos];
		y2=yrange[dwPos+1];
		}
	else yoffset=y2=yparm;

	if (yoffset<0) {
		pFrame2-=yoffset*(dwWidth<<1);
		pAlpha-=yoffset*dwWidth;
		ystart=0;
		}
	else {
		ytotal=yoffset*1440; //720*2
		ystart=yoffset;
		}

	if (y2<0) {
		pFrame2B-=y2*(dwWidth<<1);
		pAlphaB-=y2*dwWidth;
		ystartB=0;
		}
	else {
		ytotalB=y2*1440;
		ystartB=y2;
		}

	if (xoffset<0) {
		dwWidth+=(xoffset>>1);
		xexcess-=(xoffset>>1);
		pFrame2-=xoffset;
		pAlpha-=(xoffset>>1);
		xoffset=0;
		}
	else {
		endofx=(dwWidth<<1)+xoffset;
		if (endofx>1440) {
			xexcess=(endofx-1440)>>1;
			dwWidth-=xexcess;
			}
		}

	if (x2<0) {
		dwWidthB+=(x2>>1);
		xexcessB-=(x2>>1);
		pFrame2B-=x2;
		pAlphaB-=(x2>>1);
		x2=0;
		}
	else {
		endofx=(dwWidthB<<1)+x2;
		if (endofx>1440) {
			xexcessB=(endofx-1440)>>1;
			dwWidthB-=xexcessB;
			}
		}

	sourceField2=pFrame2B+(scalefieldoffset<<1);
	pAlpha2=pAlphaB+scalefieldoffset;

	while (i_--) {

		//DWORD ymin=min(240,dwHeight+yoffset);
		__asm {
			mov         ecx,[dwHeight]
			add         ecx,[yoffset]
			mov			edx,240
			cmp         ecx,240
			cmovg			ecx,edx
			mov			[ymin],ecx
			}

		for (y_ = ystart; y_ < ymin; y_++) {
			__asm {
				mov			ecx,[dwWidth]
				;
				movq			mm4,[thickness]	//mm4=______________Fm
				 shr			ecx,2					//2=4 pixels at a time
				mov			eax,pFrame1
				add			eax,xoffset
				add			eax,ytotal
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
			pFrame1 += 1440;
			pFrame2 += (dwWidth+xexcess)<<1;
			pAlpha  += (dwWidth+xexcess);
			}
		dwPos++;
		pFrame1=pField2;
		pFrame2=sourceField2;
		pAlpha=pAlpha2;
		thickness=thick2;
		dwWidth=dwWidthB;
		xexcess=xexcessB;
		xoffset=x2;
		ystart=ystartB;
		ytotal=ytotalB;
		yoffset=y2;
		}
	__asm emms
	} //end Filter_Blend
*/


//Here is John's Version


/**/
void Filter_Blend (UBYTE *pFrame1,UBYTE *pFrame2,long xparm,long yparm,
	short *xrange,short *yrange,UBYTE *pAlpha,DWORD thickparm,UBYTE *thickrange,
	long dwWidth,DWORD dwHeight,DWORD dwPos,DWORD dwLen,BOOL bField)
	{
	//const static ULONG one[2]={0x01010101,0x01010101};
	ULONG ystart,ystartB,ymin;
	ULONG endofx;
	ULONG scalefieldoffset=dwWidth*(dwHeight>>1);
	UBYTE *pField2=pFrame1+345600;
	UBYTE *sourceField2;
	UBYTE *pAlpha2;
	UBYTE *pAlphaB=pAlpha;
	UBYTE *pFrame2B=pFrame2;

	static ULONG fm[2];
	DWORD i_, y_,thickness,thick2,xexcess=0;
	DWORD xexcessB=0;
	DWORD ytotal=0;
	DWORD ytotalB=0;
	long xoffset,yoffset,x2,y2;
	long dwWidthB=dwWidth;

	//if (!bField)// 
	dwHeight>>=1;
	//i_=(bField == 0) ? 2 : 1;
	i_=2;

	//This may be questionable for 64bit compliance but is
	//a heak of a lot faster
	if ((int)thickrange&nostaticpreview) {
		thickness=*(thickrange+dwPos);
		thick2=*(thickrange+dwPos+1);
		}
	else thickness=thick2=thickparm;

	if ((int)xrange&nostaticxypreview) {
		xoffset=xrange[dwPos];
		x2=xrange[dwPos+1];
		}
	else xoffset=x2=xparm;

	if ((int)yrange&nostaticxypreview) {
		yoffset=yrange[dwPos];
		y2=yrange[dwPos+1];
		}
	else yoffset=y2=yparm;

	if (yoffset<0) {
		pFrame2-=yoffset*(dwWidth<<1);
		pAlpha-=yoffset*dwWidth;
		ystart=0;
		}
	else {
		ytotal=yoffset*1440; //720*2
		ystart=yoffset;
		}

	if (y2<0) {
		pFrame2B-=y2*(dwWidth<<1);
		pAlphaB-=y2*dwWidth;
		ystartB=0;
		}
	else {
		ytotalB=y2*1440;
		ystartB=y2;
		}

	if (xoffset<0) {
		dwWidth+=(xoffset>>1);
		xexcess-=(xoffset>>1);
		pFrame2-=xoffset;
		pAlpha-=(xoffset>>1);
		xoffset=0;
		}
	else {
		endofx=(dwWidth<<1)+xoffset;
		if (endofx>1440) {
			xexcess=(endofx-1440)>>1;
			dwWidth-=xexcess;
			}
		}

	if (x2<0) {
		dwWidthB+=(x2>>1);
		xexcessB-=(x2>>1);
		pFrame2B-=x2;
		pAlphaB-=(x2>>1);
		x2=0;
		}
	else {
		endofx=(dwWidthB<<1)+x2;
		if (endofx>1440) {
			xexcessB=(endofx-1440)>>1;
			dwWidthB-=xexcessB;
			}
		}

	sourceField2=pFrame2B+(scalefieldoffset<<1);
	pAlpha2=pAlphaB+scalefieldoffset;

	while (i_--) {

		//DWORD ymin=min(240,dwHeight+yoffset);
		__asm {
			mov         ecx,[dwHeight]
			add         ecx,[yoffset]
			mov			edx,240
			cmp         ecx,240
			cmovg			ecx,edx
			mov			[ymin],ecx
			}

		for (y_ = ystart; y_ < ymin; y_++) {
			__asm {
				mov			ecx,[dwWidth]
				;
				movq		mm4,[thickness]		//mm4=______________Fm
				 shr		ecx,2				//2=4 pixels at a time
				mov			eax,pFrame1
				add			eax,xoffset
				add			eax,ytotal
				 punpcklbw	mm4,mm4				//mm4=____________FmFm
				mov			ebx,pFrame2
				 punpcklwd	mm4,mm4				//mm4=________FmFmFmFm
				mov			esi,pAlpha
				lea			edx,fm
				pxor		mm7,mm7				//MM7 = ________________
				 punpcklbw	mm4,mm7				//MM4 = __Fm__Fm__Fm__Fm
				 movq		[edx],mm4

				align		16
												// Expect: MM4 = [edx] on entry.

				//NOTE:  Could try to reduce load on MMX shifter with PSHUFW shuffle instruction (PIII and later only).
blendloop:
				movd		mm0,[esi]			//MM0 = ________s3s2s1s0
				add			esi,4
				pxor mm6,mm6	//	pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF


				movq		mm3,[eax]			//MM3 = y3v2y2u2y1v0y0u0
				punpcklbw	mm0,mm7				//MM0 = __s3__s2__s1__s0

				paddsw		mm0,mm4				//add framemul
			//	psrlw		mm6,8				//MM6 = __FF__FF__FF__FF
				psubb mm6,mm4

			//	pxor		mm4,mm6				//MM4 = __nf__nf__nf__nf
			//	psrlw		mm6,7				//mm6 = __01__01__01__01

												//NOTE:  Use PCMPEQB mm6,mm6 / PANDN mm4,mm6 instead of PCMPEQB/PSRLW8/PXOR/PSRLW7/PADDW here.
			//	paddsw		mm4,mm6				// add one to nf (two's complement)
				movq		mm2,mm3				//MM2 = y3v2y2u2y1v0y0u0
				psubusw mm0,mm6
			//	psubusw		mm0,mm4				//sub framemul


				movq		mm4,[ebx]			//MM4 = Y3V2Y2U2Y1V0Y0U0
				pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF

				psrlw		mm6,8				//MM6 = __FF__FF__FF__FF
				add			eax, 8

				packuswb	mm0,mm0
				add			ebx,8


				punpcklbw	mm2,mm4				//MM2 = Y1y1V0v0Y0y0U0u0
				dec			ecx

				punpcklbw	mm0,mm0				//MM0 = s3s3s2s2s1s1s0s0
				movq		mm5,mm2				//MM5 = Y1y1V0v0Y0y0U0u0

				pxor		mm0,mm6				//MM0 = s3n3s2n2s1n1s0n0
				punpcklbw	mm5,mm7				//MM5 = __Y0__y0__U0__u0


				punpckhbw	mm2,mm7				//MM2 = __Y1__y1__V0__v0
				movq		mm1,mm0				//MM1 = s3n3s2n2s1n1s0n0

				punpckhbw	mm0,mm7				//MM0 = __s3__n3__s2__n2

				punpcklbw	mm1,mm7				//MM1 = __s1__n1__s0__n0

				movq		mm6,mm1				//MM6 = __s1__n1__s0__n0
				punpckldq	mm1,mm1				//MM1 = __s0__n0__s0__n0


				pmaddwd		mm1,mm5				//MM1 = ______Y0______U0
				punpckhbw	mm3,mm4				//MM3 = Y3y3V2v2Y2y2U2u2

				movq		mm5,mm3				//MM5 = Y3y3V2v2Y2y2U2u2
				pmaddwd		mm2,mm6				//MM2 = ______Y1______V0

				punpcklbw	mm5,mm7				//MM5 = __Y2__y2__U2__u2
				pcmpeqb		mm6,mm6				//MM6 = FFFFFFFFFFFFFFFF


				psrld		mm1,8				//MM1 = ______Y0______U0
				movq		mm4,mm0				//MM4 = __s3__n3__s2__n2

				psrld		mm2,8				//MM2 = ______Y1______V0


				punpckldq	mm0,mm0				//MM0 = __s2__n2__s2__n2

				pmaddwd		mm5,mm0				//MM5 = ______Y2______U2
				punpckhbw	mm3,mm7				//MM3 = __Y3__y3__V2__v2


				pmaddwd		mm3,mm4				//MM3 = ______Y3______V2
				packssdw	mm1,mm2				//MM1 = __Y1__V0__Y0__U0
movq mm4,[edx]
				//NOTE:  Try loading MM4 here - profile...

				psrlw		mm6,15				//mm6 = __01__01__01__01


				paddsw		mm1,mm6
				psrld		mm5,8				//MM5 = ______Y2______U2

				psrld		mm3,8				//MM3 = ______Y3______V2


				packssdw	mm5,mm3				//MM5 = __Y3__V2__Y2__U2

				paddsw		mm5,mm6
	
				packuswb	mm1,mm5				//MM1 = Y3V2Y2U2Y1V0Y0U0
												//paddusb		mm1,one
												// cmp			ecx, 0
				movq		[eax-8],mm1			//Save 0-3.
		//		movq		mm4,[edx]			//mm4=framemul
				jnz			blendloop


				}
			pFrame1 += 1440;
			pFrame2 += (dwWidth+xexcess)<<1;
			pAlpha  += (dwWidth+xexcess);
			}
		dwPos++;
		pFrame1=pField2;
		pFrame2=sourceField2;
		pAlpha=pAlpha2;
		thickness=thick2;
		dwWidth=dwWidthB;
		xexcess=xexcessB;
		xoffset=x2;
		ystart=ystartB;
		ytotal=ytotalB;
		yoffset=y2;
		}
	__asm emms
	} //end Filter_Blend
/**/


void filtersclass::filterimage(ULONG *videobuf,struct imagelist *mediaptr,long mediatime) {
	//heres where we find out which filter to apply by the filtertype
	//and call the correct filter procedure
	struct filterCG *mediafilterCG=(struct filterCG *)mediaptr->mediafilter;
	do {
		if ((mediatime>=mediafilterCG->in)&&(mediatime<=mediafilterCG->out))
			Filter_Blend ((UBYTE *)videobuf,(UBYTE *)mediafilterCG->yuv,
				mediafilterCG->x,mediafilterCG->y,
				mediafilterCG->xrange,mediafilterCG->yrange,
				mediafilterCG->alpha,
				(DWORD)mediafilterCG->thickness,mediafilterCG->thickrange,
				mediafilterCG->width,mediafilterCG->height,(mediatime-1)<<1,
				mediaptr->duration<<1,FALSE);
		} while (mediafilterCG=(struct filterCG *)mediafilterCG->node.next);
	}


