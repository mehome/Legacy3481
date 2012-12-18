#ifndef __SBRenderer__
#define __SBRenderer__

//**********************************************************************************************************
#define SBD_Renderer_FinishedRendering (WM_USER+1973)

class SBDDLL SBD_Renderer : public WorkerThread
{	private:	// This is the rendering flag
				bool			m_Ready;
				unsigned		m_NumberOfBuffers;
				unsigned		m_XRes,m_YRes;
				unsigned		m_NoThreads;
				double			m_FrameRate;
				SBD_Item_Info	*m_ToRender;
				HWND			m_hWndToSignal;
				bool			m_UsePostMessage;

				// The buffers to use
				SBD_Item_Render_Buffer *m_Buffers[MaximumFieldsProcessedAtOnce];

				// My thread entry point
				virtual void ThreadProcessor(void);
				virtual void ThreadProcessor_Start(void);

				// Set and remove READY
				void SetReady(bool Flag);

	public:		// Submit frames for rendering
				// Note : you can only do this when ready, it will return false
				// until it can go.
				// Field numbers are in seconds, they are clled fields because you can render a field at a time !
				bool SubmitFieldsForRendering(	SBD_Item_Info *ToRender,	// The item to render
												double		StartField,		// First field to render
												double		FieldTime,		// Difference in time between fields, cool for slowmo !
												double		StopAtPosition,	// The clip ending time ...
												LONGLONG	*Id,			// This assignes a 64 bit ID to this field. It is incremented by one for each field
												double		*EndTime);		// This is an OUT value that represents the new start field after the end
																			// of this item. i.e. StartField for next render = EndTime

				// Are we idle ?
				bool IsReady(void);

				// Get a pointer to the memory of a particular buffer
				// Once you have this pointer, you MUST do a BufferCache_FreeBuffer on it.
				// The time value is the time that this buffer exists at.
				void *GetBufferMemory(unsigned i,double *Time,LONGLONG *ID);

				// This provides the rendering thread with an HWND that will be 
				// signalled. You will get a SBD_Renderer_FinishedRendering message when completed
				// wParam=(long)(SBD_Renderer*) lParam=NULL
				// Do not get yourself in a busy loop outside of the msg queue waiting for rendering to 
				// complete if you are using this. It will hang the machine
				// UsePostMessage will solve this problem ... but might send to deleted windows !
				void SetWindowSignal(HWND hWnd,bool UsePostMessage=true);

				// Constructor and destructor
				SBD_Renderer(	unsigned BufferSizes=10,			// how many fields to read at once !
								unsigned XRes=720,
								unsigned YRes=240,					// field YRes nor frame YRes !
								double FrameRate=30000.0/1001.0);	// Framerate
				~SBD_Renderer(void);
};

#endif