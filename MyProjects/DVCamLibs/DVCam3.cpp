#include "DVCam.h"

#define WM_FGNOTIFY	WM_USER+1 //TODO handle this message

#define MENU_DIALOG0		42
#define MENU_DIALOG1		43
#define MENU_DIALOG2		44
#define MENU_DIALOG3		45
#define MENU_DIALOG4		46
#define MENU_DIALOG5		47
#define MENU_DIALOG6		48
#define MENU_DIALOG7		49
#define MENU_DIALOG8		50
#define MENU_DIALOG9		51
#define MENU_DIALOGA		52
#define MENU_DIALOGB		53
#define MENU_DIALOGC		54
#define MENU_DIALOGD		55
#define MENU_DIALOGE		56
#define MENU_DIALOGF		57	// !!! more?


/*
This section contains all interface implementation
*/

// start the capture graph
//
BOOL DVCamlib::StartCapture()
{
    BOOL fHasStreamControl; // BOOL f var here
    HRESULT hr;

    // way ahead of you
    if (gcap.fCapturing)
	return TRUE;

    // or we'll get confused
    if (gcap.fPreviewing)
	StopPreview();

    // or we'll crash
    if (!gcap.fCaptureGraphBuilt)
	return FALSE;

    // This amount will be subtracted from the number of dropped and not 
    // dropped frames reported by the filter.  Since we might be having the
    // filter running while the pin is turned off, we don't want any of the
    // frame statistics from the time the pin is off interfering with the
    // statistics we gather while the pin is on
    gcap.lDroppedBase = 0;
    gcap.lNotBase = 0;

    REFERENCE_TIME start = MAX_TIME, stop = MAX_TIME;

    // don't capture quite yet...
    hr = gcap.pBuilder->ControlStream(&PIN_CATEGORY_CAPTURE, NULL,
				NULL, &start, NULL, 0, 0);
    //DbgLog((LOG_TRACE,1,TEXT("Capture OFF returns %x"), hr));

    // Do we have the ability to control capture and preview separately?
    fHasStreamControl = SUCCEEDED(hr);

    // prepare to run the graph
    IMediaControl *pMC = NULL;
    hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (FAILED(hr)) {
	ErrMsg("Error %x: Cannot get IMediaControl", hr);
	return FALSE;
    }

    // If we were able to keep capture off, then we can
    // run the graph now for frame accurate start later yet still showing a
    // preview.   Otherwise, we can't run the graph yet without capture
    // starting too, so we'll pause it so the latency between when they
    // press a key and when capture begins is still small (but they won't have
    // a preview while they wait to press a key)

    if (fHasStreamControl)
	hr = pMC->Run();
    else
	hr = pMC->Pause();
    if (FAILED(hr)) {
	// stop parts that started
	pMC->Stop();
	pMC->Release();
	ErrMsg("Error %x: Cannot start graph", hr);
	return FALSE;
    }

/*
    // press a key to start capture
    f = DoDialog(ghwndApp, IDD_PressAKeyDialog, (DLGPROC)PressAKeyProc, 0);
    if (!f) {
	pMC->Stop();
	pMC->Release();
	if (gcap.fWantPreview) {
	    BuildPreviewGraph();
	    StartPreview();
	}
	return f;
    }
*/
	//Sleep(100); //just for latency issues

    // Start capture NOW!
    if (fHasStreamControl) {
	// we may not have this yet
        if (!gcap.pDF) {
	    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				   &MEDIATYPE_Interleaved, gcap.pVCap,
				   IID_IAMDroppedFrames, (void **)&gcap.pDF);
	    if (hr != NOERROR)
	        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				   &MEDIATYPE_Video, gcap.pVCap,
				   IID_IAMDroppedFrames, (void **)&gcap.pDF);
	}

	// turn the capture pin on now!
	hr = gcap.pBuilder->ControlStream(&PIN_CATEGORY_CAPTURE, NULL,
				NULL, NULL, &stop, 0, 0);
    	//DbgLog((LOG_TRACE,0,TEXT("Capture ON returns %x"), hr));
	// make note of the current dropped frame counts
	if (gcap.pDF) {
	    gcap.pDF->GetNumDropped(&gcap.lDroppedBase);
	    gcap.pDF->GetNumNotDropped(&gcap.lNotBase);
    	    //DbgLog((LOG_TRACE,0,TEXT("Dropped counts are %ld and %ld"),
	    //		gcap.lDroppedBase, gcap.lNotBase));
        } 
    } else {
	hr = pMC->Run();
	if (FAILED(hr)) {
	    // stop parts that started
	    pMC->Stop();
	    pMC->Release();
	    ErrMsg("Error %x: Cannot run graph", hr);
	    return FALSE;
	}
    }

    pMC->Release();

    // when did we start capture?
    gcap.lCapStartTime = timeGetTime();
/*
    // 30 times a second I want to update my status bar - #captured, #dropped
    SetTimer(ghwndApp, 1, 33, NULL);
*/
    gcap.fCapturing = TRUE;
    return TRUE;
}


// stop the capture graph
//
BOOL DVCamlib::StopCapture()
{
    // way ahead of you
    if (!gcap.fCapturing) {
	return FALSE;
    }

    // stop the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (SUCCEEDED(hr)) {
	hr = pMC->Stop();
	pMC->Release();
    }
    if (FAILED(hr)) {
	ErrMsg("Error %x: Cannot stop graph", hr);
	return FALSE;
    }

    // when the graph was stopped
    gcap.lCapStopTime = timeGetTime();
/*
    // no more status bar updates
    KillTimer(ghwndApp, 1);

    // one last time for the final count and all the stats
    UpdateStatus(TRUE);
*/
    gcap.fCapturing = FALSE;
/*
    // !!! get rid of menu garbage
    InvalidateRect(ghwndApp, NULL, TRUE);
*/
    return TRUE;
}

// pre-allocate the capture file
//
BOOL DVCamlib::AllocCaptureFile()
{
    // we'll get into an infinite loop in the dlg proc setting a value
    if (gcap.szCaptureFile[0] == 0)
	return FALSE;

	gcap.wCapFileSize = 10; //we're setting capture size to 10 megs
	// User has hit OK. Alloc requested capture file space
	BOOL f = MakeBuilder();
	if (!f)
	    return FALSE;
	WCHAR wach[_MAX_PATH];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gcap.szCaptureFile, -1,
							wach, _MAX_PATH);
	if (gcap.pBuilder->AllocCapFile(wach,
		(DWORDLONG)gcap.wCapFileSize * 1024L * 1024L) != NOERROR) {
	    MessageBoxA(window, "Error",
				"Failed to pre-allocate capture file space",
				MB_OK | MB_ICONEXCLAMATION);
	    return FALSE;
    } 
	return TRUE;
}

/*
 * Put up a dialog to allow the user to select a capture file.
 */

BOOL DVCamlib::SetCaptureFile()
{
   if (*filename) {
	OFSTRUCT os;
	wsprintf(gcap.szCaptureFile,"%s%s.avi",filepath,filename);

	// We have a capture file name

	/*
	 * if this is a new file, then invite the user to
	 * allocate some space
	 */
	if (OpenFile(gcap.szCaptureFile, &os, OF_EXIST) == HFILE_ERROR) {

	    // bring up dialog, and set new file size
	    BOOL f = AllocCaptureFile();
	    if (!f)
		return FALSE;
	}
    } else {
	return FALSE;
    }

    //SetAppCaption();    // new a new app caption

    // tell the file writer to use the new filename
    if (gcap.pSink) {
	WCHAR wach[_MAX_PATH];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gcap.szCaptureFile, -1,
							wach, _MAX_PATH);
	gcap.pSink->SetFileName(wach, NULL);
    }

    return TRUE;
}


// build the capture graph!
//
BOOL DVCamlib::BuildCaptureGraph()
{
    int cy, cyBorder;
    HRESULT hr;
    BOOL f;
    AM_MEDIA_TYPE *pmt;

    // we have one already
    if (gcap.fCaptureGraphBuilt)
	return TRUE;

    // No rebuilding while we're running
    if (gcap.fCapturing || gcap.fPreviewing)
	return FALSE;

    // We don't have the necessary capture filters
    if (gcap.pVCap == NULL)
	return FALSE;
    if (gcap.pACap == NULL && gcap.fCapAudio)
	return FALSE;

    // no capture file name yet... we need one first
//    if (gcap.szCaptureFile[0] == 0) {
	f = SetCaptureFile();
	if (!f)
	    return f;
//    }

    // we already have another graph built... tear down the old one
    if (gcap.fPreviewGraphBuilt)
	TearDownGraph();

//
// We need a rendering section that will write the capture file out in AVI
// file format
//

    WCHAR wach[_MAX_PATH];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gcap.szCaptureFile, -1, wach,
								_MAX_PATH);
    GUID guid = MEDIASUBTYPE_Avi;
    hr = gcap.pBuilder->SetOutputFileName(&guid, wach, &gcap.pRender,
								&gcap.pSink);
    if (hr != NOERROR) {
	ErrMsg("Cannot set output file");
	goto SetupCaptureFail;
    }

// Now tell the AVIMUX to write out AVI files that old apps can read properly.
// If we don't, most apps won't be able to tell where the keyframes are,
// slowing down editing considerably
// Doing this will cause one seek (over the area the index will go) when
// you capture past 1 Gig, but that's no big deal.
// NOTE: This is on by default, so it's not necessary to turn it on

// Also, set the proper MASTER STREAM

    hr = gcap.pRender->QueryInterface(IID_IConfigAviMux,
						(void **)&gcap.pConfigAviMux);
    if (hr == NOERROR && gcap.pConfigAviMux) {
	gcap.pConfigAviMux->SetOutputCompatibilityIndex(TRUE);
	if (gcap.fCapAudio) {
	    hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
	    if (hr != NOERROR)
		ErrMsg("SetMasterStream failed!");
	}
    }

//
// Render the video capture and preview pins - even if the capture filter only
// has a capture pin (and no preview pin) this should work... because the
// capture graph builder will use a smart tee filter to provide both capture
// and preview.  We don't have to worry.  It will just work.
//

// NOTE that we try to render the interleaved pin before the video pin, because
// if BOTH exist, it's a DV filter and the only way to get the audio is to use
// the interleaved pin.  Using the Video pin on a DV filter is only useful if
// you don't want the audio.

    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
					&MEDIATYPE_Interleaved,
					gcap.pVCap, NULL, gcap.pRender);
    if (hr != NOERROR) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
					&MEDIATYPE_Video,
					gcap.pVCap, NULL, gcap.pRender);
        if (hr != NOERROR) {
	    ErrMsg("Cannot render video capture stream");
	    goto SetupCaptureFail;
	}
    }

    if (gcap.fWantPreview) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Interleaved, gcap.pVCap, NULL, NULL);
        if (hr == VFW_S_NOPREVIEWPIN) {
	    // preview was faked up for us using the (only) capture pin
	    gcap.fPreviewFaked = TRUE;
	} else if (hr != S_OK) {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
            if (hr == VFW_S_NOPREVIEWPIN) {
	        // preview was faked up for us using the (only) capture pin
	        gcap.fPreviewFaked = TRUE;
	    } else if (hr != S_OK) {
	        ErrMsg("Cannot render video preview stream");
	        goto SetupCaptureFail;
	    }
        }
    }

//
// Render the audio capture pin?
//

    if (gcap.fCapAudio) {
	hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Audio, gcap.pACap, NULL, gcap.pRender);
	if (hr != NOERROR) {
	    ErrMsg("Cannot render audio capture stream");
	    goto SetupCaptureFail;
	}
    }

//
// Render the closed captioning pin? It could be a CC or a VBI category pin,
// depending on the capture driver
//

    if (gcap.fCapCC) {
	hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CC, NULL,
					gcap.pVCap, NULL, gcap.pRender);
	if (hr != NOERROR) {
	    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
					gcap.pVCap, NULL, gcap.pRender);
	    if (hr != NOERROR) {
	        ErrMsg("Cannot render closed captioning");
	        // so what? goto SetupCaptureFail;
            }
	}
 	// To preview and capture VBI at the same time, we can call this twice
        if (gcap.fWantPreview) {
	    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
					gcap.pVCap, NULL, NULL);
	}
    }

//
// Get the preview window to be a child of our app's window
//

    // This will find the IVideoWindow interface on the renderer.  It is 
    // important to ask the filtergraph for this interface... do NOT use
    // ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
    // know we own the window so it can give us display changed messages, etc.

    // NOTE: We do this even if we didn't ask for a preview, because rendering
    // the capture pin may have rendered the preview pin too (WDM overlay 
    // devices) because they must have a preview going.  So we better always
    // put the preview window in our app, or we may get a top level window
    // appearing out of nowhere!

   	hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
        if (hr != NOERROR && gcap.fWantPreview) {
	    ErrMsg("This graph cannot preview");
        } else if (hr == NOERROR) {
	    RECT rc;
	    gcap.pVW->put_Owner((long)prevwindow);    // We own the window now
	    gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child
	    // give the preview window all our space but where the status bar is
	    GetClientRect(prevwindow, &rc);
	    cyBorder = GetSystemMetrics(SM_CYBORDER);
	    cy = cyBorder; //statusGetHeight() + 
	    //rc.bottom -= cy;
	    gcap.pVW->SetWindowPosition(0, 0, rc.right, rc.bottom); // be this big
	    gcap.pVW->put_Visible(OATRUE);
        }

    // now tell it what frame rate to capture at.  Just find the format it
    // is capturing with, and leave everything alone but change the frame rate
    hr = gcap.fUseFrameRate ? E_FAIL : NOERROR;
    if (gcap.pVSC && gcap.fUseFrameRate) {
	hr = gcap.pVSC->GetFormat(&pmt);
	// DV capture does not use a VIDEOINFOHEADER
        if (hr == NOERROR) {
	    if (pmt->formattype == FORMAT_VideoInfo) {
	        VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
	        pvi->AvgTimePerFrame = (LONGLONG)(10000000 / gcap.FrameRate);
	        hr = gcap.pVSC->SetFormat(pmt);
	    }
	    DeleteMediaType(pmt);
        }
    }
    if (hr != NOERROR)
	ErrMsg("Cannot set frame rate for capture");

    // now ask the filtergraph to tell us when something is completed or aborted
    // (EC_COMPLETE, EC_USERABORT, EC_ERRORABORT).  This is how we will find out
    // if the disk gets full while capturing
    hr = gcap.pFg->QueryInterface(IID_IMediaEventEx, (void **)&gcap.pME);
    if (hr == NOERROR) {
	gcap.pME->SetNotifyWindow((LONG)window, WM_FGNOTIFY, 0);
    }

// All done.

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    gcap.fCaptureGraphBuilt = TRUE;
    return TRUE;

SetupCaptureFail:
    TearDownGraph();
    return FALSE;
}

  /******************************************/
 /*	All the menu options here and below	 */
/******************************************/

void DVCamlib::MakeMenuOptions()
{
    HRESULT hr;
    HMENU hMenuSub = GetSubMenu(GetMenu(window), 1); // Options menu

    // remove any old choices from the last device
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 0, MF_BYPOSITION);

    int zz = 0;
    gcap.iFormatDialogPos = -1;
    gcap.iSourceDialogPos = -1;
    gcap.iDisplayDialogPos = -1;
    gcap.iVCapDialogPos = -1;
    gcap.iVCrossbarDialogPos = -1;
    gcap.iTVTunerDialogPos = -1;
    gcap.iACapDialogPos = -1;
    gcap.iACrossbarDialogPos = -1;
    gcap.iTVAudioDialogPos = -1;
    gcap.iVCapCapturePinDialogPos = -1;
    gcap.iVCapPreviewPinDialogPos = -1;
    gcap.iACapCapturePinDialogPos = -1;

    // If this device supports the old legacy UI dialogs, offer them

    if (gcap.pDlg && !gcap.pDlg->HasDialog(VfwCaptureDialog_Format)) {
	AppendMenuA(hMenuSub, MF_STRING, MENU_DIALOG0 + zz, "Video Format...");
	gcap.iFormatDialogPos = zz++;
    }
    if (gcap.pDlg && !gcap.pDlg->HasDialog(VfwCaptureDialog_Source)) {
	AppendMenuA(hMenuSub, MF_STRING, MENU_DIALOG0 + zz, "Video Source...");
	gcap.iSourceDialogPos = zz++;
    }
    if (gcap.pDlg && !gcap.pDlg->HasDialog(VfwCaptureDialog_Display)) {
	AppendMenuA(hMenuSub, MF_STRING, MENU_DIALOG0 + zz, "Video Display...");
	gcap.iDisplayDialogPos = zz++;
    }

    // Also check the audio capture filter at this point, since even non wdm devices
    // may support an IAMAudioInputMixer property page (we'll also get any wdm filter
    // properties here as well). We'll get any audio capture pin property pages just
    // a bit later.
    if (gcap.pACap != NULL)
    {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;
        
        hr = gcap.pACap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
                AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"Audio Capture Filter...");
                gcap.iACapDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
    }

    // don't bother looking for new property pages if the old ones are supported
    // or if we don't have a capture filter
    if (gcap.pVCap == NULL || gcap.iFormatDialogPos != -1)
	return;

    // New WDM devices support new UI and new interfaces.
    // Your app can use some default property
    // pages for UI if you'd like (like we do here) or if you don't like our
    // dialog boxes, feel free to make your own and programmatically set 
    // the capture options through interfaces like IAMCrossbar, IAMCameraControl
    // etc.

    // There are 9 objects that might support property pages.  Let's go through
    // them.

    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;

    // 1. the video capture filter itself

    hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
    if (hr == S_OK) {
        hr = pSpec->GetPages(&cauuid);
        if (hr == S_OK && cauuid.cElems > 0) {
	    AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"Video Capture Filter...");
	    gcap.iVCapDialogPos = zz++;
	    CoTaskMemFree(cauuid.pElems);
	}
	pSpec->Release();
    }

    // 2.  The video capture capture pin

    IAMStreamConfig *pSC;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Interleaved,
			gcap.pVCap, IID_IAMStreamConfig, (void **)&pSC);
    if (hr != S_OK)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
	        AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"Video Capture Pin...");
	        gcap.iVCapCapturePinDialogPos = zz++;
	        CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
	pSC->Release();
    }

    // 3.  The video capture preview pin.
    // This basically sets the format being previewed.  Typically, you
    // want to capture and preview using the SAME format, instead of having to
    // enter the same value in 2 dialog boxes.  For a discussion on this, see
    // the comment above the MakePreviewGraph function.

    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr != NOERROR)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
	        AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"Video Preview Pin...");
	        gcap.iVCapPreviewPinDialogPos = zz++;
		CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
	pSC->Release();
    }

    // 4 & 5.  The video crossbar, and a possible second crossbar

    IAMCrossbar *pX, *pX2;
    IBaseFilter *pXF;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMCrossbar, (void **)&pX);
    if (hr != S_OK)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMCrossbar, (void **)&pX);
    if (hr == S_OK) {
        hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
        if (hr == S_OK) {
            hr = pX->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
            if (hr == S_OK) {
                hr = pSpec->GetPages(&cauuid);
                if (hr == S_OK && cauuid.cElems > 0) {
	            AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,
							"Video Crossbar...");
	            gcap.iVCrossbarDialogPos = zz++;
		    CoTaskMemFree(cauuid.pElems);
	        }
	        pSpec->Release();
            }
            hr = gcap.pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,
				IID_IAMCrossbar, (void **)&pX2);
            if (hr == S_OK) {
                hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
                if (hr == S_OK) {
                    hr = pSpec->GetPages(&cauuid);
                    if (hr == S_OK && cauuid.cElems > 0) {
	                AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,
							"Second Crossbar...");
	                gcap.iACrossbarDialogPos = zz++;
		        CoTaskMemFree(cauuid.pElems);
	            }
	            pSpec->Release();
                }
	        pX2->Release();
	    }
 	    pXF->Release();
        }
	pX->Release();
    }

    // 6.  The TVTuner

    IAMTVTuner *pTV;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMTVTuner, (void **)&pTV);
    if (hr != S_OK)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMTVTuner, (void **)&pTV);
    if (hr == S_OK) {
        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
	        AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"TV Tuner...");
	        gcap.iTVTunerDialogPos = zz++;
		CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
 	pTV->Release();
    }

    // no audio capture, we're done
    if (gcap.pACap == NULL)
	return;

    // 7.  The Audio capture filter itself... Thanks anyway, but we got these already

    // 8.  The Audio capture pin

    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Audio, gcap.pACap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
	        AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"Audio Capture Pin...");
	        gcap.iACapCapturePinDialogPos = zz++;
		CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
 	pSC->Release();
    }

    // 9.  The TV Audio filter

    IAMTVAudio *pTVA;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, 
				&MEDIATYPE_Audio, gcap.pACap,
				IID_IAMTVAudio, (void **)&pTVA);
    if (hr == S_OK) {
        hr = pTVA->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
	        AppendMenuA(hMenuSub,MF_STRING,MENU_DIALOG0+zz,"TV Audio...");
	        gcap.iTVAudioDialogPos = zz++;
		CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
 	pTVA->Release();
    }

    // 10.  Crossbar class helper menu item, to let you choose an input

    if (gcap.pCrossbar && gcap.NumberOfVideoInputs) {
        gcap.hMenuPopup = CreatePopupMenu();
        LONG j;
        LONG  PhysicalType;
        TCHAR buf[MAX_PATH];
        LONG InputToEnable = -1;

	gcap.iVideoInputMenuPos = zz++;
        AppendMenuA(hMenuSub, MF_SEPARATOR, 0, NULL);

        for (j = 0; j < gcap.NumberOfVideoInputs; j++) {
            EXECUTE_ASSERT (S_OK == gcap.pCrossbar->GetInputType (j, &PhysicalType));
            EXECUTE_ASSERT (S_OK == gcap.pCrossbar->GetInputName (j, buf, sizeof (buf)));
            AppendMenuA(gcap.hMenuPopup,MF_STRING,MENU_DIALOG0+zz, buf);
            zz++;

            // Route the first TVTuner by default
            if ((PhysicalType == PhysConn_Video_Tuner) && InputToEnable == -1) {
                InputToEnable = j;
            }
        }
            
        AppendMenuA(hMenuSub, MF_STRING | MF_POPUP, (UINT)gcap.hMenuPopup, "Video Input");

        if (InputToEnable == -1) {
            InputToEnable = 0;
        }
        CheckMenuItem(gcap.hMenuPopup, InputToEnable, MF_BYPOSITION | MF_CHECKED); 

        gcap.pCrossbar->SetInputIndex (InputToEnable);
    }
    // !!! anything needed to delete the popup when selecting a new input?
}



/*----------------------------------------------------------------------------*\
|    AppCommand()
|
|    Process all of our WM_COMMAND messages.
\*----------------------------------------------------------------------------*/
LONG PASCAL DVCamlib::AppCommand (HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    int id = GET_WM_COMMAND_ID(wParam, lParam);
    switch(id)
    {
/*
	//
	// Our about box
	//
	case MENU_ABOUT:
	    DialogBox(ghInstApp, MAKEINTRESOURCE(IDD_ABOUT), hwnd, 
							(DLGPROC)AboutDlgProc);
	    break;

	//
	// We want out of here!
	//
	case MENU_EXIT:
	    PostMessage(hwnd,WM_CLOSE,0,0L);
	    break;

	// choose a capture file
	//
	case MENU_SET_CAP_FILE:
	    SetCaptureFile(hwnd);
	    break;

	// pre-allocate the capture file
	//
	case MENU_ALLOC_CAP_FILE:
	    AllocCaptureFile(hwnd);
	    break;

	// save the capture file
	//
	case MENU_SAVE_CAP_FILE:
	    SaveCaptureFile(hwnd);
	    break;

	// start capturing
	//
	case MENU_START_CAP:
	    if (gcap.fPreviewing)
		StopPreview();
	    if (gcap.fPreviewGraphBuilt)
		TearDownGraph();
	    BuildCaptureGraph();
	    StartCapture();
	    break;

	// toggle preview
	// 
	case MENU_PREVIEW:
	    gcap.fWantPreview = !gcap.fWantPreview;
	    if (gcap.fWantPreview) {
		BuildPreviewGraph();
		StartPreview();
	    } else
		StopPreview();
	    break;

	// stop capture
	//
	case MENU_STOP_CAP:
	    StopCapture();
	    if (gcap.fWantPreview) {
		BuildPreviewGraph();
		StartPreview();
	    }
	    break;

	// select the master stream
	//
	case MENU_NOMASTER:
	    gcap.iMasterStream = -1;
	    if (gcap.pConfigAviMux) {
		hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
		if (hr != NOERROR)
		    ErrMsg("SetMasterStream failed!");
	    }
	    break;
	case MENU_AUDIOMASTER:
	    gcap.iMasterStream = 1;
	    if (gcap.pConfigAviMux) {
		hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
		if (hr != NOERROR)
		    ErrMsg("SetMasterStream failed!");
	    }
	    break;
	case MENU_VIDEOMASTER:
	    gcap.iMasterStream = 0;
	    if (gcap.pConfigAviMux) {
		hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
		if (hr != NOERROR)
		    ErrMsg("SetMasterStream failed!");
	    }
	    break;

	// toggle capturing audio
	case MENU_CAP_AUDIO:
	    if (gcap.fPreviewing)
		StopPreview();
	    gcap.fCapAudio = !gcap.fCapAudio;
	    // when we capture we'll need a different graph now
	    if (gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
		TearDownGraph();
	    if (gcap.fWantPreview) {
		BuildPreviewGraph();
		StartPreview();
	    }
	    break;

	// toggle closed captioning
	case MENU_CAP_CC:
	    if (gcap.fPreviewing)
		StopPreview();
	    gcap.fCapCC = !gcap.fCapCC;
	    // when we capture we'll need a different graph now
	    if (gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
		TearDownGraph();
	    if (gcap.fWantPreview) {
		BuildPreviewGraph();
		StartPreview();
	    }
	    break;

	// choose the audio capture format
	//
	case MENU_AUDIOFORMAT:
	    ChooseAudioFormat();
	    break;

	// pick a frame rate
	//
	case MENU_FRAMERATE:
	    ChooseFrameRate();
	    break;

	// pick a time limit
	//
	case MENU_TIMELIMIT:
	    ChooseTimeLimit();
	    break;
*/
	// pick which video capture device to use
	// pick which video capture device to use
	//
	case MENU_VDEVICE0:
	case MENU_VDEVICE1:
	case MENU_VDEVICE2:
	case MENU_VDEVICE3:
	case MENU_VDEVICE4:
	case MENU_VDEVICE5:
	case MENU_VDEVICE6:
	case MENU_VDEVICE7:
	case MENU_VDEVICE8:
	case MENU_VDEVICE9:
	    ChooseDevices(id - MENU_VDEVICE0, gcap.iAudioDevice);
	    break;

	// pick which audio capture device to use
	//
	case MENU_ADEVICE0:
	case MENU_ADEVICE1:
	case MENU_ADEVICE2:
	case MENU_ADEVICE3:
	case MENU_ADEVICE4:
	case MENU_ADEVICE5:
	case MENU_ADEVICE6:
	case MENU_ADEVICE7:
	case MENU_ADEVICE8:
	case MENU_ADEVICE9:
	    ChooseDevices(gcap.iVideoDevice, id - MENU_ADEVICE0);
	    break;

	// video format dialog
	//
	case MENU_DIALOG0:
	case MENU_DIALOG1:
	case MENU_DIALOG2:
	case MENU_DIALOG3:
	case MENU_DIALOG4:
	case MENU_DIALOG5:
	case MENU_DIALOG6:
	case MENU_DIALOG7:
	case MENU_DIALOG8:
	case MENU_DIALOG9:
	case MENU_DIALOGA:
	case MENU_DIALOGB:
	case MENU_DIALOGC:
	case MENU_DIALOGD:
	case MENU_DIALOGE:
	case MENU_DIALOGF:

 	    // they want the VfW format dialog
	    if (id - MENU_DIALOG0 == gcap.iFormatDialogPos) {
		// this dialog will not work while previewing
		if (gcap.fWantPreview)
		    StopPreview();
		HRESULT hrD;
	        hrD = gcap.pDlg->ShowDialog(VfwCaptureDialog_Format, window2);
		// Oh uh!  Sometimes bringing up the FORMAT dialog can result
		// in changing to a capture format that the current graph 
		// can't handle.  It looks like that has happened and we'll
		// have to rebuild the graph.
		if (hrD == VFW_E_CANNOT_CONNECT) {
    		    DbgLog((LOG_TRACE,1,TEXT("DIALOG CORRUPTED GRAPH!")));
		    TearDownGraph();	// now we need to rebuild
		    // !!! This won't work if we've left a stranded h/w codec
		}

		// Resize our window to be the same size that we're capturing
	        if (gcap.pVSC) {
		    AM_MEDIA_TYPE *pmt;
		    // get format being used NOW
		    hr = gcap.pVSC->GetFormat(&pmt);
	    	    // DV capture does not use a VIDEOINFOHEADER
            	    if (hr == NOERROR) {
	 		if (pmt->formattype == FORMAT_VideoInfo) {
		            // resize our window to the new capture size
		            ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
					abs(HEADER(pmt->pbFormat)->biHeight));
			}
		        DeleteMediaType(pmt);
		    }
	        }

	        if (gcap.fWantPreview) {
		    BuildPreviewGraph();
		    StartPreview();
		}
	    } else if (id - MENU_DIALOG0 == gcap.iSourceDialogPos) {
		// this dialog will not work while previewing
		if (gcap.fWantPreview)
		    StopPreview();
	        gcap.pDlg->ShowDialog(VfwCaptureDialog_Source, window2);
	        if (gcap.fWantPreview)
		    StartPreview();
	    } else if (id - MENU_DIALOG0 == gcap.iDisplayDialogPos) {
		// this dialog will not work while previewing
		if (gcap.fWantPreview)
		    StopPreview();
	        gcap.pDlg->ShowDialog(VfwCaptureDialog_Display, window2);
	        if (gcap.fWantPreview)
		    StartPreview();

	    // now the code for the new dialogs

	    } else if (id - MENU_DIALOG0 == gcap.iVCapDialogPos) {
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&gcap.pVCap, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}

	    } else if (id - MENU_DIALOG0 == gcap.iVCapCapturePinDialogPos) {
		// You can change this pin's output format in these dialogs.
		// If the capture pin is already connected to somebody who's 
		// fussy about the connection type, that may prevent using 
		// this dialog(!) because the filter it's connected to might not
		// allow reconnecting to a new format.  This might happen if
		// we are dealing with a filter with only one output pin,
		// and we are previewing by splitting its output.  In such
		// a case, I need to tear down the graph downstream of the
		// capture filter before bringing up these dialogs.
		// Don't worry about a normal capture graph where the capture
		// pin is connected to the AVI MUX; it will accept any fmt
		// change.
		// In any case, the graph must be STOPPED when calling them.
		if (gcap.fWantPreview)
		    StopPreview();	// make sure graph is stopped
		if (gcap.fPreviewFaked) {
    		    DbgLog((LOG_TRACE,1,TEXT("Tear down graph for dialog")));
		    TearDownGraph();	// graph could prevent dialog working
		}
    		IAMStreamConfig *pSC;
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
	 	if (hr != NOERROR)
    		    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pSC, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);

		    // !!! What if changing output formats couldn't reconnect
		    // and the graph is broken?  Shouldn't be possible...
		
	            if (gcap.pVSC) {
		        AM_MEDIA_TYPE *pmt;
		        // get format being used NOW
		        hr = gcap.pVSC->GetFormat(&pmt);
	    	        // DV capture does not use a VIDEOINFOHEADER
            	        if (hr == NOERROR) {
	 		    if (pmt->formattype == FORMAT_VideoInfo) {
		                // resize our window to the new capture size
		                ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
					  abs(HEADER(pmt->pbFormat)->biHeight));
			    }
		            DeleteMediaType(pmt);
		        }
	            }

		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pSC->Release();
	        if (gcap.fWantPreview) {
		    BuildPreviewGraph();
		    StartPreview();
		}

	    } else if (id - MENU_DIALOG0 == gcap.iVCapPreviewPinDialogPos) {
		// this dialog may not work if the preview pin is connected
		// already, because the downstream filter may reject a format
		// change, so we better kill the graph.
		if (gcap.fWantPreview) {
		    StopPreview();
		    TearDownGraph();
		}
    		IAMStreamConfig *pSC;
		// This dialog changes the preview format, so it might affect
		// the format being drawn.  Our app's window size is taken
		// from the size of the capture pin's video, not the preview
		// pin, so changing that here won't have any effect. All in all,
		// this probably won't be a terribly useful dialog in this app.
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
		if (hr != NOERROR)
    		    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pSC, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pSC->Release();
		if (gcap.fWantPreview) {
		    BuildPreviewGraph();
		    StartPreview();
		}

	    } else if (id - MENU_DIALOG0 == gcap.iVCrossbarDialogPos) {
    		IAMCrossbar *pX;
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMCrossbar, (void **)&pX);
		if (hr != NOERROR)
    		    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMCrossbar, (void **)&pX);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pX->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pX, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pX->Release();

	    } else if (id - MENU_DIALOG0 == gcap.iTVTunerDialogPos) {
    		IAMTVTuner *pTV;
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMTVTuner, (void **)&pTV);
		if (hr != NOERROR)
    		    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMTVTuner, (void **)&pTV);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pTV, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pTV->Release();

	    } else if (id - MENU_DIALOG0 == gcap.iACapDialogPos) {
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = gcap.pACap->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&gcap.pACap, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}

	    } else if (id - MENU_DIALOG0 == gcap.iACapCapturePinDialogPos) {
		// this dialog will not work while previewing - it might change
		// the output format!
		if (gcap.fWantPreview)
		    StopPreview();
    		IAMStreamConfig *pSC;
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Audio, gcap.pACap,
				IID_IAMStreamConfig, (void **)&pSC);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pSC, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pSC->Release();
	        if (gcap.fWantPreview)
		    StartPreview();

	    } else if (id - MENU_DIALOG0 == gcap.iACrossbarDialogPos) {
    		IAMCrossbar *pX, *pX2;
		IBaseFilter *pXF;
		// we could use better error checking here... I'm assuming
		// this won't fail
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, gcap.pVCap,
				IID_IAMCrossbar, (void **)&pX);
		if (hr != NOERROR)
    		    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMCrossbar, (void **)&pX);
		hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
    		hr = gcap.pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL,
				pXF, IID_IAMCrossbar, (void **)&pX2);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pX2, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pX2->Release();
		pXF->Release();
		pX->Release();

	    } else if (id - MENU_DIALOG0 == gcap.iTVAudioDialogPos) {
    		IAMTVAudio *pTVA;
    		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Audio, gcap.pACap,
				IID_IAMTVAudio, (void **)&pTVA);
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
    	        hr = pTVA->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    		if (hr == S_OK) {
        	    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(window2, 30, 30, NULL, 1,
                    (IUnknown **)&pTVA, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
		pTVA->Release();

        } else if (((id - MENU_DIALOG0) >  gcap.iVideoInputMenuPos) && 
                    (id - MENU_DIALOG0) <= gcap.iVideoInputMenuPos + gcap.NumberOfVideoInputs) {
            // Remove existing checks
            for (int j = 0; j < gcap.NumberOfVideoInputs; j++) {

                CheckMenuItem(gcap.hMenuPopup, j, MF_BYPOSITION | 
                              ((j == (id - MENU_DIALOG0) - gcap.iVideoInputMenuPos - 1) ?
                              MF_CHECKED : MF_UNCHECKED )); 
            }

            if (gcap.pCrossbar) {
                EXECUTE_ASSERT (S_OK == gcap.pCrossbar->SetInputIndex ((id - MENU_DIALOG0) - gcap.iVideoInputMenuPos - 1));
            }

	}

	break;

    }
    return 0L;
}
