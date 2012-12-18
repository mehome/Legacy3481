#include "StdAfx.h"

DeckControlGUI_Interface *DeckContolBase::GetGUIPlugInInterface() {
	m_GUI=new DeckControlGUIBase(GetNumOfDevices());
	return m_GUI;
	}

DeckContolBase::~DeckContolBase() {
	if (m_GUI) {
		delete m_GUI;
		m_GUI=NULL;
		}
	}

DeckControlGUIBase::DeckControlGUIBase(int devcount) {
	m_NoDevices=devcount;
	m_CurrentDevice=0;
	m_NoDevices=0;
	m_BobDotChannel=new Dynamic<int> *[devcount];
	for (unsigned i=0;i<devcount;i++) {
		m_BobDotChannel[i]=new Dynamic<int>();
		m_BobDotChannel[i]->AddDependant(this);
		//Initialize the Bob Channel to Zero
		m_BobDotChannel[i]->Set(0);
		}
	}

DeckControlGUIBase::~DeckControlGUIBase() {
	RemoveAllDependantsToThis();
	unsigned i;
	if (m_BobDotChannel) {
		for (i=0;i<m_NoDevices;i++) {
			delete m_BobDotChannel[i];
			}
		delete m_BobDotChannel;
		m_BobDotChannel=NULL;
		}
	}


void DeckControlGUIBase::SetCurrentDevice(int device) {
	m_CurrentDevice=device;
	//Change all of our bobdots to the current device
	for (unsigned i=0;i<m_OpenedBobDots.NoItems;i++) {
		BobControl_Video *bcv=m_OpenedBobDots[i];
		bcv->BobControl_SetInputChannel(m_BobDotChannel[device]->Get());
		}
	}

void DeckControlGUIBase::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	for (unsigned i=0;i<m_NoDevices;i++) {
		if (ItemChanging==m_BobDotChannel[i]) {
			//assign all the bob dots created here to the new channel
			ReceiveMessagesOff();
			for (unsigned j=0;j<m_OpenedBobDots.NoItems;j++) {
				BobControl_Video *bcv=m_OpenedBobDots[j];
				bcv->BobControl_SetInputChannel(m_BobDotChannel[i]->Get());
				}
			ReceiveMessagesOn();
			break;
			}
		}
	if (GetInterface<BobControl_Video>(ItemChanging)) {
		BobControl_Video *bcv=GetInterface<BobControl_Video>(ItemChanging);
		if (!IsDeletion(String)) {
			ReceiveMessagesOff();
			unsigned newchannel=bcv->BobControl_GetInputChannel();
			m_BobDotChannel[m_CurrentDevice]->Set(newchannel);
			//Set the rest of the BobDots to the newchannel
			for (unsigned j=0;j<m_OpenedBobDots.NoItems;j++) {
				BobControl_Video *bcv=m_OpenedBobDots[j];
				bcv->BobControl_SetInputChannel(m_BobDotChannel[i]->Set(newchannel));
				}

			ReceiveMessagesOn();
			}
		else {
			//These do not have to be in order
			m_OpenedBobDots.Delete(bcv);
			}
		}
	//No predecessor
	}

BaseWindowClass *DeckControlGUIBase::CreateNewBobDot(BaseWindowClass *Parent,RECT rect) {
	BaseWindowClass *ret=NULL;
	do {
		HWND plugin=Parent->OpenChild("BobControl_Video",rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
		if (!plugin) break;
		BaseWindowClass *bwc=GetWindowInterface<BaseWindowClass>(plugin);
		if (!bwc) break;
		BobControl_Video *bcv=GetInterface<BobControl_Video>(bwc);
		if (!bcv) break;
		ret=bwc;
		AddNewBobDotToList(bcv);
		} while (false);
	return ret;
	}

void DeckControlGUIBase::AddNewBobDotToList(BobControl_Video *bcv) {
	//set this to the current input channel
	bcv->BobControl_SetInputChannel(m_BobDotChannel[m_CurrentDevice]->Get());
	bcv->AddDependant(this);
	m_OpenedBobDots.Add(bcv);
	}

