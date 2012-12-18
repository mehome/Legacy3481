#include "StdAfx.h"

//****************************************************************************************************************************
class BufferCache_Buffer
{	public:			static VariableBlock		g_BufferCache_Buffer_Block;	// Could be problem !
					static BufferCache_Buffer	g_BufferCache_Buffer_Root;
					static unsigned				g_NoBuffers;
					static unsigned				g_Memory;

					BufferCache_Buffer *Next;
					BufferCache_Buffer *NextAll;
					unsigned Size;

					// The data !
					char Memory[0];
};

VariableBlock		BufferCache_Buffer::g_BufferCache_Buffer_Block;
BufferCache_Buffer	BufferCache_Buffer::g_BufferCache_Buffer_Root;
unsigned			BufferCache_Buffer::g_NoBuffers;
unsigned			BufferCache_Buffer::g_Memory;

void *BufferCache_GetBuffer(unsigned Size)
{	FunctionBlock MyBlock(&BufferCache_Buffer::g_BufferCache_Buffer_Block);

	// We always round the size up to a 512 byte boundary !
	Size=(Size+511)&(~511);

	// Scan through all the items looking for the bext fit
	unsigned			dClosestFit=INT_MAX;
	BufferCache_Buffer	*ClosestFit=NULL;
	BufferCache_Buffer	**PrevFit  =NULL;

	BufferCache_Buffer	*Scan =BufferCache_Buffer::g_BufferCache_Buffer_Root.Next;
	BufferCache_Buffer	**Prev=&BufferCache_Buffer::g_BufferCache_Buffer_Root.Next;
	while(Scan)
	{	// Check th size
		if (Scan->Size>=Size)
		{	unsigned dFit=Scan->Size-Size;
			if (dFit<dClosestFit)
			{	// Get the fit !
				dClosestFit=dFit;
				ClosestFit=Scan;
				PrevFit=Prev;

				// Exact fit
				if (!dClosestFit) break;
			}
		}

		// Look at the next item
		Prev=&Scan->Next;
		Scan=Scan->Next;
	}

	// A buffer was found
	if (ClosestFit) 
	{	*PrevFit=ClosestFit->Next;
		ClosestFit->Next=NULL;	// Pedantic
		return ClosestFit->Memory;
	}

	// Allocate a new memory unit !
	BufferCache_Buffer *NewBuffer=(BufferCache_Buffer*)VirtualAlloc(NULL,Size+sizeof(BufferCache_Buffer),MEM_COMMIT,PAGE_READWRITE);
	NewBuffer->Size=Size;
	NewBuffer->Next=NULL;

	NewBuffer->NextAll=BufferCache_Buffer::g_BufferCache_Buffer_Root.NextAll;
	BufferCache_Buffer::g_BufferCache_Buffer_Root.NextAll=NewBuffer;

	BufferCache_Buffer::g_NoBuffers++;
	BufferCache_Buffer::g_Memory+=Size;

#ifdef _DISPLAYINFO
	DebugOutput("Buffer Allocator : Size:%d, %d Buffers, %d bytes used\n",Size,BufferCache_Buffer::g_NoBuffers,BufferCache_Buffer::g_Memory);
#endif

	// Crikey !
	return NewBuffer->Memory;
}

void BufferCache_FreeBuffer(void *Buffer)
{	FunctionBlock MyBlock(&BufferCache_Buffer::g_BufferCache_Buffer_Block);
	BufferCache_Buffer *Item=(BufferCache_Buffer*)((byte*)Buffer - sizeof(BufferCache_Buffer));
	Item->Next=BufferCache_Buffer::g_BufferCache_Buffer_Root.Next;
	BufferCache_Buffer::g_BufferCache_Buffer_Root.Next=Item;
}

void g_BufferCache_Init(bool Init)
{	if (Init)
	{	
#ifdef _DISPLAYINFO
	DebugOutput("Buffer Allocator : Initialise Buffers !\n");
#endif		
		FunctionBlock MyBlock(&BufferCache_Buffer::g_BufferCache_Buffer_Block);
		BufferCache_Buffer::g_BufferCache_Buffer_Root.Next   =NULL;
		BufferCache_Buffer::g_BufferCache_Buffer_Root.NextAll=NULL;
		BufferCache_Buffer::g_NoBuffers=0;
		BufferCache_Buffer::g_Memory=0;
	}
	else
	{	
#ifdef _DISPLAYINFO
	DebugOutput("Buffer Allocator : Finished With Buffers !\n");
#endif				
		FunctionBlock MyBlock(&BufferCache_Buffer::g_BufferCache_Buffer_Block);
		BufferCache_Buffer *Scan=BufferCache_Buffer::g_BufferCache_Buffer_Root.NextAll;
		Scan=BufferCache_Buffer::g_BufferCache_Buffer_Root.NextAll=NULL;	// Pedantic
		while(Scan)
		{	// Store the next entry
			BufferCache_Buffer *Next=Scan->Next;

			// Free the memory
			VirtualFree(Scan,0,MEM_DECOMMIT);

			// Look at the next entry
			Scan=Next;
		}
	}
}

//****************************************************************************************************************************
bool SBD_Item_Render_Buffer::Copy_Image(SBD_Item_Render_Buffer *From)
{	// Check the resolutions
	if ((XRes!=From->XRes)||(YRes!=From->YRes)) return false;

	// Simply copy the memory across
	memcpy(Memory,From->Memory,XRes*YRes*2);

	// Success
	return true;
}

//****************************************************************************************************************************
SBD_Item_Render_Buffer::SBD_Item_Render_Buffer(void)
{	// No memory has been set up yet
	Memory=NULL;
	MemorySize=0;
	ResetTimes();
}

//****************************************************************************************************************************
SBD_Item_Render_Buffer::~SBD_Item_Render_Buffer(void)
{	FreeImage();
}

//****************************************************************************************************************************
void SBD_Item_Render_Buffer::AllocateImage(	unsigned p_XRes,
											unsigned p_YRes,
											double p_TimeToRenderAdd_Start,
											double p_TimeToRenderAdd_End,
											bool p_OddField,
											LONGLONG p_ID)
{	// Should we free an image, or can we reuse the buffer
	if ((!Memory)||(p_XRes!=XRes)||(p_YRes!=YRes)) FreeImage();

	// Allocate a new image
	if (!Memory) 
	{	Memory=BufferCache_GetBuffer(p_XRes*p_YRes*2);
		MemorySize=p_XRes*p_YRes*2;
	}

	// Setup the resolutions
	ResetTimes();
	XRes=p_XRes;
	YRes=p_YRes;	
	SetTime(p_TimeToRenderAdd_Start,p_TimeToRenderAdd_End);
	OddField=p_OddField;
	ID=p_ID;
		
}

//****************************************************************************************************************************
void SBD_Item_Render_Buffer::FreeImage(void)
{	// No image to clear
	if (!Memory) return;

	// Is it referenced ?
	BufferCache_FreeBuffer(Memory);
	Memory=NULL;
	MemorySize=0;
	ResetTimes();
}

//****************************************************************************************************************************
void SBD_Item_Render_Buffer::Blank_Image(void)
{	const unsigned Col=/*U0*/(128)+/*Y0*/(16<<8)+/*V0*/(128<<16)+/*Y1*/(16<<24);	
				
	unsigned		*Scan   =(unsigned*)Memory;
	const unsigned	*ScanEnd=(unsigned*)Memory+XRes*YRes/2;

	while(Scan<ScanEnd) 
	{	Scan[0]=Col;
		Scan[1]=Col;
		Scan[2]=Col;
		Scan[3]=Col;
		Scan[4]=Col;
		Scan[5]=Col;
		Scan[6]=Col;
		Scan[7]=Col;
		Scan+=8;
	}
}

//****************************************************************************************************************************
void SBD_Item_Render_Buffer::ResetTimes(void)
{	StackPosn=0;
}

void SBD_Item_Render_Buffer::SetTime(double StartTime,double EndTime)
{	TimeToRenderAt[StackPosn][0]=StartTime;
	TimeToRenderAt[StackPosn][1]=EndTime;
}

double SBD_Item_Render_Buffer::GetStartTime(void)
{	return TimeToRenderAt[StackPosn][0];
}

double SBD_Item_Render_Buffer::GetEndTime(void)
{	return TimeToRenderAt[StackPosn][1];
}

double SBD_Item_Render_Buffer::GetCentreTime(void)
{	return 0.5*(TimeToRenderAt[StackPosn][0] + TimeToRenderAt[StackPosn][1]);
}

void SBD_Item_Render_Buffer::Push(void)
{	StackPosn++;
	if (StackPosn>=SBD_Item_Render_Buffer_Stack) _throw "SBD_Item_Render_Buffer::PushTime exceeded stack size !";
}

void SBD_Item_Render_Buffer::Pop(void)
{	StackPosn--;
	if (StackPosn==0xffffffff) _throw "SBD_Item_Render_Buffer::PopTime underrun stack base !";
}

void SBD_Item_Render_Buffer::SetNext(SBD_Item_Render_Buffer *NewNext)
{	Next[StackPosn]=NewNext;
}

SBD_Item_Render_Buffer *SBD_Item_Render_Buffer::GetNext(void)
{	return Next[StackPosn];	
}

SBD_Item_Render_Buffer **SBD_Item_Render_Buffer::GetNextRef(void)
{	return &Next[StackPosn];	
}

LONGLONG SBD_Item_Render_Buffer::GetMinimumID(void)
{	LONGLONG Min=ID;
	SBD_Item_Render_Buffer *Item=GetNext();
	while(Item) { Min=min(ID,Item->ID); Item=Item->GetNext(); }
	return Min;
}