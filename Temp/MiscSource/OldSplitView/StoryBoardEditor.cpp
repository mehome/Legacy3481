#include "StdAfx.h"

StoryBoard_Editor::StoryBoard_Editor() {
	m_skin = NULL;
	m_Skin=StoryBoard_Editor_SkinRoot;
	m_scrollBarMin.Set(0.0f);
	m_scrollBarMax.Set(1.0f);
	m_widthBarVar.Set(0.3f);
	SetSkinRoot(FindFiles_FindFile(FINDFILE_SKINS,m_Skin));
	m_layout = NULL;
	m_timeLine=NULL;
	}

StoryBoard_Editor::~StoryBoard_Editor() {
	}


void StoryBoard_Editor::InitialiseWindow(void) {
	// Set up skin window
	m_skin = GetInterface<Auto_StretchySkin>(NewTek_New("Auto_StretchySkin"));
	if (!m_skin) _throw ("KeyerControl::Cannot create the stretchy skin !");
	Canvas_SetResource(m_skin);	
	AutoSkinControl::InitialiseWindow();
	} //end initialise window


void StoryBoard_Editor::DestroyWindow(void) {	
	// We must always call my predecssor
	AutoSkinControl::DestroyWindow();

	// Delete my resource
	if (m_skin) NewTek_Delete(m_skin);
	}

VideoEditor_Interface* StoryBoard_Editor:: CreateVideoEditor(unsigned color)
{
	m_layout = GetWindowInterface<VideoEditor_Interface>(OpenChild(color, "StoryBoard_Layout"));
	if (m_layout) m_layout->VideoEditor_SetTimeLine(m_timeLine);
	return m_layout;
}

void StoryBoard_Editor::VideoEditorClient_SetTimeLine(TimeLine *timeline) {
	m_timeLine=timeline;
	if (m_layout) m_layout->VideoEditor_SetTimeLine(m_timeLine);
	}

void StoryBoard_Editor::InitializeSubControls()
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

	AutoSkinControl::InitializeSubControls();
	}


bool StoryBoard_Editor::ExecuteCommand(	char *FunctionName,						// The string representation of the command to execute
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
