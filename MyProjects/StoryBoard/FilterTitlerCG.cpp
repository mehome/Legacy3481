#include "HandleMain.h"

#define tCG_titleED	200
#define tCG_FONT		201
#define tCG_OK			202
#define tCG_CANCEL	203

const long RGB1[2]={0xb5817062,0x0000da1d};
const long RGB2[2]={0x7fff1917,0x000041cb};
const long RGB3[2]={0xa1bcedd3,0x00007062};
const long addoffsets[2]={0x00100080,0x00100080};

filtersclass::filterscommon::filterscommon() {
	windowhead=windowindex=windowprev=NULL;
	windowlistsize=0;
	}
filtersclass::filterscommon::~filterscommon() {
	}

filtersclass::titlerCGclass::titlerCGclass() {
	windowlistsize=sizeof(struct titlergui);
	}
filtersclass::titlerCGclass::~titlerCGclass() {
	}

char *filtersclass::titlerCGclass::font2raw (struct titlerCG *source,UWORD *x,UWORD *y) {
	static BITMAPINFO bmi={
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

	HDC hdcimage;
	//HBITMAP hbm=NULL;
	HBITMAP alphahbm=NULL;
	char *dest=NULL;
	char *destindex;
	//void *buffer;
	void *alphabuffer;
	ULONG mysize;
	HFONT thefont,oldfont,alphafont;
	LOGFONT lf=source->lf;
	DWORD color=fliplong(source->dwColor)>>8;

	*y=max(16,abs(source->lf.lfHeight));
	*x=min(720,(*y * strlen(source->title)));
	*y+=(*y>>2);
	bmi.bmiHeader.biWidth=*x;
	bmi.bmiHeader.biHeight=*y;
	mysize=*x*(*y)<<2;
	if (!(dest=destindex=(char *)mynew(&pmem,mysize))) {
		wsprintf(string,"Not enough memory available");
		goto errorfont;
		}

	hdcimage=CreateCompatibleDC(NULL);
	//hbm=CreateDIBSection(hdcimage,&bmi,DIB_RGB_COLORS,&buffer,NULL,NULL);
	alphahbm=CreateDIBSection(hdcimage,&bmi,DIB_RGB_COLORS,&alphabuffer,NULL,NULL);
	//memset(buffer,0,mysize);
	memset(alphabuffer,0,mysize);
/*
	SelectObject(hdcimage,hbm);
	SetTextColor(hdcimage,source->dwColor);
	SetBkColor(hdcimage,source->dwColor);
	thefont=CreateFont(lf.lfHeight,lf.lfWidth,lf.lfEscapement,lf.lfOrientation,lf.lfWeight,
		lf.lfItalic,lf.lfUnderline,lf.lfStrikeOut,lf.lfCharSet,lf.lfOutPrecision,
		lf.lfClipPrecision,NONANTIALIASED_QUALITY,lf.lfPitchAndFamily,lf.lfFaceName);
*/

	alphafont=CreateFont(lf.lfHeight,lf.lfWidth,lf.lfEscapement,lf.lfOrientation,lf.lfWeight,
		lf.lfItalic,lf.lfUnderline,lf.lfStrikeOut,lf.lfCharSet,OUT_OUTLINE_PRECIS,
		lf.lfClipPrecision,ANTIALIASED_QUALITY,lf.lfPitchAndFamily,lf.lfFaceName);

	oldfont=(HFONT)SelectObject(hdcimage,alphafont);

	//TextOut(hdcimage,0,0,source->title,strlen(source->title));
	//memcpy(dest,buffer,mysize);

	//Now alpha settings
	SelectObject(hdcimage,alphahbm);
	//SelectObject(hdcimage,alphafont);
	SetTextColor(hdcimage,0x00ffffff);
	SetBkColor(hdcimage,0);
	TextOut(hdcimage,0,0,source->title,strlen(source->title));


	//Merge the alpha into the RGB
	__asm {
		mov	ecx,mysize
		shr	ecx,2 //bytes to long
		mov	edi,dest
		mov	esi,alphabuffer
		mov	edx,color
alphamergeloop:
		//mov	edx,[edi]

		mov	edx,color
		mov	eax,[esi]
		cmp	eax,0
		cmovz edx,eax

		shl	eax,24
		or		eax,edx

		mov	[edi],eax
		dec	ecx
		add	edi,4
		add	esi,4
		cmp	ecx, 0
		jne	alphamergeloop
		}

	SelectObject(hdcimage,oldfont);
	DeleteObject(alphafont);
	DeleteObject(thefont);
	if (alphahbm) DeleteObject(alphahbm);
	//if (hbm) DeleteObject(hbm);
	if (hdcimage) DeleteDC(hdcimage);

	return(dest);
errorfont:
	printc(string);
	if (dest) dispose((struct memlist *)dest,&pmem);
	return(dest=0);  //elogant I know
	}
/*
	//No alpha was defined so we need to go in and fill it in
	__asm {
		mov	ecx,mysize
		shr	ecx,2 //bytes to long
		mov	edi,dest
alphacheckloop:
		mov	edx,[edi]
		mov	eax,0ff000000h
		cmp	edx,0
		cmovnz	edx,eax
		and	edx,0ff000000h
		mov	eax,[edi]
		or		eax,edx

		mov	[edi],eax
		dec	ecx
		add	edi,4
		cmp	ecx, 0
		jne	alphacheckloop
		}

	__asm {
		mov	ecx,mysize
		shr	ecx,2 //bytes to long
		mov	edi,dest
		mov	esi,alphabuffer
alphamergeloop:
		mov	edx,[edi]
		mov	al,[esi]
		shl	eax,24

		or		eax,edx

		mov	[edi],eax
		dec	ecx
		add	edi,4
		inc	esi
		cmp	ecx, 0
		jne	alphamergeloop
		}
*/

int filtersclass::titlerCGclass::Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	char buffer[256];
	filterCG *filter;
	struct filterswindowlist *CGwindowptr;

	switch(uMsg) {
		case WM_SIZE:
			//TODO configure size
			return (0);

		case WM_COMMAND: {
			UWORD notifycode = HIWORD(wParam);
			UWORD buttonid = LOWORD(wParam);
			struct titlergui *CGwindowindex;
			if ((filter=(struct filterCG *)filters->getnode(w_ptr,&CGwindowptr))==0) {
				printc("Internal Link Error: filter controls will not work now until reset");
				break;
				}
			CGwindowindex=(struct titlergui *)CGwindowptr;
			struct imagelist *streamptr=controls->streamptr;

			switch (notifycode) {
				case BN_CLICKED: {
					//Handle our button up
					switch (buttonid) {
						case tCG_FONT: {
							CHOOSEFONT cf;

							//printc("FONT");

							// Initialize the CHOOSEFONT structure.
							//......................................
							memset(&cf,0,sizeof(CHOOSEFONT));
							cf.lStructSize  = sizeof(CHOOSEFONT);
							cf.hwndOwner    = w_ptr;
							cf.lpLogFont    = &filter->titleptr->lf;
							cf.Flags        = CF_SCREENFONTS | CF_EFFECTS;
							cf.rgbColors    = filter->titleptr->dwColor;

							if (filter->titleptr->lf.lfFaceName[0])
								cf.Flags|=CF_INITTOLOGFONTSTRUCT;

							// Create the choose font dialog.
							//...............................
							if (ChooseFont(&cf)) {
								// If the user chooses a font,
								// repaint the sample text.
								//............................
								filter->titleptr->dwColor = max(cf.rgbColors,1);
								memcpy(&filter->titleptr->lf,cf.lpLogFont,sizeof(LOGFONT));
								//printc("Chose font");
								dispose((struct memlist *)filter->yuv,&pmem);
								dispose((struct memlist *)filter->alpha,&pmem);
								filter->yuv=fontyuv(filter->titleptr,&filter->alpha,&filter->width,&filter->height);
								goto updatevideodisplay;
								}
							break;
							}
						default: goto noprocessCOMMAND;
						}
					return(0);
					} //end id bn clicked

				case EN_UPDATE: {
					if (GetWindowText((HWND)lParam,buffer,256)) {
						if (debug==2) {
							wsprintf(string,"EN_UPDATE %s",buffer);
							printc(string);
							}
						switch (buttonid) {
							case tCG_titleED:
								strcpy(filter->titleptr->title,buffer);
								dispose((struct memlist *)filter->yuv,&pmem);
								dispose((struct memlist *)filter->alpha,&pmem);
								filter->yuv=fontyuv(filter->titleptr,&filter->alpha,&filter->width,&filter->height);
								goto updatevideodisplay;
								//printc(buffer);
								//GetWindowText((HWND)lParam,filter->titleptr->title,filter->titleptr->buffersize);
								break;
							default: goto noprocessCOMMAND;
							} //end switch button id
						} //end if able to get text
					else {
						if (debug) printc("Update error");
						goto noprocessCOMMAND;
						}
					break;
					} // end EN_UPDATE
				default: goto noprocessCOMMAND;
				} //End switch notifycode
			return(0L);
			} //end WM_COMMAND


		default:
			//call our base class
noprocessCOMMAND:
			filters->CGobj.Callback(w_ptr,uMsg,wParam,lParam);
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}
	return(0L);

updatevideodisplay:
	//updatedisplay
	if (toaster->rtmeoutput) controls->updatevideonow();
	else {
		controls->updatevideoparm=controls->streamptr;
		controls->updatevideorequest++;
		SetEvent(arrayofevents[EVENT_VIDEO]);
		}
	return(0L);
	}

ULONG *filtersclass::titlerCGclass::fontyuv(struct titlerCG *source,UBYTE **alpha,UWORD *width,UWORD *height) {
	char *decodedbits;
	int x=0;
	int y=0;
	if (!(decodedbits=font2raw(source,(UWORD *)(&x),(UWORD *)(&y)))) {
		if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
		return (NULL);
		}
	return(alphayuv(decodedbits,x,y,TRUE,alpha,width,height));
	}

ULONG *filtersclass::CGclass::alphayuv(char *decodedbits,int x,int y,BOOL upsidedown,UBYTE **alpha,UWORD *width,UWORD *height) {
	ULONG *pixel,*videobuf,*dest;
	UBYTE *alphaindex;
	int xindex,yindex,cropy,cropx,yloop,ystep;
	long size=0;
	UBYTE field;
	ULONG bytesline;

	//Crop
	y&=0xfffffffe;
	cropy=max(0,(y-480));
	if (x>720) {
		cropx=720;
		}
	else cropx=x&0xfffffffe;
	*width=(UWORD)x;*height=(UWORD)y;

	if (!(dest=videobuf=(ULONG *)mynew(&pmem,x*y*2))) goto error;
	if (!(*alpha=alphaindex=(UBYTE *)mynew(&pmem,x*y))) goto error;
	bytesline=x<<2;
	
	__asm mov edi,videobuf
	for (field=0;field<2;field++) {
		//Test for upsidedown
		if (upsidedown) {
			yindex=y-1-field;ystep=-2; //condition>=cropy
			}
		else {
			yindex=field;ystep=2; //condition<cropy
			}
		for (yloop=((y-cropy)>>1);yloop;yloop--) {
			for (xindex=0;xindex<cropx;xindex+=2) {
				pixel=(ULONG *)(decodedbits+(xindex<<2)+(bytesline*yindex));
				__asm {
					mov			edi,videobuf
					mov			esi,alphaindex
					mov			eax,pixel
					movd			mm0,[eax]
					mov			edx,[eax+3]
					mov			[esi],dl
					pxor			mm1,mm1			//mm1 = ________________
					pcmpeqb		mm3,mm3			//mm3 = ffffffffffffffff
					add			eax,4
					psrlq			mm3,16			//mm3 = ____ffffffffffff
					punpcklbw	mm0,mm1			//mm0 = _Ap1_Rp1_Gp1_Bp1
					movd			mm2,[eax]
					mov			edx,[eax+3]
					mov			[esi+1],dl
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
					add			esi,2
					emms
					mov			[videobuf],edi
					mov			[alphaindex],esi
					}

				} //end x loop
			yindex+=ystep;
			} //end y loop
		} //end field loop

	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	return (dest);
error:
	//We are done with source and decodedbits so dispose them
	if (decodedbits) dispose((struct memlist *)decodedbits,&pmem);
	return (NULL);
	}


struct filterCG  *filtersclass::titlerCGclass::initfiltertitlerCG(struct imagelist *dragimage,struct imagelist *streamptr) {
	struct filterCG  *mediafilterCG;
	struct filternode *tail;
	UINT actualrange;
	//First allocate our filter node
	mediafilterCG=(struct filterCG  *)newnode(nodeobject,sizeof(struct filterCG ));

	mediafilterCG->node.nodeobject=createnode(&pmem,1024,0);
	//Link in
	if (streamptr->mediafilter) {
		//always insert at tail pretty simple
		tail=streamptr->mediafilter;
		while (tail->next) tail=tail->next;
		tail->next=(struct filternode *)mediafilterCG;
		mediafilterCG->node.prev=tail;
		}
	else {
		streamptr->mediafilter=(struct filternode *)mediafilterCG;
		mediafilterCG->node.prev=NULL;
		}
	mediafilterCG->node.next=NULL;

	//Fill in node
	mediafilterCG->node.filtertype=ft_CG;
	mediafilterCG->filesource=(char *)newnode(mediafilterCG->node.nodeobject,strlen(dragimage->filesource)+1);
	strcpy(mediafilterCG->filesource,dragimage->filesource);
	mediafilterCG->titleptr=(struct titlerCG *)newnode(mediafilterCG->node.nodeobject,sizeof(struct titlerCG));
	mediafilterCG->titleptr->title=(char *)newnode(mediafilterCG->node.nodeobject,24);
	mediafilterCG->titleptr->buffersize=24;
	//TODO make smaller number and resize to align to say 8 bytes
	strcpy(mediafilterCG->titleptr->title,"Title");
	memset(&mediafilterCG->titleptr->lf,0,sizeof(LOGFONT));
	mediafilterCG->titleptr->dwColor=GetSysColor(COLOR_WINDOWTEXT);
	//set defaults
	mediafilterCG->x=0;
	mediafilterCG->y=0;
	mediafilterCG->in=0;
	actualrange=streamptr->actualframes;
	if (streamptr->id==id_media) {
		if (streamptr->prev) if (streamptr->prev->id==id_dve)
		actualrange-=streamptr->prev->actualframes;
		if (streamptr->next) if (streamptr->next->id==id_dve)
		actualrange-=streamptr->next->actualframes;
		}
	mediafilterCG->out=actualrange;
	mediafilterCG->thickness=128;
	mediafilterCG->thickrange=NULL;
	mediafilterCG->thickpointshead=mediafilterCG->thickpointstail=NULL;
	mediafilterCG->thicktext=NULL;
	mediafilterCG->xrange=mediafilterCG->yrange=NULL;
	mediafilterCG->xypointshead=mediafilterCG->xypointstail=NULL;
	mediafilterCG->xytext=NULL;
	mediafilterCG->yuv=fontyuv(mediafilterCG->titleptr,&mediafilterCG->alpha,&mediafilterCG->width,&mediafilterCG->height);
	return(mediafilterCG);
	}


void filtersclass::CGclass::getcommonCGgui(struct CGgui *CGwindowindex,HWND filterwindow,HWND maincontrol,int tooly) {
	RECT rc;
	int toolx=0;

	GetClientRect(filterwindow,&rc);
	CGwindowindex->depthred=CGwindowindex->xyred=NULL;
	//And now the gadgets for the window
	toolx=rc.right-28;
	CreateWindowEx(0,"BUTTON","x",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,4,18,18,maincontrol,
		(HMENU)CG_CLOSE,hInst,NULL);
	toolx=0;
	tooly+=24;

	CreateWindowEx(0,"STATIC","In:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,24,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=24;
	CreateWindowEx(0,"BUTTON","<",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		toolx,tooly,TOOLBUTTONX,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_XP,hInst,NULL);
	toolx+=TOOLBUTTONX;
	CGwindowindex->inwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)CG_IN,hInst,NULL);
	toolx+=48;
	CreateWindowEx(0,"STATIC","Out:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,30,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=30;
	CGwindowindex->outwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)CG_OUT,hInst,NULL);
	toolx+=50;
	CreateWindowEx(0,"BUTTON",">",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_OWNERDRAW,
		toolx,tooly,TOOLBUTTONX,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_YP,hInst,NULL);
	toolx+=TOOLBUTTONX;
	CreateWindowEx(0,"STATIC","Dur:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,30,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=30;
	CGwindowindex->durwindow=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,(HMENU)CG_DUR,hInst,NULL);
	toolx=8;
	tooly+=24;

	CreateWindowEx(0,"BUTTON"," X,Y ",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_XY,hInst,NULL);
	toolx+=56;
	CreateWindowEx(0,"STATIC","X:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,24,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=24;
	CGwindowindex->x=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER,toolx,tooly,40,CONTROLBUTTONY,maincontrol,(HMENU)CG_X,hInst,NULL);
	toolx+=40;
	CreateWindowEx(0,"STATIC","Y:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,30,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=30;
	CGwindowindex->y=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER,toolx,tooly,40,CONTROLBUTTONY,maincontrol,(HMENU)CG_Y,hInst,NULL);
	toolx+=64;
	CreateWindowEx(0,"BUTTON","Plot",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,38,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_XYPLOT,hInst,NULL);

	toolx=0;
	tooly+=24;

	CreateWindowEx(0,"STATIC","Trans:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,56,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=56;
	CGwindowindex->depthed=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_VISIBLE|WS_CHILD|
		ES_CENTER|ES_NUMBER,toolx,tooly,40,CONTROLBUTTONY,maincontrol,(HMENU)CG_DEPTHED,hInst,NULL);
	toolx+=40;
	CGwindowindex->depthscrub=CreateWindowEx(0,TRACKBAR_CLASS,"",WS_VISIBLE|WS_CHILD|
		TBS_NOTICKS|TBS_HORZ|TBS_TOP,
		toolx,tooly,rc.right-toolx-48,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_DEPTHTB,hInst,NULL);
	SendMessage(CGwindowindex->depthscrub,TBM_SETRANGE,TRUE,MAKELONG(0L,255L));
	toolx=rc.right-48;
	CreateWindowEx(0,"BUTTON","Plot",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,38,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_DEPTHPLOT,hInst,NULL);
	toolx=0;
	tooly+=24;

	CreateWindowEx(0,"STATIC","Edit Ranges:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,98,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=98;
		CGwindowindex->depthrchk=CreateWindowEx(0,"BUTTON",NULL,WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		toolx,tooly,16,CONTROLBUTTONY,maincontrol,(HMENU)CG_DEPTHRCHK,hInst,NULL);
	toolx+=16;
	CreateWindowEx(0,"BUTTON","Trans",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,42,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_DEPTHRBT,hInst,NULL);
	toolx+=46;
	CGwindowindex->xyrchk=CreateWindowEx(0,"BUTTON",NULL,WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		toolx,tooly,16,CONTROLBUTTONY,maincontrol,(HMENU)CG_XYRCHK,hInst,NULL);
	toolx+=16;
	CreateWindowEx(0,"BUTTON","X,Y",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,42,CONTROLBUTTONY,maincontrol,
		(HMENU)CG_XYRBT,hInst,NULL);
	}


HWND filtersclass::titlerCGclass::getnewgui(struct filterswindowlist *CGwindowptr,HWND filterwindow,ULONG count) {
	//Gui stuff
	struct titlergui *CGwindowindex=(struct titlergui *)CGwindowptr;
	HWND maincontrol;
	int tooly=2;
	int toolx=0;
	RECT rc;

	GetClientRect(filterwindow,&rc);

	//Init all windows that will be opened later
	maincontrol=CreateWindowEx(0,"OBJGRAYWIN","",WS_VISIBLE|WS_DLGFRAME|
		WS_CHILD,0,0,300,128,filterwindow,NULL,hInst,NULL);

	//TODO move resize to separate function for handler to call
	//size100notyorh(filterwindow,maincontrol,0,124*count+CONTROLBUTTONY,100,128);
	SetWindowLong(maincontrol,GWL_USERDATA,(long)this);

	CGwindowindex->basegui.windownode.y=128;
	CreateWindowEx(0,"STATIC","Title:",WS_VISIBLE|WS_CHILD|SS_CENTER,
		toolx,tooly,48,CONTROLBUTTONY,maincontrol,NULL,hInst,NULL);
	toolx+=48;
	CGwindowindex->titleED=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL,
		toolx,tooly,rc.right-68-toolx,CONTROLBUTTONY,maincontrol,(HMENU)tCG_titleED,hInst,NULL);
	toolx+=rc.right-66-toolx;
	CreateWindowEx(0,"BUTTON","Font",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_VCENTER,
		toolx,tooly,32,CONTROLBUTTONY,maincontrol,(HMENU)tCG_FONT,hInst,NULL);

	getcommonCGgui(&CGwindowindex->basegui,filterwindow,maincontrol,1);

	return (maincontrol);
	}


void filtersclass::titlerCGclass::initcontrols(struct filterswindowlist *CGwindowptr,struct filternode *filterptr) {
	struct CGgui *CGwindowindex=(struct CGgui *)CGwindowptr;
	struct filterCG  *filterindex=(struct filterCG  *)filterptr;
	//struct imagelist *streamptr=controls->streamptr;
	SetWindowText(((struct titlergui *)CGwindowptr)->titleED,filterindex->titleptr->title);

	wsprintf(string,"%d",filterindex->in);
	SetWindowText(CGwindowindex->inwindow,string);
	wsprintf(string,"%d",filterindex->out);
	SetWindowText(CGwindowindex->outwindow,string);
	wsprintf(string,"%d",filterindex->out-filterindex->in);
	SetWindowText(CGwindowindex->durwindow,string);
	wsprintf(string,"%d",filterindex->x/4);
	SetWindowText(CGwindowindex->x,string);
	wsprintf(string,"%d",filterindex->y);
	SetWindowText(CGwindowindex->y,string);
	wsprintf(string,"%d",filterindex->thickness);
	SetWindowText(CGwindowindex->depthed,string);
	SendMessage(CGwindowindex->depthscrub,TBM_SETPOS,TRUE,filterindex->thickness);
	if (filterindex->thickrange) SendMessage(CGwindowindex->depthrchk,BM_SETCHECK,BST_CHECKED,0);
	else SendMessage(CGwindowindex->depthrchk,BM_SETCHECK,BST_UNCHECKED,0);
	if ((filterindex->xrange)||(filterindex->yrange)) SendMessage(CGwindowindex->xyrchk,BM_SETCHECK,BST_CHECKED,0);
	else SendMessage(CGwindowindex->xyrchk,BM_SETCHECK,BST_UNCHECKED,0);
	}
