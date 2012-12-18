#include "StdAfx.h"


TimeLine*	VideoEditor::s_timeLine = NULL;

VideoEditor::VideoEditor() {
	m_skin = NULL;
	m_Skin=VideoEditor_SkinRoot;
	m_scrollBarMin.Set(0.0f);
	m_scrollBarMax.Set(1.0f);
	m_widthBarVar.Set(0.3f);
	SetSkinRoot(FindFiles_FindFile(FINDFILE_SKINS,m_Skin));
	m_layout = NULL;
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


void VideoEditor::DestroyWindow(void) {	
	m_ToggleButton_dyn.DeleteDependant(this);

	// We must always call my predecssor
	AutoSkinControl::DestroyWindow();

	// Delete my resource
	if (m_skin) NewTek_Delete(m_skin);
	}

VideoEditor_Interface* StoryBoard_Editor:: CreateVideoEditor(unsigned color)
{
	m_layout = GetWindowInterface<VideoEditor_Interface>(OpenChild(color, "StoryBoard_Layout"));
	if (m_layout) m_layout->VideoEditor_SetTimeLine(s_timeLine);
	return m_layout;
}

VideoEditor_Interface* TimeLine_Editor::CreateVideoEditor(unsigned color)
{
	TimeLine_Layout* timeLine = GetWindowInterface<TimeLine_Layout>(OpenChild(color, "TimeLine_Layout"));
	if (timeLine)
	{
		timeLine->VideoEditor_SetTimeLine(s_timeLine);
		TimeBarControl* tbc = GetWindowInterface<TimeBarControl>(OpenChild(RGBA(25,52,52), "TimeBarControl"));
		if (tbc)
			tbc->TimeBarControl_SetTimeBarDynamic(timeLine->TimeLine_Layout_GetTimeBarDynamic());
	}
	return timeLine;
}

void VideoEditor::InitializeSubControls()
	// Create the scrollbar & arrows
	{
		// Use this as a temporary pointer for creating all controls
		SkinControl_SubControl* MyControl;

		m_Vslider=GetWindowInterface<UtilLib_Slider>(OpenChild(RGBA(25,73,76),"UtilLib_Slider"));
		if (m_Vslider)
		{
			// Set all of the Dynamics for the scroller
			m_Vslider->Slider_SetVariable(&m_scrollBarVar);
			m_Vslider->Slider_SetMinVariable(&m_scrollBarMin);
			m_Vslider->Slider_SetMaxVariable(&m_scrollBarMax);
			m_Vslider->Slider_SetSliderWidth(&m_scrollBarWidth);

			m_Vslider->Button_SetResource(Controls_Button_UnSelected, &m_VsliderBitmaps[Controls_Button_UnSelected]);
			m_Vslider->Button_SetResource(Controls_Button_Selected, &m_VsliderBitmaps[Controls_Button_Selected]);
			m_Vslider->Button_SetResource(Controls_Button_MouseOver, &m_VsliderBitmaps[Controls_Button_MouseOver]);
			m_Vslider->Button_SetResource(Controls_Button_MouseOverDn, &m_VsliderBitmaps[Controls_Button_MouseOverDn]);
		}

		m_layout = CreateVideoEditor(RGBA(73,52,157));
		BaseWindowLayoutManager* layout = GetInterface<BaseWindowLayoutManager>(m_layout);
		if (layout)
		{
			layout->BaseWindowLayoutManager_Y_Slider_SetMaxVariable(&m_scrollBarMax);
			layout->BaseWindowLayoutManager_Y_Slider_SetMinVariable(&m_scrollBarMin);
			layout->BaseWindowLayoutManager_Y_Slider_SetSliderWidth(&m_scrollBarWidth);
			layout->BaseWindowLayoutManager_Y_Slider_SetVariable(&m_scrollBarVar);			
		}
		else throw "VideoEditor::Could not create StoryBoard ListLayout";

		// get the Up arrow
		MyControl=OpenChild_SubControl(RGBA(25,100,76),"SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *UpArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (UpArrow)
		{
			UpArrow->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			UpArrow->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			UpArrow->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			UpArrow->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			UpArrow->Slider_SetVariable(&m_scrollBarVar);
			UpArrow->Slider_SetMinVariable(&m_scrollBarMin);
			UpArrow->Slider_SetMaxVariable(&m_scrollBarMax);
			UpArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
			UpArrow->Set_SliderMult(-20.0f);
		}

		// get the Down arrow
		MyControl=OpenChild_SubControl(RGBA(25,125,76),"SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *DnArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (DnArrow)
		{
			DnArrow->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			DnArrow->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			DnArrow->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			DnArrow->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			DnArrow->Slider_SetVariable(&m_scrollBarVar);
			DnArrow->Slider_SetMinVariable(&m_scrollBarMin);
			DnArrow->Slider_SetMaxVariable(&m_scrollBarMax);
			DnArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
			DnArrow->Set_SliderMult(20.0f);
		}

	//Heres a temporary method to open the inouts
	SkinControl_SubControl_ToggleButton	*SubControl_ToggleButton;
	// the layers for all of the buttons
	long up = Controls_Button_UnSelected;
	long upro = Controls_Button_MouseOver;
	long dn = Controls_Button_Selected;
	long dnro = Controls_Button_MouseOverDn;

	//Setup the OpenInOuts secret button
	SubControl_ToggleButton=GetInterface<SkinControl_SubControl_ToggleButton> (OpenChild_SubControl(RGBA(73,128,157),"SkinControl_SubControl_ToggleButton"));
	if (!SubControl_ToggleButton) _throw ("VideoEditor::ReadClassData Could not create InOuts button !");
	SubControl_ToggleButton->Button_SetResource(Controls_Button_UnSelected, up);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_Selected, dn);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOver, upro);
	SubControl_ToggleButton->Button_SetResource(Controls_Button_MouseOverDn, dnro);
	SubControl_ToggleButton->Button_SetSelectedState(1);
	SubControl_ToggleButton->Button_UseVariable(&m_ToggleButton_dyn);

	AutoSkinControl::InitializeSubControls();
	}

void VideoEditor::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	if (ID == (long)(&m_ToggleButton_dyn)) {
		if (strcmp(String, Controls_Button_ReleasedOn) == 0) {
			int whichButton = m_ToggleButton_dyn.Get();
			if (whichButton==1) {
				m_InOuts2 = GetInterface<InOuts2_Control>
					(NewTek_New("InOuts2_Control", GetWindowHandle(), 0, 0, 0, 0, NULL, WS_POPUP|WS_VISIBLE));
				m_InOuts2->SetVideoEditor_Interface(m_layout);
				return;
				}
			}
		else if (strcmp(String, Controls_Button_ReleasedOff) == 0) {
			if (m_InOuts2) m_InOuts2->DeferredDelete();
			m_InOuts2=NULL;
			}
		}

	AutoSkinControl::DynamicCallback(ID,String,args,ItemChanging);
	}


bool VideoEditor::ExecuteCommand(	char *FunctionName,						// The string representation of the command to execute
										ScriptExecutionStack &InputParameters,	// The set of input parameters to your function
										ScriptExecutionStack &OutputParameters,	// You can use this to access variables as well
										char *&Error)							// If you supported the command but failed for some reason
																				// return a user string here
{	
	if (!strcmp(FunctionName,"LoadScrollBarBitmapV"))
	{
		if (InputParameters.GetSizeOfStack()<2)
		{
			Error = "LoadScrollBarBitmap: Function Missing Parameters, aborted";
			return true;
		}
		if (InputParameters.GetSizeOfStack()>2)
			Error = "LoadScrollBarBitmap: Too many parameters for function, extras ignored";
		if (InputParameters[0].GetType()==Script_STRING)
		{
			int i=InputParameters[1];
			if ((i >= 0) && (i < 4))
			{
				if (!m_VsliderBitmaps[i].ChangeFilename(GetAbsolutePath(m_skinPath,InputParameters[0])))
					Error = "LoadScrollBarBitmap: Failed to Load Layer Image";
				m_VsliderBitmaps[i].SetAlignment(BitmapTile_StretchX+BitmapTile_StretchY);
			}
			else Error = "LoadScrollBarBitmap: Invalid Parameter 1 (Use 0-3), aborted";
		}
		else Error = "LoadScrollBarBitmap: Invalid Parameter 0, aborted";
		return true;
	}
	else if (!strcmp(FunctionName,"LoadScrollBarBitmapH"))
	{
		return true;
	}
	else if (!strcmp(FunctionName,"Delete"))
	{
		m_layout->VideoEditor_DeleteSelectedElements();
		return true;
	}
	return AutoSkinControl::ExecuteCommand(FunctionName, InputParameters, OutputParameters, Error);
}
