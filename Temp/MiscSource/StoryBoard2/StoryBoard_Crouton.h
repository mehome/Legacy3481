#ifndef __StoryBoard_Crouton__
#define __StoryBoard_Crouton__

#define StoryBoard_Crouton_FolderIcon	"File Icons\\StoryBoard_Folder.tga"
#define StoryBoard_Crouton_NoFileIcon	"File Icons\\VideoToasterLogo.jpg"

// This is how wide to either side of me the range in which icons can be dropped into me is
#define StoryBoard_Crouton_DragWidth	8

// This is the height of the bar used to display the in and out points
//#define StoryBoard_Crouton_InOutHeight	5
//#define StoryBoard_Crouton_AudioOffSet	3

class SB2DLL StoryBoard2_Crouton : public FileButton {
	public:			// Constructor and Destructor
		StoryBoard2_Crouton(void);
		virtual ~StoryBoard2_Crouton(void);

		// This is so that I can track parent changes
		virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

		// Overload thh changed selection status
		virtual	 void DragAndDrop_Select(bool Flag);

		// Load the default crouton
		static WindowBitmap LocalCroutonBitmap;
		static void StoryBoard_Crouton_LocalCroutonBitmap(void);

		// Am I a folder ?
		bool AmIFolder(void);

		// Drag 'n Drop overloading !
		bool DragNDrop_ShouldIDragAndDrop(HWND hWnd);

		// Mouse functions
		virtual void MouseLButtonClick(long Flags,long x,long y);
		virtual void MouseMoved(long Flags,long x,long y);
		virtual void MouseLButtonRelease(long Flags,long x,long y);
	//	virtual void MouseRButtonClick(long Flags,long x,long y);

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
		////////////////////////////////////////////////
		// We don't want mouse over animations
		virtual bool FileButton_ShouldIDisplayAnimations(void){return false;}

		TimeLineElement *GetTLE() {return m_TimeLineElement;}
		void SetTLE(TimeLineElement *tle) {m_TimeLineElement=tle;}

	protected:
		TimeLineElement *m_TimeLineElement;
	};

#endif