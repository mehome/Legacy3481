#ifndef __DISKREADA__
#define __DISKREADA__

//********************************************************************************************************************************
class SBDDLL ThreadedDiskReader_Item
{	public:		// The item to render
				SBD_Item_Info			*Base;

				// The ID value of this entity
				LONGLONG				BufferID;

				// This is the list of frames to render off the drive
				SBD_Item_Render_Buffer	*ItemsToRender;

				// The cache item to signal when completed !
				HandleCache_Item		*ItemToSignal;

				// These are stored in a linked list
				ThreadedDiskReader_Item *Next;
};

//********************************************************************************************************************************
class SBDDLL ThreadedDiskReader : public WorkerThread
{	private:	// The drive setup
				char					DriveToUse[2];
				VariableBlock			QueueBlock;

	private:	// My linked lsit of items to process
				ThreadedDiskReader_Item	*Queue;
				ThreadedDiskReader_Item	*Free;

				ThreadedDiskReader_Item	*GetFree(void);
				void FreeBlock(ThreadedDiskReader_Item *Item);

	public:		// This is the actual processing entity
				virtual void ThreadProcessor(void);

				// Submit a job, and get a handle back that will be signalled
				// when it has been loaded.
				HandleCache_Item *SubmitRequest(	SBD_Item_Info *Base,
													LONGLONG BufferID, 
													SBD_Item_Render_Buffer *ItemsToRender);

				// Constructor 
				ThreadedDiskReader(char DriveLetter='C');

				// Destructor
				~ThreadedDiskReader(void);
};

//********************************************************************************************************************************
HandleCache_Item SBDDLL *DriveReader_SubmitRequest(char *Drive,SBD_Item_Info *Base,LONGLONG BufferID,SBD_Item_Render_Buffer *ItemsToRender);
void SBDDLL  DriveReader_DisposeOfHandle(HandleCache_Item *Item);
void SBDDLL  DriveReader_DisposeOfHandle_List(HandleCache_Item *Item);	// linked list version

#endif