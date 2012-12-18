#include "StdAfx.h"

//***************************************************************************************************************************
StoryBoard_DVE::StoryBoard_DVE(char *FN)
{	FunctionBlock a(this);

	// Store the filename
	m_FileName=(char*)malloc(strlen(FN)+1);
	strcpy(m_FileName,FN);

	// Get the FX handle
	if (fxfh=fxfh_Open(FN,0))
	{	// Get the DVE header
		RTMFStreamHeader rtms;
		fxfh_GetIconFieldBuffer(fxfh,0,&rtms);

		// Setup the framerate here ... you will probably want to pull this out of the DVE file
		// Specified in the Filename. 
		switch(rtms.type.vid.yuv_422.captured_num_scanlines)
		{	// *** NTSC ***
			case 240:
			case 243:
			default:		FrameRate=2.0*30000.0/1001.0;
							break;

			// *** PAL ***
			case 288:		FrameRate=2.0*25.0;
							break;
		}
		
		// What length is the DVE ?
		unsigned NoFields=fxfh_GetNumFields(fxfh);
		NoFrames=((NoFields>>1)&(~1))-2;		
	}
	else throw "Could not open FTFX file.";
}

StoryBoard_DVE::~StoryBoard_DVE(void)
{	FunctionBlock a(this);

	// Close things
	free(m_FileName);
}


//***************************************************************************************************************************
DVECache::DVECache(void)
{	FunctionBlock a(this);

	// Clear out the memory
	memset(m_StoryBoard_DVE,0,sizeof(StoryBoard_DVE*)*DVECache_Size);
}

DVECache::~DVECache(void)
{	FunctionBlock a(this);

	// Clean up
	for(unsigned i=0;i<DVECache_Size;i++)
	if (m_StoryBoard_DVE[i]) delete m_StoryBoard_DVE[i];
}


StoryBoard_DVE *DVECache::GetDVE(char *FN)
{	Block();

	for(unsigned i=0;i<DVECache_Size;i++)	
	if ((m_StoryBoard_DVE[i])&&(!strcmp(FN,m_StoryBoard_DVE[i]->m_FileName)))
	{	// Block it
		m_StoryBoard_DVE[i]->Block();
		// Unblock myself
		UnBlock();

		// Return the result
		return m_StoryBoard_DVE[i];
	}

	// Replace a random one ...
	unsigned ItemToReplace=rand()&(DVECache_Size-1);
	if (m_StoryBoard_DVE[ItemToReplace])
	{	m_StoryBoard_DVE[ItemToReplace]->Block();
		delete m_StoryBoard_DVE[ItemToReplace];
	}

	// Create a new one
	m_StoryBoard_DVE[ItemToReplace]=new StoryBoard_DVE(FN);

	// Block it for when we return
	m_StoryBoard_DVE[ItemToReplace]->Block();

	// Unblock myself
	UnBlock();

	// Return the result
	return m_StoryBoard_DVE[ItemToReplace];
}

DVECache g_DVECache_Allocate;











