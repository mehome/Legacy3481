#include "StdAfx.h"

/*
void StoryBoard2_Crouton::MouseRButtonClick(long Flags,long x,long y)
{	DeferredMessage(StoryBoard2_Crouton_Deferred_Menu);	
}
*/

void StoryBoard2_Crouton::PaintWindow(HWND hWnd,HDC hdc) {
	// Draw the standard file button stuff
	FileButton::PaintWindow(hWnd,hdc);
	}

void StoryBoard2_Crouton::InitialiseWindow() {
	Clicked=false;
	FileButton::InitialiseWindow();
	}

void StoryBoard2_Crouton::DestroyWindow(void) {
	// Remove all the dependants
	FileButton::DestroyWindow();
	}

void StoryBoard2_Crouton::MouseLButtonClick(long Flags,long x,long y) {
	// Call the base Class
	FileButton::MouseLButtonClick(Flags,x,y);
	}

void StoryBoard2_Crouton::MouseLButtonRelease(long Flags,long x,long y) {
	FileButton::MouseLButtonRelease(Flags,x,y);
	}

// Handle the mouse functions
void StoryBoard2_Crouton::MouseMoved(long Flags,long x,long y) {
	FileButton::MouseMoved(Flags,x,y);
	}

bool StoryBoard2_Crouton::DragNDrop_ShouldIDragAndDrop(HWND hWnd) {
	return true;
	}

bool StoryBoard2_Crouton::AmIFolder(void) {
	return false;
	}

// Static members for loading the crouton overlay
WindowBitmap StoryBoard2_Crouton::LocalCroutonBitmap;

void StoryBoard2_Crouton::StoryBoard_Crouton_LocalCroutonBitmap(void) {
	LocalCroutonBitmap.ReadBitmapFile(FindFiles_FindFile(FINDFILE_SKINS,StoryBoard_Crouton_FolderIcon));
	}

void StoryBoard2_Crouton::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	FileButton::DynamicCallback(ID,String,args,ItemChanging);
	}

// Constructor and destructor
StoryBoard2_Crouton::StoryBoard2_Crouton(void) {}

StoryBoard2_Crouton::~StoryBoard2_Crouton(void) {}

void StoryBoard2_Crouton::DragAndDrop_Select(bool Flag) {
	// Pass the message back to the button
	FileButton::DragAndDrop_Select(Flag);
	}

bool StoryBoard2_Crouton::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{	// If it is a storyboard icon, it can always be dropped here !
	StoryBoard2_Crouton *SC=GetWindowInterface<StoryBoard2_Crouton>(Dropped->hWnd);
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

WindowLayout_Item *StoryBoard2_Crouton::DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
								) {
	return FileButton::DragNDrop_DropItemsHere(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);
	}
