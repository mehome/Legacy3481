#ifndef __VIDEOPREVIEWWINDOW__
#define __VIDEOPREVIEWWINDOW__

//*****************************************************************************************************************
#define VideoPreviewWindow_BarHeight			5
#define VideoPreviewWindow_DistanceBeforDrag	3
//#define VideoPreviewWindow_AudioOffset			3

#define VideoPreviewWindow_In		0
#define VideoPreviewWindow_Out		1
#define VideoPreviewWindow_Crouton	2

//*****************************************************************************************************************
// These are the colors used to draw the bars representing positions in the clip.
// They are also used by the StoryBoard_Crouton view.
#define VideoPreviewWindow_OutIn_1_Video	RGB(255,249,157)
#define VideoPreviewWindow_InOut_2_Video	RGB(251,200,200)

#define VideoPreviewWindow_OutIn_1_Audio	RGB(157,249,157)
#define VideoPreviewWindow_InOut_2_Audio	RGB(200,200,200)

#define VideoPreviewWindow_Crouton_Col		RGB(157,157,249)

//*****************************************************************************************************************
class SBDLL VideoPreviewWindow : public BaseWindowClass, public DynamicListener
{	private:		//*****************************************************************************************************************
					// Direct draw variables
					IDirectDraw			*m_pDD;
					IDirectDrawSurface	*m_pPrimary;
					IDirectDrawSurface	*m_pYUVSurface;
					IDirectDrawClipper	*m_pClipper;

					// Restore any lost surfaces
					void RestoreSurfaces(void);

					//*****************************************************************************************************************
					// Blank the video window
					void BlankVideoWindow(long Brightness=16);

					//*****************************************************************************************************************
					// Information about what to use in the viewport as the current positions
					float				LastFrameRendered[2];
					bool				LastReversed[2];
					float				LastVideoFrameRendered;

					// Clear Cache
					void				ClearCache(void);
					bool				RefreshNeeded(void);

					//*****************************************************************************************************************
					// What are we displaying ?
					unsigned			AVFlags_View;
					unsigned			InOutPointSelect;

					//*****************************************************************************************************************
					// Is the mouse button clicked ?
					bool				Clicked,Moved;
					POINT				PtClicked;					

					//*****************************************************************************************************************
					// This is the pointer to the value to edit
					StoryBoard_Crouton	*SC;

					//*****************************************************************************************************************
					// Set a clipped value
					void IncClipped(float Val,char *String="Changed");
					void DecClipped(float Val,char *String="Changed");

					//*****************************************************************************************************************
					// Get the dynamic float for a particular operation
					float GetFloat(unsigned InOutPoint,unsigned AVFlag);
					float GetClipLength(unsigned AVFlag);
					void  ChangedFloat(unsigned InOutPoint,unsigned AVFlag,char *String="Changed");

					//*****************************************************************************************************************
					// Overload this to provide your own YUV image data
					virtual void RenderVideoFrame(	float Time,byte *Memory,
													unsigned XRes,unsigned YRes,
													unsigned XWidth);					

	public:			// Constructor, Destructor
					VideoPreviewWindow(void);
					~VideoPreviewWindow(void);

					// Initialize and destroy window
					virtual void InitialiseWindow(void);
					virtual void DestroyWindow(void);					

					// Mouse movements
					virtual void MouseMoved(long Flags,long x,long y);
					virtual void MouseLButtonClick(long Flags,long x,long y);
					virtual void MouseLButtonDblClick(long Flags,long x,long y);
					virtual void MouseLButtonRelease(long Flags,long x,long y);

					// Are we editing the in out the out point
					void SetEditingPosition(	unsigned InOut=VideoPreviewWindow_In,
												unsigned View_AVFlags=StoryBoardFlag_Audio|StoryBoardFlag_Video);

					// Window painting
					virtual void PaintWindow(HWND hWnd,HDC hdc);

					// My dynamic callback
					virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

					// Set the item to edit
					void SetWindowToEdit(StoryBoard_Crouton	*sc);
};

#endif