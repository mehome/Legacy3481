#ifndef __StoryBoard_View__
#define __StoryBoard_View__

//********************************************************************************************************************
class StoryBoard;

//********************************************************************************************************************
// Deffered messages
#define StoryBoard_View_Deferred_ChangeFolder		1
#define StoryBoard_View_Deferred_InOutWindowUpdate	2

//********************************************************************************************************************
class SBDLL StoryBoard_View : public UtilLib_SimpleGrid
{	protected:		// File button characteristics to be read in from the file
					char			m_Font[256];
					bool			m_Bold[4];
					bool			m_Italic[4];
					bool			m_Underline[4];
					float			m_Size[4];
					WindowPixel		m_color[4];
					WindowPixel		m_bg_color[4];
					char			m_alignment[4];
					bool			m_AmISizing;

					// This is the storyboard item to actually edit
					StoryBoard_Item	*m_StoryboardToEdit;

					// Apply my selected colors to a file button
					void ApplyColorsTo(HWND hWnd);

					// Get my parent
					StoryBoard *GetStoryBoardParent(void);

					// Put this onto the undo buffer
					void SaveCurrentStateToUndoBuffer(void);

					// Add all the dependants
					void AddDependants(void);			

					// The deferred message type
					unsigned DeferredMessageMSG;

	public:			// Save the current selection into a list
					void SaveCurrentSelection(tList<GUID> *List);
					void RestoreCurrentSelection(tList<GUID> *List);
		
					// This function builds a folder at the current position
					void BuildFolder(void);

					// Add the list of croutons to a particular editing window
					// Return value is if there are no items selected to add ...
					bool AddSelectedCroutonsToWindow(InOuts_Control *Window);
					
					// Stuff to help initialisation
					void ReadClassData(FILE* fp);
					
					// Layout manager overloads
					WindowLayout_Item *StoryBoard_View::Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height);

					// Drag'n'drop overloads
					virtual WindowLayout_Item *DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									);
					bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);

					// Set the storyboard item to edit
					void SetStoryboardToEdit(StoryBoard_Item *ITE,bool ObserveSelections=false);
					StoryBoard_Item *GetStoryboardToEdit(void);

					// Deferred Messages
					virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

					// Get the crouton item for a particular GUID
					StoryBoard_Crouton *GetCrouton(GUID ID);

					// Constructor
					StoryBoard_View(void);

					// I listen to things going on around me
					virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

					// Window initialisation
					virtual void InitialiseWindow(void);					
};


#endif