#include "StdAfx.h"

// File loading stuff
bool SBD_Item_RTV::Render_ReadData(SBD_Item_Render_Buffer *Items)
{	SBD_Item_Render_Buffer *Item=Items;
	while(Item)
	{	// Get the field number for this buffer
		int ThisFieldNo=GetLocalFieldNo(Item->GetCentreTime());
			
		// Load the field
		if (ThisFieldNo&1)	ReadRTVFile(m_FileName,ThisFieldNo/2,NULL,Item->Memory,NULL,NULL,false);
		else				ReadRTVFile(m_FileName,ThisFieldNo/2,Item->Memory,NULL,NULL,NULL,false);

		// Look at the next field
		Item=Item->GetNext();
	}

	return true;
}

// Setup the asynchronous file transfer request
HandleCache_Item *SBD_Item_RTV::Render_Async(SBD_Item_Render_Buffer *Items)
{	return DriveReader_SubmitRequest(m_FileName,this,Items->GetMinimumID(),Items);
}

// Constructor
SBD_Item_RTV::SBD_Item_RTV(SBD_Item_Info *p_Parent,char *FileName):SBD_Item_Info(p_Parent)
{	// Setup my copy of the filename
	m_FileName=(char*)malloc(strlen(FileName)+1);;
	if (!m_FileName) _throw "SBD_Item_Info::SBD_Item_Info cannot allocate filename";
	strcpy(m_FileName,FileName);

	// Get the RTV resolution
	bool VideoChannel,AlphaChannel,AudioChannel,IsFielded;
	unsigned SrcXRes,SrcYRes,SampleRate;
	double FrameRate;
	if (!GetRTVProperties(m_FileName,VideoChannel,SrcXRes,SrcYRes,FrameRate,IsFielded,AlphaChannel,AudioChannel,SampleRate))
		_throw "SBD_Item_Info::SBD_Item_Info Cannot get resolution.";
		
	XRes=SrcXRes;
	YRes=SrcYRes;

	// Setup the local variables
	SetOriginalFrameRate(FrameRate/2);

	// Setup the lengths, etc...
	unsigned NoFrames=ReadRTVNumberOfFrames(m_FileName);
	SetOriginalLength_Frames(NoFrames);

	// Default setups
	SetInPoint(0.0);
	SetOutPoint(m_OriginalLength);
	SetDuration(m_OriginalLength);
	SetTransition(false);
	SetDescription(m_FileName);
}

// The destructor
SBD_Item_RTV::~SBD_Item_RTV(void)
{	// Free the filename
	if (m_FileName) 
	{	free(m_FileName);
		m_FileName=NULL;
	}
}