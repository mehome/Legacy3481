#include "handlemain.h"

#define NO_OVERLAY_HARDWARE "The display adapter and/or driver doesn't support overlays."

#define UNABLE_TO_CREATE_OVERLAY    "The display adapter appears to "\
				    "support overlays, but unable to "\
				    "create an overlay in YUV format. "\
				    "You may want to try shutting down other "\
				    "DirectX apps to free video memory, or try "\
				    "rerunning this app in a different display "\
				    "mode."

#define UNABLE_TO_DISPLAY_OVERLAY   "The overlay can be created, but"\
				    "unable to display it in its current display mode"\
				    "Please try rerunning this app in "\
				    "a different display mode."

int previewclass::Callback (HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam) {

	switch(uMsg) {
		case WM_MOVE:
			configuresize();
			showoverlay();
			break;

		case WM_SYSCOMMAND:
			if (wParam==SC_MINIMIZE) hideoverlay();
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));

		case WM_SIZE: {
			static swp=0;
			if (wParam==SIZE_MINIMIZED) break;
			else {
				configuresize();
				showoverlay();
				//This sends messages to recurse so we have to toggle out to prevent this
				if (!swp) {
					swp=1;
					SetWindowPos(prevwindow,NULL,NULL,NULL,prevwidth+metricsx,prevheight+metricsy,SWP_NOOWNERZORDER|SWP_NOMOVE);
					}
				else swp=0;
				}
			break;
			}
		case WM_CLOSE: {
			UINT isitchecked=GetMenuState(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND);
			if (previewoff=(isitchecked&MFS_CHECKED)) {
				CheckMenuItem(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND|MFS_UNCHECKED);
				if (debug) printc("Preview Window is now Off");
				hideoverlay();
				ShowWindow(prevwindow,SW_HIDE);
				}
			else {
				CheckMenuItem(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND|MFS_CHECKED);
				if (debug) printc("Preview Window on");
				showoverlay();
				ShowWindow(prevwindow,SW_SHOW);
				}
			return(0L);
			}
		default:
			return(DefWindowProc(w_ptr,uMsg,wParam,lParam));
		}

	return(0L);
	}

void previewclass::showoverlay() {
	dwupdateflags=DDOVER_SHOW|DDOVER_DDFX;
	if ((overlaysupport)&&(!previewoff)) lpDDSOverlay->UpdateOverlay(&rs,lpDDSPrimary,&rd,dwupdateflags,&ovfx);
	}

void previewclass::hideoverlay() {
	dwupdateflags=DDOVER_HIDE|DDOVER_DDFX;
	if (overlaysupport) lpDDSOverlay->UpdateOverlay(&rs,lpDDSPrimary,&rd,dwupdateflags,&ovfx);
	}

previewclass::previewclass() {
	UINT isitchecked;
	//printc("import construct called");
	lpDD=NULL;
	lpDD4=NULL;
	lpDDSPrimary=lpDDSOverlay=NULL;
	lpClipper=NULL;
	lpDDPal=NULL;
	isitchecked=GetMenuState(m_ptr,IDM_PREVWINDOW,MF_BYCOMMAND);
	previewoff=((isitchecked&MFS_CHECKED)==0);
	metricsx=GetSystemMetrics(SM_CXFRAME);
	metricsy=GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME)+2;
	}
previewclass::~previewclass() {
	}

void previewclass::createwindow(HWND w_ptr) {
	/*
	prevwindow=CreateWindowEx(0,"STATIC","",WS_VISIBLE|SS_BLACKRECT|
		WS_CHILD,200,260,235,300,w_ptr,
		(HMENU)IDC_PREVIEW,hInst,NULL);
	*/
	prevwindow=CreateWindowEx(0,"OBJGRAYWIN","Preview",
		WS_CHILD,250,300,235,200,w_ptr,
		NULL,hInst,NULL);
	SetWindowLong(prevwindow,GWL_USERDATA,(long)this);
	if (!lpDD4) overlaysupport=setuppreview();
	configuresize();
	if (overlaysupport) updatepreview((ULONG *)toaster->videobuf);
	else EnableMenuItem(m_ptr,IDM_PREVWINDOW,MF_GRAYED);
	}

void previewclass::resize(LONG y) {
	size100noty(screen,prevwindow,0,y-metricsy-4,26,94);
	configuresize();
	}


BOOL previewclass::configuresize() {
	RECT rc;
	ULONG usex,usey;

	//Finally lets grab the actual preview window specs for blitting;
	//Done here for speed
	GetWindowRect(prevwindow,&rc);
	prevx=rc.left+metricsx;
	prevy=rc.top+metricsy;
	//prevheight=rc.bottom-rc.top; 
	GetClientRect(prevwindow,&rc);
	prevwidth=rc.right;
	prevheight=rc.bottom-rc.top; 
	//calculate the preview windows width and height
	//to see if we need black bars...
	usex=(prevheight*4)/3;
	usey=(prevwidth*3)/4;
	if (usex<prevwidth) {
		//calculate the horx bars
		prevx+=(prevwidth-usex)/2;
		prevwidth=usex;
		}
	else if (usey<prevheight) {
		//calculate the vert bars
		prevy+=(prevheight-usey)/2;
		prevheight=usey;
		}
	//if neither bars are implemented they must be equal
	//**************************************************************
	//             Direct Draw Init II Setting up the rectangles
	//Grab some alignment vars
	DDCAPS capsDrv;
	unsigned int ustretchfactor1000,udestsizealign,usourcesizealign;
	//overlaysupport=FALSE;
	if (overlaysupport) {
		INIT_DIRECTX_STRUCT(capsDrv);
		if (!(lpDD4->GetCaps(&capsDrv,NULL)==DD_OK))  return (0);
		ustretchfactor1000=capsDrv.dwMinOverlayStretch>1000 ? capsDrv.dwMinOverlayStretch : 1000;
		udestsizealign=capsDrv.dwAlignSizeDest;
		usourcesizealign=capsDrv.dwAlignSizeSrc;
		//Set up the source rectangle
		rs.top=0;//prevy;
		rs.left=0;//prevx;
		rs.right=VIDEOX;//prevwidth;
		rs.bottom=VIDEOY;//prevheight;
		//Now to apply the alignment
		if (capsDrv.dwCaps&DDCAPS_ALIGNSIZESRC&&usourcesizealign)
			rs.right-=rs.right % usourcesizealign;
		//Set up the Destination rectangle
		rd.left=prevx;
		rd.top=prevy;
		rd.right=((prevwidth+prevx)*ustretchfactor1000+999)/1000;
		rd.bottom=(prevheight+prevy)*ustretchfactor1000/1000;
		//And now the last bit of alignment
		if (capsDrv.dwCaps&DDCAPS_ALIGNSIZEDEST&&udestsizealign)
			rs.right=(int)((rd.right+udestsizealign-1)/udestsizealign)*udestsizealign;

		}
	return (TRUE);
	}

BOOL previewclass::setuppreview() {
	//**************************************************************
	//             Direct Draw Init
	//Lets Create the surface for Preview and Monitor
	//create the main DirectDraw object
	//DDPIXELFORMAT YUY2format={sizeof(DDPIXELFORMAT),DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0};  // YUY2
	//UYVY below need to enum types
	DDPIXELFORMAT YUY2format={sizeof(DDPIXELFORMAT),DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0}; // UYVY
	DDSURFACEDESC ddsd,ddsdOverlay;
	DDCAPS capsDrv;
	char *errormsg;
	//HRESULT ddrval;
	
	errormsg="Uknown"; //default errormsg for this section
	if (!(DirectDrawCreate(NULL,&lpDD,NULL)==DD_OK)) {return initFail(errormsg);}
	if (!(lpDD->QueryInterface(IID_IDirectDraw2, (void **)&lpDD4)==DD_OK)) {return initFail("Need newer version");}

	lpDD->Release();lpDD=NULL; //We don't need this anymore

	//For Normal operations we no longer need to provide an HWND
	if (!(lpDD4->SetCooperativeLevel(NULL,DDSCL_NORMAL)==DD_OK)) {return initFail(errormsg);}

	// Create the primary surface
	INIT_DIRECTX_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS;
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
	if (!(lpDD4->CreateSurface(&ddsd,&lpDDSPrimary,NULL)==DD_OK))  {return initFail(errormsg);}

	// create a clipper for the primary surface
	if (!(lpDD4->CreateClipper(0,&lpClipper,NULL)==DD_OK))  {return initFail(errormsg);}
	if (!(lpClipper->SetHWnd(0,prevwindow)==DD_OK))  {return initFail(errormsg);}
	if (!(lpDDSPrimary->SetClipper(lpClipper)==DD_OK))  {return initFail(errormsg);}

	//Now to check to see if the PC supports overlays, Yes Storyboard will now only
	//work on machines that have overlay I'm not about to have to convert to RGB
	//At least for this first release.
	errormsg=NO_OVERLAY_HARDWARE;
	// Get driver capabilities to determine Overlay support.
	INIT_DIRECTX_STRUCT(capsDrv);
	if (!(lpDD4->GetCaps(&capsDrv,NULL)==DD_OK))  {return initFail(errormsg);}
	// Does the driver support overlays in the current mode? 
	// (Currently the DirectDraw emulation layer does not support overlays.
	// Overlay related APIs will fail without hardware support).  
	if (!(capsDrv.dwCaps & DDCAPS_OVERLAY)) {return initFail(errormsg);}

	//Now to create the YUV overlay surface
	errormsg=UNABLE_TO_CREATE_OVERLAY;
	//Set up the overlay struct
	INIT_DIRECTX_STRUCT(ddsdOverlay);
	ddsdOverlay.dwBackBufferCount=0;
	ddsdOverlay.ddsCaps.dwCaps=DDSCAPS_OVERLAY|DDSCAPS_VIDEOMEMORY;
	ddsdOverlay.dwFlags= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
	ddsdOverlay.dwWidth=VIDEOX;
	ddsdOverlay.dwHeight=VIDEOY;
    ddsdOverlay.dwBackBufferCount=1;
	//We want raw frame, YUV422 surface, U0Y0V0Y1 U0Y0V0Y1 U0Y0V0Y1
	//Note PC lobyte hibyte makes it appear 0xYVYU reversed
	//It appears Matrox only supports YUY2 oh well
	ddsdOverlay.ddpfPixelFormat=YUY2format;
	// Try to create the overlay surface
	if (!(lpDD4->CreateSurface(&ddsdOverlay,&lpDDSOverlay,NULL)==DD_OK))  {return initFail(errormsg);}
	// Create an overlay FX structure so we can specify a source color key.
	// This information is ignored if the DDOVER_SRCKEYOVERRIDE flag isn't set.
	INIT_DIRECTX_STRUCT(ovfx);
	ovfx.dckSrcColorkey.dwColorSpaceLowValue=0; // Specify black as the color key
	ovfx.dckSrcColorkey.dwColorSpaceHighValue=0;

	return (TRUE);
	}


BOOL previewclass::initFail(char *reason)  {
	printc("Warning: No Overlay Support Detected Reason: ");
	printc(reason);
	printc("Preview will be transfered to the console");
	return (FALSE);
	} // End initFail

BOOL previewclass::startup() {
	//Preset Flags that will be sent to UpdateOverlay
	if (!previewoff) dwupdateflags=DDOVER_SHOW|DDOVER_DDFX;
	else dwupdateflags=DDOVER_HIDE|DDOVER_DDFX;
	return (TRUE);
	}

void previewclass::shutdown() {
	//Lets Cleanup the surface for Preview and Video
	if (lpDD4)  {
		if(lpDDSPrimary)  {
			lpDDSPrimary->Release();
			lpDDSPrimary=NULL;
			}
		lpDD4->Release();
		lpDD4=NULL;
		}
	if (lpDD) {
		lpDD->Release();
		lpDD = NULL;
		}
	//End DirectX surface stuff
	if (prevwindow) DestroyWindow(prevwindow);
	}


DWORD previewclass::updatepreview(ULONG *videobuf) {
	HRESULT hr;
	DDSURFACEDESC ddsd;
	DWORD dwWidth,dwHeight;
	ULONG lPitch;
	ULONG *psurf;
	ULONG dwBytesInRow;
	ULONG	widthexcess;
	//ULONG *videobuf;
	//COLORREF color;

	if ((lpDDSPrimary==NULL)||(!overlaysupport)) return (DWORD)E_FAIL; //TODO put on thread maker
	// make sure this surface is restored.
	//lpDDSOverlay->Restore();

	INIT_DIRECTX_STRUCT(ddsd);
	// Lock down the surface so we can modify it's contents.
	if (!(lpDDSOverlay->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)==DD_OK)) return ((DWORD)E_FAIL);
	psurf=(ULONG *)ddsd.lpSurface;

	dwWidth=ddsd.dwWidth>>1;
	dwHeight=ddsd.dwHeight>>1;   //TEMP stuff here
	lPitch=ddsd.lPitch;
	dwBytesInRow=ddsd.dwWidth<<1;
	widthexcess=720-dwBytesInRow;

	//if (videobuf=(ULONG *)toaster->videobuf) {
/*Here is the UYVY version*/
	__asm	{
		mov		edi, psurf
		 mov		ecx, dwHeight
		mov		esi, videobuf
loopy:
		mov		ebx,dwWidth
		shr		ebx,2
loopx:
		movq		mm0, [esi]
		 dec		ebx
		movq		mm1, [esi+8]
		movq		[edi], mm0
		movq		[edi+1408], mm0
		 add		esi, 16
		movq		[edi+8], mm1
		movq		[edi+1416], mm1
		add		edi, 16
		cmp		ebx,0
		jne		loopx
		add		edi,dwBytesInRow
		add		esi,32	//This may change
		dec		ecx
		jne		loopy
		emms
		}
//		}
/**/

/* This is the inverted YUY2 version of copy
	__asm {
	//for(y=0; y<dwHeight; y++) {
		mov		esi,videobuf
		mov		edi,psurf
		mov		ecx,dwHeight
		//for(x=0; x<dwWidth; x+=2) *psurf++=*videobuf++;
loopy:
		mov		ebx,dwWidth
loopx:
		mov		eax,[esi]
		mov		[edi+1],al
		mov		[edi+1409],al
		shr		eax,8
		mov		[edi+2],al
		mov		[edi+1410],al
		shr		eax,8
		mov		[edi+3],al
		mov		[edi+1411],al
		shr		eax,8
		mov		[edi],al
		mov		[edi+1408],al
		add		esi,4
		add		edi,4
		dec		ebx
		cmp		ebx,0
		jne		loopx
		//psurf+=(lPitch-dwBytesInRow);
		add		edi,dwBytesInRow
		add		esi,32	//This may change
		dec		ecx
		jne		loopy
		//}
		}
*/

	lpDDSOverlay->Unlock(NULL);
	hr=lpDDSOverlay->UpdateOverlay(&rs,lpDDSPrimary,&rd,dwupdateflags,&ovfx);
	return (hr);
	}


DWORD previewclass::updatepreviewthread() { 
	HRESULT hr;
	DDSURFACEDESC ddsd;
	DWORD dwWidth,dwHeight;
	ULONG lPitch;
	ULONG *psurf;
	ULONG dwBytesInRow;
	ULONG	widthexcess;
	ULONG *videobuf;

	//COLORREF color;
	WaitForSingleObject(arrayofevents[EVENT_PREVIEW],INFINITE);
	while (!killpreviewthread) {

		if ((lpDDSPrimary==NULL)||(!overlaysupport)) return (DWORD)E_FAIL; //TODO put on thread maker
		// make sure this surface is restored.
		//lpDDSOverlay->Restore();

		INIT_DIRECTX_STRUCT(ddsd);
		// Lock down the surface so we can modify it's contents.
		if (!(lpDDSOverlay->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)==DD_OK)) return ((DWORD)E_FAIL);
		psurf=(ULONG *)ddsd.lpSurface;

		dwWidth=ddsd.dwWidth>>1;
		dwHeight=ddsd.dwHeight>>1;   //TEMP stuff here
		lPitch=ddsd.lPitch;
		dwBytesInRow=ddsd.dwWidth<<1;
		widthexcess=720-dwBytesInRow;
		
		if (videobuf=(ULONG *)toaster->videobuf) {
			/*Here is the UYVY version*/
			__asm	{
				mov		edi, psurf
				 mov		ecx, dwHeight
				mov		esi, videobuf
loopy:
				mov		ebx,dwWidth
				shr		ebx,2
loopx:
				movq		mm0, [esi]
				 dec		ebx
				movq		mm1, [esi+8]
				movq		[edi], mm0
				movq		[edi+1408], mm0
				 add		esi, 16
				movq		[edi+8], mm1
				movq		[edi+1416], mm1
				add		edi, 16
				cmp		ebx,0
				jne		loopx
				add		edi,dwBytesInRow
				add		esi,32	//This may change
				dec		ecx
				jne		loopy
				emms
				}
			}
		lpDDSOverlay->Unlock(NULL);
		hr=lpDDSOverlay->UpdateOverlay(&rs,lpDDSPrimary,&rd,dwupdateflags,&ovfx);
		WaitForSingleObject(arrayofevents[EVENT_PREVIEW],INFINITE);
		}
	return (hr);
	} 
