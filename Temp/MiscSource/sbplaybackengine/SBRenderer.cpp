#include "StdAfx.h"

//*********************************************************************************************************
void *SBD_Renderer::GetBufferMemory(unsigned i,double *Time,LONGLONG *ID)
{	// Block this thread off ...
	FunctionBlock a(this);

	// If we are not ready, then do not advance
	if (!m_Ready) return NULL;
	
	// If we have asked for more buffers than are available.
	if (i>=m_NumberOfBuffers) return NULL;
	void *Ret=m_Buffers[i]->Memory;
	
	// This buffer is no longer in existance, the user must free it !!!
	m_Buffers[i]->Memory=NULL;
	if (Time)	*Time	=m_Buffers[i]->GetStartTime();
	if (ID)		*ID		=m_Buffers[i]->ID;

	// Return the result
	return Ret;
}

//*********************************************************************************************************
void SBD_Renderer::SetWindowSignal(HWND hWnd,bool UsePostMessage)
{	m_hWndToSignal=hWnd;
	m_UsePostMessage=UsePostMessage;
}

//*********************************************************************************************************
void SBD_Renderer::ThreadProcessor_Start(void)
{	// I must be high priority !
	::SetThreadPriority(WorkerThread::MyHandle,THREAD_PRIORITY_TIME_CRITICAL);

	Block();
	m_Ready=true;
	UnBlock();
}


//*********************************************************************************************************
void SBD_Renderer::ThreadProcessor(void)
{	// While we need to run
	Block();
		
	// Is there anything to render ?
	if ((m_Ready==false)&&(m_Buffers[0]->Memory))
	{	// Allow people tov verify stuff while I am rendering
		UnBlock();
		
		// Now execute the rendering !
		if (m_ToRender) m_ToRender->Render(m_Buffers[0]);

		// Send a message to signal that the rendering has finished
		// If there is a message to signal, just do it
		Block();
		HWND l_hWnd=m_hWndToSignal;	// I do this to avoid blocking if the sendmessage takes a long time to finish
		bool l_UsePost=m_UsePostMessage;
		UnBlock();

		// Send the notification message
		if (l_hWnd) 
		{	if (l_UsePost)	PostMessage(l_hWnd,SBD_Renderer_FinishedRendering,(long)this,0);	// Safer
			else			SendMessage(l_hWnd,SBD_Renderer_FinishedRendering,(long)this,0);	// Faster
		}

		// We are now ready to go again
		Block();
		m_Ready=true;
		UnBlock();
	}
	// nothing was found, so we can sleep for a tiny bit ...
	else
	{	// Release things for others to write into
		UnBlock();

		// Take a cat nap
		Sleep(10);
	}
}

//*********************************************************************************************************
bool SBD_Renderer::IsReady(void)
{	FunctionBlock a(this);
	if (m_Ready) return true;
	return false;
}

//*********************************************************************************************************
bool SBD_Renderer::SubmitFieldsForRendering(	SBD_Item_Info	*ToRender,
												double			StartTime,
												double			TimeAdvance,
												double			StopAtPosition,
												LONGLONG		*Id,
												double			*EndTime)
{	// Block this thread off ...
	FunctionBlock a(this);

	LONGLONG l_ID=0;
	if (!Id) Id=&l_ID;

	// If we are not ready, then do not advance
	if (!m_Ready) return false;

	// We prepare all the buffers that we need
	unsigned Posn=0;
	while((StartTime<StopAtPosition)&&(Posn<m_NumberOfBuffers))
	{	// Is it an odd or even field ?
		unsigned FieldNo=NewTek_fRound(StartTime*2.0*m_FrameRate);
		bool OddField=FieldNo&1;
		
		// Allocate an image for it if necessary
		if (!m_Buffers[Posn]->Memory)
		{	m_Buffers[Posn]->AllocateImage(	m_XRes,m_YRes,
											StartTime,StartTime+TimeAdvance,
											OddField,(*Id)++);
		}
		else
		{	// Simply put the time value into the buffer
			// Note that time values are centred on the fields so that we can tolerate
			// the maximum error accumulation before we slip a field.
			m_Buffers[Posn]->SetTime(StartTime,StartTime+TimeAdvance);
			m_Buffers[Posn]->OddField=OddField;
			m_Buffers[Posn]->ID      =(*Id)++;
		}

		// Update the linked list
		m_Buffers[Posn]->SetNext(NULL);
		if (Posn) m_Buffers[Posn-1]->SetNext(m_Buffers[Posn]);

		// Increment the time
		StartTime+=TimeAdvance;
		Posn++;
	}

	// Setup the correct item to render
	m_ToRender=ToRender;
	if (EndTime) *EndTime=StartTime;
	m_Ready=false;

	return true;
}

//*********************************************************************************************************
SBD_Renderer::SBD_Renderer(unsigned NumberOfBuffers,unsigned XRes,unsigned YRes,double FrameRate)
{	// We must be thread safe
	Block();

	// Initialise things
	m_Ready=false;

	// Setup the resolutions
	m_XRes=XRes;
	m_YRes=YRes;
	m_FrameRate=FrameRate;

	// Nothing to render
	m_ToRender=NULL;

	// Noone to signal
	m_hWndToSignal=NULL;
	m_UsePostMessage=true;

	// Allocate the buffers
	m_NumberOfBuffers=NumberOfBuffers;
	for(unsigned i=0;i<NumberOfBuffers;i++)
		m_Buffers[i]=new SBD_Item_Render_Buffer;

	// We are ready to go 
	UnBlock();

	// Start up the thread
	StartThread();	
}

//*********************************************************************************************************
SBD_Renderer::~SBD_Renderer(void)
{	// Signal that the thread must exit and wait for it to exit
	StopThread();

	// Delete the buffers
	for(unsigned i=0;i<m_NumberOfBuffers;i++)
		delete m_Buffers[i];
}