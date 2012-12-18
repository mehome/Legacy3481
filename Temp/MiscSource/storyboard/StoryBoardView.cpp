#include "StdAfx.h"

//*************************************************************************************************************************************
void StoryBoard_View::SaveCurrentSelection(tList<GUID> *List)
{	for(unsigned i=0;i<GetNoChildren();i++)	
	{	BaseWindowClass *BWC=GetChild(i);
		if ((BWC)&&(BWC->DragAndDrop_AmISelected())) 
		{	StoryBoard_Crouton *SC=GetInterface<StoryBoard_Crouton>(BWC);
			if (SC) List->Add(SC->GetGUIDOfTree());
		}
	}
}

//*************************************************************************************************************************************
void StoryBoard_View::RestoreCurrentSelection(tList<GUID> *List)
{	// Delect everything
	DragAndDrop_SelectAllChildren(false);

	// Now select everything that is there
	for(unsigned i=0;i<GetNoChildren();i++)	
	{	StoryBoard_Crouton *SC=GetInterface<StoryBoard_Crouton>(GetChild(i));
		if (SC) SC->DragAndDrop_SelectMe(List->Exists(SC->GetGUIDOfTree()));
	}
}

//*************************************************************************************************************************************
StoryBoard_Crouton *StoryBoard_View::GetCrouton(GUID ID)
{	// Cycle through all children
	for(unsigned i=0;i<GetNoChildren();i++)
	{	StoryBoard_Crouton *SC=GetInterface<StoryBoard_Crouton>(GetChild(i));
		if ((SC)&&(SC->GetGUIDOfTree()==ID)) return SC;
	}
	
	// It was not found 
	return NULL;
}

//*************************************************************************************************************************************
bool StoryBoard_View::AddSelectedCroutonsToWindow(InOuts_Control *Window)
{	// pedantic
	if (!Window) return false;

	// We are going to build a tList of the items to view
	tList<StoryBoard_Crouton*> MyListOfItems;
	
	// Look for all items of interest ...
	for(unsigned i=0;i<GetNoChildren();i++)
	{	// Get the child
		StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(GetChildhWnd(i));

		// Add this item to the wish-list
		if ((SC)&&(SC->DragAndDrop_AmISelected())) MyListOfItems.Add(SC);
	}

	// If there are no items there return false !
	if (!MyListOfItems.NoItems) return false;

	// Add the items to the window
	Window->InOuts_Control_Set(&MyListOfItems);

	// There where items to add !
	return true;
}

//*************************************************************************************************************************************
void StoryBoard_View::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{	// Which message is it ?
	switch(ID1)
	{	// Change folder ?
		case StoryBoard_View_Deferred_ChangeFolder:
			{	// Which folder should we be changed to ?
				StoryBoard_Item	*NewFolder=(StoryBoard_Item*)ID2;

				// Get my parent
				StoryBoard *SB=GetWindowInterface<StoryBoard>(GetParent());

				// Change the path :0
				if (SB) SB->SetViewPath(NewFolder);

			} break;

		case StoryBoard_View_Deferred_InOutWindowUpdate:
			{	GetStoryBoardParent()->UpdateSelectedInOutWindow();
			} break;
	
		default:	_throw("Unknown StoryBoard_View::ReceiveDeferredMessage message !");
	}
}

//*************************************************************************************************************************************
void StoryBoard_View::AddDependants(void)
{	for(unsigned i=0;i<GetNoChildren();i++)
	{	// Get the BWC
		BaseWindowClass *BWC=GetChild(i);

		// If there is a window, check whether it is already talking to us !
		if (!BWC->IsDependant(this)) BWC->AddDependant(this,(long)BWC);
	}
}

//*************************************************************************************************************************************
void StoryBoard_View::SaveCurrentStateToUndoBuffer(void)
{	// We put the items in the undo buffer
	StoryBoard *SB=GetStoryBoardParent();
	if (SB) SB->SaveCurrentStateToUndoBuffer();
}

//*************************************************************************************************************************************
void StoryBoard_View::InitialiseWindow(void)
{	// Call my predecessor
	UtilLib_SimpleGrid::InitialiseWindow();

	// I am dependant on myself
	AddDependant(this);
}

//*************************************************************************************************************************************
void StoryBoard_View::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	// Is it a double click ?
	if (!strcmp(String,BaseWindowClass_LButtonDClick))
	{	// Is it a child of mine ?
		BaseWindowClass *BWC=(BaseWindowClass*)ID;
		for(unsigned i=0;i<GetNoChildren();i++)
		{	// Get the BWC
			BaseWindowClass *BWChld=GetChild(i);
			if (BWChld==BWC)
			{	// It is a child, so see if it is a folder !
				StoryBoard_Crouton *SCC=GetInterface<StoryBoard_Crouton>(BWC);
				if (SCC)
				{	// Get the storyboard entry being editied by this guy
					StoryBoard_Item *SBI=SCC->GetStoryboardTree();

					if (SBI)
					{	// If it has some children, then it is a folder, so execute the deferred message
						if (SCC->AmIFolder()) 
						{	DeferredMessage(StoryBoard_View_Deferred_ChangeFolder,(unsigned)SBI);
							
							// We have to ensure that the selected items are handled properly
							DeferredMessage(StoryBoard_View_Deferred_InOutWindowUpdate);
						}

						// Finished
						return;
					}
				}

				// Finished
				break;
			}
		}
	}	
	
	// If it is a window message, the get the hell outta here
	else if (IsWindowMessage(String)) return;	

	// Deletion
	else if (IsDeletion(String))
	{	// Call the parent
		UtilLib_SimpleGrid::DynamicCallback(ID,String,args,ItemChanging);
		return;
	}

	/*else if ((MyWindowEditState)&&(ID==(long)MyWindowEditState))
	{	bool State=MyWindowEditState->Get();
		if (!DragAndDrop_NoSelectedChildren()) State=false;
		if (MyInOutWindow) MyInOutWindow->ShowWindow(State);
		else
		{	if (State)
			{	// We have to ensure that the selected items are handled properly
				GetStoryBoardParent()->UpdateSelectedInOutWindow();
			}
		}
	}*/

	// Drag'n' drop changes
	else if (!strcmp(String,DragNDropContext_AllFinished))
	{	// We have to ensure that the selected items are handled properly
		GetStoryBoardParent()->UpdateSelectedInOutWindow();
	}
	
	// Has an item been removed from the S'board ?
	else if (!strcmp(DragNDropContext_DragFinishedExited,String))
	{	// We put the items in the undo buffer
		StoryBoard *SB=GetStoryBoardParent();
		if (SB) SB->SaveCurrentStateToUndoBuffer();		

		// We have to ensure that the selected items are handled properly
		GetStoryBoardParent()->UpdateSelectedInOutWindow();
	}	
}

//*************************************************************************************************************************************
// Build a folder at the current position
void StoryBoard_View::BuildFolder(void)
{	// We do not do any layouts !
	MultipleLayouts_Start();
	
	// We create a new crouton in the file bin, but it is a folder !
	HWND MyWindow=OpenChild("StoryBoard_Crouton",-FileButton_Size_IconLargeX,-FileButton_Size_IconLargeY,
							FileButton_Size_IconLargeX,FileButton_Size_IconLargeY,WS_CHILD);
	StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(MyWindow);			

	// Setup the crouton properties
	if (SC)
	{	// Is there anything selected ?
		bool AnythingSelected=false;		
		
		// We start by finding the first selected child
		for(unsigned i=0;i<GetNoChildren();i++)
		{	StoryBoard_Crouton	*ChldSC=GetWindowInterface<StoryBoard_Crouton>(GetChildhWnd(i));
			if ((ChldSC)&&(ChldSC->DragAndDrop_AmISelected()))
			{	// Ok something has been found
				AnythingSelected=true;

				// We move my item into the correct drawing order
				if (i==0)	SetWindowPos(MyWindow,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				else		SetWindowPos(MyWindow,GetChildhWnd(i-1),0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

				// Now we create a child item, and swap it into the correct position
				StoryBoard_Item *SI=GetStoryboardToEdit()->NewStoryboardChild();

				// Get a filename
				SI->SetAlias("Folder");				
				
				// Now we scan through the list of all items and move everything into the new parent
				for(unsigned j=0;j<GetNoChildren();j++)
				{	StoryBoard_Crouton	*ChldSC=GetWindowInterface<StoryBoard_Crouton>(GetChildhWnd(j));
					if ((ChldSC)&&(ChldSC->DragAndDrop_AmISelected()))
					{	// Ok something has been found, change the parent
						// Change this item to being a parent of the new SC
						ChldSC->GetStoryboardTree()->SetSelected(false);
						ChldSC->GetStoryboardTree()->ChangeParent(SI);
							
						// Delete the entry
						NewTek_Delete(ChldSC);

						// Since there is one fewer children, we need to jump back a number
						j--;
					}
				}

				// Move it into place
				for(j=GetStoryboardToEdit()->GetNoChildren()-1;j>i;j--)
					Swap((*GetStoryboardToEdit())[j],(*GetStoryboardToEdit())[j-1]);

				// Now assign the new tree to the window
				SC->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon);
				SC->SetStoryboardTree(SI);
				ApplyColorsTo(MyWindow);
				SC->DragAndDrop_SelectMe(false);

				// We are finished
				break;
			}
		}

		// If there was nothing selected, then we simply create a folder at the end
		if (!AnythingSelected)
		{	// Create a storyboard icon.
			StoryBoard_Item *SI=GetStoryboardToEdit()->NewStoryboardChild();
			SI->SetAlias("Folder");

			// Assign it to the window that has been created			
			SC->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon);
			SC->SetStoryboardTree(SI);
			ApplyColorsTo(MyWindow);
			SC->DragAndDrop_SelectMe(false);

			// It will always be in hte last position, so there is no need to actually 
			// set it's redraw position
		}
	}

	// We redo the layout now
	MultipleLayouts_Finished(true);

	// Ensure we know when the correct things change !
	AddDependants();

	// If the window is visible, we need to change what is being viewed !
	GetStoryBoardParent()->UpdateSelectedInOutWindow();

	// We need to be able to undo to this position
	SaveCurrentStateToUndoBuffer();
}

//*************************************************************************************************************************************
// Apply my selected colors to a file button
void StoryBoard_View::ApplyColorsTo(HWND hWnd)
{	FileButton *FB=GetWindowInterface<FileButton>(hWnd);
	if (!FB) return;
	
	// Setup the status
	for (unsigned buttonState = 0; buttonState < 4; buttonState++)
	{	TextItem* ti=FB->GetTextItem(buttonState);
		long alignmentFlags = TextItem_CentreVAlign;
		switch(m_alignment[buttonState])
		{	case 'c':	alignmentFlags += TextItem_CentreHAlign;	break;
			case 'r':	alignmentFlags += TextItem_RightAlign;		break;
			default:	alignmentFlags += TextItem_LeftAlign;		break;
		}

		// We do not need to send changed messages here
		ti->MessagesOff();
		ti->Set_BackgroundColor(m_bg_color[buttonState].r, m_bg_color[buttonState].g, m_bg_color[buttonState].b);
		ti->SetAlignment(alignmentFlags); ti->SetFont(m_Font);
		ti->SetFontSize(m_Size[buttonState]); ti->SetBold(m_Bold[buttonState]);
		ti->SetItalic(m_Italic[buttonState]); ti->SetUnderline(m_Underline[buttonState]);
		ti->SetColor(m_color[buttonState].r, m_color[buttonState].g, m_color[buttonState].b);
		ti->MessagesOn();
	}
}

//*************************************************************************************************************************************
StoryBoard_Item *StoryBoard_View::GetStoryboardToEdit(void)
{	return m_StoryboardToEdit;
}

//*************************************************************************************************************************************
void StoryBoard_View::SetStoryboardToEdit(StoryBoard_Item *ITE,bool ObserveSelections)
{	// Do not do any kind of layout operations now
	MultipleLayouts_Start();
	
	// We need to remove all children that currently exist
	CloseAllChildren();

	// Now we need to go ahead and add all the new children as required
	m_StoryboardToEdit=ITE;

	if (m_StoryboardToEdit)
	{	// Cycle through all the children
		for(unsigned i=0;i<m_StoryboardToEdit->GetNoChildren();i++)
		{	// We create a new window 
			HWND MyWindow=OpenChild("StoryBoard_Crouton",0,0,100,100,WS_CHILD);
			StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(MyWindow);
			
			if (SC)
			{	// Setup the attributes of this text item
				SC->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon);				
				
				// We need to set up the correct filename
				SC->SetStoryboardTree((*m_StoryboardToEdit)[i],true);

				// Select it as necessary
				if (ObserveSelections) SC->DragAndDrop_SelectMe((*m_StoryboardToEdit)[i]->GetSelected());
				
				// Apply the colors
				ApplyColorsTo(MyWindow);
			}
		}
	}

	// Go ahead and do the layout
	MultipleLayouts_Finished();

	// Ensure we know when the correct things change !
	AddDependants();
}

//*************************************************************************************************************************************
WindowLayout_Item *StoryBoard_View::DragNDrop_DropItemsHere(	
											int Width,int Height,
											int MousePosnX,int MousePosnY,
											WindowLayout_Item *ObjectsInWindow,
											WindowLayout_Item *ObjectsDropped,
											bool Resizing,bool DroppedHere)
{	// Have the items now actually been dropped ?
	if (DroppedHere)
	{	// Pedantic, but useful
		OutputDebugString("StoryBoard_View::Items have been dropped into the storyboard\n");	
		
		// We do not want to do lots of layouts
		MultipleLayouts_Start();

		// If the items are from another window, then we need to deselect anything currently in the window
		bool DeterminedParent=false;				
		
		// We scan through all the items
		WindowLayout_Item *Scan=ObjectsDropped;

		// Look at all the objects
		while(Scan)
		{	// We need to get the BaseWindowClass interface
			BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
			if (BWC)
			{	// If this is a FileButton, but NOT a crouton already
				FileButton			*FB=GetInterface<FileButton>(BWC);
				StoryBoard_Crouton	*SC=GetInterface<StoryBoard_Crouton>(BWC);

				// If it is already a storyboard crouton, we need to become it's owner
				if (SC)
				{	// We can easily determine whether these are from my view !
					if (!DeterminedParent)
					{	DeterminedParent=true;
						if (SC->GetStoryboardTree()->GetParent()!=GetStoryboardToEdit())
								DragAndDrop_SelectAllChildren(false);
					}
					
					// Change the parent, but only if it needs to be changed ... saves time
					if (SC->GetStoryboardTree()->GetParent()!=GetStoryboardToEdit())
						SC->GetStoryboardTree()->ChangeParent(m_StoryboardToEdit);
					SC->DragAndDrop_SelectMe(true);
				}
				// If it is a file button, nbut not already a crouton
				else if ((!SC)&&(FB))
				{	// We are sure that these items came from another window
					if (!DeterminedParent)
					{	DeterminedParent=true;
						DragAndDrop_SelectAllChildren(false);
					}
					
					// We create a new window 
					HWND MyWindow=OpenChild("StoryBoard_Crouton",0,0,100,100,WS_CHILD);
					SC=GetWindowInterface<StoryBoard_Crouton>(MyWindow);

					if (SC)
					{	// Setup the storyboard stuff
						StoryBoard_Item *SCI=m_StoryboardToEdit->NewStoryboardChild();
						char Temp[MAX_PATH];
						FB->GetFileName(Temp);
						SCI->SetFileName(Temp);

						FB->GetAlias(Temp);
						SCI->SetAlias(Temp);
						
						// We now make this look at the filename of the button that was inserted
						FB->CopyTo(SC);
						// We kill off the original window
						SC->SetStoryboardTree(SCI);

						// Delete the original window
						NewTek_Delete(BWC);

						// And place me in the original position
						Scan->hWnd=MyWindow;

						// We make sure all of our colors and size match
						SC->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon);
						SC->DragAndDrop_SelectMe(true);
						Scan->XSize=SC->GetWindowWidth();
						Scan->YSize=SC->GetWindowHeight();

						// Apply the colors
						ApplyColorsTo(SC->GetWindowHandle());
					}
				}
			}

			// Look at the next item
			Scan=Scan->Next;
		}

		// We are finished doing all the funky layout stuff
		// The reason why I do not force a layout is that I am already doing one since I 
		// am just about to call DragNDrop_DropItemsHere.
		MultipleLayouts_Finished(false);	

		// Sort out the dependants
		AddDependants();
	}

	// Drop them in the correct positions
	WindowLayout_Item *Ret=UtilLib_SimpleGrid::DragNDrop_DropItemsHere(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);

	// Handle the fact that we need to be able to undo this configuration
	if (DroppedHere)
	{	// Cycle through all the elements, and modify the storyboard order !
		WindowLayout_Item *Scan=Ret;
		unsigned i=0;
		while(Scan)
		{	// Get this item
			StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(Scan->hWnd);
			if (SC)
			{	// Put this into the child list, so that the order is correct
				(*m_StoryboardToEdit)[i++]=SC->GetStoryboardTree();
			}

			// Look at the next item
			Scan=Scan->Next;
		}
		
		// We need to be able to undo to this change
		SaveCurrentStateToUndoBuffer();

		// Update the selected window
		DeferredMessage(StoryBoard_View_Deferred_InOutWindowUpdate);
	}

	// Success
	return Ret;
}

//*************************************************************************************************************************************
WindowLayout_Item *StoryBoard_View::Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height)
{	if (!m_AmISizing)
	{	// We do the layout as desired, and do the correct resizing
		m_AmISizing=true;
		WindowLayout_Item *Ret=UtilLib_SimpleGrid::Layout_ComputeLayout(Children,Width,Height);
		m_AmISizing=false;
		return Ret;
	}

	// Do noting ...
	return Children;
}

//*************************************************************************************************************************************
bool StoryBoard_View::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{	// If it is a storyboard icon, it can always be dropped here !
	StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(Dropped->hWnd);
	if (SC) return true;
	
	// If this is a file button, we might allow it to be dropped here
	FileButton* thisButton=GetWindowInterface<FileButton>(Dropped->hWnd);
	if (thisButton)
	{	// We check whether it is actually a file
		if (thisButton->GetStatus()==FileButton_File) return true;

		// It was a drive, folder, or something else, so we probably cannot allow it to be dropped here safely
		return false;
	}

	// Success
	return false;
}

//*************************************************************************************************************************************
void StoryBoard_View::ReadClassData(FILE* fp)
{	// Read in the font
	char buffer[256];
	char* line = StretchyRegion::ReadNextString(fp, buffer, 256);
	strcpy(m_Font, line);

	// Read in each of the values for the four states
	for (int i = 0; i < 4; i++)
	{
		line = StretchyRegion::ReadNextLine(fp, buffer, 256);
		long r,g,b,br,bg,bb,bold,ital,under;
		sscanf(line, "%c%ld%ld%ld%ld%ld%ld%ld%ld%ld%f",&m_alignment[i], 
				&r,&g,&b,&br,&bg,&bb,
				&bold,&ital,&under,&m_Size[i]);

		m_bg_color[i].r = (unsigned char)br;
		m_bg_color[i].g = (unsigned char)bg;
		m_bg_color[i].b = (unsigned char)bb;
		m_bg_color[i].a = (unsigned char)255;

		m_color[i].r = (unsigned char)r;
		m_color[i].g = (unsigned char)g;
		m_color[i].b = (unsigned char)b;
		m_color[i].a = (unsigned char)255;

		m_Bold[i] = (bold != 0);
		m_Italic[i] = (ital != 0);
		m_Underline[i] = (under != 0);
	}

	UtilLib_SimpleGrid_PutSpacingX(5);
	UtilLib_SimpleGrid_PutSpacingY(8);
}

//*************************************************************************************************************************************
StoryBoard *StoryBoard_View ::GetStoryBoardParent(void)
{	BaseWindowClass *BWC=GetParentBWC();
	if (!BWC) return NULL;
	return GetInterface<StoryBoard>(BWC);
}

//*************************************************************************************************************************************
StoryBoard_View::StoryBoard_View(void)
{	m_AmISizing=false; 
	m_StoryboardToEdit=NULL;
}