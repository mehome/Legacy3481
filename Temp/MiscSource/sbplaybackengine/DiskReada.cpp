#include "StdAfx.h"

//*****************************************************************************************************
class DriveItems_Cache : public VariableBlock
{	public:	
		ThreadedDiskReader *Reader[26];

		ThreadedDiskReader *GetItem(char *Drive)
		{	FunctionBlock a(this);
			
			char DriveToUse[2];
			DriveToUse[0] = Drive[0];
			DriveToUse[1] = 0;
			_strupr(DriveToUse);		

			if (!Reader[DriveToUse[0]-'A'])
					Reader[DriveToUse[0]-'A']=new ThreadedDiskReader(DriveToUse[0]-'A');

			return Reader[DriveToUse[0]-'A'];
		}

		DriveItems_Cache(void)
		{	memset(Reader,0,26*sizeof(ThreadedDiskReader*));
		}

		~DriveItems_Cache(void)
		{	for(unsigned i=0;i<26;i++)
			if (Reader[i]) 
			{	
				Reader[i]->StopThread();
				delete Reader[i];
			}
		}

} *_DriveItems_Cache;

void DriveItems_Cache_Init(bool Init)
{	if (Init)	_DriveItems_Cache=new DriveItems_Cache;
	else		delete _DriveItems_Cache;
};

//*****************************************************************************************************
HandleCache_Item *DriveReader_SubmitRequest(char *Drive,SBD_Item_Info *Base,LONGLONG BufferID,SBD_Item_Render_Buffer *ItemsToRender)
{	// Get the drive to use
	ThreadedDiskReader *Item=_DriveItems_Cache->GetItem(Drive);
	if (!Item) return NULL;
	return Item->SubmitRequest(Base,BufferID,ItemsToRender);
}	

void DriveReader_DisposeOfHandle(HandleCache_Item *Item)
{	g_HandleCache.ReleaseHandle(Item);
}

void DriveReader_DisposeOfHandle_List(HandleCache_Item *Item)
{	while(Item)
	{	HandleCache_Item *Next=Item->Next;
		DriveReader_DisposeOfHandle(Item);
		Item=Next;
	}
}

//*****************************************************************************************************
void ThreadedDiskReader::ThreadProcessor(void)
{	// We QueueBlock.Block the Queue from modification
	QueueBlock.Block();
	if (Queue)
	{	// remove the first item off the list !
		ThreadedDiskReader_Item	*ItemToProcess=Queue;
		Queue=Queue->Next;

		// We have now modified the queue, so we can release the lock
		QueueBlock.UnBlock();		

		// Read the blocks in the Async way !
		ItemToProcess->Base->Render_ReadData(ItemToProcess->ItemsToRender);

		// We are finished, so we signal the handle
		ItemToProcess->ItemToSignal->Signal();

		// Dispose of the read request
		FreeBlock(ItemToProcess);
	}
	else
	{	// No requests where outstading, so do a little sleep.
		QueueBlock.UnBlock();
		Sleep(1);
	}
}

//*****************************************************************************************************
ThreadedDiskReader::ThreadedDiskReader(char DriveLetter)
{	// we want it uppercase
	DriveToUse[0] = DriveLetter;
	DriveToUse[1] = 0;
	_strupr(DriveToUse);

	// Nothing is being used
	Queue=Free=NULL;

	// Start the thread processing
	StartThread();
}

//*****************************************************************************************************
ThreadedDiskReader::~ThreadedDiskReader(void)
{	// Stop the thread
	StopThread();

	// Close all my buffers
	while(Free)
	{	ThreadedDiskReader_Item	*Next=Free->Next;
		delete Free; Free=Next;
	}

	while(Queue)
	{	ThreadedDiskReader_Item	*Next=Queue->Next;
		delete Queue; Queue=Next;
	}
}

//*****************************************************************************************************
ThreadedDiskReader_Item	*ThreadedDiskReader::GetFree(void)
{	// Noone can modify the Queue while we are thinking here
	FunctionBlock Local(QueueBlock);
	
	ThreadedDiskReader_Item	*Ret=Free;
	if (Free) Free=Free->Next;
	if (!Ret) Ret=new ThreadedDiskReader_Item;
	return Ret;
}

void ThreadedDiskReader::FreeBlock(ThreadedDiskReader_Item *Item)
{	// Noone can modify the Queue while we are thinking here
	FunctionBlock Local(QueueBlock);
	Item->Next=Free;
	Free=Item;
}

HandleCache_Item *ThreadedDiskReader::SubmitRequest(	SBD_Item_Info *Base,
														LONGLONG BufferID, 
														SBD_Item_Render_Buffer *ItemsToRender)
{	// Noone can modify the Queue while we are thinking here
	FunctionBlock Local(QueueBlock);

	// We need to get a buffer to submit
	ThreadedDiskReader_Item	*Ret=GetFree();

	// Lets hope that this does not happen to often
	if (!Ret) return NULL;

	// We fill in the elements
	Ret->BufferID		=BufferID;
	Ret->ItemsToRender	=ItemsToRender;
	Ret->Base			=Base;

	// Get the handle
	Ret->ItemToSignal	=g_HandleCache.GetHandle();
	Ret->ItemToSignal->Reset();

	// Now we insert it into the list at the correct point
	ThreadedDiskReader_Item	* Item=Queue;
	ThreadedDiskReader_Item	**Prev=&Queue;

	// Search through the list looking for the correct position
	while(Item)
	{	if (Item->BufferID>BufferID) break;
		Prev=&Item->Next;
		Item=Item->Next;
	}

	// Insertion into the linked list !
	Ret->Next=*Prev;
	*Prev=Ret;

	// Return the handle to wait for
	return Ret->ItemToSignal;
}




















