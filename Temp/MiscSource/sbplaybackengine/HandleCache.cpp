#define __DEVIOUSNESS__
#include "StdAfx.h"

//****************************************************************************************************************8
HandleCache g_HandleCache;

//****************************************************************************************************************8
HandleCache_Item::HandleCache_Item(void)
{	m_Handle=CreateEvent(NULL,false,false,NULL);	
	if (!m_Handle) _throw("HandleCache_Item::HandleCache_Item Crikey, cannot create the handle !");
}

//****************************************************************************************************************8
HandleCache_Item::~HandleCache_Item(void)
{	if (m_Handle) CloseHandle(m_Handle);
}

//****************************************************************************************************************8
bool HandleCache_Item::WaitFor(unsigned Time)
{	if ((m_Handle)&&(WaitForSingleObject(m_Handle,Time)==WAIT_OBJECT_0)) return true;
	return false;
}

//****************************************************************************************************************8
void HandleCache_Item::Signal(void)
{	if (m_Handle) SetEvent(m_Handle);
}

void HandleCache_Item::Reset(void)
{	if (m_Handle) ResetEvent(m_Handle);
}

//****************************************************************************************************************8
bool HandleCache_WaitForHandle(HandleCache_Item *Item,unsigned Time)
{	if (Item) return Item->WaitFor(Time);
	return false;
}

//****************************************************************************************************************8
bool HandleCache_WaitForHandles(unsigned No,HandleCache_Item **Items,unsigned Time)
{	// Setup an array of handles
	HANDLE MyHandles[128];
	if (No>128) return false;
	
	// Get the list of handles
	unsigned TrueNum=0;
	for(unsigned i=0;i<No;i++)
	if (Items[i]->m_Handle) MyHandles[TrueNum++]=Items[i]->m_Handle;
			
	// Wait for all the handles to succeed
	if (WaitForMultipleObjects(TrueNum,MyHandles,true,Time)==WAIT_TIMEOUT) return false;
	return true;
}

//****************************************************************************************************************
bool HandleCache_WaitForHandles(HandleCache_Item *Items,unsigned Time)
{	// Setup an array of handles
	HANDLE MyHandles[128];

	// Get the list of handles
	unsigned TrueNum=0;
	while(Items)
	{	MyHandles[TrueNum++]=Items->m_Handle;
		Items=Items->Next;
	}

	// Wait for all the handles to succeed
	if (WaitForMultipleObjects(TrueNum,MyHandles,true,Time)==WAIT_TIMEOUT) return false;
	return true;
}

//****************************************************************************************************************8
HandleCache_Item *HandleCache::GetHandle(void)
{	// We are thread safe
	FunctionBlock a(this);

	// Pull one of the list of used items
	if (FreeItems.NoItems) 
	{	HandleCache_Item *Ret=FreeItems[FreeItems.NoItems-1];
		FreeItems.DeleteEntry(FreeItems.NoItems-1);
		return Ret;
	}

	// Create a new one
	HandleCache_Item *Ret=new HandleCache_Item;
	AllItems.Add(Ret);
	return Ret;
}

//****************************************************************************************************************8
void HandleCache::ReleaseHandle(HandleCache_Item *Item)
{	// We are threadsafe
	FunctionBlock a(this);
	FreeItems.Add(Item);
}

//****************************************************************************************************************8
HandleCache::~HandleCache(void)
{	Block();
	for(unsigned i=0;i<AllItems.NoItems;i++)
		delete AllItems[i];
	UnBlock();
}

