#ifndef __StoryBoardPath__
#define __StoryBoardPath__

//********************************************************************************************************************
// Deffered messages
#define StoryBoardPath_Deferred_ChangeFolder	1

//********************************************************************************************************************
class SBDLL StoryBoardPath : public UtilLib_SimpleGrid
{	protected:		// This is the storyboard item to actually edit
					StoryBoard_Item	*m_StoryboardToEdit;

					// File button characteristics to be read in from the file
					char			m_Font[256];
					bool			m_Bold[4];
					bool			m_Italic[4];
					bool			m_Underline[4];
					float			m_Size[4];
					WindowPixel		m_color[4];
					char			m_alignment[4];

					WindowPixel		m_C1, m_C2;
					WindowPixel*	m_currentColor;

					// Add all the dependants
					void AddDependants(void);

	public:			//**********************************************************************************************
					//**** From DragNDrop ****
					virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
					virtual bool DragNDrop_ShouldIRubberBandSelect(HWND hWnd);
					virtual WindowLayout_Item *Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height);

					//**********************************************************************************************
					// My dynamic callback stuff, also deferred messages in here

					// I listen to things going on around me
					virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

					// Deferred Messages
					virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

					//**********************************************************************************************
					// Set the storyboard item to edit
					void SetStoryboardToEdit(StoryBoard_Item *ITE);
					StoryBoard_Item *GetStoryboardToEdit(void);

					// Set the icons up
					void SetChildViewMode(FileButton* p_child);
					void ReadClassData(FILE* fp);

					// Constructor
					StoryBoardPath(void);
};

#endif