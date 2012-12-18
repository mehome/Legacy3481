#include "StdAfx.h"


TimeLine*	VideoEditor::s_timeLine = NULL;

VideoEditor::VideoEditor() {
	m_skin = NULL;
	m_Skin=VideoEditor_SkinRoot;
	SetSkinRoot(FindFiles_FindFile(FINDFILE_SKINS,m_Skin));
	//m_layout = NULL;
	m_ToggleButton_LastState=0;
	m_StoryBoardView=m_TimeLineView=m_SplitView=NULL;
	m_VC=NULL;
	m_ContentInstance=NULL;
	}

VideoEditor::~VideoEditor() {
	}

void VideoEditor::OnProcessAttach()
{
	if (!s_timeLine)
	{
		// Create the static TimeLine
		s_timeLine = NULL;

		// Try to Load the new TimeLine
		char loadFile[MAX_PATH];
		sprintf(loadFile, "%s\\%s", FindFiles_FindFile(FINDFILE_SKINS, VideoEditor_SkinRoot), VideoEditor_TimeLineFile);
		LoadData ld(loadFile, LoadData_Flag_Launch);
		LoadPlugin_GetType_Info info;
		if (LoadPlugin_Type(&ld, &info))
		{
			BaseLoadSave* plugin = NULL;
			LoadPlugin_Data(&info, plugin, &ld, NULL, true);
			s_timeLine = GetInterface<TimeLine>(plugin);
			if (!s_timeLine) NewTek_Delete(plugin);
		}
		// If nothing loaded
		if (!s_timeLine) s_timeLine = new TimeLine();
	}
}

void VideoEditor::OnProcessDetach()
{
	// Delete the Static TimeLine
	if (s_timeLine)
	{
		char saveFile[MAX_PATH];
		sprintf(saveFile, "%s\\%s", FindFiles_FindFile(FINDFILE_SKINS, VideoEditor_SkinRoot), VideoEditor_TimeLineFile);
		SaveData sd(saveFile, SaveData_Flag_None);
		if (!SavePlugin(s_timeLine, &sd))
		{
			char error[MAX_PATH + 40];
			sprintf(error, "There was an error writing to the file %s.", saveFile);
			NewTek_MessageBox(NULL, error, "Error Writing File", MB_OK, NULL);
		}

		delete s_timeLine;
	}
}

void VideoEditor::InitialiseWindow(void) {
	// Set up skin window
	m_ToggleButton_dyn.AddDependant(this,(long)&m_ToggleButton_dyn);
	m_skin = GetInterface<Auto_StretchySkin>(NewTek_New("Auto_StretchySkin"));
	if (!m_skin) _throw ("KeyerControl::Cannot create the stretchy skin !");
	Canvas_SetResource(m_skin);	
	AutoSkinControl::InitialiseWindow();
	} //end initialise window


void VideoEditor::InitializeSubControls()
	// Create the scrollbar & arrows
	{
	//Heres a temporary method to open the inouts
	SkinControl_SubControl_ToggleButton	*SubControl_ToggleButton;
	// the layers for all of the buttons
	long up = Controls_Button_UnSelected;
	long upro = Controls_Button_MouseOver;
	long dn = Controls_Button_Selected;
	long dnro = Controls_Button_MouseOverDn;

	//Setup the OpenInOuts secret button
	SubControl_ToggleButton=GetInterface<SkinControl_SubControl_ToggleButton> (OpenChild_SubControl(RGBA(255,119,0),"SkinControl_SubControl_ToggleButton"));
	if (!SubControl_ToggleButton) _throw ("VideoEditor::ReadClassData Could not create InOuts button !");
	SubControl_ToggleButton->Button_SetResource(Controls_Button_UnSelected, up);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_Selected, dn);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOver, upro);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOverDn, dnro);
	SubControl_ToggleButton->Button_SetSelectedState(InOutButton);
	SubControl_ToggleButton->Button_UseVariable(&m_ToggleButton_dyn);

	//Setup the Tabbed buttons
	//TimeLine
	SubControl_ToggleButton=GetInterface<SkinControl_SubControl_ToggleButton> (OpenChild_SubControl(RGBA(255,0,0),"SkinControl_SubControl_ToggleButton"));
	if (!SubControl_ToggleButton) _throw ("VideoEditor::ReadClassData Could not create VideoEditor button !");
	SubControl_ToggleButton->Button_SetResource(Controls_Button_UnSelected, up);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_Selected, dn);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOver, upro);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOverDn, dnro);
	SubControl_ToggleButton->Button_SetSelectedState(TimeLineButton);
	SubControl_ToggleButton->Button_UseVariable(&m_ToggleButton_dyn);

	//StoryBoard
	SubControl_ToggleButton=GetInterface<SkinControl_SubControl_ToggleButton> (OpenChild_SubControl(RGBA(255,34,0),"SkinControl_SubControl_ToggleButton"));
	if (!SubControl_ToggleButton) _throw ("VideoEditor::ReadClassData Could not create VideoEditor button !");
	SubControl_ToggleButton->Button_SetResource(Controls_Button_UnSelected, up);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_Selected, dn);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOver, upro);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOverDn, dnro);
	SubControl_ToggleButton->Button_SetSelectedState(StoryBoardButton);
	SubControl_ToggleButton->Button_UseVariable(&m_ToggleButton_dyn);

	//Both
	SubControl_ToggleButton=GetInterface<SkinControl_SubControl_ToggleButton> (OpenChild_SubControl(RGBA(255,85,0),"SkinControl_SubControl_ToggleButton"));
	if (!SubControl_ToggleButton) _throw ("VideoEditor::ReadClassData Could not create VideoEditor button !");
	SubControl_ToggleButton->Button_SetResource(Controls_Button_UnSelected, up);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_Selected, dn);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOver, upro);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOverDn, dnro);
	SubControl_ToggleButton->Button_SetSelectedState(BothButton);
	SubControl_ToggleButton->Button_UseVariable(&m_ToggleButton_dyn);

	m_TimeLineView=GetWindowInterface<VideoEditorClient_Interface>(OpenChild(RGBA(73,52,157), "TimeLine_Editor"));
	if (!m_TimeLineView) _throw ("VideoEditor::ReadClassData Could not create VideoEditor TimeLine !");
	m_TimeLineView->VideoEditorClient_SetTimeLine(s_timeLine);

	m_StoryBoardView=GetWindowInterface<VideoEditorClient_Interface>(OpenChild(RGBA(73,52,157), "StoryBoard_Editor"));
	if (!m_StoryBoardView) _throw ("VideoEditor::ReadClassData Could not create VideoEditor StoryBoard !");
	m_StoryBoardView->VideoEditorClient_SetTimeLine(s_timeLine);

	m_SplitView=GetWindowInterface<SplitView>(OpenChild(RGBA(73,52,157), "SplitView"));
	if (!m_SplitView) _throw ("VideoEditor::ReadClassData Could not create VideoEditor Split !");
	if (m_SplitView) m_SplitView->VideoEditorClient_SetTimeLine(s_timeLine);

	m_SplitTimeLineView=GetWindowInterface<VideoEditorClient_Interface>(m_SplitView->OpenChild(RGBA(85,255,0), "TimeLine_Editor"));
	if (!m_SplitTimeLineView) _throw ("VideoEditor::ReadClassData Could not create VideoEditor SplitTimeLine !");
	m_SplitTimeLineView->VideoEditorClient_SetTimeLine(s_timeLine);

	m_SplitStoryBoardView=GetWindowInterface<VideoEditorClient_Interface>(m_SplitView->OpenChild(RGBA(73,52,157), "StoryBoard_Editor"));
	if (!m_StoryBoardView) _throw ("VideoEditor::ReadClassData Could not create VideoEditor SplitStoryBoard !");
	m_SplitStoryBoardView->VideoEditorClient_SetTimeLine(s_timeLine);
	//TODO flip flop support
	m_SplitView->SetChildWindows(m_SplitStoryBoardView->GetWindowHandle(),m_SplitTimeLineView->GetWindowHandle());

	// Add the dependants
	VideoEditor_Crouton::s_CroutonListener.AddDependant(this);

	AutoSkinControl::InitializeSubControls();
	}


void VideoEditor::DestroyWindow(void) {	
	if (m_ContentInstance) {
		m_ContentInstance->DeleteDependant(this);
		m_ContentInstance=NULL;
		}
	// Remove dependants
	m_ToggleButton_dyn.DeleteDependant(this);
	VideoEditor_Crouton::s_CroutonListener.DeleteDependant(this);

	// We must always call my predecssor
	AutoSkinControl::DestroyWindow();

	// Delete my resource
	if (m_skin) NewTek_Delete(m_skin);
	}


void VideoEditor::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	if (IsDeletion(String)) {
		if (ItemChanging==m_InOuts2) {
			m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()&(~(InOutButton)));
			m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
			}
		if (ItemChanging==m_ContentInstance) {
			m_ContentInstance=NULL;
			m_VC=NULL;
			}
		}
	else if (ID == (long)(&m_ToggleButton_dyn)) {
		if (strcmp(String, Controls_Button_ReleasedOn) == 0) {
			int whichButton = (m_ToggleButton_dyn.Get()^m_ToggleButton_LastState)&m_ToggleButton_dyn.Get();
			m_ToggleButton_LastState=m_ToggleButton_dyn.Get();

			if (whichButton==InOutButton) {
				m_InOuts2 = GetInterface<InOuts2_Control>
					(NewTek_New("InOuts2_Control", GetWindowHandle(), 0, 0, 0, 0, NULL, WS_POPUP));
				//Try to pass as much information as possible
				m_InOuts2->AddDependant(this); //listen for deletion

				if (m_VC) {
					VideoEditor_Crouton *vc=m_VC;
					InOut_Interface* inOutToEdit = vc->VideoEditor_Crouton_GetInOutInterface();
					InOut_Editor*	 inOutCompletor = vc->VideoEditor_Crouton_GetInOutEditor();
					ContentInstance *ci;
					if (ci=vc->VideoEditor_Crouton_GetContentInstance()) {
						VideoEditor_Interface *ve=GetWindowInterface<VideoEditor_Interface>(vc->GetParent());
						m_InOuts2->SetInOutInterfaces(inOutToEdit, inOutCompletor, ci);
						m_InOuts2->SetVideoEditor_Interface(ve);
						//Set up Listening to this for deletion
						if (m_ContentInstance) m_ContentInstance->DeleteDependant(this);
						m_ContentInstance = ci;
						if (m_ContentInstance) m_ContentInstance->AddDependant(this);
						}
					}
/*				We need to leave Video EditorInterface NULL because it doesn't match whenever you select the 
				first crouton of the class... and I'm not sure why... 
				else {
					//we can still give InOuts the VideoEditor Interface
					if (m_StoryBoardView) {
						m_InOuts2->SetVideoEditor_Interface(m_StoryBoardView->VideoEditorClient_GetLayout());
						}
					else if (m_TimeLineView) {
						m_InOuts2->SetVideoEditor_Interface(m_TimeLineView->VideoEditorClient_GetLayout());
						}
					}
*/
				return;
				}
			else if (whichButton==TimeLineButton) {
				//Hide the others
				m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()&(~(StoryBoardButton|BothButton)));
				m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
				m_StoryBoardView->ShowWindow(SW_HIDE);
				m_SplitView->ShowWindow(SW_HIDE);
				m_TimeLineView->ShowWindow(SW_SHOW);
				}

			else if (whichButton==StoryBoardButton) {
				//Hide the others
				m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()&(~(TimeLineButton|BothButton)));
				m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
				m_TimeLineView->ShowWindow(SW_HIDE);
				m_SplitView->ShowWindow(SW_HIDE);
				m_StoryBoardView->ShowWindow(SW_SHOW);
				}

			else if (whichButton==BothButton) {
				//Hide the others
				m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()&(~(StoryBoardButton|TimeLineButton)));
				m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
				m_StoryBoardView->ShowWindow(SW_HIDE);
				m_TimeLineView->ShowWindow(SW_HIDE);
				m_SplitView->ShowWindow(SW_SHOW);
				}
			}
		else if (strcmp(String, Controls_Button_ReleasedOff) == 0) {
			int whichButton = (m_ToggleButton_dyn.Get()^m_ToggleButton_LastState)&m_ToggleButton_LastState;
			m_ToggleButton_LastState=m_ToggleButton_dyn.Get();

			if (whichButton==InOutButton) {
				if (m_InOuts2) {
					m_InOuts2->DeleteDependant(this);
					m_InOuts2->DeferredDelete();
					m_InOuts2=NULL;
					}
				}
			else if (whichButton==TimeLineButton) {
				m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()|TimeLineButton);
				m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
				}
			else if (whichButton==StoryBoardButton) {
				m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()|StoryBoardButton);
				m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
				}
			else if (whichButton==BothButton) {
				m_ToggleButton_dyn.Set(m_ToggleButton_dyn.Get()|BothButton);
				m_ToggleButton_LastState=m_ToggleButton_dyn.Get();
				}
			}
		}
	else if (!strcmp(String,VideoEditor_CroutonListener_DNDSelect)) {
		void *Args=args;
		m_VC= NewTek_GetArguement<VideoEditor_Crouton *>(Args);
		}
	AutoSkinControl::DynamicCallback(ID,String,args,ItemChanging);
	}


