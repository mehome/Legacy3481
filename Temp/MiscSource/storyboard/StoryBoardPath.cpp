#include "StdAfx.h"

//**********************************************************************************************
// Add all the dependants
void StoryBoardPath::AddDependants(void)
{	for(unsigned i=0;i<GetNoChildren();i++)
	{	// Get the BWC
		BaseWindowClass *BWC=GetChild(i);

		// If there is a window, check whether it is already talking to us !
		if (!BWC->IsDependant(this)) 
			BWC->AddDependant(this,(long)BWC);
	}
}

//**********************************************************************************************
void StoryBoardPath::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	// Is it a double click ?
	if (!strcmp(String,BaseWindowClass_DNDSelect))
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
						if (SBI->GetNoChildren()) 
								DeferredMessage(StoryBoardPath_Deferred_ChangeFolder,(unsigned)SBI);

						// Finished
						return;
					}
				}

				// Finished
				break;
			}
		}
	}
}

void StoryBoardPath::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{	// Which message is it ?
	switch(ID1)
	{	// Change folder ?
		case StoryBoardPath_Deferred_ChangeFolder:
			{	// Which folder should we be changed to ?
				StoryBoard_Item	*NewFolder=(StoryBoard_Item*)ID2;

				// Get my parent
				StoryBoard *SB=GetWindowInterface<StoryBoard>(GetParent());

				// Change the path :0
				if (SB) SB->SetViewPath(NewFolder);

			} break;
	
		default:	_throw("Unknown StoryBoard_View::ReceiveDeferredMessage message !");
	}
}

//**********************************************************************************************
void StoryBoardPath::SetStoryboardToEdit(StoryBoard_Item *ITE)
{	// Determine how many parents there are
	unsigned NoParents=0;
	StoryBoard_Item *Scan=ITE;
	while(Scan)  { NoParents++; Scan=Scan->GetParent(); }
	
	// I always want the colors to start with the same sequence
	if (NoParents&1)	m_currentColor=&m_C1;
	else				m_currentColor=&m_C2;
	
	// We do not do any layouts !
	MultipleLayouts_Start();

	// Remove any previous windows that might be laying around	
	CloseAllChildren();

	// Now we create the items in reverse order ...
	Scan=ITE;
	while(Scan)
	{	// Add this item
		HWND hWnd=OpenChild("StoryBoard_Crouton_NoOwn",-100,-100);

		// Set the position to be at the top of the list
		SetWindowPos(hWnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		// Now set things up
		StoryBoard_Crouton *SC=GetWindowInterface<StoryBoard_Crouton>(hWnd);
		if (!SC) _throw("Could not create the StoryBoard_Crouton crouton");
		SC->SetStoryboardTree(Scan);
		SetChildViewMode(SC);

		// If we are the first item in the tree, select me
		SC->DragAndDrop_SelectMe(Scan==ITE);		

		// Look at the parent
		Scan=Scan->GetParent();
	}
	
	// We start by removing all the current paths
	m_StoryboardToEdit=ITE;

	// We redo the layout now
	MultipleLayouts_Finished();

	// Make sure that we have our dependants correct
	AddDependants();
}

StoryBoard_Item *StoryBoardPath::GetStoryboardToEdit(void)
{	return m_StoryboardToEdit;
}

bool StoryBoardPath::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{	return false;
}

bool StoryBoardPath::DragNDrop_ShouldIRubberBandSelect(HWND hWnd)
{	return false;
}

WindowLayout_Item *StoryBoardPath::Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height)
{	// Start at the top left of the dialog box
	long CurrentXPosn=GetWindowWidth()-1;

	// Look at all children
	WindowLayout_Item *Item=Children;
	WindowLayout_Item *lastItem = NULL;
	while(Item)
	{	
		// Reposition this item
		CurrentXPosn-=(Item->XSize+SpacingX);
		Item->XPosn=CurrentXPosn;
		Item->YPosn=0;

		// Look at the next child
		lastItem = Item;
		Item=Item->Next;
	}

	// Move all of the children back to the beginning if we are too far right
	if (lastItem)
	{
		long XOffset = lastItem->XPosn;
		if (XOffset > 0)
		{
			Item = Children;
			while(Item)
			{
				Item->XPosn -= XOffset;
				Item=Item->Next;
			}
		}
	}
	
	// There is never a change in order
	return Children;
}

StoryBoardPath::StoryBoardPath(void)
{	m_StoryboardToEdit=NULL;
}

//*********************************************************************************************************************************************
void StoryBoardPath::ReadClassData(FILE* fp)
{
	// Read in the font
	char buffer[256];
	char* line = StretchyRegion::ReadNextString(fp, buffer, 256);
	strcpy(m_Font, line);

	// Read in each of the values for the four states
	for (int i = 0; i < 4; i++)
	{
		line = StretchyRegion::ReadNextLine(fp, buffer, 256);
		long r, g, b, bold, ital, under;
		sscanf(line, "%c%ld%ld%ld%ld%ld%ld%f", &m_alignment[i], &r, &g, &b, &bold, &ital, &under, &m_Size[i]);
		m_color[i].r = (unsigned char)r;
		m_color[i].g = (unsigned char)g;
		m_color[i].b = (unsigned char)b;
		m_color[i].a = (unsigned char)255;

		m_Bold[i] = (bold != 0);
		m_Italic[i] = (ital != 0);
		m_Underline[i] = (under != 0);
	}

	// Read in the 2 different colors for button bg.
	long r,g,b;
	line = StretchyRegion::ReadNextLine(fp, buffer, 256);
	sscanf(line, "%ld%ld%ld", &r, &g, &b);
	m_C1.r = (unsigned char)r;
	m_C1.g = (unsigned char)g;
	m_C1.b = (unsigned char)b;
	m_C1.a = (unsigned char)255;
	line = StretchyRegion::ReadNextLine(fp, buffer, 256);
	sscanf(line, "%ld%ld%ld", &r, &g, &b);
	m_C2.r = (unsigned char)r;
	m_C2.g = (unsigned char)g;
	m_C2.b = (unsigned char)b;
	m_C2.a = (unsigned char)255;
}

//*********************************************************************************************************************************************
void StoryBoardPath::SetChildViewMode(FileButton* p_child)
{
	// Flip the colors
	if (m_currentColor == &m_C1) m_currentColor = &m_C2;
	else m_currentColor = &m_C1;

	long textWidth = 0;

	for (unsigned buttonState = 0; buttonState < 4; buttonState++)
	{
		TextItem* ti = p_child->GetTextItem(buttonState);
		// figure out the alignment
		long alignmentFlags = TextItem_CentreVAlign;
		switch(m_alignment[buttonState])
		{
		case 'c':
			alignmentFlags += TextItem_CentreHAlign;
			break;
		case 'r':
			alignmentFlags += TextItem_RightAlign;
			break;
		default:
			alignmentFlags += TextItem_LeftAlign;
		}

		// We do not need to send changed messages here
		ti->MessagesOff();
		ti->Set_BackgroundColor(m_currentColor->r, m_currentColor->g, m_currentColor->b);
		ti->SetAlignment(alignmentFlags);
		ti->SetFont(m_Font);
		ti->SetFontSize(m_Size[buttonState]);
		ti->SetBold(m_Bold[buttonState]);
		ti->SetItalic(m_Italic[buttonState]);
		ti->SetUnderline(m_Underline[buttonState]);
		ti->SetColor(m_color[buttonState].r, m_color[buttonState].g, m_color[buttonState].b);

		// Get the width and see if it is the longest
		long w = ti->ScreenObject_GetPreferedXSize();
		if (w > textWidth) textWidth = w;
		ti->MessagesOn();
	}

	long height = GetWindowHeight();
	long width	= ((FileButton_Size_IconSmallX * height)/FileButton_Size_IconSmallY) + textWidth + 20;
	p_child->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_SmallIcon, width, height);	
}