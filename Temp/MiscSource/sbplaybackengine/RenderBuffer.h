#ifndef __RenderBuffer__
#define __RenderBuffer__

//****************************************************************************************************************************
// Global buffer acces
void SBDDLL *BufferCache_GetBuffer(unsigned Size);
void SBDDLL BufferCache_FreeBuffer(void *Buffer);

//****************************************************************************************************************************
#define SBD_Item_Render_Buffer_Stack	32

class SBDDLL SBD_Item_Render_Buffer
{	private:	// The time to render at (in frames)
				unsigned				StackPosn;
				double					TimeToRenderAt[SBD_Item_Render_Buffer_Stack][2];

				// These are stored in a linkes list
				SBD_Item_Render_Buffer	*Next[SBD_Item_Render_Buffer_Stack];

				void ResetTimes(void);

	public:		// Destination buffer information
				unsigned	XRes,YRes;	// Current X,Y resolutions
				bool		OddField;	// This is the result onto the output monitor !
				void		*Memory;	// The buffer of memoery
				unsigned	MemorySize;	// The size of the buffer

				// A user defined id number.
				LONGLONG	ID;				

				//**************
				void 	SetTime(double StartTime,double EndTime);
				double	GetStartTime(void);
				double	GetEndTime(void);
				double	GetCentreTime(void);

				void SetNext(SBD_Item_Render_Buffer	*NewNext);
				SBD_Item_Render_Buffer *GetNext(void);
				SBD_Item_Render_Buffer **GetNextRef(void);	// Use with utmost caution !

				void	Push(void);
				void	Pop(void);

				//**************
				// Copy the image contents from another buffer
				bool Copy_Image(SBD_Item_Render_Buffer *From);

				// A useful utility when you want a blank image
				void Blank_Image(void);

				//**************
				// Buffer management
				void AllocateImage(	unsigned p_XRes,unsigned p_YRes,
									double p_TimeToRenderAdd_Start,
									double p_TimeToRenderAdd_End,
									bool p_OddField,
									LONGLONG ID);
				void FreeImage(void);

				// This is used when you want to point a new buffer at another bitmap
				// but have a different time value on it ... this is crucial when traversing
				// down the tree so that you do not need to allocate (and copy) buffers on 
				// all child entries ...
				void ReferenceImage(SBD_Item_Render_Buffer *InheritFrom);

				//**************
				// Get the minimum ID in a linked list
				LONGLONG GetMinimumID(void);

				//**************
				// Constructor and destructo
				SBD_Item_Render_Buffer(void);
				~SBD_Item_Render_Buffer(void);
};


#endif