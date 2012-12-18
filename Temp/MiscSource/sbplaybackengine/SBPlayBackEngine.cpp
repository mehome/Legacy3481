#include "StdAfx.h"

void StoryBoard_PlayBack::ThreadProcessor_Start(void)
{	// I must be quite high priority, but slightly lower than the rendering threads
	::SetThreadPriority(Thread::MyHandle,THREAD_PRIORITY_HIGHEST);

	// Setup the rendering IDs
	m_RenderID=0;
	m_PlayID=0;

	// The deck is currently in an unknown state !
	m_DeckState=StoryBoard_PlayBack_Unknown;
}

void StoryBoard_PlayBack::ThreadProcessor(void)
{	// If we are in pause mode, and have moved back into playmode, react corrspondingly
	Block();
	if ((m_DeckState==StoryBoard_PlayBack_Pause)&&
		(m_DeckState_Req==StoryBoard_PlayBack_Play))
	{	// Go into play mode
		m_DeckState=StoryBoard_PlayBack_Play;
	}

	// Handle a request to change position and framerates
	if (m_FP_Req)
	{	if (m_FrameNo_Req!=DBL_MAX)
		{	// Now we need to wait until all rendering is finished :(
			for(unsigned i=0;i<m_NoThreads;i++)
			{	// Wait for all rendering to be finished
				while(!RenderThreads[i]->IsReady())
					Sleep(10);

				// Get all the buffers from the renderer
				unsigned Pos;
				while(true)
				{	// Get the buffer at this position
					OutputBufferDesc Insertion;
					Insertion.Memory=RenderThreads[i]->GetBufferMemory(Pos,NULL,&Insertion.ID);

					// If there was no buffer, then we are done
					if (!Insertion.Memory) break;

					// We overwrite the i'th position
					OutputBuffers.Add(Insertion);

					// Look at the next buffer
					Pos++;
				}
			}

			// Delete ALL the buffers
			for(i=0;i<OutputBuffers.NoItems;i++)
				BufferCache_FreeBuffer(OutputBuffers[i].Memory);
			OutputBuffers.DeleteAll();

			m_FrameNo=m_FrameNo_Req;

			// Setup the rendering IDs		
			m_RenderID=0;
			m_PlayID=0;
		}

		// Change the values
		if (m_PlaybackSpeed_Req!=DBL_MAX)	m_PlaybackSpeed=m_PlaybackSpeed_Req;		

		// Request fullfilled
		m_FP_Req=false;
	}

	// Handle a change of 'item to render' ... although this one is tough !
	if (m_ItemToRender_Req!=m_ItemToRender)
	{	// Start by computing the new end position
		//m_EndPosition=m_ItemToRender_Req->Calculate();

		// Now we need to wait until all rendering is finished :(
		for(unsigned i=0;i<m_NoThreads;i++)
		{	// Wait for all rendering to be finished
			while(!RenderThreads[i]->IsReady())
				Sleep(10);

			// Get all the buffers from the renderer
			unsigned Pos;
			while(true)
			{	// Get the buffer at this position
				OutputBufferDesc Insertion;
				Insertion.Memory=RenderThreads[i]->GetBufferMemory(Pos,NULL,&Insertion.ID);

				// If there was no buffer, then we are done
				if (!Insertion.Memory) break;

				// We overwrite the i'th position
				OutputBuffers.Add(Insertion);

				// Look at the next buffer
				Pos++;
			}
		}

		// Delete ALL the buffers
		for(i=0;i<OutputBuffers.NoItems;i++)
			BufferCache_FreeBuffer(OutputBuffers[i].Memory);
		OutputBuffers.DeleteAll();

		// Setup the rendering IDs		
		m_RenderID=0;
		m_PlayID=0;
		m_FrameNo=0.0;

		// And notify whoever that we have changed
		m_ItemToRender=m_ItemToRender_Req;
	}

	UnBlock();
	
	// See if any threads have returned yet ...
	for(unsigned i=0;i<m_NoThreads;i++)
	if (RenderThreads[i]->IsReady())
	{	// We are going to capture all the buffers that it has
		unsigned Pos=0;
		while(true)
		{	// Get the buffer at this position
			OutputBufferDesc Insertion;
			Insertion.Memory=RenderThreads[i]->GetBufferMemory(Pos,NULL,&Insertion.ID);

			// If there was no buffer, then we are done
			if (!Insertion.Memory) break;
			
			// We overwrite the i'th position
			OutputBuffers.Add(Insertion);

			// Look at the next buffer
			Pos++;
		}

		// We need to verify that we have not got more lead fields than we are allowed !
		if (OutputBuffers.NoItems<m_MaximumLeadFields)
			// We can set this one off rendering again
			RenderThreads[i]->SubmitFieldsForRendering(	m_ItemToRender,m_FrameNo,
														0.5*m_PlaybackSpeed/m_FrameRate,
														FLT_MAX,&m_RenderID,&m_FrameNo);
	}

	// If we are in pause, then we do not display anything other than the first frame !
	if ((m_DeckState!=StoryBoard_PlayBack_Pause)||
		(m_RenderID==0))	// We always allow the first frame to be sent to the card ... so that the viewport is changed correctly
	{	// Play back any frames that are in the cache
		while(true)
		{	// Look for the odd and even fields with the correct ID
			for(unsigned P1=0;P1<OutputBuffers.NoItems;P1++)
			if (OutputBuffers[P1].ID==m_PlayID) break;

			// If the even field was not found, finish
			if (P1==OutputBuffers.NoItems) break;

			for(unsigned P2=0;P2<OutputBuffers.NoItems;P2++)
			if (OutputBuffers[P2].ID==m_PlayID+1) break;			

			// If the odd field was not found, finish
			if (P2==OutputBuffers.NoItems) break;
			
			// Try getting a frame on the Toaster
			void *Buf=NULL;
			long Size;
			long BufID=rtme_output_AllocFrame(m_OutputEngine,&Buf,&Size);

			// If we could not get a buffer, then we are finished on this time around
			if (!BufID) break;

			// Get access to the two fields
			long Field0Size,Field1Size;
			void *Field0=rtme_output_GetFrameFieldBuffer(m_OutputEngine,BufID,&Field0Size,0);
			void *Field1=rtme_output_GetFrameFieldBuffer(m_OutputEngine,BufID,&Field1Size,1);

			// Copy my image data into them. Release memory as early as possible
			memcpy(Field0,OutputBuffers[P1].Memory,Field0Size);
			BufferCache_FreeBuffer(OutputBuffers[P1].Memory);

			memcpy(Field1,OutputBuffers[P2].Memory,Field1Size);
			BufferCache_FreeBuffer(OutputBuffers[P2].Memory);

			// and send it.
			rtme_output_SendFrame(m_OutputEngine,BufID,Size);

			// Delete the entries from the list ...
			OutputBuffers.DeleteEntry(P1);
			OutputBuffers.DeleteEntry(P2);

			// Advance two fields
			m_PlayID+=2;

			// Has pause been hit ?
			Block();
			if (m_DeckState_Req==StoryBoard_PlayBack_Pause)
			{	m_DeckState=StoryBoard_PlayBack_Pause;
				UnBlock();

				// Stop displaying frames ...
				break;
			}
			UnBlock();
		}
	}

	// Take a nap ... it can be long since the above can dispatch many frames at one go
	Sleep(10);

#ifdef _DISPLAYINFO_3
	static unsigned SpinLoop;
	if (((++SpinLoop)&15)==0)
		DebugOutput("RealTime Playback Engine : ~%d fields ahead of video output.\n",OutputBuffers.NoItems);
#endif
}

void StoryBoard_PlayBack::RequestDeckState(unsigned NewState,bool WaitFor)
{	{	FunctionBlock a(this);
		m_DeckState_Req=NewState;
	}

	if (WaitFor)
	{	while(true)
		{	Block();
			if (GetDeckState()==NewState)
			{	UnBlock();
				break;
			}
			UnBlock();
			Sleep(10);
		}
	}
}

unsigned StoryBoard_PlayBack::GetDeckState(void)
{	FunctionBlock a(this);
	return m_DeckState;
}

StoryBoard_PlayBack::StoryBoard_PlayBack(SBD_Item_Info *Project,unsigned NoThreads,unsigned XRes,unsigned YRes,double FrameRate,unsigned MaximumLeadFields)
{	Block();

	// Get the ending position
	//m_EndPosition=Project->Calculate();

	// Setup the default variables
	m_ItemToRender_Req=
	m_ItemToRender=Project;		// We start of with both equal

	m_XRes=XRes;
	m_YRes=YRes;
	m_FrameRate=FrameRate;
	m_FieldTime=0.5/FrameRate;
	m_FieldStart=0.0;
	m_FrameNo=0.0;
	m_PlaybackSpeed=1.0;
	m_MaximumLeadFields=MaximumLeadFields;
	m_FP_Req=false;

	// Setup the deck formats
	m_DeckState=StoryBoard_PlayBack_Unknown;
	m_DeckState_Req=StoryBoard_PlayBack_Pause;

	// Allocate the RTME output engine
	unsigned Flags;
	if (YRes==240)		Flags=0; // The default
	else if (YRes==243) Flags|=RTME_OUT_CREATE_VID_IS_NTSC_486;
	else if (YRes==288) Flags|=RTME_OUT_CREATE_VID_IS_PAL;

	// Make a name for myself
	LARGE_INTEGER Timer;  
	QueryPerformanceCounter(&Timer);
	char Temp[256]; 
	sprintf(Temp,"Apps.Deliverance.StoryBoard.%dI64",Timer);
	m_OutputEngine=rtme_output_Create(Temp,0,0,Flags);
	if (!m_OutputEngine) _throw "StoryBoard_PlayBack::ThreadEntryPoint not connect to the output";
	else rtme_output_SetMaximumLeadTime(m_OutputEngine,10);

	// Stop All Input DME ...
	rtme_output_OutputToMain	(m_OutputEngine,false);
	rtme_output_OutputToPreview	(m_OutputEngine,false);
	rtme_output_OutputToKey		(m_OutputEngine,false);
	rtme_output_OutputToDownKey	(m_OutputEngine,false);

	UnBlock();

	// Start all the rendering threads
	m_NoThreads=NoThreads;
	RenderThreads=new SBD_Renderer *[NoThreads];
	if (!RenderThreads) _throw "StoryBoard_PlayBack::StoryBoard_PlayBack Cannot allocate outer array.";
	for(unsigned i=0;i<NoThreads;i++)
	{	RenderThreads[i]=new SBD_Renderer(10,XRes,YRes,FrameRate);
		if (!RenderThreads[i]) _throw "StoryBoard_PlayBack::StoryBoard_PlayBack Cannot allocate inner array.";
	}

	// Start the thread
	StartThread();	

	// We want to be in pause mode !
}

StoryBoard_PlayBack::~StoryBoard_PlayBack(void)
{	// Signal that the thread must exit and wait for it to exit
	StopThread();

	// Stop all worker threads
	for(unsigned i=0;i<m_NoThreads;i++)
		delete RenderThreads[i];

	// Clean up all remaining buffers
	for(i=0;i<OutputBuffers.NoItems;i++)
		BufferCache_FreeBuffer(OutputBuffers[i].Memory);

	// Release the RTME output engine
	if (m_OutputEngine)
			rtme_output_Delete(m_OutputEngine);
}

void StoryBoard_PlayBack::Pause(bool WaitFor)
{	// Put the deck into pause mode ...
	RequestDeckState(StoryBoard_PlayBack_Pause,WaitFor);
}

void StoryBoard_PlayBack::Play(bool WaitFor)
{	// Put the deck into pause mode ...
	RequestDeckState(StoryBoard_PlayBack_Play,WaitFor);
}

void StoryBoard_PlayBack::ChangeItemBeingViewed(SBD_Item_Info *NewProject,bool WaitFor)
{	Block();
	m_ItemToRender_Req=NewProject;
	UnBlock();

	if(WaitFor)
	{	while(true)
		{	Block();
			if (m_ItemToRender==NewProject)
			{	UnBlock();
				break;
			}
			UnBlock();
			Sleep(10);
		}
	}
}

void StoryBoard_PlayBack::ChangePlaybackPosition(double NewTime,double NewPlaybackRate,bool WaitFor)
{	Block();
	m_FrameNo_Req=NewTime;
	m_PlaybackSpeed_Req=NewPlaybackRate;
	m_FP_Req=true;
	UnBlock();

	if (WaitFor)
	{	while(true)
		{	Block();
			if (m_FP_Req==false)
			{	UnBlock();
				break;
			}
			UnBlock();
			Sleep(10);
		}
	}
}









