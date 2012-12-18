#ifndef __SBItem__
#define __SBItem__

//****************************************************************************************************************************
// This is the temporal tolerance. It is to solve rounding issues when true frame numbers appear 'incredibly' close to the 
// correct values. 
#define SB_TemporalTolerace 1.67E-4

//****************************************************************************************************************************
// This is the maximum number of buffers that can be passed into a threaded renderer at one time
#define MaximumFieldsProcessedAtOnce 32

//****************************************************************************************************************************
class HandleCache_Item;

//****************************************************************************************************************************
// Predeclerations
class SBD_Item_Folder;
class SBD_Item_Info;

// Used by transitions !
class SBDDLL SBD_Item_Info_From_To
{	private:	// Rendering time !
				double m_RenderingTime[2];

	public:		// The "on top" source
				SBD_Item_Render_Buffer *From;

				// The "on bototm" sourve
				SBD_Item_Render_Buffer *To;

				// The linked list
				SBD_Item_Info_From_To *Next;

				// 
				void SetStartTime(double X) { m_RenderingTime[0]=X; }
				void SetEndTime  (double X) { m_RenderingTime[1]=X; }
				double GetStartTime(void)	{ return m_RenderingTime[0]; }
				double GetEndTime(void)		{ return m_RenderingTime[1]; }
				double GetCentreTime(void)	{ return 0.5*(m_RenderingTime[0] + m_RenderingTime[1]); }
};	

//****************************************************************************************************************************
class SBDDLL SBD_Item_Info
{	protected:	// These entries are user specified
				double	m_InPoint;
				double	m_OutPoint;
				double	m_StretchDuration,m_StretchMultiply;

				// These entries are filled in by the plugin
				double	m_OriginalFrameRate,m_FrameTime;
				double	m_OriginalLength;

				// Does this act as a transition ?
				bool	m_Transition;

				// My list of children
				tList2<SBD_Item_Info*>	MyChildren;

				// This is my parent
				SBD_Item_Info *Parent;

				// A short description, for debuggin purposes
				char *Description;

	public:		// Set things up
				void SetInPoint(double x)				{ m_InPoint=x; }
				void SetOutPoint(double x)				{ m_OutPoint=x; }
				void SetDuration(double x)				{ m_StretchDuration=x; m_StretchMultiply=(m_OutPoint-m_InPoint)/m_StretchDuration; }
				void SetOriginalFrameRate(double x)		{ m_OriginalFrameRate=x; m_FrameTime=1.0/m_OriginalFrameRate; }
				void SetOriginalLength(double x)		{ m_OriginalLength=x; }
				void SetOriginalLength_Frames(double x)	{ m_OriginalLength=x/m_OriginalFrameRate; }
				void SetTransition(bool Flag=false)		{ m_Transition=Flag; }
				void SetDescription(char *Name);

				//********************************************************************************
				// Utility functions
					bool IsOddField(double FieldTime);

				//********************************************************************************
				// Time functions
					int		GetLocalFieldNo(double Time);
					double	GetClipLengh(void)		  { return m_StretchDuration; }

					// Get information
					double GetInPoint(void)		{ return m_InPoint; }
					double GetOutPoint(void)	{ return m_OutPoint; }

				//********************************************************************************
				// These are the rendering functions
					// **** Multithreaded reading stuff ****
					// This is the function called from the disk reading thread.
					virtual bool Render_ReadData(SBD_Item_Render_Buffer *Items);
					
					// **** Async Rendering Support ****
					// This is the function called from the asynchronous disk reader
					virtual HandleCache_Item *Render_Async(SBD_Item_Render_Buffer *Items);

					// **** Folders / Rendering ****
					// Render this item !
					virtual bool DoRender(unsigned NoItems,SBD_Item_Render_Buffer **Items);				// If you want a linear sorted list instead of item by item, override this one
					virtual bool Render(SBD_Item_Render_Buffer *Items);									// Linked list

					// **** Transitions ****
					virtual bool DoRender_Transition(SBD_Item_Info_From_To *Item,int LocalFieldNum);	// Single entry always for transitions. You are always called in order

					// This is how transitions are rendered ...
					virtual bool Render_Transition(SBD_Item_Info_From_To *Items);						// Linked list

				//********************************************************************************
				// Constructor and destructor
				SBD_Item_Info(SBD_Item_Info *p_Parent);
				virtual ~SBD_Item_Info(void);	
				
				// This is to notify you that you have a new child
				virtual void NewChild(SBD_Item_Info *Chld) {}; // it works like an overloadeable callback

				// Called just before rendering 
				// Should return the length of the item !
				// For projects/folders, it returns the total length !
				virtual double Calculate(void) { return m_OriginalLength; };

				// I am unwanted
				friend SBD_Item_Folder;
};


#endif