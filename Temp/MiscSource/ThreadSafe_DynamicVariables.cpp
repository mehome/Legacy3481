#include "StdAfx.h"

#ifdef _DEBUG
#define TrackVariables
//#pragma message( "\n")
//#pragma message( "### WARNING ### Variable tracking of dynamic variables is enabled. When you exit, if it crashes\n" )
//#pragma message( "                The debug log will contain a list of all unfreed memory.\n" )
//#pragma message( "\n")
#else
#undef TrackVariables
#endif

#undef TrackVariables

//**************************************************************************************************************
#ifdef TrackVariables
VariableBlock					g_All_ThreadSafe_DynamicTalkers_Block;
tListExpandOnly<ThreadSafe_DynamicTalker*>	g_All_ThreadSafe_DynamicTalkers;
#endif TrackVariables

void ThreadSafe_DynamicVars_Init(void)
{	
}

void ThreadSafe_DynamicVars_Free(void)
{	
#ifdef TrackVariables
	if (g_All_ThreadSafe_DynamicTalkers.NoItems) 
	{	g_All_ThreadSafe_DynamicTalkers_Block.Block();
		for(unsigned i=0;i<g_All_ThreadSafe_DynamicTalkers.NoItems;i++)
			DebugOutput("## ERROR ## Memory not freed %d %s\n",long(g_All_ThreadSafe_DynamicTalkers[i]),GetTypeName(g_All_ThreadSafe_DynamicTalkers[i]));
		_throw("Please read DebugOutput for list of unfreed variables !\n");	
		g_All_ThreadSafe_DynamicTalkers_Block.UnBlock();
	}
#endif
}

void ThreadSafe_DynamicTalker::AddDependant(ThreadSafe_DynamicListener *Lstn,long ID)
{	if (!Lstn) return;
	// Create the item
	DynamicTalker_Data l_DynamicTalker_Data={Lstn,ID};

	m_Block.Block();
	// Add this item to the list
	m_Dependants.Add(l_DynamicTalker_Data);	
	m_Block.UnBlock();

	// Add myself to the list of the items being talked to by me	
	Lstn->AddDynamicTalker(this);
}

void ThreadSafe_DynamicTalker::DeleteDependant(ThreadSafe_DynamicListener *Lstn)
{	
	FunctionBlock block(m_Block);
	// Pedantic but makes things more robust !
	if (!this) return;
	// Are we currently somewhere inside a dynamic callback ?
	if (m_DynamicCallback_RefCount)
	{	// Scan through all the dependants
		for(unsigned i=0;i<m_Dependants.NoItems;i++)
		{	if (m_Dependants[i].m_Item==Lstn)
			{	// Add myself to the list of the items being talked to by me
				m_Dependants[i].m_Item->RemoveDynamicTalker(this);
				m_Dependants[i].m_Item=NULL;				
			}
		}
	}
	else
	{	// Scan through all the dependants
		for(unsigned i=0;i<m_Dependants.NoItems;)
		{	if (m_Dependants[i].m_Item==Lstn)
			{	// Add myself to the list of the items being talked to by me
				m_Dependants[i].m_Item->RemoveDynamicTalker(this);
				// Remove this item from my list
				m_Dependants.DeleteEntry(i);				
			}
			else i++;
		}
	}
}

bool ThreadSafe_DynamicTalker::IsDependant(ThreadSafe_DynamicListener *Lstn,long ID)
{
	FunctionBlock block(m_Block);
	// Scan through all the dependants
	for(unsigned i=0;i<m_Dependants.NoItems;i++)
	if (m_Dependants[i].m_Item==Lstn) return true;
	return false;
}

void ThreadSafe_DynamicTalker::ChangedARGS(char *String, void *args)
{	
	FunctionBlock block(m_Block);

//#ifndef __DISABLE_EXCEPTION_HANDLING
//try {
//#endif
	// Stored messages
	if (InterlockedCompareExchange(&ReferenceCount,0,0)!=0) 
	{	MessagesSent=true; 
		return; 
	}
	// We are now inside a callback !
	m_DynamicCallback_RefCount++;
	// Make a call to all the callbacks
	for(unsigned i=0;i<m_Dependants.NoItems;i++)
	if ((m_Dependants[i].m_Item)&&	// Check that this item has not been ->DeleteDependant() while inside this call
		(!m_Dependants[i].m_Item->GetDoNotSendMeMessages())
	   )
			m_Dependants[i].m_Item->DynamicCallback(m_Dependants[i].m_ID,String,args,this);
	// We are finished being inside the callback
	m_DynamicCallback_RefCount--;
	// If we are no exited completely, remove all dependants
	if (!m_DynamicCallback_RefCount)
	{	// Remove all NULL dependancies
		for(unsigned i=0;i<m_Dependants.NoItems;)
		{	if (!m_Dependants[i].m_Item)
				m_Dependants.DeleteEntry(i);
				// !!!!! DeleteDependant(m_Dependants[i].m_Item);
			else i++;
		}
	}
//#ifndef __DISABLE_EXCEPTION_HANDLING
//	} catch(...)
//	{ NewTek_MessageBox(NewTek_GetGlobalParentHWND(),"Error in DynamicCallback", "ERROR", MB_OK, NULL);
//	}
//#endif	
}

void ThreadSafe_DynamicTalker::Changed(char *String, ... )
{
	va_list marker;
	va_start(marker,String);
	ChangedARGS(String,marker);
	va_end(marker);	
}

bool ThreadSafe_DynamicTalker::AreMessagesOn(void)
{
	FunctionBlock block(m_Block);
	return ReferenceCount==0;
}

void ThreadSafe_DynamicTalker::MessagesOff(void)
{
	FunctionBlock block(m_Block);
	if ( InterlockedIncrement(&ReferenceCount) == 1 )
		MessagesSent=false;
}

bool ThreadSafe_DynamicTalker::MessagesOn(bool SendChangedMessageIfnecessary)
{
	FunctionBlock block(m_Block);
	bool Ret=false;
	if (InterlockedDecrement(&ReferenceCount)==0) 
	{	// Store whether messages where send
		Ret = MessagesSent;		
		
		// Send a change message if necessary
		if ((SendChangedMessageIfnecessary)&&(MessagesSent)) 
			Changed();

		// No messages sent
		MessagesSent=false;
	}
	if (ReferenceCount<0) ReferenceCount=0;

	return Ret;
}

ThreadSafe_DynamicTalker::ThreadSafe_DynamicTalker(void)
{	// Reference counting for message blocking
	ReferenceCount=0;
	// We are not in a callback
	m_DynamicCallback_RefCount=0;
#ifdef TrackVariables
	g_All_ThreadSafe_DynamicTalkers_Block.Block();
	g_All_ThreadSafe_DynamicTalkers.Add(this);	
	g_All_ThreadSafe_DynamicTalkers_Block.UnBlock();
#endif TrackVariables
}

ThreadSafe_DynamicTalker::~ThreadSafe_DynamicTalker(void)
{	
#ifdef _DEBUG
	// Rick Addition, Throw here if someone forgot to turn Messages Back ON!!
	if (ReferenceCount > 0)
		_throw ("ThreadSafe_DynamicTalker::~ThreadSafe_DynamicTalker Messages not turned on!\n");
#else
	// In release mode I don't want to crash, but I probably do want to send deletion messages !
	ReferenceCount=0;
#endif
	
	// Notify Everyone that we are being deleted
	Changed(DynamicTalker_Delete,this);
	m_Block.Block();
	// I remove myself from the list of all talkers talking to me
	for(unsigned i=0;i<m_Dependants.NoItems;i++)
	{	if (m_Dependants[i].m_Item)
			m_Dependants[i].m_Item->RemoveDynamicTalker(this);
	}
	m_Block.UnBlock();
#ifdef TrackVariables
	g_All_ThreadSafe_DynamicTalkers_Block.Block();
	g_All_ThreadSafe_DynamicTalkers.Delete(this);
	g_All_ThreadSafe_DynamicTalkers_Block.UnBlock();
#endif TrackVariables
}

long ThreadSafe_DynamicTalker::GetListenerID(ThreadSafe_DynamicListener* listener)
{
	FunctionBlock block(m_Block);
	// SAcan through, looking for the item
	for(unsigned i=0;i<m_Dependants.NoItems;i++)
	if (m_Dependants[i].m_Item==listener) return m_Dependants[i].m_ID;
	return -1;
}

//********************************************************************************************************************************
void ThreadSafe_DynamicListener::RemoveAllDependantsToThis(void)
{	// Scan across all the dependants on me
	tList<ThreadSafe_DynamicTalker *>	l_Temp;
	m_Block.Block();
	l_Temp.ExchangeData(&m_TalkersTalkingToMe);
	m_Block.UnBlock();
	for(unsigned i=0;i<l_Temp.NoItems;i++)
		l_Temp[i]->DeleteDependant(this);
}

ThreadSafe_DynamicListener::~ThreadSafe_DynamicListener(void)
{	RemoveAllDependantsToThis();
}

unsigned ThreadSafe_DynamicListener::GetDoNotSendMeMessages()
{
	FunctionBlock block(m_Block);
	return m_DoNotSendMeMessages;
}

void ThreadSafe_DynamicListener::ReceiveMessagesOff(void)
{
	FunctionBlock block(m_Block);
	m_DoNotSendMeMessages++;
}

void ThreadSafe_DynamicListener::ReceiveMessagesOn(void)
{
	FunctionBlock block(m_Block);
	if (m_DoNotSendMeMessages) m_DoNotSendMeMessages--;
	//if (m_DoNotSendMeMessages==(unsigned)-1) m_DoNotSendMeMessages=0;
}

ThreadSafe_DynamicListener::ThreadSafe_DynamicListener(void)
{	m_DoNotSendMeMessages=0;
}


ThreadSafe_DynamicListener::AddDynamicTalker(ThreadSafe_DynamicTalker *DT)
{
	FunctionBlock block(m_Block);
	m_TalkersTalkingToMe.Add(DT);
}

ThreadSafe_DynamicListener::RemoveDynamicTalker(ThreadSafe_DynamicTalker *DT)
{
	FunctionBlock block(m_Block);
	m_TalkersTalkingToMe.Delete(DT);
}


