#include "StdAfx.h"

//*************************************************************************************************************************************
void StoryBoard::SaveCurrentWindowSelections(tList<StoryBoard_SelectionSave*> *ListOfWindows)
{	for(unsigned i=0;i<InOutWindowsOpen.NoItems;i++)
	{	// Get a new entry
		StoryBoard_SelectionSave *Entry=new StoryBoard_SelectionSave;
		ListOfWindows->Add(Entry);
		Entry->WindowOfInterest=InOutWindowsOpen[i];
		
		// Store the GUIDs of all entries that where in this item
		InOutWindowsOpen[i]->SaveGUIDs(&Entry->GUIDs);
	}
}

void StoryBoard::RestoreCurrentWindowSelections(tList<StoryBoard_SelectionSave*> *ListOfWindows,bool FreeMemory)
{	for(unsigned i=0;i<ListOfWindows->NoItems;i++)
	{	// Check it exists on my list
		if (InOutWindowsOpen.Exists((*ListOfWindows)[i]->WindowOfInterest))
		{	// This list of the croutons to add
			tList<StoryBoard_Crouton *> WindowCroutons;
			
			// Now look through the viewport for all the GUIDs
			for(unsigned j=0;j<(*ListOfWindows)[i]->GUIDs.NoItems;j++)
			{	StoryBoard_Crouton *SC=m_StoryBoard_View->GetCrouton((*ListOfWindows)[i]->GUIDs[j]);
				if (SC) WindowCroutons.Add(SC);
			}

			// Add these items to the window
			if (WindowCroutons.NoItems)
				(*ListOfWindows)[i]->WindowOfInterest->InOuts_Control_Set(&WindowCroutons);

			// If we are freeing the memory, just do it
			if (FreeMemory) delete (*ListOfWindows)[i];
		}
	}
}
void StoryBoard::CloseExtraWindows(void)
{	for(unsigned i=0;i<InOutWindowsOpen.NoItems;i++)
	{	// If there are no entries left ...
		if (!InOutWindowsOpen[i]->InOuts_Control_AnyEntriesLeft())
		{	// There are no entries left in this window
			if (InOutWindowsOpen[i]==MyInOutWindow)
			{	// If it is the main window open, then it must just be hidden
				MyInOutWindow->ShowWindow(false);
			}
			else
			{	// Otherwise, we close it
				NewTek_Delete(InOutWindowsOpen[i]);

				// Since the list is now one short, we do not advance through it
				i--;
			}
		}
	}
}

//*************************************************************************************************************************************
InOuts_Control *StoryBoard::OpenInOutWindow(POINT *pt)
{	POINT l_pt;
	if (!pt)	
	{	if (!MyInOutWindow)	GetCursorPos(&l_pt);
		else 
		{	RECT rect;
			GetWindowRect(MyInOutWindow->GetWindowHandle(),&rect);
			l_pt.x=rect.left;
			l_pt.y=rect.top;
		}
	}
	else		l_pt=*pt;

	// If a window is available already
	if (!MyInOutWindow)
	{	// We need to open the properties window
		MyInOutWindow=GetInterface<InOuts_Control>(NewTek_New("InOuts_Control_Default",GetWindowHandle(),0,0,100,100,"",WS_POPUP|WS_VISIBLE));
		if (!MyInOutWindow) _throw("StoryBoard_View::OpenInOutWindow could not create InOuts_Control_Default");		
		InOutWindowsOpen.Add(MyInOutWindow);
		MyInOutWindow->AddDependant(this,(long)&InOutWindowsOpen);
	}

	// Move it to the correct posiiton and show it
	SetWindowPos(MyInOutWindow->GetWindowHandle(),NULL,l_pt.x,l_pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);

	// Success
	return MyInOutWindow;
}

//*************************************************************************************************************************************
void StoryBoard::UpdateSelectedInOutWindow(void)
{	// Handle the window correctly
	MyInOutWindow=OpenInOutWindow(NULL);

	// Put the correct items in the window
	if (!m_StoryBoard_View->AddSelectedCroutonsToWindow(MyInOutWindow))			
								MyInOutWindow->ShowWindow(false);
	else	
	{	if (m_EditState.Get())	MyInOutWindow->ShowWindow(true);
		else					MyInOutWindow->ShowWindow(false);					
	}
}

//*************************************************************************************************************************************
void StoryBoard::OpenMenu(StoryBoard_Crouton *Item)
{	tList<char*> MyMenu;
	MyMenu.Add(" Cut ");		// 0
	MyMenu.Add(" Copy ");		// 1
	MyMenu.Add(" Paste ");		// 2
	MyMenu.Add(" Clone ");		// 3
	MyMenu.Add("");				// 4
	MyMenu.Add(" Undo ");		// 5
	MyMenu.Add(" Redo ");		// 6
	MyMenu.Add("");				// 7
	MyMenu.Add(" Properties");	// 8

	RECT menuRect=CreateMenu(&MyMenu);

	POINT pt;
	GetCursorPos(&pt);
	menuRect.left	+= pt.x;
	menuRect.right	+= pt.x;
	menuRect.top	+= pt.y;
	menuRect.bottom += pt.y;

	// Show the menu
	NewTek_FixRectForScreen(menuRect);
	int Ret=ShowMenu(menuRect);

	// Now execute the command
	switch(Ret)
	{	case 0:	// *** Cut ***
		{	if (Item->DragAndDrop_AmISelected())	Cut();
			else									Cut(Item);

			// We need to update the undo buffer appropriately
			SaveCurrentStateToUndoBuffer();

		} break;

		case 1: // *** Copy ***
		{	if (Item->DragAndDrop_AmISelected())	Copy();
			else									Copy(Item);

			// We need to update the undo buffer appropriately
			SaveCurrentStateToUndoBuffer();

		} break;

		case 2: // *** Paste ***
		{	if (Item->DragAndDrop_AmISelected())	Paste();
			else									Paste(Item);

			// We need to update the undo buffer appropriately
			SaveCurrentStateToUndoBuffer();
		} break;

		case 3: // *** Clone ***
		{	if (Item->DragAndDrop_AmISelected())	Clone();
			else									Clone(Item);

			// We need to update the undo buffer appropriately
			SaveCurrentStateToUndoBuffer();
		} break;

		case 5: // *** Undo ***
		{	Undo();
		} break;

		case 6: // *** Redo ***
		{	Redo();
		} break;

		case 8: // *** Properties ***
		{	// Get My Position
			RECT rect;
			GetWindowRect(Item->GetWindowHandle(),&rect);

			// Build the list of items to edit
			tList<StoryBoard_Crouton*> Items;

			if (Item->DragAndDrop_AmISelected())
			{	for(unsigned i=0;i<m_StoryBoard_View->GetNoChildren();i++)
				{	StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(m_StoryBoard_View->GetChildhWnd(i));
					if ((SC)&&(SC->DragAndDrop_AmISelected())) Items.Add(SC);
				}
			}
			else Items.Add(Item);

			// We need to open the properties window
			InOuts_Control *InOuts=GetInterface<InOuts_Control>(	NewTek_New("InOuts_Control",GetWindowHandle(),
																	rect.right+3,rect.top,100,100,"",WS_POPUP|WS_VISIBLE));
			if (!InOuts) _throw("StoryBoard_Crouton::MouseRButtonClick could not create InOuts_Control");
			InOuts->InOuts_Control_Set(&Items);
			InOutWindowsOpen.Add(InOuts);
			InOuts->AddDependant(this,(long)&InOutWindowsOpen);

		} break;
	}
}

//********************************************************************************************************************
// Get the current path of the item being edited
StoryBoard_Item *StoryBoard::GetItemBeingEdited(void)
{	return m_StoryBoard_View->GetStoryboardToEdit();
}

//********************************************************************************************************************
// Update the current view
void StoryBoard::UpdateCurrentView(void)
{	// Update the view
	m_StoryBoard_View->SetStoryboardToEdit(m_StoryBoard_View->GetStoryboardToEdit());	
}

//********************************************************************************************************************
void StoryBoard::SetViewPath(StoryBoard_Item *SI)
{	// Check that there has been a change. No point in wasting CPU time
	// on a refresh that does nothing !
	if (SI==m_StoryBoard_View->GetStoryboardToEdit()) return;
	
	// Change both my children
	m_StoryBoardPath->SetStoryboardToEdit(SI);
	m_StoryBoard_View->SetStoryboardToEdit(SI);

	// Close down any extra windows
	CloseExtraWindows();
}

//********************************************************************************************************************
bool StoryBoard::Cut(StoryBoard_Crouton *ItemToCut)
{	// Delete the selected items from the tree
	m_StoryBoard_View->MultipleLayouts_Start();

	if (!ItemToCut)
	{	// Copy the data to the selection buffer
		StoryBoard_Copy(m_StoryBoard_View->GetStoryboardToEdit());
	
		// Cycle through all children removing all selected ones
		for(unsigned i=0;i<m_StoryBoard_View->GetNoChildren();i++)
		{	BaseWindowClass *BWC=m_StoryBoard_View->GetChild(i);
			if ((BWC)&&(BWC->DragAndDrop_AmISelected()))
			{	// I am selected, so delete it !
				NewTek_Delete(BWC);
				
				// Since the list is one shorter, we need to backstep by one
				i--;
			}
		}

		// Now delete all selected children
		m_StoryBoard_View->GetStoryboardToEdit()->DeleteSelectedChildren();	
	}
	else
	{	// Copy the data to the selection buffer
		tList<StoryBoard_Item*> ItemsToCut;
		ItemsToCut.Add(ItemToCut->GetStoryboardTree());
		StoryBoard_Copy(&ItemsToCut);
		
		// Delete a single child
		ItemToCut->GetStoryboardTree()->GetParent()->DeleteChild(ItemToCut->GetStoryboardTree());
		
		// This was passed in by the window, so delete it !
		NewTek_Delete(ItemToCut);
	}	

	// Perform the layout
	m_StoryBoard_View->MultipleLayouts_Finished();

	// We have to ensure that the selected items are handled properly
	UpdateSelectedInOutWindow();
	CloseExtraWindows();

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::Copy(StoryBoard_Crouton *ItemToCopy)
{	if (!ItemToCopy)
	{	// Copy the data to the selection buffer
		StoryBoard_Copy(m_StoryBoard_View->GetStoryboardToEdit());
	}
	else
	{	// Copy the data to the selection buffer
		tList<StoryBoard_Item*> ItemsToCopy;
		ItemsToCopy.Add(ItemToCopy->GetStoryboardTree());
		StoryBoard_Copy(&ItemsToCopy);
	}

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::Delete(void)
{	// Delete the selected items from the tree
	m_StoryBoard_View->MultipleLayouts_Start();

	// Cycle through all children removing all selected ones
	for(unsigned i=0;i<m_StoryBoard_View->GetNoChildren();i++)
	{	BaseWindowClass *BWC=m_StoryBoard_View->GetChild(i);
		if ((BWC)&&(BWC->DragAndDrop_AmISelected()))
		{	// I am selected, so delete it !
			NewTek_Delete(BWC);
			
			// Since the list is one shorter, we need to backstep by one
			i--;
		}
	}

	// Now delete all selected children
	m_StoryBoard_View->GetStoryboardToEdit()->DeleteSelectedChildren();	

	// Perform the layout
	m_StoryBoard_View->MultipleLayouts_Finished();

	// We have to ensure that the selected items are handled properly
	UpdateSelectedInOutWindow();

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::Paste(StoryBoard_Crouton *ItemToPasterAfter)
{	// We get access to the last selected item
	unsigned LastSelected=m_StoryBoard_View->GetStoryboardToEdit()->GetLastSelected();
	if (LastSelected==0xffffffff) LastSelected=m_StoryBoard_View->GetStoryboardToEdit()->GetNoChildren();

	// Get the position to past after if there is one
	if (ItemToPasterAfter)
	{	for(unsigned i=0;i<m_StoryBoard_View->GetNoChildren();i++)
		{	if (GetWindowInterface<StoryBoard_Crouton>(m_StoryBoard_View->GetChildhWnd(i))==ItemToPasterAfter)
			{	LastSelected=i;
				break;
			}
		}
	}

	// We insert it at the relevant position
	StoryBoard_Item *PasteItem=StoryBoard_Paste();
	if (!PasteItem) return false;

	// Get my local copy
	StoryBoard_Item LocalCopy;
	LocalCopy.CopyFrom(PasteItem);

	// We deselect all children ?
	m_StoryBoard_View->GetStoryboardToEdit()->DeselectAllChildren();

	// Now we merge them into the existing tree
	m_StoryBoard_View->GetStoryboardToEdit()->MergeTree(&LocalCopy,LastSelected+1);

	// We need to update all the icons
	m_StoryBoard_View->SetStoryboardToEdit(m_StoryBoard_View->GetStoryboardToEdit());

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::Clone(StoryBoard_Crouton *ItemToClone)
{	// We save all the window statuses
	tList<StoryBoard_SelectionSave*> StoreWindowSelections;
	SaveCurrentWindowSelections(&StoreWindowSelections);	
	
	// Get the last selected item
	unsigned LastSelected=m_StoryBoard_View->GetStoryboardToEdit()->GetLastSelected();

	// We build a tree of only the selected items
	StoryBoard_Item LocalTree;

	// Get the position to past after if there is one
	if (ItemToClone)
	{	for(unsigned i=0;i<m_StoryBoard_View->GetNoChildren();i++)
		{	if (GetWindowInterface<StoryBoard_Crouton>(m_StoryBoard_View->GetChildhWnd(i))==ItemToClone)
			{	LastSelected=i;
				break;
			}
		}

		// Copy the data to the selection buffer
		StoryBoard_Item *Chld=LocalTree.NewStoryboardChild();
		if (Chld) Chld->CopyFrom(ItemToClone->GetStoryboardTree());			
	}
	else
	{	// Copy only the selected items into this view
		LocalTree.CopyFrom(m_StoryBoard_View->GetStoryboardToEdit(),true);
	}		

	// We can just skip over this if there is nothing to do
	if (LocalTree.GetNoChildren())
	{	// Deslect all the currently selected items
		m_StoryBoard_View->GetStoryboardToEdit()->DeselectAllChildren();

		// All the new items should be selected
		LocalTree.SelectAllChildren();
		
		// Now we merge them into the existing tree
		m_StoryBoard_View->GetStoryboardToEdit()->MergeTree(&LocalTree,LastSelected+1);

		// We need to update all the icons
		m_StoryBoard_View->SetStoryboardToEdit(m_StoryBoard_View->GetStoryboardToEdit(),true);
		UpdateSelectedInOutWindow();
	}

	// Restore the selections
	RestoreCurrentWindowSelections(&StoreWindowSelections);
	UpdateSelectedInOutWindow();

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::Redo(void)
{	// Make anyone aware of what is going on
	DebugOutput("StoryBoard::Redo, Level=%d\n",m_StoryBoardUndo_Position);

	// We save all the window statuses
	tList<StoryBoard_SelectionSave*> StoreWindowSelections;
	SaveCurrentWindowSelections(&StoreWindowSelections);	
	tList<GUID> StoreSelections;
	m_StoryBoard_View->SaveCurrentSelection(&StoreSelections);

	// CHeck we are not stepping psat the end
	if (m_StoryBoardUndo_Position>=m_StoryBoardUndo.NoItems) return false;

	// We need to get the GUID of the item that we are editing
	GUID GuidToView=m_StoryBoard_View->GetStoryboardToEdit()->GetSerialNunber(); 

	// To start with, we cannot tolerate any layout manager changes right now
	m_StoryBoard_View->MultipleLayouts_Start();

		// We start by killing all the child windows of the item :(
		m_StoryBoard_View->CloseAllChildren();

		// We need a loading context
		m_StoryBoardUndo_Position++;		
		CopyFrom(m_StoryBoardUndo[m_StoryBoardUndo_Position-1],false,true);

	// We need to find the item that has the same GUID
	StoryBoard_Item* ItemToView=FindItem(GuidToView);
	if (!ItemToView) ItemToView=this;

	// Update the view
	if (ItemToView==m_StoryBoard_View->GetStoryboardToEdit()) 
			m_StoryBoard_View->SetStoryboardToEdit(ItemToView);
	else	SetViewPath(ItemToView);

	// Now we should redo the window layout
	m_StoryBoard_View->MultipleLayouts_Finished();

	// Restore the selections
	m_StoryBoard_View->RestoreCurrentSelection(&StoreSelections);
	RestoreCurrentWindowSelections(&StoreWindowSelections);
	CloseExtraWindows();

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::Undo(void)
{	// Make anyone aware of what is going on
	DebugOutput("StoryBoard::Undo, Level=%d\n",m_StoryBoardUndo_Position);

	// We save all the window statuses
	tList<StoryBoard_SelectionSave*> StoreWindowSelections;
	SaveCurrentWindowSelections(&StoreWindowSelections);	
	tList<GUID> StoreSelections;
	m_StoryBoard_View->SaveCurrentSelection(&StoreSelections);

	// We need to get the GUID of the item that we are editing
	GUID GuidToView=m_StoryBoard_View->GetStoryboardToEdit()->GetSerialNunber(); 

	// To start with, we cannot tolerate any layout manager changes right now
	m_StoryBoard_View->MultipleLayouts_Start();

		// We start by killing all the child windows of the item :(
		m_StoryBoard_View->CloseAllChildren();

		// Now we reload the configuration from one of the undo buffers
		if (m_StoryBoardUndo_Position>1) m_StoryBoardUndo_Position--;

		// Copy the data across
		CopyFrom(m_StoryBoardUndo[m_StoryBoardUndo_Position-1],false,true);
		
	// Update the view, We need to find the item that has the same GUID
	StoryBoard_Item* ItemToView=FindItem(GuidToView);
	if (!ItemToView) ItemToView=this;

	// Update the view
	if (ItemToView==m_StoryBoard_View->GetStoryboardToEdit()) 
			m_StoryBoard_View->SetStoryboardToEdit(ItemToView);
	else	SetViewPath(ItemToView);

	// Now we should redo the window layout
	m_StoryBoard_View->MultipleLayouts_Finished();

	// Restore the selections
	m_StoryBoard_View->RestoreCurrentSelection(&StoreSelections);
	RestoreCurrentWindowSelections(&StoreWindowSelections);
	CloseExtraWindows();

	// Success
	return true;
}

//********************************************************************************************************************
bool StoryBoard::SaveCurrentStateToUndoBuffer(void)
{	// Make anyone aware of what is going on
	DebugOutput("StoryBoard::SaveCurrentStateToUndoBuffer, Level=%d\n",m_StoryBoardUndo_Position);
	
	// If there are redo buffers ahead of me in the queue, then we need to ditch them
	for(int i=m_StoryBoardUndo.NoItems-1;i>=m_StoryBoardUndo_Position;i--)
	{	// Free the memory
		delete m_StoryBoardUndo[i];

		// Delete it
		m_StoryBoardUndo.DeleteEntry(i);
	}

	// Save the memory in the undo buffer
	StoryBoard_Item *SI=new StoryBoard_Item;
	if (!SI) return false;
	SI->CopyFrom(this,false,false);
	m_StoryBoardUndo.Add(SI);

	// We move down the undo list by one !
	m_StoryBoardUndo_Position++;

	// Create a storyboard instance of this !!!!!!!!!!
	// We pause the playback
	if (PlayBackEngine) PlayBackEngine->Pause(true);

	/* We generate a new tree */
	SBD_Item_Info	*New_PlaybackTree=BuildTreeForPlayback(NULL);
	if (New_PlaybackTree) 
	{	// Compute the tree
		New_PlaybackTree->Calculate();
		
		// Setup a playback engine if necessary
		if (!PlayBackEngine)
				PlayBackEngine=new StoryBoard_PlayBack(New_PlaybackTree);
		else	PlayBackEngine->ChangeItemBeingViewed(New_PlaybackTree,true);

		// We delete the old tree
		if (PlaybackTree) delete PlaybackTree;
		PlaybackTree=New_PlaybackTree;
	}

	// Success
	return true;
}

//********************************************************************************************************************
StoryBoard::StoryBoard(void)
{	// No undo buffers are used
	m_StoryBoardUndo_Position=0;	

	// This is the stroyboard root node
	SetAlias("Storyboard");

	// There is no playback engine running, and no tree built
	PlaybackTree=NULL;
	PlayBackEngine=NULL;

	// The base storyboard project information ...
	SaveCurrentStateToUndoBuffer();	
}

StoryBoard::~StoryBoard(void)
{	// Remove all dependants
	m_Buttons.DeleteDependant(this);
	m_PlayState.DeleteDependant(this);
	m_scrollBarVar.DeleteDependant(this);
	m_scrollBarMin.DeleteDependant(this);
	m_scrollBarMax.DeleteDependant(this);
	m_scrollBarWidth.DeleteDependant(this);
	m_exitInt.DeleteDependant(this);

	// Delete any playback tree
	if (PlaybackTree) 
	{	delete PlaybackTree;
		PlaybackTree=NULL;
	}

	if (PlayBackEngine)
	{	delete PlayBackEngine;
		PlayBackEngine=NULL;
	}
	
	// Any undo buffers that are still lying around need to be freed
	for(unsigned i=0;i<m_StoryBoardUndo.NoItems;i++)
		delete m_StoryBoardUndo[i];
}

//********************************************************************************************************************
void StoryBoard::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	if (!strcmp(BaseWindowClass_Close,String))
	{	void *Args=args;
		BaseWindowClass *BWC=NewTek_GetArguement<BaseWindowClass*>(Args);
		for(unsigned i=0;i<InOutWindowsOpen.NoItems;i++)
		if (GetInterface<BaseWindowClass>(InOutWindowsOpen[i])==BWC)
		{	InOutWindowsOpen.DeleteEntry(i);
			break;
		}
		else if (BWC==GetInterface<BaseWindowClass>(MyInOutWindow)) MyInOutWindow=NULL;
		StretchySkinControl::DynamicCallback(ID,String,args,ItemChanging);
	}
	// FIlter out windows messages (which there are loads of !
	else if (IsWindowMessage(String)) return;
	// The status of the on/off edit button ...
	else if (ID==(long)&m_EditState)
	{	bool State=(m_EditState==1);
		if (!m_StoryBoard_View->DragAndDrop_NoSelectedChildren()) State=false;
		if (MyInOutWindow) MyInOutWindow->ShowWindow(State);
		// We have to ensure that the selected items are handled properly
		else if (State) UpdateSelectedInOutWindow();
	}
	// Has the playstate been changed ?
	else if (ID==(long)&m_PlayState)
	{	if (!strcmp(Controls_Button_ReleasedOn,String))
		{	switch(m_PlayState)
			{	case 1:		
				{	// Rew
					if (PlayBackEngine) 
					{	PlayBackEngine->ChangePlaybackPosition(DBL_MAX,-4.0,true); // do not wait for it to pause
						PlayBackEngine->Play(false);
					}
				} break;

				case 2:		
				{	// We Continue the playback
					if (PlayBackEngine) 
					{	PlayBackEngine->ChangePlaybackPosition(0,1.0,true); // do not wait for it to pause
						PlayBackEngine->Play(false);
					}

				} break;

				case 3:		
				{	// Play From

					// We Continue the playback
					if (PlayBackEngine) 
					{	PlayBackEngine->ChangePlaybackPosition(DBL_MAX,1.0,true); // do not wait for it to pause
						PlayBackEngine->Play(false);
					}

				} break;

				case 4:		
				{	// FFwd
					if (PlayBackEngine) 
					{	PlayBackEngine->ChangePlaybackPosition(DBL_MAX,4.0,true); // do not wait for it to pause
						PlayBackEngine->Play(false);
					}
				} break;

				case 5:		
				{	// Stop

					// We pause the playback
					if (PlayBackEngine) PlayBackEngine->Pause(false); // do not wait for it to pause

				} break;
			}
		}
	}
	// Check the ID
	else if (ID==(long)&m_Buttons)
	{	// Has the button been released ?
		if (!strcmp(Controls_Button_ReleasedOn,String))
		{	switch(m_Buttons)
			{	case 1:		// File button
							break;

				case 2:		// Folder button
							m_StoryBoard_View->BuildFolder();					
							break;

				case 8:		// Undo
							Undo();	
							break;

				case 9:		// Redo
							Redo();	
							break;
							
				case 4:		// Cut
							Cut();

							// We need to update the undo buffer appropriately
							SaveCurrentStateToUndoBuffer();
							
							break;

							
				case 5:		// Copy
							Copy();

							// We need to update the undo buffer appropriately
							SaveCurrentStateToUndoBuffer();

							break;

							
				case 6:		// Paste
							Paste();

							// We need to update the undo buffer appropriately
							SaveCurrentStateToUndoBuffer();

							break;

				case 7:		// Delete
							Delete();

							// We need to update the undo buffer appropriately
							SaveCurrentStateToUndoBuffer();

							break;

							
				case 3:		// Clone
							Clone();

							// We need to update the undo buffer appropriately
							SaveCurrentStateToUndoBuffer();

							// Finished
							break;

				default:	// Do nothing
							break;
			}

			// Set the state back to nonselected
			m_Buttons=0;
		}
	}
	else if (ID == (long)&m_exitInt)
	{
		if (strcmp(String,Controls_Button_ReleasedOn)==0)	
		{	m_exitInt=0;
			DeferredMessage();
		}
	}

	// Call my parent
	StretchySkinControl::DynamicCallback(ID,String,args,ItemChanged);
}

void StoryBoard::ReadClassData(FILE* fp)
{	// Read in the layers for the button states
	long up, upro, dn, dnro;
	char buffer[MAX_PATH];
	char* line = StretchyRegion::ReadNextLine(fp, buffer, MAX_PATH);
	sscanf(line, "%ld%ld%ld%ld", &up, &upro, &dn, &dnro);

	// Set up the Exit button
	m_exitInt.AddDependant(this, (long)&m_exitInt);
	SkinControl_SubControl *MyControl=OpenChild_SubControl(RGBA(54,244,94),"SkinControl_SubControl_Button");
	SkinControl_SubControl_Button *exitButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
	if (exitButton)
	{
		exitButton->Button_SetResource(Controls_Button_UnSelected, up);
		exitButton->Button_SetResource(Controls_Button_Selected, dn);
		exitButton->Button_SetResource(Controls_Button_MouseOver, upro);
		exitButton->Button_SetResource(Controls_Button_MouseOverDn, dnro);
		exitButton->Button_UseVariable(&m_exitInt);
	}
	
	// Setup the scrollbar
	HWND hWndScrollbar=OpenChild(RGBA(130,244,94),"UtilLib_Slider");
	UtilLib_Slider* scrollBar=GetWindowInterface<UtilLib_Slider>(hWndScrollbar);
	if (!scrollBar) _throw("StretchyStoryBoard::scrollBar could not case to UtilLib_Slider.");

	// Setup the click buttons
	m_Buttons=0;
	m_Buttons.AddDependant((StretchySkinControl*)this,(long)&m_Buttons);
	
	unsigned ButtonColors[] = {	RGBA(0,0,255), RGBA(30,0,255), RGBA(62,0,255), RGBA(100,0,255), RGBA(136,0,255),
								RGBA(174,0,255), RGBA(206,0,255), RGBA(239,0,255), RGBA(255,0,255) };

	for(unsigned i=0;i<11;i++)
	{	SkinControl_SubControl *MyControl=OpenChild_SubControl(ButtonColors[i],"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *l_Button=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!l_Button) _throw("StoryBoard::CutButton Could not case to SkinControl_SubControl_Button");
		l_Button->Button_SetResource(Controls_Button_UnSelected, up);
		l_Button->Button_SetResource(Controls_Button_Selected, dn);
		l_Button->Button_SetResource(Controls_Button_MouseOver, upro);
		l_Button->Button_SetResource(Controls_Button_MouseOverDn, dnro);
		l_Button->Button_SetSelectedState(i+1);
		l_Button->Button_UseVariable(&m_Buttons);
	}

	// Setup the play/stop/... buttons
	m_PlayState=0;
	m_PlayState.AddDependant(this,(long)&m_PlayState);

	unsigned PlayStateColors[]	=	{	RGBA(190,140,0), 
										RGBA(150,150,0), 
										RGBA(54,138,225), 
										RGBA(160,160,0), 
										RGBA(140,140,0) 	
									};
	unsigned PlayStateVals[]	= { 1,2,3,4,5 };

	for(i=0;i<5;i++)
	{	SkinControl_SubControl *MyControl=OpenChild_SubControl(PlayStateColors[i],"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *l_Button=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!l_Button) _throw("StoryBoard::CutButton Could not case to SkinControl_SubControl_Button");
		l_Button->Button_SetResource(Controls_Button_UnSelected, up);
		l_Button->Button_SetResource(Controls_Button_Selected, dn);
		l_Button->Button_SetResource(Controls_Button_MouseOver, upro);
		l_Button->Button_SetResource(Controls_Button_MouseOverDn, dnro);
		l_Button->Button_SetSelectedState(PlayStateVals[i]);
		l_Button->Button_UseVariable(&m_PlayState);
	}	

	// The Edit Button
		m_EditState=0;
		MyControl=OpenChild_SubControl(RGBA(155,222,152),"SkinControl_SubControl_ToggleButton");
		SkinControl_SubControl_ToggleButton *l_Button=GetInterface<SkinControl_SubControl_ToggleButton>(MyControl);
		if (!l_Button) _throw("StoryBoard::CutButton Could not case to SkinControl_SubControl_ToggleButton");
		l_Button->Button_SetResource(Controls_Button_UnSelected, up);
		l_Button->Button_SetResource(Controls_Button_Selected, dn);
		l_Button->Button_SetResource(Controls_Button_MouseOver, upro);
		l_Button->Button_SetResource(Controls_Button_MouseOverDn, dnro);
		l_Button->Button_SetSelectedState(1);
		l_Button->Button_UseVariable(&m_EditState);

	// Setup the scrollbar
		m_scrollBarVar	=0;
		m_scrollBarMin	=0;
		m_scrollBarMax	=1;
		m_scrollBarWidth=0.1f;

		// Read in the other Screen objects
		long numImages;
		line = StretchyRegion::ReadNextLine(fp, buffer, MAX_PATH);
		sscanf(line, "%ld", &numImages);

		sprintf(buffer, "%s\\", GetCurrentSkinPath());
		char	*pathEnd=buffer + strlen(buffer);
		for(i=0;i<numImages;i++)
		{	StretchyRegion::ReadNextString(fp, pathEnd, MAX_PATH-(pathEnd-buffer));
			m_sliderBitmaps.Add(new ScreenObject_BitmapFile(buffer));
			m_sliderBitmaps[i]->SetAlignment(BitmapTile_StretchX+BitmapTile_StretchY);
		}

		// Set the screen objects
		line = StretchyRegion::ReadNextLine(fp, buffer, MAX_PATH);
		sscanf(line, "%ld%ld%ld%ld", &up, &upro, &dn, &dnro);
		scrollBar->Button_SetResource(Controls_Button_UnSelected, m_sliderBitmaps[up]);
		scrollBar->Button_SetResource(Controls_Button_Selected, m_sliderBitmaps[dn]);
		scrollBar->Button_SetResource(Controls_Button_MouseOver, m_sliderBitmaps[upro]);
		scrollBar->Button_SetResource(Controls_Button_MouseOverDn, m_sliderBitmaps[dnro]);

		// Set all of the Dynamics for the scroller
		scrollBar->Slider_SetVariable(&m_scrollBarVar);
		scrollBar->Slider_SetMinVariable(&m_scrollBarMin);
		scrollBar->Slider_SetMaxVariable(&m_scrollBarMax);
		scrollBar->Slider_SetSliderWidth(&m_scrollBarWidth);

		// get the up arrow
		MyControl=OpenChild_SubControl(RGBA(184,244,94),"SkinControl_SubControl_SliderButton");
		if (!MyControl) _throw("StoryBoard::Could not create SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *UpArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (!UpArrow) _throw("StoryBoard::UpArrow Could not case to SkinControl_SubControl_SliderButton");
		UpArrow->Button_SetResource(Controls_Button_UnSelected, up);
		UpArrow->Button_SetResource(Controls_Button_Selected, dn);
		UpArrow->Button_SetResource(Controls_Button_MouseOver, upro);
		UpArrow->Button_SetResource(Controls_Button_MouseOverDn, dnro);
		UpArrow->Slider_SetVariable(&m_scrollBarVar);
		UpArrow->Slider_SetMinVariable(&m_scrollBarMin);
		UpArrow->Slider_SetMaxVariable(&m_scrollBarMax);
		UpArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
		UpArrow->Set_SliderMult(-20.0f);

		// get the down arrow
		MyControl=OpenChild_SubControl(RGBA(214,244,94),"SkinControl_SubControl_SliderButton");
		if (!MyControl) _throw("StoryBoard::Could not create SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *DnArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (!DnArrow) _throw("StoryBoard::DnArrow Could not case to SkinControl_SubControl_SliderButton");
		DnArrow->Button_SetResource(Controls_Button_UnSelected, up);
		DnArrow->Button_SetResource(Controls_Button_Selected, dn);
		DnArrow->Button_SetResource(Controls_Button_MouseOver, upro);
		DnArrow->Button_SetResource(Controls_Button_MouseOverDn, dnro);
		DnArrow->Slider_SetVariable(&m_scrollBarVar);
		DnArrow->Slider_SetMinVariable(&m_scrollBarMin);
		DnArrow->Slider_SetMaxVariable(&m_scrollBarMax);
		DnArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
		DnArrow->Set_SliderMult(20.0f);

	// Create the main window !		
		HWND hWndScrollWindow=OpenChild(RGBA(255,98,255),"StoryBoard_View");
		m_StoryBoard_View=GetWindowInterface<StoryBoard_View>(hWndScrollWindow);
		if (!m_StoryBoard_View) _throw("StoryBoard::Could not create the StoryBoard_View");

		// Setup the sliders
		m_StoryBoard_View->BaseWindowLayoutManager_Y_Slider_SetVariable(&m_scrollBarVar);
		m_StoryBoard_View->BaseWindowLayoutManager_Y_Slider_SetMaxVariable(&m_scrollBarMax);
		m_StoryBoard_View->BaseWindowLayoutManager_Y_Slider_SetSliderWidth(&m_scrollBarWidth);

		// Read the data out of the file
		m_StoryBoard_View->ReadClassData(fp);

	// This is the path bar
		HWND hStoryBoardPath=OpenChild(RGBA(228,138,255),"StoryBoardPath");		
		m_StoryBoardPath=GetWindowInterface<StoryBoardPath>(hStoryBoardPath);
		if (!m_StoryBoardPath) _throw("Could not create the StoryBoardPath in the StoryBoard");
		m_StoryBoardPath->ReadClassData(fp);

	// Make sure that we are all viewing the same thing
		SetViewPath(this);
}

void StoryBoard::InitialiseWindow(void)
{	// No SB items are open
	MyInOutWindow=NULL;

	// Open the skin'n'stuff !
	m_ScreenObject_StretchySkin=GetInterface<ScreenObject_StretchySkin>(NewTek_New("ScreenObject_StretchySkin"));
	if (!m_ScreenObject_StretchySkin) _throw("Cannot create the stretchy skin !");
	Canvas_SetResource(m_ScreenObject_StretchySkin);
	ReadScriptFile(FindFiles_FindFile(FINDFILE_SKINS,"Storyboard\\SB.txt"));
	StretchySkinControl::InitialiseWindow();

	// I need to know what is going on with my edit state
	m_EditState.AddDependant(this,(long)&m_EditState);
}

void StoryBoard::DestroyWindow(void)
{	m_EditState.DeleteDependant(this);	
	for(unsigned i=0;i<m_sliderBitmaps.NoItems;i++)
		delete m_sliderBitmaps[i];
	StretchySkinControl::DestroyWindow();
	NewTek_Delete(m_ScreenObject_StretchySkin);
}

void StoryBoard::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{
	// All we do is DIE!
	NewTek_Delete(this);
}