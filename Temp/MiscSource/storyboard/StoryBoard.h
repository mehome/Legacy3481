#ifndef __StoryBoard__
#define __StoryBoard__

//********************************************************************************************************************
void SBDLL StoryBoard_Copy(StoryBoard_Item			*ItemToCopy);
void SBDLL StoryBoard_Copy(tList<StoryBoard_Item*>	*ItemsToCopy);
StoryBoard_Item SBDLL *StoryBoard_Paste(void);
void SBDLL StoryBoard_Init(void);
void SBDLL StoryBoard_Free(void);

//********************************************************************************************************************
// Forward declerations
class StoryBoardPath;

//********************************************************************************************************************
// This class is used to store selections across an undo/redo operation
class SBDLL StoryBoard_SelectionSave
{	public:		// The window that we are doing the selection for
				InOuts_Control	*WindowOfInterest;

				// The GUIDs of the items in this window
				tList<GUID>		GUIDs;
};

//********************************************************************************************************************
class SBDLL StoryBoard :	public StretchySkinControl, 
							public StoryBoard_Item
{	protected:	// The skin
				ScreenObject_StretchySkin	*m_ScreenObject_StretchySkin;

				// The button variable
				Dynamic<int>	m_Buttons;
				Dynamic<int>	m_PlayState;
				Dynamic<int>	m_exitInt;

				Dynamic<int>	m_EditState;

				// Scroller bitmaps
				tList<ScreenObject_BitmapFile*>	m_sliderBitmaps;

				// The scrollbar stuff
				Dynamic<float>	m_scrollBarVar;
				Dynamic<float>	m_scrollBarMin;
				Dynamic<float>	m_scrollBarMax;
				Dynamic<float>	m_scrollBarWidth;

				// The window manager for the storyboard !
				StoryBoard_View			*m_StoryBoard_View;

				// Undo and redo buffers
				int						m_StoryBoardUndo_Position;
				tList<StoryBoard_Item*>	m_StoryBoardUndo;

				// My Own Path Layout
				StoryBoardPath			*m_StoryBoardPath;

				// The default in/out point controls
				InOuts_Control			*MyInOutWindow;

				// A list of all thw in/out windows that are currently open
				tList<InOuts_Control*>	InOutWindowsOpen;

				// **************************************************************************
				// **** This is the playback engine *****************************************
				SBD_Item_Info			*PlaybackTree;
				StoryBoard_PlayBack		*PlayBackEngine;

				// **************************************************************************

				// Save the current information into a list
				void SaveCurrentWindowSelections(tList<StoryBoard_SelectionSave*> *ListOfWindows);
				void RestoreCurrentWindowSelections(tList<StoryBoard_SelectionSave*> *ListOfWindows,bool FreeMemory=true);
				void CloseExtraWindows(void);				
			
	public:		// Open the menu for a particular item
				void OpenMenu(StoryBoard_Crouton *Item);
		
				// From StretchySkinControl
				virtual void ReadClassData(FILE* fp);

				// BaseWindowClass stuff
				virtual void InitialiseWindow(void);
				virtual void DestroyWindow(void);

				// My dynamic callbacks
				virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

				// Undo / Redo functions
					// Save the current state to the undo buffer
					bool SaveCurrentStateToUndoBuffer(void);

					// Undo the last operation
					bool Undo(void);
					bool Redo(void);

					// Cut Copy and paste and clone operations
					bool Cut	(StoryBoard_Crouton *ItemToCut=NULL);
					bool Copy	(StoryBoard_Crouton *ItemToCopy=NULL);
					bool Paste	(StoryBoard_Crouton *ItemToPasterAfter=NULL);
					bool Clone	(StoryBoard_Crouton *ItemToClone=NULL);
					bool Delete	(void);

				// Change the path being viewed !
				void SetViewPath(StoryBoard_Item *SI);

				// Get the current path of the item being edited
				StoryBoard_Item *GetItemBeingEdited(void);

				// Update the current view
				void UpdateCurrentView(void);

				// Get a in/out window if there is one open.
				InOuts_Control *OpenInOutWindow(POINT *pt);
				void UpdateSelectedInOutWindow(void);

				// Constructor and destructor
				StoryBoard(void);
				~StoryBoard(void);

				// Deferred callback for closing
				void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);
};

#endif