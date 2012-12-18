#ifndef __HandleCache__
#define __HandleCache__

//************************************************************************************
class SBDDLL HandleCache_Item
{	
#ifndef __DEVIOUSNESS__
	private:	
#else
	public:
#endif
				HANDLE m_Handle;

	public:		// Constructor and destructor
				HandleCache_Item(void);
				~HandleCache_Item(void);

				// This allows items to be stored in a linked list
				HandleCache_Item *Next;

				// Wait for this item to be signalled
				bool WaitFor(unsigned Time=INFINITE);

				// Signal this item
				void Signal(void);
				void Reset(void);
};

// Single handles
bool SBDDLL HandleCache_WaitForHandle(HandleCache_Item *Item,unsigned Time=INFINITE);

// Array of handles
bool SBDDLL HandleCache_WaitForHandles(unsigned No,HandleCache_Item **Items,unsigned Time=INFINITE);

// Linked list of handles
bool SBDDLL HandleCache_WaitForHandles(HandleCache_Item *Items,unsigned Time=INFINITE);

//************************************************************************************
class SBDDLL HandleCache : public VariableBlock
{	private:	tListExpandOnly<HandleCache_Item*> FreeItems;
				tListExpandOnly<HandleCache_Item*> AllItems;
	public:		// Get a handle
				HandleCache_Item *GetHandle(void);
				void ReleaseHandle(HandleCache_Item *Item);

				// Destructor
				~HandleCache(void);
};

//************************************************************************************
extern SBDDLL HandleCache g_HandleCache;

#endif