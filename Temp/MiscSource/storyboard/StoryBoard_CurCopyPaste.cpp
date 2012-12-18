#include "Stdafx.h"

// We store a current copy buffer pointer
StoryBoard_Item *StoryBoard_CopyBuffer;

void StoryBoard_Copy(tList<StoryBoard_Item*> *ItemsToCopy)
{	// If an item exists, we need to delete it
	if (!StoryBoard_CopyBuffer) StoryBoard_CopyBuffer=new StoryBoard_Item;

	// If there are items, then kill then
	if (StoryBoard_CopyBuffer->GetNoChildren()) StoryBoard_CopyBuffer->KillAll();

	// Now copy the items across
	for(unsigned int i=0;i<ItemsToCopy->NoItems;i++)
	{	StoryBoard_Item *NewChild=StoryBoard_CopyBuffer->NewStoryboardChild();
		if (!NewChild) return;
		NewChild->CopyFrom((*ItemsToCopy)[i]);
	}
}

// Copy
void StoryBoard_Copy(StoryBoard_Item *ItemToCopy)
{	// If an item exists, we need to delete it
	if (!StoryBoard_CopyBuffer) StoryBoard_CopyBuffer=new StoryBoard_Item;

	// Copy the item across
	StoryBoard_CopyBuffer->CopyFrom(ItemToCopy,true);
}

// Get the value from the current stack
StoryBoard_Item *StoryBoard_Paste(void)
{	return StoryBoard_CopyBuffer;
}

// Initialise the cut copy past buffer
void StoryBoard_Init(void)
{	StoryBoard_CopyBuffer=NULL;
}

// Clean-up the cut copy past buffer if necessary
void StoryBoard_Free(void)
{	if (StoryBoard_CopyBuffer) delete StoryBoard_CopyBuffer;
	StoryBoard_CopyBuffer=NULL;
}