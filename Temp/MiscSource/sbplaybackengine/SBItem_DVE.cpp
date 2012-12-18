#include "StdAfx.h"

HandleCache_Item *SBD_Item_DVE::Render_Async(SBD_Item_Render_Buffer *Items)
{	return DriveReader_SubmitRequest(m_FileName,this,Items->GetMinimumID(),Items);
}

bool SBD_Item_DVE::Render_ReadData(SBD_Item_Render_Buffer *Items)
{	StoryBoard_DVE *MyDVE=g_DVECache_Allocate.GetDVE(m_FileName);
	if (!MyDVE) throw "SBD_Item_DVE::DoRender_Transition cannot get DVE";
	
	SBD_Item_Render_Buffer *Item=Items;
	while(Item)
	{	// Get the field number for this buffer
		int ThisFieldNo=GetLocalFieldNo(Item->GetCentreTime());
			
		// Force Asynchronous Read Initiation
		fxfh_StartFieldPreload(MyDVE->fxfh,ThisFieldNo);

		// Look at the next field
		Item=Item->GetNext();
	}

	Item=Items;
	while(Item)
	{	// Get the field number for this buffer
		int ThisFieldNo=GetLocalFieldNo(Item->GetCentreTime());

		// Read in the Field
		fxfh_LockWarpField(MyDVE->fxfh,ThisFieldNo);

		// Look at the next field
		Item=Item->GetNext();
	}

	MyDVE->UnBlock();

	return true;
}

bool SBD_Item_DVE::DoRender_Transition(SBD_Item_Info_From_To *Item,int LocalFieldNum)
{	StoryBoard_DVE *MyDVE=g_DVECache_Allocate.GetDVE(m_FileName);
	if (!MyDVE) throw "SBD_Item_DVE::DoRender_Transition cannot get DVE";
	void *warp;

	if (warp=fxfh_LockWarpField(MyDVE->fxfh,LocalFieldNum))
	{	if (fxfh_IsAVideoWarping(MyDVE->fxfh,LocalFieldNum))
		{	fxfh_ExecuteWarpField(MyDVE->fxfh,warp,Item->From->Memory,Item->To->Memory);
			Swap(Item->To->Memory,Item->From->Memory);
		}
		else	
		{	fxfh_ExecuteWarpField(MyDVE->fxfh,warp,Item->To->Memory,Item->From->Memory);			
		}

		// If the source and destination are reversed, we need to act correspondingly
		fxfh_UnlockWarpField(MyDVE->fxfh,LocalFieldNum);	
		fxfh_UnlockWarpField(MyDVE->fxfh,LocalFieldNum);
	}
	MyDVE->UnBlock();
	
	return true;
}

bool SBD_Item_DVE::Render_Transition(SBD_Item_Info_From_To *Items)
{	bool Ret=SBD_Item_Info::Render_Transition(Items);	
	return Ret;
}

SBD_Item_DVE::SBD_Item_DVE(SBD_Item_Info *Parent,char *FileName) 
:SBD_Item_Info(Parent)
{	// Setup the default settings
	SetDescription("DVE");	

	// Store the filename
	m_FileName=(char*)malloc(strlen(FileName)+1);
	strcpy(m_FileName,FileName);

	// Get a handle to a cache item
	StoryBoard_DVE *MyDVE=g_DVECache_Allocate.GetDVE(m_FileName);
	if (!MyDVE) throw "SBD_Item_DVE::DoRender_Transition cannot get DVE";

	SetOriginalLength		(MyDVE->NoFrames/MyDVE->FrameRate);
	SetOriginalFrameRate	(MyDVE->FrameRate);
	MyDVE->UnBlock();
		
	// Set the in and out points of the DVE, the following is correct.
	SetInPoint(0.0);
	SetOutPoint(m_OriginalLength/2);
	SetDuration(m_OriginalLength/4);

	// Set it as a transition, no change needed
	SetTransition(true);
}

SBD_Item_DVE::~SBD_Item_DVE(void)
{	// Free any DVE Handles that you might have lying around
	// Free(My(DVE)); :))
	if (m_FileName)	free(m_FileName);
}