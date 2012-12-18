#ifndef	__ThreadSafe_DynamicVariables__
#define __ThreadSafe_DynamicVariables__

class ThreadSafe_DynamicTalker;

class DECK_CONTROL_DLL ThreadSafe_DynamicListener
{

	public:			/*! This is the newer version of the call, it adds an extra parameter that
						tells you what the item that is changing is */
					virtual void DynamicCallback(	long ID=-1,							//! This is the ID value that you filled in with your AddDependant
													char *String=NULL,					/*! This is a string describing the change that was made. 
																							You specify this string when you change a variable. Most people can ignore it. */
																						//! Default messages : "Change" "Delete"
													void *args=NULL,
													ThreadSafe_DynamicTalker *ItemChanging=NULL)	//! This is the new arguement, that allows you to directly determine what is changing !
					{}

					//! The constructor and destructor
					ThreadSafe_DynamicListener(void);
					~ThreadSafe_DynamicListener(void);

					//! Stop messages from being sent to me
					void ReceiveMessagesOff(void);
					void ReceiveMessagesOn(void);

					//! A useful function
					void RemoveAllDependantsToThis(void);


					//Onlyt the DynamicTalker should be using these methods
					unsigned GetDoNotSendMeMessages();
					AddDynamicTalker(ThreadSafe_DynamicTalker *DT);
					RemoveDynamicTalker(ThreadSafe_DynamicTalker *DT);

	private:
					VariableBlock m_Block; //This is what makes it threadsafe :)

					//! Should messages be sent to this item ?
					unsigned m_DoNotSendMeMessages;
					//! We store a list of all DynamicTalkers that are talking to me
					tList<ThreadSafe_DynamicTalker*>	m_TalkersTalkingToMe;					
};

class DECK_CONTROL_DLL ThreadSafe_DynamicTalker
{
	public:
					//! Constructor and destructor
					ThreadSafe_DynamicTalker(void);
					virtual ~ThreadSafe_DynamicTalker(void);	//! Sends Changed("Delete");

					//! Add and remove dependants
					void AddDependant	(ThreadSafe_DynamicListener *Lstn,long ID=-1);

					void DeleteDependant(ThreadSafe_DynamicListener *Lstn);

					bool IsDependant	(ThreadSafe_DynamicListener *Lstn,long ID = -1);	//! insert an id here to look for a specific one

					void Changed(char *String="Changed", ... );

					void ChangedARGS(char *String="Changed", void *args=NULL );

					//! A way of finding the ID of a particular listener
					long GetListenerID(ThreadSafe_DynamicListener* listener);

					//! Switch message sending off
					void MessagesOff(void);

					//! If the flag is true, then any changed messages that occured are concatinated
					//! and a single changed mesasge is sent
					bool MessagesOn(bool SendChangedMessageIfnecessary=false);

					//! Are messages currently on ?
					bool AreMessagesOn(void);



	protected:		
					VariableBlock m_Block; //This is what makes it threadsafe :)

					//! Information about the dynamic listener
					class ControlDLL DynamicTalker_Data
					{	public:		ThreadSafe_DynamicListener *m_Item;
									unsigned		m_ID;
					};

					//! The list of DynamicListener objects dependant on me
					tList<DynamicTalker_Data>	m_Dependants;
					unsigned					m_DynamicCallback_RefCount;

					//! Utility for keeping track of previous settings
					long						ReferenceCount;
					bool						MessagesSent;

					//! At least I have one
					friend void DynamicVars_Free(void);
					
};


#endif	__ThreadSafe_DynamicVariables__