#include "StdAfx.h"
  /***********************************************************************************************/
 /*									ButtonRequestorBitmapResource								*/
/***********************************************************************************************/
long ButtonRequestorBitmapResource::s_Ref=0;
ButtonRequestorBitmapResource *ButtonRequestorBitmapResource::s_Instance=NULL;

void ButtonRequestorBitmapResource::Release() {
	if ((InterlockedDecrement(&s_Ref)==0)&&(s_Instance)) {
		delete s_Instance;
		s_Instance=NULL;
		}
	}

ScreenObject_BitmapFile *ButtonRequestorBitmapResource::GetBitMaps(unsigned ButtonState) {
	if (s_Ref==0) 
		_dthrow("ButtonRequestorBitmapResource::GetBitMaps Someone didn't add a reference");

	if (!s_Instance) {
		char TempBuffer[MAX_PATH];

		s_Instance=new ButtonRequestorBitmapResource();
		for (unsigned i=0;i<4;i++) {
			const char * const BitmapNames[4] = {
				"Off.png",
				"On.png",
				"Offro.png",
				"On.png"
				};

			s_Instance->m_RequestorBitmaps[i].ChangeFilename(NewTek_GetAbsolutePath(
				FindFiles_FindFile(FINDFILE_SKINS,"BaseWindowClass\\FileSelectorRequest"),const_cast<char *>(BitmapNames[i]),TempBuffer));

			s_Instance->m_RequestorBitmaps[i].ForceTo32BPP();
			}

		}
	assert(s_Instance);
	return &s_Instance->m_RequestorBitmaps[ButtonState];
	}



  /***********************************************************************************************/
 /*										UtilLib_Edit_FileSelector								*/
/***********************************************************************************************/

UtilLib_Edit_FileSelector::UtilLib_Edit_FileSelector() {
	m_FSEC_Listener=new FSEC_Listener(this);
	m_RequestorButton=NULL;
	m_FileSelector=NULL;
	// Add all the dependancies
	InternalTextItem.AddDependant(m_FSEC_Listener);
	m_ButtonToUse.AddDependant(m_FSEC_Listener);
	ButtonRequestorBitmapResource::AddRef();
	}

UtilLib_Edit_FileSelector::~UtilLib_Edit_FileSelector() {
	if (m_FSEC_Listener) {
		delete(m_FSEC_Listener);
		m_FSEC_Listener=NULL;
		}
	ButtonRequestorBitmapResource::Release();
	}

void UtilLib_Edit_FileSelector::InitialiseWindow(void) {
	UtilLib_Edit::InitialiseWindow();
	}

void UtilLib_Edit_FileSelector::DestroyWindow(void) {
	if (m_RequestorButton) {
		NewTek_Delete(m_RequestorButton);
		m_RequestorButton=NULL;
		}
	RemoveAllDependantsToThis();

	UtilLib_Edit::DestroyWindow();
	}

void UtilLib_Edit_FileSelector::Controls_Edit_SetItem(TextItem *ItemToEdit) {
	if (m_FileSelector) {
		NewTek_Delete(m_FileSelector);
		m_FileSelector=NULL;
		}
	// Remove the previous dependant
	if (TextItemToUse) TextItemToUse->DeleteDependant(m_FSEC_Listener);
	// NOw setup the new value
	if (ValueToUse) ValueToUse->DeleteDependant(m_FSEC_Listener);

	UtilLib_Edit::Controls_Edit_SetItem(ItemToEdit);

	// Setup the new dependant if necessary
	if (TextItemToUse)  {
		m_FSEC_Listener->DynamicCallback(0,"",0,TextItemToUse);
		TextItemToUse->AddDependant(m_FSEC_Listener);
		}
	}


void UtilLib_Edit_FileSelector::OpenRequestButton() {
	if (!m_RequestorButton) {
		//Find a region for the child window
		long width=20; //todo provide method to declare the width
		long height=GetWindowHeight();
		long x=GetWindowWidth()-width;
		long y=2;

		try {
			if (this->IsWindowVisible()) {
				RECT windowRect;
				GetWindowRect(GetWindowHandle(), &windowRect);
				POINT pt; pt.x = windowRect.left; pt.y = windowRect.top;

				//Create the Requestor Button
				ReceiveMessagesOff();
				m_FSEC_Listener->ReceiveMessagesOff();
				m_RequestorButton=GetWindowInterface<UtilLib_Button>(OpenChild("UtilLib_Button",pt.x+x,pt.y+y,width,height,WS_POPUP|WS_VISIBLE));
				m_FSEC_Listener->ReceiveMessagesOn();
				ReceiveMessagesOn();
				if (!m_RequestorButton) throw 0;

				ReceiveMessagesOff();
				m_FSEC_Listener->ReceiveMessagesOff();
				for (unsigned i=0;i<4;i++) {
					m_RequestorButton->Button_SetResource(i,ButtonRequestorBitmapResource::GetBitMaps(i));
					}
				this->SetFocus(); //set this back in focus
				m_RequestorButton->Button_UseVariable(&m_ButtonToUse);
				m_FSEC_Listener->ReceiveMessagesOn();
				ReceiveMessagesOn();
				}
			}
		catch (int ErrorCode) {
			DebugOutput("UtilLib_Edit_FileSelector::OpenRequestButton failed : %d\n",ErrorCode);
			}
		}
	}

void UtilLib_Edit_FileSelector::CloseRequestButton() {
	if (m_RequestorButton) {
		//is the mouse over the requestor?
		POINT mousept;
		GetCursorPos(&mousept);
		RECT windowrect;
		GetWindowRect(m_RequestorButton->GetWindowHandle(),&windowrect);
		if ((mousept.x>=windowrect.left)&&(mousept.x<=windowrect.right)&&
			(mousept.y>=windowrect.top)&&(mousept.y<=windowrect.bottom)) {
			//DebugOutput("UtilLib_Edit_FileSelector::CloseRequestButton button selected\n");
			m_ButtonToUse.Set(1); //select it
			OpenFileSelector();
			}

		//Destroy the Requestor Button
		NewTek_Delete(m_RequestorButton);
		m_RequestorButton=NULL;
		m_ButtonToUse.Set(0); //deselect it
		}
	}


void UtilLib_Edit_FileSelector::OpenFileSelector() {
	if (m_FileSelector) {
		m_FileSelector->DeleteDependant(m_FSEC_Listener);
		NewTek_Delete(m_FileSelector);
		m_FileSelector=NULL;
		}
	HWND MyChild=this->BaseWindowClass::OpenChild(const_cast<char *>(GetFileSelectorPluginName()),0,0,0,0,WS_POPUP|WS_VISIBLE|DS_CENTER);
	m_FileSelector=GetWindowInterface<FileSelector>(MyChild);
	if (!m_FileSelector) _throw("UtilLib_Edit_FileSelector::OpenFileSelector could not create the file selector.");

	//Set some attributes
	m_FileSelector->FileSelector_UseOpen(true);

	//TODO interface this out to an interface (so other classes can override it to fill in a default path)
	//SetDirectory()
	m_FileSelector->AddDependant(m_FSEC_Listener);
	}

void UtilLib_Edit_FileSelector::FSEC_Listener::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	_CodeBlockStart;
	//DebugOutput("FSEC_Listener->%x %s\n",this,String);
	if (ItemChanging==m_Parent->TextItemToUse) {
		if (!(strcmp(String,UtilLib_Edit_GetFocus))) {
			m_Parent->OpenRequestButton();
			}
		else if (!(strcmp(String,UtilLib_Edit_LoseFocus))) {
			m_Parent->CloseRequestButton();
			}
		_CodeBlockExit;
		}

	if (ItemChanging==m_Parent->m_FileSelector) {
		bool CloseSelector=false;

		if (IsDeletion(String)) {
			m_Parent->m_FileSelector=NULL;
			}
		//TODO move these string types to our interface...
		else if (!strcmp(String,"FileSelector_OKPressed")) {
			tList<char *> MyFiles;
			m_Parent->m_FileSelector->FileSelector_GetSelectedFiles(&MyFiles);
			if (MyFiles.NoItems) {
				char Buffer[MAX_PATH];
				strcpy(Buffer,MyFiles[0]);

				//If we are selecting folders it is good to always end with a slash (especially when selecting a drive)
				if (m_Parent->ClickToSelectFolders())
					NewTek_StrCatSlashIfNeeded(Buffer);

				m_Parent->TextItemToUse->SetText(Buffer); //Just grab the first entry
				}
			CloseSelector=true;
			}
		else if (!strcmp(String,"FileSelector_CancelPressed")) {
			CloseSelector=true;
			}

		if (CloseSelector) {
			m_Parent->m_FileSelector->DeleteDependant(this);
			m_Parent->m_FileSelector->FileSelector_DeferredDelete();
			m_Parent->m_FileSelector=NULL;
			}
		}

	//No predecessor
	_CodeBlockEnd;
	}
