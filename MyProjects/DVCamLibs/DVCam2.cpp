#include "DVCam.h"


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


/*----------------------------------------------------------------------------*\
|   ErrMsg - Opens a Message box with a error message in it.  The user can     |
|            select the OK button to continue                                  |
\*----------------------------------------------------------------------------*/
int DVCamlib::ErrMsg (LPTSTR sz,...)
{
    static TCHAR ach[2000];
    va_list va;

    va_start(va, sz);
    wvsprintf (ach,sz, va);
    va_end(va);
    MessageBox(window,ach,NULL, MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);
    return FALSE;
}

// Make a graph builder object we can use for capture graph building
//
BOOL DVCamlib::MakeBuilder() {
    // we have one already
    if (gcap.pBuilder)
	return TRUE;

    HRESULT hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2,
			NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder2,
			(void **)&gcap.pBuilder);
    return (hr == NOERROR) ? TRUE : FALSE;
	}


// Make a graph object we can use for capture graph building
//
BOOL DVCamlib::MakeGraph() {
    // we have one already
    if (gcap.pFg)
	return TRUE;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			       IID_IGraphBuilder, (LPVOID *)&gcap.pFg);
    return (hr == NOERROR) ? TRUE : FALSE;
	}


// make sure the preview window inside our window is as big as the
// dimensions of captured video, or some capture cards won't show a preview.
// (Also, it helps people tell what size video they're capturing)
// We will resize our app's window big enough so that once the status bar
// is positioned at the bottom there will be enough room for the preview
// window to be w x h
//
void DVCamlib::ResizeWindow(int w, int h) {
    RECT rcW, rcC;
    int xExtra, yExtra;
    int cyBorder = GetSystemMetrics(SM_CYBORDER);

    GetWindowRect(prevwindow, &rcW);
    GetClientRect(prevwindow, &rcC);
    xExtra = rcW.right - rcW.left - rcC.right;
	 // + statusGetHeight() add extra stuff like this if we want a pretty frame :)
    yExtra = rcW.bottom - rcW.top - rcC.bottom + cyBorder;
    
    rcC.right = w;
    rcC.bottom = h;
    SetWindowPos(prevwindow, NULL, 0, 0, rcC.right + xExtra,
				rcC.bottom + yExtra, SWP_NOZORDER | SWP_NOMOVE);
    if ((rcC.right + xExtra != rcW.right - rcW.left && w > GetSystemMetrics(SM_CXMIN)) ||
		rcC.bottom + yExtra != rcW.bottom - rcW.top)
	ResizeWindow(w,h);
	}


// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
//
BOOL DVCamlib::InitCapFilters() {
    HRESULT hr;
    BOOL f;
    UINT uIndex = 0;

    gcap.fCCAvail = FALSE;	// assume no closed captioning support

    f = MakeBuilder();
    if (!f) {
	ErrMsg("Cannot instantiate graph builder");
	return FALSE;
    }

	//
	// First, we need a Video Capture filter, and some interfaces
	//

    // !!! There's got to be a way to cache these from building the menu
    // Enumerate all the video devices.  We want #gcap.iVideoDevice
    ICreateDevEnum *pCreateDevEnum;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	return FALSE;
    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
								&pEm, 0);
    pCreateDevEnum->Release();
    if (hr != NOERROR)
	return FALSE;
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    gcap.pVCap = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	// this is the one we want.  Get its name, and instantiate it.
	if ((int)uIndex == gcap.iVideoDevice) {
	    IPropertyBag *pBag;
	    gcap.achFriendlyName[0] = 0;
	    hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	    if(SUCCEEDED(hr)) {
		VARIANT var;
		var.vt = VT_BSTR;
		hr = pBag->Read(L"FriendlyName", &var, NULL);
		if (hr == NOERROR) {
		    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
					gcap.achFriendlyName, 80, NULL, NULL);
		    SysFreeString(var.bstrVal);
		}
		pBag->Release();
	    }
	    hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pVCap);
	    pM->Release();
	    break;
	}
	pM->Release();
	uIndex++;
    }
    pEm->Release();
    if (gcap.pVCap == NULL) {
	ErrMsg("Error %x: Cannot create video capture filter", hr);
	goto InitCapFiltersFail;
    }

    //
    // make a filtergraph, give it to the graph builder and put the video
    // capture filter in the graph
    //

    f = MakeGraph();
    if (!f) {
	ErrMsg("Cannot instantiate filtergraph");
	goto InitCapFiltersFail;
    }
    hr = gcap.pBuilder->SetFiltergraph(gcap.pFg);
    if (hr != NOERROR) {
	ErrMsg("Cannot give graph to builder");
	goto InitCapFiltersFail;
    }

    hr = gcap.pFg->AddFilter(gcap.pVCap, NULL);
    if (hr != NOERROR) {
	ErrMsg("Error %x: Cannot add vidcap to filtergraph", hr);
	goto InitCapFiltersFail;
    }

    // Calling FindInterface below will result in building the upstream
    // section of the capture graph (any WDM TVTuners or Crossbars we might
    // need).

    // we use this interface to get the name of the driver
    // Don't worry if it doesn't work:  This interface may not be available
    // until the pin is connected, or it may not be available at all.
    // (eg: interface may not be available for some DV capture)
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		    &MEDIATYPE_Interleaved,
		    gcap.pVCap, IID_IAMVideoCompression, (void **)&gcap.pVC);
    if (hr != S_OK) {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		    &MEDIATYPE_Video,
		    gcap.pVCap, IID_IAMVideoCompression, (void **)&gcap.pVC);
    }

    // !!! What if this interface isn't supported?
    // we use this interface to set the frame rate and get the capture size
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Interleaved,
			gcap.pVCap, IID_IAMStreamConfig, (void **)&gcap.pVSC);
    if (hr != NOERROR) {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMStreamConfig, (void **)&gcap.pVSC);
        if (hr != NOERROR) {
	    // this means we can't set frame rate (non-DV only)
	    ErrMsg("Error %x: Cannot find VCapture:IAMStreamConfig", hr);
	}
    }

    gcap.fCapAudioIsRelevant = TRUE;

    AM_MEDIA_TYPE *pmt;
    // default capture format
    if (gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK) {
        // DV capture does not use a VIDEOINFOHEADER
	if (pmt->formattype == FORMAT_VideoInfo) {
            // resize our window to the default capture size
            ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
				abs(HEADER(pmt->pbFormat)->biHeight));
	}
	if (pmt->majortype != MEDIATYPE_Video) {
	    // This capture filter captures something other that pure video.
	    // Maybe it's DV or something?  Anyway, chances are we shouldn't
	    // allow capturing audio separately, since our video capture    
	    // filter may have audio combined in it already!
    	    gcap.fCapAudioIsRelevant = FALSE;
    	    gcap.fCapAudio = FALSE;
	}
        DeleteMediaType(pmt);
    }

    // we use this interface to bring up the 3 dialogs
    // NOTE:  Only the VfW capture filter supports this.  This app only brings
    // up dialogs for legacy VfW capture drivers, since only those have dialogs
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, gcap.pVCap,
				IID_IAMVfwCaptureDialogs, (void **)&gcap.pDlg);


    // Use the crossbar class to help us sort out all the possible video inputs
    // The class needs to be given the capture filters ANALOGVIDEO input pin
    {
        IPin        *pP = 0;
        IEnumPins   *pins;
        ULONG        n;
        PIN_INFO     pinInfo;
        BOOL         Found = FALSE;
	IKsPropertySet *pKs;
	GUID guid;
	DWORD dw;
	BOOL fMatch = FALSE;
        
        gcap.pCrossbar = NULL;

        if(SUCCEEDED(gcap.pVCap->EnumPins(&pins))) {            
            while (!Found && (S_OK == pins->Next(1, &pP, &n))) {
                if (S_OK == pP->QueryPinInfo(&pinInfo)) {
                    if (pinInfo.dir == PINDIR_INPUT) {
			
			// is this pin an ANALOGVIDEOIN input pin?
    			if (pP->QueryInterface(IID_IKsPropertySet,
						(void **)&pKs) == S_OK) {
			    if (pKs->Get(AMPROPSETID_Pin,
					AMPROPERTY_PIN_CATEGORY, NULL, 0,
					&guid, sizeof(GUID), &dw) == S_OK) {
	    			if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
					fMatch = TRUE;
			    }
			    pKs->Release();
    			}

			if (fMatch) {
                            gcap.pCrossbar = new CCrossbar (pP);
                            hr = gcap.pCrossbar->GetInputCount
						(&gcap.NumberOfVideoInputs);
                            Found = TRUE;
			}
                    }
                    pinInfo.pFilter->Release();
                }
                pP->Release();
            }
            pins->Release();
        }
    }

    // there's no point making an audio capture filter
    if (gcap.fCapAudioIsRelevant == FALSE)
	goto SkipAudio;

	// create the audio capture filter, even if we are not capturing audio right
	// now, so we have all the filters around all the time.

    //
    // We want an audio capture filter and some interfaces
    //

    // !!! There's got to be a way to cache these from building the menu
    // Enumerate all the audio devices.  We want #gcap.iAudioDevice
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	goto InitCapFiltersFail;
    uIndex = 0;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,
								&pEm, 0);
    pCreateDevEnum->Release();
    if (hr != NOERROR) {
	// there are no audio capture devices. We'll only allow video capture
	gcap.fCapAudio = FALSE;
	goto SkipAudio;
    }
    pEm->Reset();
    gcap.pACap = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	// this is the one we want!
	if ((int)uIndex == gcap.iAudioDevice) {
	    hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pACap);
	    pM->Release();
	    break;
	}
	pM->Release();
	uIndex++;
    }
    pEm->Release();
    if (gcap.pACap == NULL) {
	// there are no audio capture devices. We'll only allow video capture
	gcap.fCapAudio = FALSE;
	ErrMsg("Cannot create audio capture filter");
	goto SkipAudio;
    }

    //
    // put the audio capture filter in the graph
    //

    // We'll need this in the graph to get audio property pages
    hr = gcap.pFg->AddFilter(gcap.pACap, NULL);
    if (hr != NOERROR) {
        ErrMsg("Error %x: Cannot add audcap to filtergraph", hr);
        goto InitCapFiltersFail;
    }

    // Calling FindInterface below will result in building the upstream
    // section of the capture graph (any WDM TVAudio's or Crossbars we might
    // need).

    // !!! What if this interface isn't supported?
    // we use this interface to set the captured wave format
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
			gcap.pACap, IID_IAMStreamConfig, (void **)&gcap.pASC);
    if (hr != NOERROR) {
	    ErrMsg("Cannot find ACapture:IAMStreamConfig");
    }

SkipAudio:

    // Can this filter do closed captioning?
    IPin *pPin;
    hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_VBI,
				NULL, FALSE, 0, &pPin);
    if (hr != S_OK)
        hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_CC,
				NULL, FALSE, 0, &pPin);
    if (hr == S_OK) {
	pPin->Release();
	gcap.fCCAvail = TRUE;
    } else {
	gcap.fCapCC = FALSE;	// can't capture it, then
    }

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    return TRUE;

InitCapFiltersFail:
    FreeCapFilters();
    return FALSE;
	} //end InitCapFilters



// all done with the capture filters and the graph builder
//
void DVCamlib::FreeCapFilters() {
    if (gcap.pFg)
	gcap.pFg->Release();
    gcap.pFg = NULL;
    if (gcap.pBuilder)
	gcap.pBuilder->Release();
    gcap.pBuilder = NULL;
    if (gcap.pVCap)
	gcap.pVCap->Release();
    gcap.pVCap = NULL;
    if (gcap.pACap)
	gcap.pACap->Release();
    gcap.pACap = NULL;
    if (gcap.pASC)
	gcap.pASC->Release();
    gcap.pASC = NULL;
    if (gcap.pVSC)
	gcap.pVSC->Release();
    gcap.pVSC = NULL;
    if (gcap.pVC)
	gcap.pVC->Release();
    gcap.pVC = NULL;
    if (gcap.pDlg)
	gcap.pDlg->Release();
    gcap.pDlg = NULL;
    if (gcap.pCrossbar) {
       delete gcap.pCrossbar;
       gcap.pCrossbar = NULL;
    }
	}  // end freecapfilters


// Tear down everything downstream of a given filter
void DVCamlib::NukeDownstream(IBaseFilter *pf) {
    //DbgLog((LOG_TRACE,1,TEXT("Nuking...")));

    IPin *pP, *pTo;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;
    HRESULT hr = pf->EnumPins(&pins);
    pins->Reset();
    while (hr == NOERROR) {
        hr = pins->Next(1, &pP, &u);
	if (hr == S_OK && pP) {
	    pP->ConnectedTo(&pTo);
	    if (pTo) {
	        hr = pTo->QueryPinInfo(&pininfo);
	        if (hr == NOERROR) {
		    if (pininfo.dir == PINDIR_INPUT) {
		        NukeDownstream(pininfo.pFilter);
		        gcap.pFg->Disconnect(pTo);
		        gcap.pFg->Disconnect(pP);
	                gcap.pFg->RemoveFilter(pininfo.pFilter);
		    }
	            pininfo.pFilter->Release();
		}
		pTo->Release();
	    }
	    pP->Release();
	}
    }
    if (pins)
        pins->Release();
	} //end NukeDownStream

// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
void DVCamlib::TearDownGraph() {
    if (gcap.pSink)
	gcap.pSink->Release();
    gcap.pSink = NULL;
    if (gcap.pConfigAviMux)
	gcap.pConfigAviMux->Release();
    gcap.pConfigAviMux = NULL;
    if (gcap.pRender)
	gcap.pRender->Release();
    gcap.pRender = NULL;
    if (gcap.pVW) {
	// stop drawing in our window, or we may get wierd repaint effects
	gcap.pVW->put_Owner(NULL);
	gcap.pVW->put_Visible(OAFALSE);
	gcap.pVW->Release();
    }
    gcap.pVW = NULL;
    if (gcap.pME)
	gcap.pME->Release();
    gcap.pME = NULL;
    if (gcap.pDF)
	gcap.pDF->Release();
    gcap.pDF = NULL;

    // destroy the graph downstream of our capture filters
    if (gcap.pVCap)
	NukeDownstream(gcap.pVCap);
    if (gcap.pACap)
	NukeDownstream(gcap.pACap);

    // potential debug output - what the graph looks like
    // if (gcap.pFg) DumpGraph(gcap.pFg, 1);

    gcap.fCaptureGraphBuilt = FALSE;
    gcap.fPreviewGraphBuilt = FALSE;
    gcap.fPreviewFaked = FALSE;
	} // end Tear down graph


// build the preview graph!
//
// !!! PLEASE NOTE !!!  Some new WDM devices have totally separate capture
// and preview settings.  An application that wishes to preview and then 
// capture may have to set the preview pin format using IAMStreamConfig on the
// preview pin, and then again on the capture pin to capture with that format.
// In this sample app, there is a separate page to set the settings on the 
// capture pin and one for the preview pin.  To avoid the user
// having to enter the same settings in 2 dialog boxes, an app can have its own
// UI for choosing a format (the possible formats can be enumerated using
// IAMStreamConfig) and then the app can programmatically call IAMStreamConfig
// to set the format on both pins.
//
BOOL DVCamlib::BuildPreviewGraph() {
    int cy, cyBorder;
    HRESULT hr;
    AM_MEDIA_TYPE *pmt;
    BOOL fPreviewUsingCapturePin = FALSE;

    // we have one already
    if (gcap.fPreviewGraphBuilt)
	return TRUE;

    // No rebuilding while we're running
    if (gcap.fCapturing || gcap.fPreviewing)
	return FALSE;

    // We don't have the necessary capture filters
    if (gcap.pVCap == NULL)
	return FALSE;
    if (gcap.pACap == NULL && gcap.fCapAudio)
	return FALSE;

    // we already have another graph built... tear down the old one
    if (gcap.fCaptureGraphBuilt)
	TearDownGraph();

	//
	// Render the preview pin - even if there is not preview pin, the capture
	// graph builder will use a smart tee filter and provide a preview.
	//
	// !!! what about latency/buffer issues?

	// NOTE that we try to render the interleaved pin before the video pin, because
	// if BOTH exist, it's a DV filter and the only way to get the audio is to use
	// the interleaved pin.  Using the Video pin on a DV filter is only useful if
	// you don't want the audio.

    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Interleaved, gcap.pVCap, NULL, NULL);
    if (hr == VFW_S_NOPREVIEWPIN) {
	// preview was faked up for us using the (only) capture pin
	gcap.fPreviewFaked = TRUE;
    } else if (hr != S_OK) {
	// maybe it's DV?
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
        if (hr == VFW_S_NOPREVIEWPIN) {
	    // preview was faked up for us using the (only) capture pin
	    gcap.fPreviewFaked = TRUE;
        } else if (hr != S_OK) {
	    ErrMsg("This graph cannot preview!");
	}
    }

	//
	// Render the closed captioning pin? It could be a CC or a VBI category pin,
	// depending on the capture driver
	//

    if (gcap.fCapCC) {
	hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CC, NULL,
					gcap.pVCap, NULL, NULL);
	if (hr != NOERROR) {
	    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
					gcap.pVCap, NULL, NULL);
	    if (hr != NOERROR) {
	        ErrMsg("Cannot render closed captioning");
	        // so what? goto SetupCaptureFail;
            }
        }
    }

	//
	// Get the preview window to be a child of our app's window
	//

    // This will find the IVideoWindow interface on the renderer.  It is 
    // important to ask the filtergraph for this interface... do NOT use
    // ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
    // know we own the window so it can give us display changed messages, etc.

    hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
    if (hr != NOERROR) {
	ErrMsg("This graph cannot preview properly");
    } else {
	RECT rc;
	gcap.pVW->put_Owner((long)prevwindow);    // We own the window now
	gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child
	// give the preview window all our space but where the status bar is
	GetClientRect(prevwindow, &rc);
	cyBorder = GetSystemMetrics(SM_CYBORDER);
	cy = cyBorder; // No statusGetHeight() +  here
	rc.bottom -= cy;
	gcap.pVW->SetWindowPosition(0, 0, rc.right, rc.bottom); // be this big
	gcap.pVW->put_Visible(OATRUE);
    }

    // now tell it what frame rate to capture at.  Just find the format it
    // is capturing with, and leave everything alone but change the frame rate
    // No big deal if it fails.  It's just for preview
    // !!! Should we then talk to the preview pin?
    if (gcap.pVSC && gcap.fUseFrameRate) {
	hr = gcap.pVSC->GetFormat(&pmt);
	// DV capture does not use a VIDEOINFOHEADER
        if (hr == NOERROR) {
	    if (pmt->formattype == FORMAT_VideoInfo) {
	        VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
	        pvi->AvgTimePerFrame = (LONGLONG)(10000000 / gcap.FrameRate);
	        hr = gcap.pVSC->SetFormat(pmt);
		if (hr != NOERROR)
		    ErrMsg("%x: Cannot set frame rate for preview", hr);
	    }
	    DeleteMediaType(pmt);
	}
    }

	// All done.

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    gcap.fPreviewGraphBuilt = TRUE;
    return TRUE;
	} // end build preview graph


// Start previewing
//
BOOL DVCamlib::StartPreview() {
    BOOL f = TRUE;

    // way ahead of you
    if (gcap.fPreviewing)
	return TRUE;

    if (!gcap.fPreviewGraphBuilt)
	return FALSE;

    // run the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (SUCCEEDED(hr)) {
	hr = pMC->Run();
	if (FAILED(hr)) {
	    // stop parts that ran
	    pMC->Stop();
	}
	pMC->Release();
    }
    if (FAILED(hr)) {
	ErrMsg("Error %x: Cannot run preview graph", hr);
	return FALSE;
    }

    gcap.fPreviewing = TRUE;
    return TRUE;
	} //end start preview



BOOL DVCamlib::StopPreview() {
	IMediaControl *pMC;
	HRESULT hr;

	// way ahead of you
	if (!gcap.fPreviewing) {
		return FALSE;
		}

	// stop the graph
	pMC = NULL;
	hr = gcap.pFg->QueryInterface(IID_IMediaControl,(void **)&pMC);
	if (SUCCEEDED(hr)) {
		hr = pMC->Stop();
		pMC->Release();
		}
	if (FAILED(hr)) {
	ErrMsg("Error %x: Cannot stop preview graph", hr);
		return FALSE;
		}

	gcap.fPreviewing = FALSE;

	// !!! get rid of menu garbage
	InvalidateRect(prevwindow, NULL, TRUE);

	return TRUE;
	} // end stop preview


// Check the devices we're currently using and make filters for them
//
void DVCamlib::ChooseDevices(int idV, int idA) {
    #define VERSIZE 40
    #define DESCSIZE 80
    int versize = VERSIZE;
    int descsize = DESCSIZE;
    WCHAR wachVer[VERSIZE], wachDesc[DESCSIZE];
    char /*achStatus[VERSIZE + DESCSIZE + 5],*/ achDesc[DESCSIZE], achVer[VERSIZE];

/**/
	if (idV != gcap.iVideoDevice) {
		// uncheck the old, check the new
		if (gcap.iVideoDevice >= 0)     // might be uninitialized
			CheckMenuItem(GetMenu(window), MENU_VDEVICE0 + gcap.iVideoDevice,MF_UNCHECKED); 
		CheckMenuItem(GetMenu(window), MENU_VDEVICE0 + idV, MF_CHECKED); 
		}

	if (idA != gcap.iAudioDevice) {
	// uncheck the old, check the new
		if (gcap.iAudioDevice >= 0)     // might be uninitialized
			CheckMenuItem(GetMenu(window), MENU_ADEVICE0 + gcap.iAudioDevice,MF_UNCHECKED); 
		CheckMenuItem(GetMenu(window), MENU_ADEVICE0 + idA, MF_CHECKED); 
		}
/**/
    // they chose a new device. rebuild the graphs
	if (gcap.iVideoDevice != idV || gcap.iAudioDevice != idA) {
		gcap.iVideoDevice = idV;
		gcap.iAudioDevice = idA;
		if (gcap.fPreviewing) StopPreview();
		if (gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt) TearDownGraph();
		FreeCapFilters();
		InitCapFilters();
		if (gcap.fWantPreview) { // were we previewing?
			BuildPreviewGraph();
			StartPreview();
			}
		MakeMenuOptions();	// the UI choices change per device
		}

	gcap.iVideoDevice = idV;
	gcap.iAudioDevice = idA;

	// Put the video driver name in the status bar - if the filter supports
	// IAMVideoCompression::GetInfo, that's the best way to get the name and
	// the version.  Otherwise use the name we got from device enumeration
	// as a fallback.
	if (gcap.pVC) {
		HRESULT hr =gcap.pVC->GetInfo(wachVer,&versize,wachDesc,&descsize,NULL,NULL,NULL,NULL);
		if (hr == S_OK) {
			WideCharToMultiByte(CP_ACP,0,wachVer,-1,achVer,VERSIZE,NULL,NULL);
			WideCharToMultiByte(CP_ACP,0,wachDesc,-1,achDesc,DESCSIZE,NULL,NULL);
			//wsprintf(achStatus,"%s - %s",achDesc,achVer);
			//if (debug) printc(achStatus);printc(gcap.achFriendlyName);
			return;
			}
		}
	} //end choose device


// put all installed video and audio devices in the menus
//
void DVCamlib::AddDevicesToMenu()
{
    UINT    uIndex = 0;
    HMENU   hMenuSub;
    HRESULT hr;

    hMenuSub = GetSubMenu(GetMenu(window),0);        // Devices menu

    // remove the bogus separator
    RemoveMenu(hMenuSub, 0, 0);

    // enumerate all video capture devices
    ICreateDevEnum *pCreateDevEnum;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	goto EnumAudio;
    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
								&pEm, 0);
    pCreateDevEnum->Release();
    if (hr != NOERROR) {
	ErrMsg("Sorry, you have no video capture hardware");
	goto EnumAudio;
    }
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	IPropertyBag *pBag;
	hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if(SUCCEEDED(hr)) {
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
		char achName[80];
		WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achName, 80,
								NULL, NULL);
		AppendMenuA(hMenuSub, MF_STRING, MENU_VDEVICE0 + uIndex,
								achName);
		SysFreeString(var.bstrVal);
	    }
	    pBag->Release();
	}
	pM->Release();
	uIndex++;
    }
    pEm->Release();

    // separate the video and audio devices
    AppendMenuA(hMenuSub, MF_SEPARATOR, 0, NULL);

EnumAudio:

    // enumerate all audio capture devices
    uIndex = 0;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	return;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,
								&pEm, 0);
    pCreateDevEnum->Release();
    if (hr != NOERROR)
	return;
    pEm->Reset();
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	IPropertyBag *pBag;
	hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if(SUCCEEDED(hr)) {
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
		char achName[80];
		WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achName, 80,
								NULL, NULL);
		AppendMenuA(hMenuSub, MF_STRING, MENU_ADEVICE0 + uIndex,
								achName);
		SysFreeString(var.bstrVal);
	    }
	    pBag->Release();
	}
	pM->Release();
	uIndex++;
    }
    pEm->Release();
}


DVCamlib::DVCamlib() {
	CoInitialize(NULL);
	DbgInitialise(hInst);
	}

BOOL DVCamlib::startup(HWND prevwindowparm,HWND windowparm,HWND windowparm2,char *filepathparm,char *filenameparm,BOOL preview) {
	int idV,idA;
	//Note with these com functions this class must only be one instance
	memset(&gcap,0,sizeof(struct _capstuff));
	//TODO get some of these members defaults from config file
	prevwindow=prevwindowparm;
	window=windowparm;
	window2=windowparm2;
	filepath=filepathparm;
	filename=filenameparm;

	gcap.iVideoDevice = -1;     // force update
	gcap.iAudioDevice = -1;     // force update
	idV=0; //get these
	idA=0;

	AddDevicesToMenu(); // list all capture devices in the system as menu items
	//We may want to use this in our main startup

	// do we want audio?
	gcap.fCapAudio=0; //these too maybe
	gcap.fCapCC=0;

	// do we want preview?
	gcap.fWantPreview = preview;
	// which stream should be the master? NONE(-1) means nothing special happens
	// AUDIO(1) means the video frame rate is changed before written out to keep
	// the movie in sync when the audio and video capture crystals are not the
	// same (and therefore drift out of sync after a few minutes).  VIDEO(0)
	// means the audio sample rate is changed before written out
	gcap.iMasterStream = 1; //shouldn't need to change this

	// get the frame rate from win.ini before making the graph
	gcap.fUseFrameRate = 1;
	int units_per_frame = 666667;  // 15fps note winini used 667111
	gcap.FrameRate = 10000000. / units_per_frame;
	gcap.FrameRate = (int)(gcap.FrameRate * 100) / 100.;
	// reasonable default
	if (gcap.FrameRate <= 0.) gcap.FrameRate = 15.0;
	 
    gcap.fUseTimeLimit = 0;  //hopefully get rid of this!
    gcap.dwTimeLimit = 10; //Temp 10 seconds

    // instantiate the capture filters we need to do the menu items
    // this will start previewing, if wanted
    ChooseDevices(idV, idA);    // make these the official devices we're using
	// and builds a partial filtergraph.
	return(TRUE);
	} //end startup

BOOL DVCamlib::startcapture() {
	    if (gcap.fPreviewing)
		StopPreview();
	    if (gcap.fPreviewGraphBuilt)
		TearDownGraph();
	    BuildCaptureGraph();
	    StartCapture();
		 return(TRUE);
	}

void DVCamlib::stopcapture() {
	    StopCapture();
	    if (gcap.fWantPreview) {
		BuildPreviewGraph();
		StartPreview();
	    }
	}

void DVCamlib::shutdown() {
    StopPreview();
    //StopCapture();
    TearDownGraph();
    FreeCapFilters();
    // store current settings in win.ini for next time
	}

DVCamlib::~DVCamlib() {
    DbgTerminate();
    CoUninitialize();
	}
