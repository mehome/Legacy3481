#include "StdAfx.h"

SplitView::SplitView() {
	m_skin = NULL;
	m_Skin=SplitView_SkinRoot;
	SetSkinRoot(FindFiles_FindFile(FINDFILE_SKINS,m_Skin));
	m_layout = NULL;
	m_timeLine=NULL;
	m_SplitView_Button=NULL;
	}

SplitView::~SplitView() {
	}


void SplitView::InitialiseWindow(void) {
	// Set up skin window
	m_skin = GetInterface<Auto_StretchySkin>(NewTek_New("Auto_StretchySkin"));
	if (!m_skin) _throw ("KeyerControl::Cannot create the stretchy skin !");
	Canvas_SetResource(m_skin);	
	AutoSkinControl::InitialiseWindow();
	this->AddDependant(this);
	} //end initialise window


void SplitView::InitializeSubControls() {
	//Setup the OpenInOuts secret button
	m_SplitView_Button=GetWindowInterface<SplitView_Control> (OpenChild(RGBA(255,60,52),"SplitView_Control"));
	if (!m_SplitView_Button) _throw ("VideoEditor::ReadClassData Could not create SplitView button !");
	}

void SplitView::DestroyWindow(void) {
	this->DeleteDependant(this);
	// We must always call my predecssor
	AutoSkinControl::DestroyWindow();

	// Delete my resource
	if (m_skin) NewTek_Delete(m_skin);
	}


void SplitView::VideoEditorClient_SetTimeLine(TimeLine *timeline) {
	m_timeLine=timeline;
	if (m_layout) m_layout->VideoEditor_SetTimeLine(m_timeLine);
	}

void SplitView::SetChildWindows(HWND upper,HWND lower) {
	if (m_SplitView_Button) {
		m_SplitView_Button->SetChildWindows(upper,lower);
		m_SplitView_Button->UpdateChildRects();
		}
	}


void SplitView::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	//Listen for size changes
	if ((ItemChanging==this)&&(IsWindowVisible())) {
		if (!(strcmp(String,BaseWindowClass_Sized))) {
			if (m_SplitView_Button) m_SplitView_Button->UpdateChildRects();
			}
		}
	AutoSkinControl::DynamicCallback(ID,String,args,ItemChanging);
	}


SplitView_Control::SplitView_Control() {
	m_UpperChild=m_LowerChild=NULL;
	m_OffsetY=0;
	}

void SplitView_Control::MouseLButtonClick(long Flags,long x,long y) {
	// The button was clicked inside me !
	if (Clicked) 
		MouseLButtonRelease(Flags,x,y);
	Clicked=true;

	POINT PtNew;
	long SplitButtonY=this->GetWindowPosY();
	GetCursorPos(&PtNew);	

	m_OffsetY=PtNew.y-SplitButtonY;

	// We want exclusive input !
	ExclusiveInput(true);
	// Get the current cursor position
	//GetCursorPos(&Pt);
	//ShowCursor(false);	

	// The button has been clicked
	Changed(Controls_Button_Clicked);
	Changed(Controls_Button_DragToggleOn);
	}

void SplitView_Control::MouseMoved(long Flags,long x,long y) {
	// Set the mouse cursor
	BaseWindowClass::MouseMoved(Flags,x,y);

	// If the button was not clicked iside, me exit !
	if (!Clicked) return;
	POINT PtNew;
	GetCursorPos(&PtNew);
	RECT rc,clientrc,parentclient;

	GetWindowRect(this->GetParent(),&rc);
	GetClientRect(this->GetWindowHandle(),&clientrc);
	GetClientRect(this->GetParent(),&parentclient);
	long maxy=(parentclient.bottom-parentclient.top)-(clientrc.bottom-clientrc.top);
	long newposition=(PtNew.y-m_OffsetY)-rc.top;
	//We'll move this window and resize the 2 children based off of where this window is moved
	if ((newposition<maxy)&&(newposition>parentclient.top)) {
		this->SetWindowPosition(clientrc.left,newposition);
		UpdateChildRects();
		}
	}

void SplitView_Control::MouseLButtonRelease(long Flags,long x,long y) {
	if (!Clicked) return;
	// The mouse is no longer clicked
	Clicked=false;

	// We no longer want exclusive input !
	ExclusiveInput(false);
	//ShowCursor(true);

	Changed(Controls_Button_ReleasedOff);
	}

void SplitView_Control::UpdateChildRects() {
	RECT parentrc,rc,parentwindowrc;
	GetClientRect(this->GetParent(),&parentrc);
	GetWindowRect(this->GetWindowHandle(),&rc);
	GetWindowRect(this->GetParent(),&parentwindowrc);

	if (m_UpperChild) {
		BaseWindowClass *bwc=GetWindowInterface<BaseWindowClass>(m_UpperChild);
		long width=parentrc.right-parentrc.left;
		long height=rc.top-parentwindowrc.top;
		bwc->SetWindowPositionAndSize(parentrc.left,parentrc.top,width,height);
		}
	if (m_LowerChild) {
		BaseWindowClass *bwc=GetWindowInterface<BaseWindowClass>(m_LowerChild);
		long width=parentrc.right-parentrc.left;
		long ypos=rc.bottom-parentwindowrc.top;
		bwc->SetWindowPositionAndSize(parentrc.left,ypos,width,parentrc.bottom-ypos);
		}
	}
