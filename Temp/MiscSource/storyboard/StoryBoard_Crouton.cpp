#include "StdAfx.h"

//******************************************************************************************************************
GUID StoryBoard_Crouton::GetGUIDOfTree(void)
{	if (GetStoryboardTree())
		return GetStoryboardTree()->GetSerialNunber();

	// We generate a GUID that cannot have been sued anywhere else
	GUID Ret;
	if (FAILED(CoCreateGuid(&Ret))) _throw("StoryBoard_Crouton::GetGUIDOfTree Cannot get guid");
	return Ret;
}

//******************************************************************************************************************
void StoryBoard_Crouton::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{	switch(ID1)
	{	case StoryBoard_Crouton_Deferred_Menu:
		{	// Get the storyboard parent
			StoryBoard *SB=GetParentStoryboard();
			if (!SB) return;
			SB->OpenMenu(this);
			return;
		}

		default:
		{	_throw("StoryBoard_Crouton::ReceiveDeferredMessage Unknown Message.");
		}
	}
}

//******************************************************************************************************************
void StoryBoard_Crouton::MouseRButtonClick(long Flags,long x,long y)
{	DeferredMessage(StoryBoard_Crouton_Deferred_Menu);	
}

//******************************************************************************************************************
void StoryBoard_Crouton::RecomputeDuration(void)
{	// Pendantic
	if (!GetStoryboardTree()) return;
	
	// We do the computations for both audio and video
	for(unsigned i=StoryBoard_Video;i<=StoryBoard_Audio;i++)
	{	// Get the length in frames of the source
		float SrcFrames=fabs(GetStoryboardTree()->GetOutPoint(i) - GetStoryboardTree()->GetInPoint(i));
		float DstFrames=GetStoryboardTree()->GetStretchCoeff(i)*SrcFrames;
		
		// Set the current duration
		StretchDuration[i].Set(DstFrames,StoryBoard_Crouton_DoNotUpdateCoeffs);
	}
}

//******************************************************************************************************************
float StoryBoard_Crouton::GetDuration(unsigned AudioOrVideo)
{	// video is the preference
	if (AudioOrVideo==StoryBoard_Video)			return StretchDuration[StoryBoard_Video];
	else if (AudioOrVideo==StoryBoard_Audio)	return StretchDuration[StoryBoard_Audio];
	else return 0;
}

//******************************************************************************************************************
void StoryBoard_Crouton::SetDuration(float Frames,unsigned Flags)
{	// Clipping
	if (Frames<1) Frames=1;
	
	// Set all the values for the correct flags
	if (Flags&StoryBoardFlag_Audio)		StretchDuration[StoryBoard_Audio].SetNM(Frames);
	if (Flags&StoryBoardFlag_Video)		StretchDuration[StoryBoard_Video].SetNM(Frames);	

	// Now send the respective change messages as required
	if (Flags&StoryBoardFlag_Audio)		StretchDuration[StoryBoard_Audio].Changed();
	if (Flags&StoryBoardFlag_Video)		StretchDuration[StoryBoard_Video].Changed();
}

//******************************************************************************************************************
StoryBoard *StoryBoard_Crouton::GetParentStoryboard(void)
{	// Scan through my parents, until I find a storyboard window
	HWND Scan=GetParent();

	// Now scan back up the tree
	while(true)
	{	// Is this of the correct type ?
		StoryBoard *SB=GetWindowInterface<StoryBoard>(Scan);
		if (SB) return SB;

		// Look at the parent
		Scan=::GetParent(Scan);
	}

	// No go !
	return NULL;
}

//******************************************************************************************************************
bool StoryBoard_Crouton::CopyToUndoBuffer(void)
{	StoryBoard *SB=GetParentStoryboard();
	if (SB) 
	{	SB->SaveCurrentStateToUndoBuffer();
		return true;
	}
	else return false;
}

//******************************************************************************************************************
bool IsCapsLockPressed(void)
{	if ((GetAsyncKeyState(VK_MENU)&((short)(1<<15)))) return true;
	BYTE keyState[256];
	GetKeyboardState((LPBYTE)&keyState);
	return keyState[VK_CAPITAL]&1;
}

//******************************************************************************************************************
void StoryBoard_Crouton::PaintWindow(HWND hWnd,HDC hdc)
{	// Draw the standard file button stuff
	FileButton::PaintWindow(hWnd,hdc);

	// If the clip has a duration !
	if (GetViewMode()==FileButton_ViewMode_LargeIcon)
	{	// Display the audio In/Out points
		if ((Flags&StoryBoardFlag_Audio)&&(TotalClipLength[StoryBoard_Audio]))
		{	// Compute the in and out points for the bar
			float StartPosnX=	(float)(GetWindowWidth()-2*FileButton_EdgeWidth)*InPoint[StoryBoard_Audio]/
								(float)TotalClipLength[StoryBoard_Audio];
			float EndPosnX=		(float)(GetWindowWidth()-2*FileButton_EdgeWidth)*OutPoint[StoryBoard_Audio]/
								(float)TotalClipLength[StoryBoard_Audio];

			// The in and out points
			LastIn[StoryBoard_Audio]	=InPoint[StoryBoard_Audio];
			LastOut[StoryBoard_Audio]	=OutPoint[StoryBoard_Audio];

			// Is it reversed ?
			bool Reversed=EndPosnX<StartPosnX;

			// Now draw the in and out points of this clip
			NewTek_DrawRectangle(	hdc,min(StartPosnX,EndPosnX)+FileButton_EdgeWidth,FileButton_EdgeWidth+StoryBoard_Crouton_AudioOffSet,
									max(StartPosnX,EndPosnX)+FileButton_EdgeWidth,StoryBoard_Crouton_InOutHeight+FileButton_EdgeWidth+StoryBoard_Crouton_AudioOffSet,
									Reversed?VideoPreviewWindow_InOut_2_Audio:VideoPreviewWindow_OutIn_1_Audio);
		}
		
		// Display the video In/Out points
		if ((Flags&StoryBoardFlag_Video)&&(TotalClipLength[StoryBoard_Video]))
		{	// Compute the in and out points for the bar
			float StartPosnX=	(float)(GetWindowWidth()-2*FileButton_EdgeWidth)*InPoint[StoryBoard_Video]/
								(float)TotalClipLength[StoryBoard_Video];
			float EndPosnX=		(float)(GetWindowWidth()-2*FileButton_EdgeWidth)*OutPoint[StoryBoard_Video]/
								(float)TotalClipLength[StoryBoard_Video];

			// The in and out points
			LastIn[StoryBoard_Video]	=InPoint[StoryBoard_Video];
			LastOut[StoryBoard_Video]	=OutPoint[StoryBoard_Video];

			// Is it reversed ?
			bool Reversed=EndPosnX<StartPosnX;

			// Now draw the in and out points of this clip
			NewTek_DrawRectangle(	hdc,min(StartPosnX,EndPosnX)+FileButton_EdgeWidth,FileButton_EdgeWidth,
									max(StartPosnX,EndPosnX)+FileButton_EdgeWidth,StoryBoard_Crouton_InOutHeight+FileButton_EdgeWidth,
									Reversed?VideoPreviewWindow_InOut_2_Video:VideoPreviewWindow_OutIn_1_Video);
		}
	}
}

//******************************************************************************************************************
void StoryBoard_Crouton::InitialiseWindow(void)
{	Clicked=false;

	// Set up the default stuff for the text item
	AliasName.SetFont("MS Sans Serif");
	AliasName.SetBold(true);
	AliasName.SetAlignment(TextItem_CentreHAlign|TextItem_CentreVAlign);

	// Set all the dependants
	TotalClipLength[0].AddDependant(this,(long)&TotalClipLength[0]);
	TotalClipLength[1].AddDependant(this,(long)&TotalClipLength[1]);
	InPoint[0].AddDependant(this,(long)&InPoint[0]);
	InPoint[1].AddDependant(this,(long)&InPoint[1]);
	OutPoint[0].AddDependant(this,(long)&OutPoint[0]);
	OutPoint[1].AddDependant(this,(long)&OutPoint[1]);
	CroutonPoint[0].AddDependant(this,(long)&CroutonPoint[0]);
	CroutonPoint[1].AddDependant(this,(long)&CroutonPoint[1]);
	OffSetPoint[0].AddDependant(this,(long)&OffSetPoint[0]);
	OffSetPoint[1].AddDependant(this,(long)&OffSetPoint[1]);
	StretchDuration[0].AddDependant(this,(long)&StretchDuration[0]);
	StretchDuration[1].AddDependant(this,(long)&StretchDuration[1]);
	OriginalFrameRate[0].AddDependant(this,(long)&OriginalFrameRate[0]);
	OriginalFrameRate[1].AddDependant(this,(long)&OriginalFrameRate[1]);
	Flags.AddDependant(this,(long)&Flags);
	AliasName.AddDependant(this,(long)&AliasName);	
}

void StoryBoard_Crouton::DestroyWindow(void)
{	// Remove all the dependants
	TotalClipLength[0].DeleteDependant(this);
	TotalClipLength[1].DeleteDependant(this);
	InPoint[0].DeleteDependant(this);
	InPoint[1].DeleteDependant(this);
	OutPoint[0].DeleteDependant(this);
	OutPoint[1].DeleteDependant(this);
	CroutonPoint[0].DeleteDependant(this);
	CroutonPoint[1].DeleteDependant(this);
	OffSetPoint[0].DeleteDependant(this);
	OffSetPoint[1].DeleteDependant(this);
	StretchDuration[0].DeleteDependant(this);
	StretchDuration[1].DeleteDependant(this);
	OriginalFrameRate[0].DeleteDependant(this);
	OriginalFrameRate[1].DeleteDependant(this);
	Flags.DeleteDependant(this);
	AliasName.DeleteDependant(this);
	FileButton::DestroyWindow();
}

//******************************************************************************************************************
void StoryBoard_Crouton::MouseLButtonClick(long Flags,long x,long y)
{	if (!IsCapsLockPressed()) FileButton::MouseLButtonClick(Flags,x,y);
	
	// Capture mouse input
	Clicked=true;
	ExclusiveInput(true);

	// Hide the mouse cursor
	ShowCursor(false);

	// Get the mouse position
	GetCursorPos(&pt);

	// Check whether we are modifying the in our the outpoint
	ModifyingInPoint=(x<=GetWindowWidth()/2);

	// Call the base Class
	FileButton::MouseLButtonClick(Flags,x,y);
}

//******************************************************************************************************************
void StoryBoard_Crouton::MouseLButtonRelease(long Flags,long x,long y)
{	if (Clicked)
	{	// Set the mouse position
		// Move back to the original position
		SetCursorPos(pt.x,pt.y);	
		
		// Capture mouse input
		Clicked=false;
		ExclusiveInput(false);

		// Show the mouse cursor
		ShowCursor(true);

		// Copy the current state to the undo buffer since things have changed
		CopyToUndoBuffer();
	}
	else if (!IsCapsLockPressed()) FileButton::MouseMoved(Flags,x,y);
}

//******************************************************************************************************************
// Handle the mouse functions
void StoryBoard_Crouton::MouseMoved(long Flags,long x,long y)
{	// Are we dragging ?
	if (Clicked)
	{	// How far have we moved ?
		POINT Newpt;
		GetCursorPos(&Newpt);
		int dx=Newpt.x-pt.x;

		// Move back to where the mouse was clicked
		SetCursorPos(pt.x,pt.y);

		// Should we only shift the audio ?
		bool OnlyShiftAudio=Flags&MK_CONTROL;

		//**** We lock all the variables ! *********************************************************
		InPoint [StoryBoard_Video].MessagesOff(); 
		OutPoint[StoryBoard_Video].MessagesOff();
		InPoint [StoryBoard_Audio].MessagesOff(); 
		OutPoint[StoryBoard_Audio].MessagesOff();

		//**** AUDIO *******************************************************************************
			// Check that there is actually a clip to edit !
			if (TotalClipLength[StoryBoard_Audio])
			{	// Work out which point to modify
				bool EditInPoint =ModifyingInPoint;
				bool EditOutPoint=!ModifyingInPoint;
				bool Changed=false;

				// If shift,ctrl is pressed we are shifting both points
				if (Flags&MK_SHIFT) EditInPoint=EditOutPoint=true;
				
				// Move the in point
				if (EditInPoint)
				{	// Compute and clip the inpoint
					InPoint[StoryBoard_Audio]=InPoint[StoryBoard_Audio]+dx;
				}

				// Move the out point
				if (EditOutPoint)
				{	// Compute and clip the inpoint
					OutPoint[StoryBoard_Audio]=OutPoint[StoryBoard_Audio]+dx;
				}
			}

		//**** VIDEO *******************************************************************************
			// Check that there is actually a clip to edit !
			if ((TotalClipLength[StoryBoard_Video])&&(!OnlyShiftAudio))
			{	// Work out which point to modify
				bool EditInPoint =ModifyingInPoint;
				bool EditOutPoint=!ModifyingInPoint;
				bool Changed=false;

				// If shift,ctrl is pressed we are shifting both points
				if (Flags&MK_SHIFT) EditInPoint=EditOutPoint=true;
				
				// Move the in point
				if (EditInPoint)
				{	// Compute and clip the inpoint
					InPoint[StoryBoard_Video]=InPoint[StoryBoard_Video]+dx;
				}

				// Move the out point
				if (EditOutPoint)
				{	// Compute and clip the inpoint
					OutPoint[StoryBoard_Video]=OutPoint[StoryBoard_Video]+dx;
				}
			}

		//**** We now flush the queue of messages **************************************************
		InPoint [StoryBoard_Video].MessagesOn(true); 
		OutPoint[StoryBoard_Video].MessagesOn(true); 
		InPoint [StoryBoard_Audio].MessagesOn(true); 
		OutPoint[StoryBoard_Audio].MessagesOn(true); 
	}
	// Handle hte mouse cursor
	else if (!IsCapsLockPressed())
		FileButton::MouseMoved(Flags,x,y);	
	else
	{	// Select the correct mouse cursor
		if (x>GetWindowWidth()/2) 
				::SetCursor(LoadCursor(h_sb_HINSTANCE,MAKEINTRESOURCE(IDC_OUTPOINTER)));
		else	::SetCursor(LoadCursor(h_sb_HINSTANCE,MAKEINTRESOURCE(IDC_INPOINTER)));
	}
}

//******************************************************************************************************************
bool StoryBoard_Crouton::DragNDrop_ShouldIDragAndDrop(HWND hWnd)
{	return !IsCapsLockPressed();
}

//******************************************************************************************************************
bool StoryBoard_Crouton::AmIFolder(void)
{	return !(GetStoryboardTree()->GetFileName());
}

//******************************************************************************************************************
// Static members for loading the crouton overlay
WindowBitmap StoryBoard_Crouton::LocalCroutonBitmap;
void StoryBoard_Crouton::StoryBoard_Crouton_LocalCroutonBitmap(void)
{	LocalCroutonBitmap.ReadBitmapFile(FindFiles_FindFile(FINDFILE_SKINS,StoryBoard_Crouton_FolderIcon));
}

//******************************************************************************************************************
void StoryBoard_Crouton::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	// Filter out a lot of crap
	if (IsWindowMessage(String)) return;

	// Has the parent of this item changed ?
	if (!strcmp(String,BaseWindowClass_ParentChanged))
	{	// Here we need to modify the parent of this item
		if (m_SubTree) m_SubTree->ChangeParent(NULL);
	}

	// The alias name has been changed
	else if (ID==(long)&AliasName)
	{	// We change the filebutton alias name
		if (!strlen(AliasName.GetText())) 
				RenameAlias(NULL);
		else	RenameAlias(AliasName.GetText());

		// Set the name in the storyboard tree
		if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetAlias(AliasName.GetText());
		}
	}

	// We need to update all changes into my storyboard item as requested

	// These two need the view to be repainted if they have changed.
	else if (ID==(long)&InPoint[StoryBoard_Video])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetInPoint(InPoint[StoryBoard_Video],StoryBoard_Video);

			// If we are not in the right icon mode, then there is no need to update the window
			if (GetViewMode()!=FileButton_ViewMode_LargeIcon) return;

			// If the points have changed, then we must force a repaint !
			if ( (LastIn[StoryBoard_Video] !=InPoint [StoryBoard_Video]) || 
				 (LastOut[StoryBoard_Video]!=OutPoint[StoryBoard_Video]) ||
				 (LastIn[StoryBoard_Audio] !=InPoint [StoryBoard_Audio]) || 
				 (LastOut[StoryBoard_Audio]!=OutPoint[StoryBoard_Audio]))	
						RePaint();
		}
	}

	else if (ID==(long)&OutPoint[StoryBoard_Video])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetOutPoint(OutPoint[StoryBoard_Video],StoryBoard_Video);

			// If we are not in the right icon mode, then there is no need to update the window
			if (GetViewMode()!=FileButton_ViewMode_LargeIcon) return;

			// If the points have changed, then we must force a repaint !
			if ( (LastIn[StoryBoard_Video] !=InPoint [StoryBoard_Video]) || 
				 (LastOut[StoryBoard_Video]!=OutPoint[StoryBoard_Video]) ||
				 (LastIn[StoryBoard_Audio] !=InPoint [StoryBoard_Audio]) || 
				 (LastOut[StoryBoard_Audio]!=OutPoint[StoryBoard_Audio]))	
						RePaint();
		}
	}

	else if (ID==(long)&InPoint[StoryBoard_Audio])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetInPoint(InPoint[StoryBoard_Audio],StoryBoard_Audio);

			// If we are not in the right icon mode, then there is no need to update the window
			if (GetViewMode()!=FileButton_ViewMode_LargeIcon) return;

			// If the points have changed, then we must force a repaint !
			if ( (LastIn[StoryBoard_Video] !=InPoint [StoryBoard_Video]) || 
				 (LastOut[StoryBoard_Video]!=OutPoint[StoryBoard_Video]) ||
				 (LastIn[StoryBoard_Audio] !=InPoint [StoryBoard_Audio]) || 
				 (LastOut[StoryBoard_Audio]!=OutPoint[StoryBoard_Audio]))	
						RePaint();
		}
	}

	else if (ID==(long)&OutPoint[StoryBoard_Audio])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetOutPoint(OutPoint[StoryBoard_Audio],StoryBoard_Audio);

			// If we are not in the right icon mode, then there is no need to update the window
			if (GetViewMode()!=FileButton_ViewMode_LargeIcon) return;

			// If the points have changed, then we must force a repaint !
			if ( (LastIn[StoryBoard_Video] !=InPoint [StoryBoard_Video]) || 
				 (LastOut[StoryBoard_Video]!=OutPoint[StoryBoard_Video]) ||
				 (LastIn[StoryBoard_Audio] !=InPoint [StoryBoard_Audio]) || 
				 (LastOut[StoryBoard_Audio]!=OutPoint[StoryBoard_Audio]))	
						RePaint();
		}
	}

	// These are the items that do not actually reflect the current position in the icon
	else if (ID==(long)&CroutonPoint[StoryBoard_Video])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetCroutonFrame(CroutonPoint[StoryBoard_Video],StoryBoard_Video);
		}
	}
	else if (ID==(long)&OffSetPoint[StoryBoard_Video])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetOffSet(OffSetPoint[StoryBoard_Video],StoryBoard_Video);
		}
	}
	else if (ID==(long)&StretchDuration[StoryBoard_Video])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			bool RecomputeCoeffs=strcmp(StoryBoard_Crouton_DoNotUpdateCoeffs,String);
			GetStoryboardTree()->SetStretchDuration(StretchDuration[StoryBoard_Video],StoryBoard_Video,RecomputeCoeffs);
		}
	}
	else if (ID==(long)&CroutonPoint[StoryBoard_Audio])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetCroutonFrame(CroutonPoint[StoryBoard_Audio],StoryBoard_Audio);
		}
	}
	else if (ID==(long)&OffSetPoint[StoryBoard_Audio])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetOffSet(OffSetPoint[StoryBoard_Audio],StoryBoard_Audio);
		}
	}
	else if (ID==(long)&StretchDuration[StoryBoard_Audio])
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			bool RecomputeCoeffs=strcmp(StoryBoard_Crouton_DoNotUpdateCoeffs,String);
			GetStoryboardTree()->SetStretchDuration(StretchDuration[StoryBoard_Audio],StoryBoard_Audio,RecomputeCoeffs);
		}
	}
	else if (ID==(long)&Flags)
	{	if (GetStoryboardTree())
		{	// Update the storyboard with the item
			GetStoryboardTree()->SetFlags(Flags);
		}
	}
	// Call my parent
	else  FileButton::DynamicCallback(ID,String,args,ItemChanged);
}

//******************************************************************************************************************
StoryBoard_Item *StoryBoard_Crouton::GetStoryboardTree(void)
{	return m_SubTree;
}

//******************************************************************************************************************
// Use an existing storyboard tree
void StoryBoard_Crouton::SetStoryboardTree(StoryBoard_Item *Tree,bool ObserverSelected)
{	// If the tree has no parent, it is sitting in a file bin, or somewhere else,
	// so it does not do anything outside of the crouton. As a result it should be
	// deleted
	if ((m_SubTree)&&(!m_SubTree->GetParent())) NewTek_Delete(m_SubTree);
	
	// Assign this tree !
	m_SubTree=Tree;

	// If the subtree exists, then we must adopt its filename !
	if (m_SubTree) 
	{	// Set the local parameters
		Zero=0;
		for(unsigned i=StoryBoard_Video;i<=StoryBoard_Audio;i++)
		{	TotalClipLength[i]		=m_SubTree->GetClipDuration(i);
			NveTotalClipLength[i]	=-m_SubTree->GetClipDuration(i);
			StretchDuration[i]		=m_SubTree->GetStretchDuration(i);
			OriginalFrameRate[i]	=m_SubTree->GetOriginalFrameRate(StoryBoard_Video);

			InPoint[i]=m_SubTree->GetInPoint(i);
			InPoint[i].SetMin(&Zero);
			InPoint[i].SetMax(&TotalClipLength[i]);

			OutPoint[i]=m_SubTree->GetOutPoint(i);
			OutPoint[i].SetMin(&Zero);
			OutPoint[i].SetMax(&TotalClipLength[i]);

			CroutonPoint[i]=m_SubTree->GetCroutonFrame(i);
			CroutonPoint[i].SetMin(&Zero);
			CroutonPoint[i].SetMax(&TotalClipLength[i]);

			OffSetPoint[i]=m_SubTree->GetOffSet(i);
			OffSetPoint[i].SetMin(&NveTotalClipLength[i]);
			OffSetPoint[i].SetMax(&TotalClipLength[i]);
		}		
		Flags=m_SubTree->GetFlags();

		// Get the filename and alias
		char FileName[MAX_PATH],Alias[MAX_PATH];
		GetFileName(FileName);
		GetAlias(Alias);		

		if ((!m_SubTree->GetFileName())||
			(strcmp(FileName,m_SubTree->GetFileName())))
		{	// We change my filename to agree with the filename of the SBI
			ChangeFileName(m_SubTree->GetFileName());
			RenameAlias(m_SubTree->GetAlias());
		}

		AliasName.SetText(m_SubTree->GetAlias());

		// Assign the selection status
		if (ObserverSelected)	DragAndDrop_SelectMe(m_SubTree->GetSelected());		
		else					m_SubTree->SetSelected(false);
	}
}

//******************************************************************************************************************
// Constructor and destructor
StoryBoard_Crouton::StoryBoard_Crouton(void)
{	m_SubTree=NULL;
	LastIn[StoryBoard_Video]=LastOut[StoryBoard_Video]=FLT_MAX;
	LastIn[StoryBoard_Audio]=LastOut[StoryBoard_Audio]=FLT_MAX;
}

StoryBoard_Crouton::~StoryBoard_Crouton(void)
{	SetStoryboardTree(NULL);
}

//******************************************************************************************************************
void StoryBoard_Crouton::DragAndDrop_Select(bool Flag)
{	// Pass the message back to the button
	FileButton::DragAndDrop_Select(Flag);

	// I select my child
	m_SubTree->SetSelected(Flag);
}

//******************************************************************************************************************
bool StoryBoard_Crouton::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{	// If it is a storyboard icon, it can always be dropped here !
	StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(Dropped->hWnd);
	FileButton* thisButton=GetWindowInterface<FileButton>(Dropped->hWnd);
	
	// If this is a file button, we might allow it to be dropped here	
	if ((SC)||((thisButton)&&(thisButton->GetStatus()==FileButton_File)))
	{	// We need to look at what the children are
		if (AmIFolder()) 
		{	// Where is the mouse
			POINT pt; GetCursorPos(&pt);
			RECT rect; GetWindowRect(GetWindowHandle(),&rect);

			// Is it to either side of the window ?
			if (pt.x>rect.right-StoryBoard_Crouton_DragWidth)	return false;
			if (pt.x<rect.left+StoryBoard_Crouton_DragWidth)	return false;

			// Success, you can drop into me !
			return true;
		}
	}		

	// No, You cannot drop things into me I am afraid
	return false;
}

//******************************************************************************************************************
WindowLayout_Item *StoryBoard_Crouton::DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
								)
{	// Is the item being dropped ?
	if (DroppedHere)
	{	// Cycle through all the items
		WindowLayout_Item *Item=ObjectsDropped;

		while(Item)
		{	// The item has to have the correct filenames, etc...
			BaseWindowClass		*BWC=GetWindowInterface<BaseWindowClass>(Item->hWnd);
			FileButton			*FB=GetInterface<FileButton>(BWC);
			StoryBoard_Crouton	*SC=GetInterface<StoryBoard_Crouton>(BWC);

			if (SC) 
			{	// Add this item as a child of myself
				SC->GetStoryboardTree()->ChangeParent(GetStoryboardTree());
			}
			else if (FB)
			{	// We insert these items as my children !
				StoryBoard_Item *SCI=GetStoryboardTree()->NewStoryboardChild();
			
				// We copy stuff across
				char Temp[MAX_PATH];
				FB->GetFileName(Temp);
				SCI->SetFileName(Temp);
				FB->GetAlias(Temp);
				SCI->SetAlias(Temp);
			}

			// Delete the item now !
			NewTek_Delete(BWC);

			// Look At the next item ...
			Item=Item->Next;
		}

		// Clean up any remaining memory
		WindowLayout WindowLayoutManager;
		WindowLayoutManager.FreeLayout(ObjectsDropped);

		// Try to get a parent
		BaseWindowLayoutManager *BWLM=GetWindowInterface<BaseWindowLayoutManager>(GetParent());
		if (BWLM) BWLM->Layout_PerformLayout(GetParent());

		// Here we refresh the updates if we are in a StoryBoard window (which we usually are !)
		BaseWindowClass *BWC_Parent=GetWindowInterface<BaseWindowClass>(GetParent());
		BaseWindowClass *BWC_Parent2=BWC_Parent->GetParentBWC();
		
		// Try casting it to a SBoard
		StoryBoard		*SB=GetInterface<StoryBoard>(BWC_Parent2);

		// Ask it to update itself if necessary
		if (SB)
		{	// Ask it to update its children
			if (GetStoryboardTree()==SB->GetItemBeingEdited()) 
				SB->UpdateCurrentView();

			// The original item needs to be put into the undo buffer
			SB->SaveCurrentStateToUndoBuffer();
		}

		// The result
		return ObjectsInWindow;
	}
	else
	{	// we move all the items off into nowhere
		WindowLayout_Item *Item=ObjectsDropped;

		while(Item)
		{	// Set it all up
			WindowLayout_Item *Next=Item->Next;

			Item->XPosn=-Item->XSize;
			Item->YPosn=-Item->YSize;
			Item->Next=ObjectsInWindow;
			ObjectsInWindow=Item;


			// Look at the next item on the list
			Item=Next;
		}
		
		// We are dragging into the position
		return ObjectsInWindow;
	}
}

void StoryBoard_Crouton::ParentChanged(HWND From,HWND To)
{	// I no longer belong to the same parent !
	GetStoryboardTree()->ChangeParent(NULL);
}

//*****************************************************************************
StoryBoard_Crouton_NoOwn::~StoryBoard_Crouton_NoOwn(void)
{	m_SubTree=NULL;
}

