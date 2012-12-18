#include "StdAfx.h"

//*****************************************************************************************************************************************************
void VideoPreviewWindow::ClearCache(void)
{	LastFrameRendered[0]=LastFrameRendered[1]=
	LastVideoFrameRendered=FLT_MAX;
	LastReversed[0]=LastReversed[1]=false;
}

bool VideoPreviewWindow::RefreshNeeded(void)
{	if ((InOutPointSelect==VideoPreviewWindow_Out)||
		(InOutPointSelect==VideoPreviewWindow_In))
	{	// Have any of the video settings changed ?
		if (AVFlags_View&StoryBoardFlag_Video)
		{	// We are about to draw the Discreet style bar
			float f_ValueToUse	=GetFloat(InOutPointSelect,StoryBoard_Video);
			if (LastFrameRendered[0]!=f_ValueToUse) return true;

			// Is the clip reversed ?
			bool Reversed=(GetFloat(0,StoryBoard_Video)>GetFloat(1,StoryBoard_Video));
			if (LastReversed[0]!=Reversed) return true;
		}

		// Have any of the audio settings changed ?
		if (AVFlags_View&StoryBoardFlag_Audio)
		{	// We are about to draw the Discreet style bar
			float f_ValueToUse=GetFloat(InOutPointSelect,StoryBoard_Audio);
			if (LastFrameRendered[1]!=f_ValueToUse) return true;

			// Is the clip reversed ?
			bool Reversed=(GetFloat(0,StoryBoard_Audio)>GetFloat(1,StoryBoard_Audio));
			if (LastReversed[1]!=Reversed) return true;
		}
	}
	// Crouton view !
	else
	{	// Get the view to use, and get the corresponding value
		int CroutonToView=(!(AVFlags_View&StoryBoardFlag_Video))?StoryBoard_Audio:StoryBoard_Video;
		float f_ValueToUse=GetFloat(VideoPreviewWindow_Crouton,CroutonToView); 
		if (LastFrameRendered[0]!=f_ValueToUse) return true;
	}

	// Nothing has changed
	return false;
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::SetWindowToEdit(StoryBoard_Crouton	*sc)
{	// Remove any previous dependants
	if (SC) 
	{	for(unsigned i=StoryBoard_Video;i<=StoryBoard_Audio;i++)
		{	SC->InPoint[i].DeleteDependant(this);
			SC->OutPoint[i].DeleteDependant(this);
			SC->CroutonPoint[i].DeleteDependant(this);				
		}
		SC->DeleteDependant(this);
	}
		
	SC=sc;

	// Add any new dependants
	if (SC)
	{	SC->AddDependant(this);

		for(unsigned i=StoryBoard_Video;i<=StoryBoard_Audio;i++)
		{	SC->InPoint[i].AddDependant(this);
			SC->OutPoint[i].AddDependant(this);
			SC->CroutonPoint[i].AddDependant(this);				
		}

		// Refresh stuff
		ClearCache();
		RePaint();
	}	
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::SetEditingPosition(unsigned InOut,unsigned View_AVFlags)
{	InOutPointSelect=InOut;
	AVFlags_View=View_AVFlags;
	ClearCache();
	RePaint();
}

//*****************************************************************************************************************************************************
float VideoPreviewWindow::GetFloat(unsigned InOutPoint,unsigned AVFlag)
{	if (!SC) return 0.0;
	else if (InOutPoint==VideoPreviewWindow_In)			return SC->InPoint		[AVFlag];
	else if (InOutPoint==VideoPreviewWindow_Out)		return SC->OutPoint		[AVFlag];
	else if (InOutPoint==VideoPreviewWindow_Crouton)	return SC->CroutonPoint	[AVFlag];
	else return 0.0;
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::ChangedFloat(unsigned InOutPoint,unsigned AVFlag,char *String)
{	if (!SC) return;
	else if (InOutPoint==VideoPreviewWindow_In)				SC->InPoint		[AVFlag].Changed(String);
	else if (InOutPoint==VideoPreviewWindow_Out)		SC->OutPoint	[AVFlag].Changed(String);
	else if (InOutPoint==VideoPreviewWindow_Crouton)	SC->CroutonPoint[AVFlag].Changed(String);
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::IncClipped(float Val,char *String)
{	// Send the message
	Changed(String,(double)Val);
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::DecClipped(float Val,char *String)
{	IncClipped(-Val,String);
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::MouseMoved(long Flags,long x,long y)
{	// Set the cursor ...

	// If we are not dragging
	if (!Moved)
	{	// Which side are we ?
		RECT rect;
		GetClientRect(GetWindowHandle(),&rect);
		if (x>rect.right/2)
				::SetCursor(LoadCursor(h_sb_HINSTANCE,MAKEINTRESOURCE(IDC_RIGHTCURSOR)));
		else	::SetCursor(LoadCursor(h_sb_HINSTANCE,MAKEINTRESOURCE(IDC_LEFTCURSOR)));
	}
	else
	{	
	}
	
	// If we are not tracking the mouse
	if (!Clicked) return;

	// How far has the mouse moved ?
	POINT pt; GetCursorPos(&pt);
	if (abs(pt.x-PtClicked.x) >= VideoPreviewWindow_DistanceBeforDrag)
	{	if (!Moved) ShowCursor(false);
		Moved=true;		
	}
	
	if (Moved)
	{	// Move back to the original position
		SetCursorPos(PtClicked.x,PtClicked.y);	

		// Handle the change
		IncClipped(pt.x-PtClicked.x,Controls_Slider_Slide);	
	}
}

void VideoPreviewWindow::MouseLButtonClick(long Flags,long x,long y)
{	// Capture the mouse
	ExclusiveInput(true);
	Clicked=true;
	Moved=false;

	// Get the position, then hide the mouse
	GetCursorPos(&PtClicked);	

	// Send the relevant messages
	Changed(Controls_Slider_Clicked);
}

void VideoPreviewWindow::MouseLButtonDblClick(long Flags,long x,long y)
{	// Just look like another click
	MouseLButtonClick(Flags,x,y);
}

void VideoPreviewWindow::MouseLButtonRelease(long Flags,long x,long y)
{	// Release the mouse
	ExclusiveInput(false);
	if (!Clicked) return;

	// Have we not moved ?
	if (!Moved)
	{	// Which side are we ?
		RECT rect;
		GetClientRect(GetWindowHandle(),&rect);

		// Handle the change
		if (x>rect.right/2) IncClipped( 1,Controls_Slider_PageUp);
		else				IncClipped(-1,Controls_Slider_PageUp);		
	}
	// Show the cursor again
	else ShowCursor(true);
	
	// Reset stuff !
	Clicked=false;	
	Moved=false;	

	// Send the relevant messages
	Changed(Controls_Slider_Released);
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	// Filter out windows messages
	if (IsWindowMessage(String)) return;

	// Handle deletion
	if (IsDeletion(String)) 
	{	void *Args=args;
		DynamicTalker *DT=NewTek_GetArguement<DynamicTalker*>(Args);
		if (DT==SC) 
						SC=NULL;
		return;
	}	

	// Handle the local messages
	if (RefreshNeeded()) RePaint();
}

//*****************************************************************************************************************************************************
// Restore any lost surfaces
void VideoPreviewWindow::RestoreSurfaces(void)
{	if ((m_pYUVSurface)&&(m_pYUVSurface->IsLost()==DDERR_SURFACELOST)) 
	{	m_pYUVSurface->Restore();
		BlankVideoWindow();
	}

	if ((m_pPrimary)&&(m_pPrimary->IsLost()==DDERR_SURFACELOST)) 
	{	m_pPrimary->Restore();
	}
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::BlankVideoWindow(long Brightness)
{	// If there is no DDraw window, we cannot do anything
	if (!m_pYUVSurface) return;

	// Clamp the brightness
	Brightness=max(min(Brightness,255),0);
	
	// Now handle the DDraw stuff
	DDSURFACEDESC	Desc;
	Desc.dwSize=sizeof(Desc);
	if (m_pYUVSurface->Lock(NULL,&Desc,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)==DD_OK)
	{	// Get the color
		const unsigned YUVBlack=(128<<24)+(Brightness<<16)+(128<<8)+Brightness;
		
		// I am using YUY2 FourCC ?
		unsigned *Scan=(unsigned *)Desc.lpSurface;
		const unsigned *ScanEnd=Scan+Desc.lPitch*Desc.dwHeight/4;

		while(Scan<ScanEnd) *(Scan++)=YUVBlack;

		// Unlock the surface
		m_pYUVSurface->Unlock(NULL);
	}	
}

//*****************************************************************************************************************************************************
// Default video-frame rendering
void VideoPreviewWindow::RenderVideoFrame(	float Time,byte *Memory,
											unsigned XRes,unsigned YRes,
											unsigned XWidth)
{	BlankVideoWindow();
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::PaintWindow(HWND hWnd,HDC hdc)
{	//If we have nothing to view yet, do not bother drawing !
	if (!SC) return;
	
	// Restore the surfaces
	RestoreSurfaces();

	// Get the window size
	RECT rect;
	GetClientRect(hWnd,&rect);
	
	// If there is no DDraw window, we cannot do anything
	if ((m_pPrimary)&&(m_pYUVSurface))
	{	// Get the window size
		RECT Rect;
		GetWindowRect(hWnd,&Rect);

		// We might even be displaying audio !
		int CroutonToView=(!(AVFlags_View&StoryBoardFlag_Video))?StoryBoard_Audio:StoryBoard_Video;

		// We now render the frame
		if (LastVideoFrameRendered!=GetFloat(InOutPointSelect,CroutonToView))
		{	// Render the frame for the user
			
			// Now handle the DDraw stuff
			DDSURFACEDESC	Desc;
			Desc.dwSize=sizeof(Desc);
			if (m_pYUVSurface->Lock(NULL,&Desc,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)==DD_OK)
			{	// Render the frame
				RenderVideoFrame(	GetFloat(InOutPointSelect,CroutonToView),
									(byte*)Desc.lpSurface,rect.right,rect.bottom,Desc.lPitch/2);

				// Unlock the surface
				m_pYUVSurface->Unlock(NULL);
			}

			// We now have the correct frame cached
			LastVideoFrameRendered=GetFloat(InOutPointSelect,CroutonToView);
		}

		// BitBlt
		//if (m_pPrimary->Blt(&Rect,m_pYUVSurface,NULL,DDBLT_WAIT,NULL)!=DD_OK)
		{	}
	}	

	// We add a constant to the YPosition each time
	unsigned YPosn=0;

	if ((InOutPointSelect==VideoPreviewWindow_Out)||
		(InOutPointSelect==VideoPreviewWindow_In))
	{	//**** Display the Video *********************************************************************************88
		if (AVFlags_View&StoryBoardFlag_Video)
		{	// We are about to draw the Discreet style bar
			float f_ValueToUse	=GetFloat(InOutPointSelect,StoryBoard_Video);
			LastFrameRendered[0]=f_ValueToUse;
			float f_MinToUse	=SC->Zero.Get();
			float f_MaxToUse	=SC->TotalClipLength[StoryBoard_Video].Get();
			float XPosn			=(float)rect.right*(f_ValueToUse-f_MinToUse)/(f_MaxToUse-f_MinToUse);

			// Is the clip reversed ?
			bool Reversed=(GetFloat(0,StoryBoard_Video)>GetFloat(1,StoryBoard_Video));
			LastReversed[0]=Reversed;

			// Are we drawing the in our out point ?
			if (InOutPointSelect==VideoPreviewWindow_In)
					NewTek_DrawRectangle(	hdc,0,YPosn,
											XPosn,VideoPreviewWindow_BarHeight+YPosn,
											Reversed?VideoPreviewWindow_InOut_2_Video:VideoPreviewWindow_OutIn_1_Video);
			else	NewTek_DrawRectangle(	hdc,XPosn,YPosn,
											rect.right,VideoPreviewWindow_BarHeight+YPosn,
											Reversed?VideoPreviewWindow_InOut_2_Video:VideoPreviewWindow_OutIn_1_Video);

			// Increment the position
			YPosn+=StoryBoard_Crouton_AudioOffSet;
		} else LastFrameRendered[0]=FLT_MAX,LastReversed[0]=false;

		//**** Display the audio *********************************************************************************
		if (AVFlags_View&StoryBoardFlag_Audio)
		{	// We are about to draw the Discreet style bar
			float f_ValueToUse	=GetFloat(InOutPointSelect,StoryBoard_Audio);
			LastFrameRendered[1]=f_ValueToUse;
			float f_MinToUse	=SC->Zero.Get();
			float f_MaxToUse	=SC->TotalClipLength[StoryBoard_Audio].Get();
			float XPosn			=(float)rect.right*(f_ValueToUse-f_MinToUse)/(f_MaxToUse-f_MinToUse);

			// Is the clip reversed ?
			bool Reversed=(GetFloat(0,StoryBoard_Audio)>GetFloat(1,StoryBoard_Audio));
			LastReversed[1]=Reversed;

			// Are we drawing the in our out point ?
			if (InOutPointSelect==VideoPreviewWindow_In)
					NewTek_DrawRectangle(	hdc,0,YPosn,
											XPosn,VideoPreviewWindow_BarHeight+YPosn,
											Reversed?VideoPreviewWindow_InOut_2_Audio:VideoPreviewWindow_OutIn_1_Audio);
			else	NewTek_DrawRectangle(	hdc,XPosn,YPosn,
											rect.right,VideoPreviewWindow_BarHeight+YPosn,
											Reversed?VideoPreviewWindow_InOut_2_Audio:VideoPreviewWindow_OutIn_1_Audio);

			// Increment the position
			YPosn+=StoryBoard_Crouton_AudioOffSet;
		} else LastFrameRendered[1]=FLT_MAX,LastReversed[1]=false;
	}
	//**** Display the Crouton *********************************************************************************
	else if (InOutPointSelect==VideoPreviewWindow_Crouton)	
	{	// We are about to draw the Discreet style bar
		int CroutonToView=(!(AVFlags_View&StoryBoardFlag_Video))?StoryBoard_Audio:StoryBoard_Video;
		float f_ValueToUse	=GetFloat(VideoPreviewWindow_Crouton,CroutonToView); 
		LastFrameRendered[0]=LastFrameRendered[1]=f_ValueToUse;

		float f_MinToUse	=SC->Zero.Get();
		float f_MaxToUse	=SC->TotalClipLength[CroutonToView].Get();
		float XPosn			=(float)rect.right*(f_ValueToUse-f_MinToUse)/(f_MaxToUse-f_MinToUse);

		// Draw the bar
		NewTek_DrawRectangle(hdc,0,YPosn,XPosn,VideoPreviewWindow_BarHeight+YPosn,VideoPreviewWindow_Crouton_Col);
		
		// Increment the position
		YPosn+=StoryBoard_Crouton_AudioOffSet;

		// NO reversal
		LastReversed[0]=LastReversed[1]=false;
	}
}

//*****************************************************************************************************************************************************
VideoPreviewWindow::VideoPreviewWindow(void)
{	// we are not editing anything yet
	SC=NULL;

	// DirectDraw stuff
	m_pClipper=NULL;
	m_pPrimary=NULL;
	m_pDD=NULL;
	m_pYUVSurface=NULL;

	// We are editing the in point by default
	InOutPointSelect=VideoPreviewWindow_In;

	// The flags
	AVFlags_View=StoryBoardFlag_Audio|StoryBoardFlag_Video;
}

//*****************************************************************************************************************************************************
VideoPreviewWindow::~VideoPreviewWindow(void)
{	SetWindowToEdit(NULL);
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::InitialiseWindow(void)
{	// For safety sake, we do not have anything created yet
	m_pClipper=NULL;
	m_pPrimary=NULL;
	m_pDD=NULL;
	m_pYUVSurface=NULL;
	Clicked=false;
	Moved=false;
	SC=NULL;
	
	// Create main DirectDraw object
	if (DirectDrawCreate(NULL,&m_pDD,NULL)!=DD_OK) 
	{	m_pDD=NULL;
		return;
	}

	if (m_pDD->SetCooperativeLevel(m_hWnd,DDSCL_NORMAL) != DD_OK) 
	{	m_pDD->Release();
		m_pDD=NULL;
		return;
	}

	// Create the primary buffer
	DDSURFACEDESC 		ddsd;
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize			= sizeof(ddsd);
    ddsd.dwFlags		= DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (m_pDD->CreateSurface(&ddsd,&m_pPrimary,NULL) != DD_OK)
	{	m_pDD->Release(); m_pDD=NULL;
		return;
	}

	// Create the clipper
	if (m_pDD->CreateClipper(0,&m_pClipper,NULL)!=DD_OK)
	{	m_pPrimary->Release(); m_pPrimary=NULL;
		m_pDD->Release(); m_pDD=NULL;
		return;
	}

	// Assign my Window's HWND to the clipper
	if (m_pClipper->SetHWnd(0,m_hWnd)!=DD_OK)
	{	m_pClipper->Release(); m_pClipper=NULL;
		m_pPrimary->Release(); m_pPrimary=NULL;
		m_pDD->Release(); m_pDD=NULL;
		return;
	}

	// Attach the clipper to the primary surface
	if (m_pPrimary->SetClipper(m_pClipper) != DD_OK)
	{	m_pClipper->Release(); m_pClipper=NULL;
		m_pPrimary->Release(); m_pPrimary=NULL;
		m_pDD->Release(); m_pDD=NULL;
		return;
	}

	// open the YUY2 surface
	ddsd.dwFlags						= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps					= DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth						= GetWindowWidth();
	ddsd.dwHeight						= GetWindowHeight(); 
	ddsd.ddpfPixelFormat.dwSize			= sizeof(ddsd.ddpfPixelFormat);
	ddsd.ddpfPixelFormat.dwFlags		= DDPF_FOURCC;
	ddsd.ddpfPixelFormat.dwFourCC		= ('2'<<24)+('Y'<<16)+('U'<<8)+'Y';

	if(m_pDD->CreateSurface(&ddsd, &m_pYUVSurface, NULL)!= DD_OK)
	{	m_pClipper->Release(); m_pClipper=NULL;
		m_pPrimary->Release(); m_pPrimary=NULL;
		m_pDD->Release(); m_pDD=NULL;
		return;
	}

	// We start off with a blank window
	ClearCache();
	RePaint();
}

//*****************************************************************************************************************************************************
void VideoPreviewWindow::DestroyWindow(void)
{	// We do not want to be dependant on anything
	SetWindowToEdit(NULL);
	
	// Release the surfaces
	if (m_pYUVSurface)	m_pYUVSurface->Release();
	if (m_pClipper)		m_pClipper->Release();
	if (m_pPrimary)		m_pPrimary->Release();	
	if (m_pDD)			m_pDD->Release();	

	// Clear up the pointers
	m_pClipper=NULL;
	m_pPrimary=NULL;
	m_pDD=NULL;
	m_pYUVSurface=NULL;

	BaseWindowClass::DestroyWindow();
}