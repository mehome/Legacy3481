#ifndef __StoryBoard_Crouton__
#define __StoryBoard_Crouton__

//********************************************************************************************************************
// A predecleration
class InOuts_Control;
class VideoPreviewWindow;
class StoryBoard;

//********************************************************************************************************************
#define StoryBoard_Crouton_FolderIcon	"File Icons\\StoryBoard_Folder.tga"
#define StoryBoard_Crouton_NoFileIcon	"File Icons\\VideoToasterLogo.jpg"

// This is how wide to either side of me the range in which icons can be dropped into me is
#define StoryBoard_Crouton_DragWidth	8

// This is the height of the bar used to display the in and out points
#define StoryBoard_Crouton_InOutHeight	5
#define StoryBoard_Crouton_AudioOffSet	3

//********************************************************************************************************************
#define StoryBoard_Crouton_Deferred_Menu			0

#define StoryBoard_Crouton_DoNotUpdateCoeffs "StoryBoard_Crouton::RecomputeDuration"

//********************************************************************************************************************
class SBDLL StoryBoard_Crouton : public FileButton
{	protected:		// I must always have a storyboard tree below me
					StoryBoard_Item				*m_SubTree;

					// When we are operating as a in/out point editor, we need to 
					// track mouse clicks
					bool		Clicked;
					POINT		pt;
					bool		ModifyingInPoint;					

					//**********************************************************************************************
					// The last set of in and out points painted
					float LastIn[2],LastOut[2];

					// These are dynamic members that I use for updates
					Dynamic<int>		Flags;
					Dynamic<float>		TotalClipLength[2];					
					Dynamic<float>		NveTotalClipLength[2];
					Dynamic<float>		Zero;
					Dynamic<float>		StretchDuration[2];
					Dynamic<float>		OriginalFrameRate[2];
					DynamicRange<float>	InPoint[2];
					DynamicRange<float>	OutPoint[2];
					DynamicRange<float>	CroutonPoint[2];
					DynamicRange<float>	OffSetPoint[2];

					// This is the hard part of setting the framerate
					void SetDuration(float Frames,unsigned Flags);
					float GetDuration(unsigned AudioOrVideo);

					// This recomuptes the duration of the clip
					// It works on both the audio and the video elements
					void RecomputeDuration(void);

					// This is the alias name
					TextItem		AliasName;

					// The window pointer for the in and out points, etc...
					InOuts_Control	*InOuts;

					// Copy to undo buffer as necessary
					bool CopyToUndoBuffer(void);

	public:			// Constructor and Destructor
					StoryBoard_Crouton(void);
					virtual ~StoryBoard_Crouton(void);

					// Get and set the storyboard tree
					void SetStoryboardTree(StoryBoard_Item *Tree=NULL,bool ObserverSelected=false);
					StoryBoard_Item *GetStoryboardTree(void);

					// This is so that I can track parent changes
					virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

					// Get the parent storyboard of this crouton, if there is one
					StoryBoard *GetParentStoryboard(void);

					// Overload thh changed selection status
					virtual	 void DragAndDrop_Select(bool Flag);

					// Load the default crouton
					static WindowBitmap LocalCroutonBitmap;
					static void StoryBoard_Crouton_LocalCroutonBitmap(void);

					// Am I a folder ?
					bool AmIFolder(void);

					// My parent has changed
					virtual void ParentChanged(HWND From,HWND To);

					// Drag 'n Drop overloading !
					bool DragNDrop_ShouldIDragAndDrop(HWND hWnd);

					// Mouse functions
					virtual void MouseLButtonClick(long Flags,long x,long y);
					virtual void MouseMoved(long Flags,long x,long y);
					virtual void MouseLButtonRelease(long Flags,long x,long y);
					virtual void MouseRButtonClick(long Flags,long x,long y);

					// Initialise window
					virtual void InitialiseWindow(void);
					virtual void DestroyWindow(void);

					// Overloaded window drawing
					virtual void PaintWindow(HWND hWnd,HDC hdc);

					// Funky DragNDrop stuff
					virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
					virtual WindowLayout_Item *DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									);					

					// Deferred messages
					virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

					// Get the GUID of the selected item
					GUID GetGUIDOfTree(void);

					// Since the in/out point window is a direct editor of my information, it saves a lot of code
					// to allow it to access my private members directly.
					friend InOuts_Control;
					friend VideoPreviewWindow;

					////////////////////////////////////////////////
					// We don't want mouse over animations
					virtual bool FileButton_ShouldIDisplayAnimations(void){return false;}
};

//********************************************************************************************************************
// The SB crouton, without ownership !
class SBDLL StoryBoard_Crouton_NoOwn : public StoryBoard_Crouton
{	public:			virtual ~StoryBoard_Crouton_NoOwn(void);
};

#endif