#include "StdAfx.h"

//**************************************************************************************************************************
void InOuts_Control::SaveGUIDs(tList<GUID> *ListOfGUIDs)
{	// Add thelist of GUIDs being edited
	for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
	{	GUID Temp=m_ItemsBeingEdited[i]->GetGUIDOfTree();
		ListOfGUIDs->Add(Temp);
	}
}

//**************************************************************************************************************************
float InOuts_Control::GetFrameRateToUse(void)
{	if ((EditingAudio())&&(m_ItemsBeingEdited.NoItems))	
			return m_ItemsBeingEdited[0]->OriginalFrameRate[GetAVFlag()];
	else	return 29.97f;
}

//**************************************************************************************************************************
void InOuts_Control::SetupTextBoxes(void)
{	char Temp[16]; 

	ConvertTimecodeToString(	Temp,GetFrameRateToUse(),
								NewTek_fRound(	(m_ItemsBeingEdited.NoItems)?
												m_ItemsBeingEdited[0]->InPoint[GetAVFlag()]
												:0)); 
	m_TextItem_In.SetText(Temp);

	ConvertTimecodeToString(	Temp,GetFrameRateToUse(),
								NewTek_fRound(	(m_ItemsBeingEdited.NoItems)?
												m_ItemsBeingEdited[0]->OutPoint[GetAVFlag()]
												:0));
	m_TextItem_Out.SetText(Temp);

	ConvertTimecodeToString(	Temp,GetFrameRateToUse(),
								NewTek_fRound(	(m_ItemsBeingEdited.NoItems)?
												m_ItemsBeingEdited[0]->CroutonPoint[GetAVFlag()]
												:0));
	m_TextItem_Crouton.SetText(Temp);

	ConvertTimecodeToString(Temp,GetFrameRateToUse(),
								NewTek_fRound(	(m_ItemsBeingEdited.NoItems)?
												m_ItemsBeingEdited[0]->OffSetPoint[GetAVFlag()]
												:0));
	m_TextItem_OffSet.SetText(Temp);

	ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(DurationPoint)); 
	m_TextItem_Duration.SetText(Temp);

	ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(CroutonDurationPoint)); 
	m_TextItem_CroutonDuration.SetText(Temp);	
}

//**************************************************************************************************************************
void InOuts_Control::DeleteDependants(void)
{	if (!m_LastDependants) return;

	for(unsigned i=StoryBoard_Video;i<=StoryBoard_Audio;i++)
	{	m_LastDependants->TotalClipLength[i].DeleteDependant(this);
		m_LastDependants->StretchDuration[i].DeleteDependant(this);
		m_LastDependants->InPoint[i].DeleteDependant(this);
		m_LastDependants->OutPoint[i].DeleteDependant(this);
		m_LastDependants->CroutonPoint[i].DeleteDependant(this);
		m_LastDependants->OffSetPoint[i].DeleteDependant(this);		
	}

	m_LastDependants->AliasName.DeleteDependant(this);
	m_LastDependants->DeleteDependant(this);
}

void InOuts_Control::AddDependants(StoryBoard_Crouton* Item)
{	DeleteDependants();
	m_LastDependants=Item;

	if (m_LastDependants)
	{	for(unsigned i=StoryBoard_Video;i<=StoryBoard_Audio;i++)
		{	m_LastDependants->TotalClipLength[i].AddDependant(this,InOuts_Control_Dependant_TotalClipLength);
			m_LastDependants->StretchDuration[i].AddDependant(this,InOuts_Control_Dependant_StretchDuration);
			m_LastDependants->InPoint[i].AddDependant(this,InOuts_Control_Dependant_InPoint);
			m_LastDependants->OutPoint[i].AddDependant(this,InOuts_Control_Dependant_OutPoint);
			m_LastDependants->CroutonPoint[i].AddDependant(this,InOuts_Control_Dependant_CroutonPoint);
			m_LastDependants->OffSetPoint[i].AddDependant(this,InOuts_Control_Dependant_OffSetPoint);
		}
		m_LastDependants->AddDependant(this,InOuts_Control_Dependant_Generic);
		m_LastDependants->AliasName.AddDependant(this,InOuts_Control_Dependant_Alias);
	}
}

//**************************************************************************************************************************
void InOuts_Control::DoUndo(void)
{	// We need to make a list of all the StoryBoard parents that will be used
	tList<StoryBoard*> l_SB;

	// Cycle through all items and locate their parents
	for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
	{	// get the parent
		StoryBoard *SB=m_ItemsBeingEdited[i]->GetParentStoryboard();
		if (!SB) continue;

		// If it is not on the list, then add it
		if (!l_SB.Exists(SB)) l_SB.Add(SB);
	}

	// Now do all the undo operations
	for(i=0;i<l_SB.NoItems;l_SB[i++]->SaveCurrentStateToUndoBuffer());
}

//**************************************************************************************************************************
bool InOuts_Control::SetDuration(Dynamic<float> *In,Dynamic<float> *Out,float &Duration,float ClipLength)
{	// Ug, this is rather more complex than I would like ...
	float Centre=0.5*(In->Get()+Out->Get());
	float NewInPoint =Centre-Duration*0.5;
	float NewOutPoint=Centre+Duration*0.5;

	// Shifting clip
	if (NewInPoint<0)
	{	NewInPoint=0;
		NewOutPoint=Duration;
	}

	if (NewOutPoint>ClipLength)
	{	NewOutPoint=ClipLength;
		NewInPoint =ClipLength-Duration;
	}

	// Straight Clip
	if (NewInPoint<0)			NewInPoint=0;
	if (NewOutPoint>ClipLength)	NewOutPoint=ClipLength;

	// Set the values
	In ->Set(NewInPoint ,"InOuts_Control::NoDurationUpdate");
	Out->Set(NewOutPoint,"InOuts_Control::NoDurationUpdate");

	// Return the value
	if (fabs(NewOutPoint-NewInPoint-Duration)>1E-6) 
	{	Duration=NewOutPoint-NewInPoint;
		return true;
	}
	return false;	
}

//**************************************************************************************************************************
bool InOuts_Control::EditingAudio(void)
{	return true;
}

bool InOuts_Control::EditingVideo(void)
{	return true;
}

unsigned InOuts_Control::GetAVFlag(void)
{	return StoryBoard_Video;
}

//**************************************************************************************************************************
void InOuts_Control::ReConfigure(void)
{	// If there are any items on the list
	if (m_ItemsBeingEdited.NoItems)
	{	// Get the entries from the parent
		TotalClipLength		=m_ItemsBeingEdited[0]->TotalClipLength[GetAVFlag()];
		CroutonDurationPoint=m_ItemsBeingEdited[0]->StretchDuration[GetAVFlag()];
		DurationPoint		=fabs(m_ItemsBeingEdited[0]->OutPoint[GetAVFlag()]-m_ItemsBeingEdited[0]->InPoint[GetAVFlag()]);

		// Get the alias
		char Temp[MAX_PATH];
		m_ItemsBeingEdited[0]->GetAlias(Temp);
		m_Title.SetText(Temp);

		// Set the item for the videopreview windows to view
		if (VPW_InPoint)		VPW_InPoint->SetWindowToEdit(m_ItemsBeingEdited[0]);
		if (VPW_OutPoint)		VPW_OutPoint->SetWindowToEdit(m_ItemsBeingEdited[0]);
		if (VPW_CroutonPoint)	VPW_CroutonPoint->SetWindowToEdit(m_ItemsBeingEdited[0]);

		// We must be dependant on the correct thing
		AddDependants(m_ItemsBeingEdited[0]);
	}
	// If there are no items on the list
	else
	{	// Set the default parameters
		TotalClipLength=600;		
		CroutonDurationPoint=TotalClipLength;
		m_Title.SetText("Magic");
	}

	// Set all the standard variables
	NveTotalClipLength=-TotalClipLength;
	Zero=0;
	PlusOne=1;
	MinusOne=-1;
	BigNo=FLT_MAX;

	// Make sure that the text boxes are correct
	SetupTextBoxes();
}

//**************************************************************************************************************************
void InOuts_Control::InOuts_Control_Set(tList<StoryBoard_Crouton*> *SelectedItems)
{	// Delete all dependants
	for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		m_ItemsBeingEdited[i]->DeleteDependant(this);
	
	// Swap the data with the current list
	SelectedItems->ExchangeData(&m_ItemsBeingEdited);

	// Add all dependants
	for(i=0;i<m_ItemsBeingEdited.NoItems;i++)
		m_ItemsBeingEdited[i]->AddDependant(this);

	// Reconfigure the window
	ReConfigure();
}

bool InOuts_Control::InOuts_Control_AnyEntriesLeft(void)
{	return (m_ItemsBeingEdited.NoItems>0);
}

//**************************************************************************************************************************
void InOuts_Control::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	// Call my parent
	SkinControl::DynamicCallback(ID,String,args,ItemChanging);

	// if it is a window message, then don't waste my time
	if (IsWindowMessage(String)) return;

	// Is it a deletion, maybe of one of the items I am editing ?
	if (IsDeletion(String))
	{	void *Args=args;
		DynamicTalker *DT=NewTek_GetArguement<DynamicTalker*>(Args);

		// Check the dependant item
		if (DT==m_LastDependants) m_LastDependants=NULL;

		// Deletion of any of the items I am dependant on
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		if (DT==m_ItemsBeingEdited[i]) 
		{	// Delete the entry
			m_ItemsBeingEdited.DeleteEntry(i);

			// If there are no entries left, then we should close this window
			if (!m_ItemsBeingEdited.NoItems)	DeferredMessage();
			// If it is the first entry then we have to specify a new item
			else if (i==0)						DeferredMessage();

			// Since we found it, we are finished
			break;
		}

		// Video Preview Windows
		if (DT==VPW_InPoint)			VPW_InPoint=NULL;
		if (DT==VPW_OutPoint)			VPW_OutPoint=NULL;
		if (DT==VPW_CroutonPoint)		VPW_CroutonPoint=NULL;

		// Up down arrows
		if (DT==SSCS_InPoint_Up)		SSCS_InPoint_Up=NULL;
		if (DT==SSCS_InPoint_Dn)		SSCS_InPoint_Dn=NULL;
		if (DT==SSCS_OutPoint_Up)		SSCS_OutPoint_Up=NULL;
		if (DT==SSCS_OutPoint_Dn)		SSCS_OutPoint_Dn=NULL;
		if (DT==SSCS_CroutonPoint_Up)	SSCS_CroutonPoint_Up=NULL;
		if (DT==SSCS_CroutonPoint_Up)	SSCS_CroutonPoint_Dn=NULL;
		if (DT==SSCS_Offset_Up)			SSCS_Offset_Up=NULL;
		if (DT==SSCS_Offset_Dn)			SSCS_Offset_Dn=NULL;

		// Parent items
		if (DT==m_LastDependants)		m_LastDependants=NULL;

		SkinControl::DynamicCallback(ID,String,args,ItemChanged);

		return;
	}

	//***********************************************************************************************************************
	if ((ID==(long)VPW_InPoint)||(ID==(long)VPW_OutPoint)||(ID==(long)VPW_CroutonPoint))	// Is it in a video preview window ?
	{	if (!IsSliderMessage_Change(String))
		{	// Place all the items on the undo buffer
			if (!strcmp(Controls_Slider_Released,String)) DoUndo();
		}
		else
		{	// Get the slider ammount
			void	*Args=args;
			double	Inc=NewTek_GetArguement<double>(Args);

			// Which are we editing ?
			bool EditInPoint =(ID==(long)VPW_InPoint);
			bool EditOutPoint=(ID==(long)VPW_OutPoint);
			bool EditCrouton =(ID==(long)VPW_CroutonPoint);
			bool EditAudio	 =EditingAudio();
			bool EditVideo	 =EditingVideo();

			// If shift is held down then we move both in and out point
			if (GetAsyncKeyState(VK_SHIFT)&((short)(1<<15)))
			{	if ((EditInPoint)||(EditOutPoint))
					EditInPoint=EditOutPoint=true;		
			}

			// If control is help down then we only move the audio
			if (GetAsyncKeyState(VK_CONTROL)&((short)(1<<15)))
				EditVideo=false;
			
			// One of the points is being dragged
			for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
			{	// Change the In Point
				if (EditInPoint)
				{	if (EditAudio) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio].IncNM(Inc); 
					if (EditVideo) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video].IncNM(Inc);
				}

				// Change the Out Point
				if (EditOutPoint)
				{	if (EditAudio) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio].IncNM(Inc); 
					if (EditVideo) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video].IncNM(Inc);
				}

				// Change the Crouton Position
				if (EditCrouton)
				{	if (EditAudio) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Audio].IncNM(Inc); 
					if (EditVideo) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Video].IncNM(Inc);
				}
				// Recompute the duration
				else m_ItemsBeingEdited[i]->RecomputeDuration();

				// Issue the changed messages
				if (EditInPoint)
				{	if (EditAudio) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio].Changed(); 
					if (EditVideo) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video].Changed();
				}
				if (EditOutPoint)
				{	if (EditAudio) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio].Changed(); 
					if (EditVideo) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video].Changed();
				}
				if (EditCrouton)
				{	if (EditAudio) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Audio].Changed(); 
					if (EditVideo) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Video].Changed();
				}
			} // if (!IsSliderMessage_Change(String))
		}		
	}

	// Crouton changed
	else if (ID==InOuts_Control_Dependant_CroutonPoint)
	{	// What are we interested in ?
		int ChannelOfInterest=EditingVideo()?StoryBoard_Video:StoryBoard_Audio;
		
		// Turn the frame number into timecode
		char Temp[16]; 
		ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(m_ItemsBeingEdited[0]->CroutonPoint[ChannelOfInterest])); 		
		m_TextItem_Crouton.SetText(Temp);
	}

	else if (ID==(long)&m_TextItem_Crouton)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// Cycle through all the items to change
		bool EditAudio	 =EditingAudio();
		bool EditVideo	 =EditingVideo();
		
		// Turn the timecode into a frame number		
		int Frame;
		ConvertStringToTimecode(m_TextItem_Crouton.GetText(),GetFrameRateToUse(),Frame);		

		// Change all the entries
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	// Handle the inpoint change
			if (EditAudio) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Audio].SetNM(Frame); 
			if (EditVideo) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Video].SetNM(Frame);
	
			// Now execute the messages
			if (EditAudio) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Audio].Changed(); 
			if (EditVideo) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Video].Changed();
		}

		// Place all the items on the undo buffer
		DoUndo();
	}

	// Offset changed
	else if (ID==(long)SSCS_Offset_Up)
	{	// The correct message ?
		if (!IsSliderMessage_Change(String))
		{	// The value has changed, so I should probalby update my textbox
			char Temp[16]; 
			ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(DurationPoint)); 
			m_TextItem_Duration.SetText(Temp);
		}
		else
		{	// Which are we editing ?
			bool EditAudio=EditingAudio();
			bool EditVideo=EditingVideo();
			
			// How much have we moved ?
			void	*Args=args;
			double	Inc=NewTek_GetArguement<double>(Args);

			// One of the points is being dragged
			for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
			{	// Change the In Point
				if (EditAudio) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Audio].IncNM(Inc); 
				if (EditVideo) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Video].IncNM(Inc);			

				if (EditAudio) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Audio].Changed();
				if (EditVideo) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Video].Changed();
			}

			// Place all the items on the undo buffer
			DoUndo();
		}
	}

	else if (ID==(long)&m_TextItem_OffSet)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// Cycle through all the items to change
		bool EditAudio	 =EditingAudio();
		bool EditVideo	 =EditingVideo();

		// Turn the timecode into a frame number		
		int Frame;
		ConvertStringToTimecode(m_TextItem_OffSet.GetText(),GetFrameRateToUse(),Frame);

		// Change all the entries
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	// Handle the inpoint change
			if (EditAudio) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Audio].SetNM(Frame); 
			if (EditVideo) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Video].SetNM(Frame);
	
			// Now execute the messages
			if (EditAudio) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Audio].Changed(); 
			if (EditVideo) m_ItemsBeingEdited[i]->OffSetPoint[StoryBoard_Video].Changed();
		}

		// Place all the items on the undo buffer
		DoUndo();
	}

	// The duration value has been changed
	else if (ID==(long)&DurationPoint)
	{	// Check that we are responding only to +/i changes
		if (!IsSliderMessage_Change(String)) 
		{	// Place all the items on the undo buffer			
			if (!strcmp(Controls_Slider_Released,String)) DoUndo();
			
			// The value has changed, so I should probalby update my textbox
			char Temp[16]; 
			ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(DurationPoint)); 
			m_TextItem_Duration.SetText(Temp);
		}
		else
		{	// Get the slider ammount
			void	*Args=args;
			double	Inc=NewTek_GetArguement<double>(Args);			

			// Cycle through all the items to change
			bool EditAudio	 =EditingAudio();
			bool EditVideo	 =EditingVideo();

			// Setup the duration on all my children
			for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
			{	if (EditingAudio()) //.Set(m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio]+Inc); 
				{	// Ug this is rather more complex than I would like ...
					float l_Duration=fabs(m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio]-m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio])+Inc;
					if (SetDuration(&m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio],
									&m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio],
									l_Duration,
									m_ItemsBeingEdited[i]->TotalClipLength[StoryBoard_Audio]))
					{}
				}
				if (EditingVideo()) //m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video].Set(m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video]+Inc);
				{	// Ug this is rather more complex than I would like ...
					float l_Duration=fabs(m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video]-m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video])+Inc;
					if (SetDuration(&m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video],
									&m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video],
									l_Duration,
									m_ItemsBeingEdited[i]->TotalClipLength[StoryBoard_Video]))
					{}
				}

				m_ItemsBeingEdited[i]->RecomputeDuration();
			}			
		}
	}

	// The offset point was set from somewhere
	else if (ID==InOuts_Control_Dependant_OffSetPoint)
	{	// What are we interested in ?
		int ChannelOfInterest=EditingVideo()?StoryBoard_Video:StoryBoard_Audio;
		
		// Turn the frame number into timecode
		char Temp[16]; 
		ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(m_ItemsBeingEdited[0]->OffSetPoint[ChannelOfInterest])); 
		m_TextItem_OffSet.SetText(Temp);
	}

	// The InPoint or OutPoint where set from some control
	else if ((ID==InOuts_Control_Dependant_InPoint)||
			 (ID==InOuts_Control_Dependant_OutPoint))
	{	// What are we interested in ?
		int ChannelOfInterest=EditingVideo()?StoryBoard_Video:StoryBoard_Audio;
		
		// Turn the frame number into timecode
		char Temp[16]; 

		if (ID==InOuts_Control_Dependant_InPoint)
		{	ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(m_ItemsBeingEdited[0]->InPoint[ChannelOfInterest])); 
			m_TextItem_In.SetText(Temp);
		}
		else
		{	ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(m_ItemsBeingEdited[0]->OutPoint[ChannelOfInterest])); 
			m_TextItem_Out.SetText(Temp);
		}

		// Set the duration
		DurationPoint=fabs(m_ItemsBeingEdited[0]->OutPoint[ChannelOfInterest]-m_ItemsBeingEdited[0]->InPoint[ChannelOfInterest]);		
	}

	//***********************************************************************************************************************
	// Now handle things myself

	// In point text entry
	else if (ID==(long)&m_TextItem_In)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// Cycle through all the items to change
		bool EditAudio	 =EditingAudio();
		bool EditVideo	 =EditingVideo();
		
		// Turn the timecode into a frame number		
		int Frame;
		ConvertStringToTimecode(m_TextItem_In.GetText(),GetFrameRateToUse(),Frame);		

		// Change all the entries
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	// Handle the inpoint change
			if (EditAudio) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio].SetNM(Frame); 
			if (EditVideo) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video].SetNM(Frame);
	
			// Now execute the messages
			if (EditAudio) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio].Changed(); 
			if (EditVideo) m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video].Changed();

			// Compute the clip durations
			m_ItemsBeingEdited[i]->RecomputeDuration();
		}

		// Place all the items on the undo buffer
		DoUndo();
	}
	// Out point text entry
	else if (ID==(long)&m_TextItem_Out)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// Cycle through all the items to change
		bool EditAudio	 =EditingAudio();
		bool EditVideo	 =EditingVideo();
		
		// Turn the timecode into a frame number		
		int Frame;
		ConvertStringToTimecode(m_TextItem_Out.GetText(),GetFrameRateToUse(),Frame);		

		// Convert it to frames from the end
		if (!m_ItemsBeingEdited.NoItems) return;
		int dFrames_Audio=m_ItemsBeingEdited[0]->TotalClipLength[StoryBoard_Audio]-Frame;
		int dFrames_Video=m_ItemsBeingEdited[0]->TotalClipLength[StoryBoard_Video]-Frame;

		// Cycle through all children making the changes
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	// Handle the inpoint change
			if (EditAudio) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio].SetNM(
								m_ItemsBeingEdited[i]->TotalClipLength[StoryBoard_Audio]-dFrames_Audio); 
			if (EditVideo) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video].SetNM(
								m_ItemsBeingEdited[i]->TotalClipLength[StoryBoard_Video]-dFrames_Video); 

			// Now execute the messages
			if (EditAudio) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio].Changed(); 
			if (EditVideo) m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video].Changed();

			// Compute the clip durations
			m_ItemsBeingEdited[i]->RecomputeDuration();
		}

		// Place all the items on the undo buffer
		DoUndo();
	}

	// Crouton text entry
	else if (ID==(long)&m_UtilLib_Edit_Crouton)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;

		// Cycle through all the items to change
		bool EditAudio=EditingAudio();
		bool EditVideo=EditingVideo();

		// Turn the timecode into a frame number		
		int Frame;
		ConvertStringToTimecode(m_TextItem_Crouton.GetText(),GetFrameRateToUse(),Frame);		

		// Change all the entries
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	// Handle the inpoint change
			if (EditAudio) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Audio].SetNM(Frame); 
			if (EditVideo) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Video].SetNM(Frame);
	
			// Now execute the messages
			if (EditAudio) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Audio].Changed(); 
			if (EditVideo) m_ItemsBeingEdited[i]->CroutonPoint[StoryBoard_Video].Changed();
		}

		// Place all the items on the undo buffer
		DoUndo();
	}

	// Manually set the duration of all the clips
	else if (ID==(long)&m_TextItem_Duration)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// Cycle through all the items to change
		bool EditAudio	 =EditingAudio();
		bool EditVideo	 =EditingVideo();		

		// Turn the timecode into a frame number		
		int Duration;
		ConvertStringToTimecode(m_TextItem_Duration.GetText(),GetFrameRateToUse(),Duration);

		// Setup the duration on all my children
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	if (EditingAudio()) //.Set(m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio]+Inc); 
			{	// Ug this is rather more complex than I would like ...
				float l_Duration=Duration;
				if (SetDuration(&m_ItemsBeingEdited[i]->InPoint[StoryBoard_Audio],
								&m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Audio],
								l_Duration,
								m_ItemsBeingEdited[i]->TotalClipLength[StoryBoard_Audio]))
				{}
			}
			if (EditingVideo()) //m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video].Set(m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video]+Inc);
			{	// Ug this is rather more complex than I would like ...
				float l_Duration=Duration;
				if (SetDuration(&m_ItemsBeingEdited[i]->InPoint[StoryBoard_Video],
								&m_ItemsBeingEdited[i]->OutPoint[StoryBoard_Video],
								l_Duration,
								m_ItemsBeingEdited[i]->TotalClipLength[StoryBoard_Video]))
				{}
			}

			// Compute the clip durations
			m_ItemsBeingEdited[i]->RecomputeDuration();
		}

		// Place all the items on the undo buffer
		DoUndo();
	}

	// The title has changed
	else if (ID==(long)&m_Title)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// We assign the title to all selected items !
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
		{	// Setup the alias in the StoryBoard item
			m_ItemsBeingEdited[i]->AliasName.SetText(m_Title.GetText(),UtilLib_Edit_LoseFocusChanged);
		}

		// Place all the items on the undo buffer
		DoUndo();
	}
	else if (ID==InOuts_Control_Dependant_Alias)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;

		// Get the alias, and change my text entry
		if (m_ItemsBeingEdited.NoItems)
			m_Title.SetText(m_ItemsBeingEdited[0]->AliasName.GetText());
	}

	else if (ID==InOuts_Control_Dependant_StretchDuration)
	{	// We need to update the text item for this one
		if (m_ItemsBeingEdited.NoItems)
		{	int		ChannelOfInterest=EditingVideo()?StoryBoard_Video:StoryBoard_Audio;
			float	CurrentFrames=m_ItemsBeingEdited[0]->GetDuration(ChannelOfInterest);

			char Temp[16];
			ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(CurrentFrames));
			m_TextItem_CroutonDuration.SetText(Temp);
		}
	}

	// The crouton duration point has changed
	else if (ID==(long)&CroutonDurationPoint)
	{	if (!IsSliderMessage_Change(String))
		{	// Place all the items on the undo buffer
			if (!strcmp(Controls_Slider_Released,String)) DoUndo();
		}
		else
		{	// Get the slider ammount
			void	*Args=args;
			double	Inc=NewTek_GetArguement<double>(Args);

			// Cycle through all the items to change
			bool EditAudio=EditingAudio();
			bool EditVideo=EditingVideo();
		
			// Cycle through all the children, and modify all the lengths of all the clips
			for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
			{	// The audio
				if (EditAudio)
				{	float CurrentFrames=m_ItemsBeingEdited[i]->GetDuration(StoryBoard_Audio);
					CurrentFrames+=Inc;
					m_ItemsBeingEdited[i]->SetDuration(CurrentFrames,StoryBoardFlag_Audio);
				}

				// The audio
				if (EditVideo)
				{	float CurrentFrames=m_ItemsBeingEdited[i]->GetDuration(StoryBoard_Video);
					CurrentFrames+=Inc;
					m_ItemsBeingEdited[i]->SetDuration(CurrentFrames,StoryBoardFlag_Video);
				}
			}

			// What are we interested in ?			
			if (m_ItemsBeingEdited.NoItems)
			{	int		ChannelOfInterest=EditingVideo()?StoryBoard_Video:StoryBoard_Audio;
				float	CurrentFrames=m_ItemsBeingEdited[0]->GetDuration(ChannelOfInterest);

				char Temp[16];
				ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(CurrentFrames));
				m_TextItem_CroutonDuration.SetText(Temp);
			}
		}
	}

	// Has the time changed ?
	else if (ID==(long)&m_TextItem_CroutonDuration)
	{	// Only when the keyboard entry loses focus !
		if (strcmp(String,UtilLib_Edit_LoseFocusChanged)) return;
		
		// Here we turn timecode into the value
		// Turn the timecode into a frame number		
		int Frame;
		ConvertStringToTimecode(m_TextItem_CroutonDuration.GetText(),GetFrameRateToUse(),Frame);

		// Cycle through all the items to change
		bool EditAudio=EditingAudio();
		bool EditVideo=EditingVideo();
		unsigned Flags=(EditAudio?StoryBoardFlag_Audio:0)|(EditVideo?StoryBoardFlag_Video:0);

		// We assign the title to all selected items !
		for(unsigned i=0;i<m_ItemsBeingEdited.NoItems;i++)
			m_ItemsBeingEdited[i]->SetDuration(Frame,Flags);

		// Validate the timecode !
		if (m_ItemsBeingEdited.NoItems)
		{	int		ChannelOfInterest=EditingVideo()?StoryBoard_Video:StoryBoard_Audio;
			float	CurrentFrames=m_ItemsBeingEdited[0]->GetDuration(ChannelOfInterest);
			char Temp[16];
			ConvertTimecodeToString(Temp,GetFrameRateToUse(),NewTek_fRound(CurrentFrames));
			m_TextItem_CroutonDuration.SetText(Temp);
		}

		// Copy to the undo buffer
		DoUndo();
	}

	// The mouse cursor was released in one of my controls, so the current state should go into the undo buffer
	else if (!strcmp(Controls_Slider_Released,String))
	{	DoUndo();
	}
}

//**************************************************************************************************************************
void InOuts_Control::InitialiseWindow(void)
{	// Setup the variables
		ReConfigure();
	
	// We load in my resources
		PluginClass	*PC=NewTek_New("ScreenObjectSkin");	
		m_ScreenObjectSkin=GetInterface<ScreenObjectSkin>(PC);
		if (!m_ScreenObjectSkin) _throw "InOuts_Control::InitialiseWindow could not create skin.";

	// Are we a default IO Control, or the standard on ?
	if (!strcmp("InOuts_Control",GetTypeName(this)))
	{	// Load the layers
		m_ScreenObjectSkin->ScreenObjectSkin_LoadRegionBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts\\IOpropMask.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts\\IOpropOFF.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts\\IOpropON.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts\\IOpropOFFro.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts\\IOpropONro.gif"));
	} else
	{	// Load the layers
		m_ScreenObjectSkin->ScreenObjectSkin_LoadRegionBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts_Default\\IOpropMask.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts_Default\\IOpropOFF.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts_Default\\IOpropON.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts_Default\\IOpropOFFro.gif"));
		m_ScreenObjectSkin->ScreenObjectSkin_LoadLayerBmp	(FindFiles_FindFile(FINDFILE_SKINS,"StoryBoard\\InOuts_Default\\IOpropONro.gif"));
	}

	// Create the window moving control
		SkinControl_SubControl *MyControl=OpenChild_SubControl(RGBA(0,0,0),"SkinControl_SubControl_Move");
		SkinControl_SubControl_Move	*sccm_Moving=GetInterface<SkinControl_SubControl_Move>(MyControl);
		if (!sccm_Moving) _throw "Could not case to SkinControl_SubControl_Move";	

	// This is now the resource
		Canvas_SetResource(m_ScreenObjectSkin);

	// We create the in/out textbox stuff		

		//**** In Points *********************************************
			HWND hWnd=OpenChild(RGBA(229,254,15),"UtilLib_Edit");
			m_UtilLib_Edit_In=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_In) _throw "InOuts_Control::InitialiseWindow Cannot create in window.";
			m_TextItem_In.SetFont("MS Sans Serif");
			m_TextItem_In.SetFontSize(-1.5);
			m_TextItem_In.SetAlignment(TextItem_RightAlign|TextItem_CentreVAlign|TextItem_TextLabel);			
			m_UtilLib_Edit_In->Controls_Edit_SetItem(&m_TextItem_In);

			MyControl=OpenChild_SubControl(RGBA(150,14,254),"SkinControl_SubControl_SliderButton");
			SSCS_InPoint_Up=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			SSCS_InPoint_Up->Changed();
			if (!SSCS_InPoint_Up) _throw "InOuts_Control::InitialiseWindow Could not create slider button !";
			SSCS_InPoint_Up->Slider_SetSliderWidth(&PlusOne);
			SSCS_InPoint_Up->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_InPoint_Up->Button_SetResource(Controls_Button_Selected,2);
			SSCS_InPoint_Up->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_InPoint_Up->Button_SetResource(Controls_Button_MouseOverDn,3);

			MyControl=OpenChild_SubControl(RGBA(212,15,254),"SkinControl_SubControl_SliderButton");
			SSCS_InPoint_Dn=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_InPoint_Dn) _throw "InOuts_Control::InitialiseWindow Could not create slider button !";
			SSCS_InPoint_Dn->Slider_SetSliderWidth(&MinusOne);
			SSCS_InPoint_Dn->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_InPoint_Dn->Button_SetResource(Controls_Button_Selected,2);
			SSCS_InPoint_Dn->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_InPoint_Dn->Button_SetResource(Controls_Button_MouseOverDn,3);

		//**** Out Points *********************************************			
			hWnd=OpenChild(RGBA(144,254,15),"UtilLib_Edit");
			m_UtilLib_Edit_Out=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_Out) _throw "InOuts_Control::InitialiseWindow Cannot create out window.";
			m_TextItem_Out.SetFont("MS Sans Serif");
			m_TextItem_Out.SetFontSize(-1.5);
			m_TextItem_Out.SetAlignment(TextItem_RightAlign|TextItem_CentreVAlign|TextItem_TextLabel);
			m_UtilLib_Edit_Out->Controls_Edit_SetItem(&m_TextItem_Out);

			MyControl=OpenChild_SubControl(RGBA(241,254,29),"SkinControl_SubControl_SliderButton");
			SSCS_OutPoint_Up=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_OutPoint_Up) _throw "OutOuts_Control::InitialiseWindow Could not create slider button !";
			SSCS_OutPoint_Up->Slider_SetSliderWidth(&PlusOne);
			SSCS_OutPoint_Up->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_OutPoint_Up->Button_SetResource(Controls_Button_Selected,2);
			SSCS_OutPoint_Up->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_OutPoint_Up->Button_SetResource(Controls_Button_MouseOverDn,3);

			MyControl=OpenChild_SubControl(RGBA(167,254,29),"SkinControl_SubControl_SliderButton");
			SSCS_OutPoint_Dn=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_OutPoint_Dn) _throw "OutOuts_Control::Slider_SetMinVariable Could not create slider button !";
			SSCS_OutPoint_Dn->Slider_SetSliderWidth(&MinusOne);
			SSCS_OutPoint_Dn->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_OutPoint_Dn->Button_SetResource(Controls_Button_Selected,2);
			SSCS_OutPoint_Dn->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_OutPoint_Dn->Button_SetResource(Controls_Button_MouseOverDn,3);

		//**** Crouton Position *********************************************
			hWnd=OpenChild(RGBA(12,92,254),"UtilLib_Edit");
			m_UtilLib_Edit_Crouton=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_Crouton) _throw "InOuts_Control::InitialiseWindow Cannot create out window.";
			m_TextItem_Crouton.SetFont("MS Sans Serif");
			m_TextItem_Crouton.SetAlignment(TextItem_RightAlign|TextItem_CentreVAlign|TextItem_TextLabel);
			m_TextItem_Crouton.SetFontSize(-1.1f);
			m_UtilLib_Edit_Crouton->Controls_Edit_SetItem(&m_TextItem_Crouton);

			MyControl=OpenChild_SubControl(RGBA(254,15,201),"SkinControl_SubControl_SliderButton");
			SSCS_CroutonPoint_Up=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_CroutonPoint_Up) _throw "CroutonCroutons_Control::InitialiseWindow Could not create slider button !";
			SSCS_CroutonPoint_Up->Slider_SetSliderWidth(&PlusOne);
			SSCS_CroutonPoint_Up->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_CroutonPoint_Up->Button_SetResource(Controls_Button_Selected,2);
			SSCS_CroutonPoint_Up->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_CroutonPoint_Up->Button_SetResource(Controls_Button_MouseOverDn,3);

			MyControl=OpenChild_SubControl(RGBA(254,14,116),"SkinControl_SubControl_SliderButton");
			SSCS_CroutonPoint_Dn=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_CroutonPoint_Dn) _throw "CroutonCroutons_Control::Slider_SetMinVariable Could not create slider button !";
			SSCS_CroutonPoint_Dn->Slider_SetSliderWidth(&MinusOne);
			SSCS_CroutonPoint_Dn->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_CroutonPoint_Dn->Button_SetResource(Controls_Button_Selected,2);
			SSCS_CroutonPoint_Dn->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_CroutonPoint_Dn->Button_SetResource(Controls_Button_MouseOverDn,3);

		//**** Duration Position *********************************************			
			hWnd=OpenChild(RGBA(12,12,254),"UtilLib_Edit");
			m_UtilLib_Edit_Duration=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_Duration) _throw "InOuts_Control::InitialiseWindow Cannot create out window.";
			m_TextItem_Duration.SetFont("MS Sans Serif");
			m_TextItem_Duration.SetAlignment(TextItem_RightAlign|TextItem_CentreVAlign|TextItem_TextLabel);
			m_TextItem_Duration.SetFontSize(-1.1f);
			m_UtilLib_Edit_Duration->Controls_Edit_SetItem(&m_TextItem_Duration);

			MyControl=OpenChild_SubControl(RGBA(254,118,28),"SkinControl_SubControl_SliderButton");
			SkinControl_SubControl_SliderButton *SB=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SB) _throw "DurationDurations_Control::InitialiseWindow Could not create slider button !";
			SB->Slider_SetVariable(&DurationPoint);
			SB->Slider_SetMaxVariable(&TotalClipLength);
			SB->Slider_SetMinVariable(&Zero);
			SB->Slider_SetSliderWidth(&PlusOne);
			SB->Button_SetResource(Controls_Button_UnSelected,0);
			SB->Button_SetResource(Controls_Button_Selected,2);
			SB->Button_SetResource(Controls_Button_MouseOver,1);
			SB->Button_SetResource(Controls_Button_MouseOverDn,3);

			MyControl=OpenChild_SubControl(RGBA(254,194,30),"SkinControl_SubControl_SliderButton");
			SB=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SB) _throw "DurationDurations_Control::Slider_SetMinVariable Could not create slider button !";
			SB->Slider_SetVariable(&DurationPoint);
			SB->Slider_SetMaxVariable(&TotalClipLength);
			SB->Slider_SetMinVariable(&Zero);
			SB->Slider_SetSliderWidth(&MinusOne);
			SB->Button_SetResource(Controls_Button_UnSelected,0);
			SB->Button_SetResource(Controls_Button_Selected,2);
			SB->Button_SetResource(Controls_Button_MouseOver,1);
			SB->Button_SetResource(Controls_Button_MouseOverDn,3);

		//**** CroutonDuration Position *********************************************
			hWnd=OpenChild(RGBA(102,110,176),"UtilLib_Edit");
			m_UtilLib_Edit_CroutonDuration=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_CroutonDuration) _throw "InOuts_Control::InitialiseWindow Cannot create out window.";
			m_TextItem_CroutonDuration.SetFont("MS Sans Serif");
			m_TextItem_CroutonDuration.SetAlignment(TextItem_RightAlign|TextItem_CentreVAlign|TextItem_TextLabel);
			m_TextItem_CroutonDuration.SetFontSize(-1.1f);
			m_UtilLib_Edit_CroutonDuration->Controls_Edit_SetItem(&m_TextItem_CroutonDuration);

			MyControl=OpenChild_SubControl(RGBA(237,107,158),"SkinControl_SubControl_SliderButton");
			SB=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SB) _throw "CroutonDurationCroutonDurations_Control::InitialiseWindow Could not create slider button !";
			SB->Slider_SetVariable(&CroutonDurationPoint);
			SB->Slider_SetMaxVariable(&BigNo);
			SB->Slider_SetMinVariable(&PlusOne);
			SB->Slider_SetSliderWidth(&PlusOne);
			SB->Button_SetResource(Controls_Button_UnSelected,0);
			SB->Button_SetResource(Controls_Button_Selected,2);
			SB->Button_SetResource(Controls_Button_MouseOver,1);
			SB->Button_SetResource(Controls_Button_MouseOverDn,3);

			MyControl=OpenChild_SubControl(RGBA(236,107,118),"SkinControl_SubControl_SliderButton");
			SB=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SB) _throw "CroutonDurationCroutonDurations_Control::Slider_SetMinVariable Could not create slider button !";
			SB->Slider_SetVariable(&CroutonDurationPoint);
			SB->Slider_SetMaxVariable(&BigNo);
			SB->Slider_SetMinVariable(&PlusOne);
			SB->Slider_SetSliderWidth(&MinusOne);
			SB->Button_SetResource(Controls_Button_UnSelected,0);
			SB->Button_SetResource(Controls_Button_Selected,2);
			SB->Button_SetResource(Controls_Button_MouseOver,1);
			SB->Button_SetResource(Controls_Button_MouseOverDn,3);

		//**** OffSet Position *********************************************			
			hWnd=OpenChild(RGBA(30,42,99),"UtilLib_Edit");
			m_UtilLib_Edit_OffSet=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_OffSet) _throw "InOuts_Control::InitialiseWindow Cannot create out window.";
			m_TextItem_OffSet.SetFont("MS Sans Serif");
			m_TextItem_OffSet.SetAlignment(TextItem_RightAlign|TextItem_CentreVAlign|TextItem_TextLabel);
			m_UtilLib_Edit_OffSet->Controls_Edit_SetItem(&m_TextItem_OffSet);

			MyControl=OpenChild_SubControl(RGBA(151,0,56),"SkinControl_SubControl_SliderButton");
			SSCS_Offset_Up=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_Offset_Up) _throw "DurationDurations_Control::InitialiseWindow Could not create slider button !";
			SSCS_Offset_Up->Slider_SetSliderWidth(&PlusOne);
			SSCS_Offset_Up->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_Offset_Up->Button_SetResource(Controls_Button_Selected,2);
			SSCS_Offset_Up->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_Offset_Up->Button_SetResource(Controls_Button_MouseOverDn,3);

			MyControl=OpenChild_SubControl(RGBA(109,0,38),"SkinControl_SubControl_SliderButton");
			SSCS_Offset_Dn=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
			if (!SSCS_Offset_Dn) _throw "DurationDurations_Control::Slider_SetMinVariable Could not create slider button !";
			SSCS_Offset_Dn->Slider_SetSliderWidth(&MinusOne);
			SSCS_Offset_Dn->Button_SetResource(Controls_Button_UnSelected,0);
			SSCS_Offset_Dn->Button_SetResource(Controls_Button_Selected,2);
			SSCS_Offset_Dn->Button_SetResource(Controls_Button_MouseOver,1);
			SSCS_Offset_Dn->Button_SetResource(Controls_Button_MouseOverDn,3);

		//**** The Title Window ********************************************
			hWnd=OpenChild(RGBA(0,117,36),"UtilLib_Edit");
			m_UtilLib_Edit_Title=GetWindowInterface<UtilLib_Edit>(hWnd);
			if (!m_UtilLib_Edit_Title) _throw "InOuts_Control::InitialiseWindow Cannot create out window.";
			m_Title.SetFont("MS Sans Serif");
			m_Title.SetBold(true);
			m_Title.SetAlignment(TextItem_CentreHAlign|TextItem_CentreVAlign|TextItem_TextLabel);			
			m_UtilLib_Edit_Title->Controls_Edit_SetItem(&m_Title);

		//**** The DirectDraw windows **************************************
			hWnd=OpenChild(RGBA(254,15,15), "VideoPreviewWindow");
			VPW_InPoint=GetWindowInterface<VideoPreviewWindow>(hWnd);
			VPW_InPoint->SetEditingPosition(VideoPreviewWindow_In);

			hWnd=OpenChild(RGBA(254,184,15), "VideoPreviewWindow");
			VPW_OutPoint=GetWindowInterface<VideoPreviewWindow>(hWnd);
			VPW_OutPoint->SetEditingPosition(VideoPreviewWindow_Out);

			hWnd=OpenChild(RGBA(54,254,15),"VideoPreviewWindow");
			VPW_CroutonPoint=GetWindowInterface<VideoPreviewWindow>(hWnd);			
			VPW_CroutonPoint->SetEditingPosition(VideoPreviewWindow_Crouton);

			// Setup the views
			ReConfigure();

		//**** Add Dependants **********************************************
			m_TextItem_In		.AddDependant(this,(long)&m_TextItem_In);
			m_TextItem_Out		.AddDependant(this,(long)&m_TextItem_Out);		
			m_TextItem_Crouton	.AddDependant(this,(long)&m_TextItem_Crouton);
			m_TextItem_Duration	.AddDependant(this,(long)&m_TextItem_Duration);
			m_TextItem_OffSet	.AddDependant(this,(long)&m_TextItem_OffSet);
			m_TextItem_CroutonDuration.AddDependant(this,(long)&m_TextItem_CroutonDuration);

			m_Title				.AddDependant(this,(long)&m_Title);

			DurationPoint		.AddDependant(this,(long)&DurationPoint);			
			CroutonDurationPoint.AddDependant(this,(long)&CroutonDurationPoint);
			TotalClipLength		.AddDependant(this,(long)&TotalClipLength);								

			VPW_InPoint	   ->AddDependant(this,(long)VPW_InPoint);
			SSCS_InPoint_Up->AddDependant(this,(long)VPW_InPoint);
			SSCS_InPoint_Dn->AddDependant(this,(long)VPW_InPoint);

			VPW_OutPoint	->AddDependant(this,(long)VPW_OutPoint);
			SSCS_OutPoint_Up->AddDependant(this,(long)VPW_OutPoint);
			SSCS_OutPoint_Dn->AddDependant(this,(long)VPW_OutPoint);

			VPW_CroutonPoint	->AddDependant(this,(long)VPW_CroutonPoint);
			SSCS_CroutonPoint_Up->AddDependant(this,(long)VPW_CroutonPoint);
			SSCS_CroutonPoint_Dn->AddDependant(this,(long)VPW_CroutonPoint);

			SSCS_Offset_Up->AddDependant(this,(long)SSCS_Offset_Up);
			SSCS_Offset_Dn->AddDependant(this,(long)SSCS_Offset_Up);
}

//**************************************************************************************************************************
void InOuts_Control::DestroyWindow(void)
{	// We scan back up my parents, looking for a StoryBoard window,
	// which is what is the normal entry to open these windows
	HWND hWnd=GetWindowHandle();

	while(true)
	{	// Try casting it to a SBoard member
		StoryBoard *SB=GetWindowInterface<StoryBoard>(hWnd);

		// If it was found, we are done !
		if (SB)
		{	// Notify it that I am now on my way out !


			// Finished my searching !
			break;
		}

		// Get the parent
		hWnd=::GetParent(hWnd);
		if (!hWnd) break;
	};
	
	// remove all dependants
	TotalClipLength.DeleteDependant(this);
	Zero.DeleteDependant(this);
	m_TextItem_In.DeleteDependant(this);
	m_TextItem_Out.DeleteDependant(this);
	m_TextItem_Crouton.DeleteDependant(this);
	m_TextItem_Duration.DeleteDependant(this);
	DurationPoint.DeleteDependant(this);
	m_TextItem_OffSet.DeleteDependant(this);
	TotalClipLength.DeleteDependant(this);
	m_Title.DeleteDependant(this);
	CroutonDurationPoint.DeleteDependant(this);
	m_TextItem_CroutonDuration.DeleteDependant(this);

	// Remove inpoint dependants
	if (VPW_InPoint)			VPW_InPoint->DeleteDependant(this);
	if (SSCS_InPoint_Up)		SSCS_InPoint_Up->DeleteDependant(this);
	if (SSCS_InPoint_Dn)		SSCS_InPoint_Dn->DeleteDependant(this);

	// Remove outpoint dependants
	if (VPW_OutPoint)			VPW_OutPoint->DeleteDependant(this);
	if (SSCS_OutPoint_Up)		SSCS_OutPoint_Up->DeleteDependant(this);
	if (SSCS_OutPoint_Dn)		SSCS_OutPoint_Dn->DeleteDependant(this);

	// Remove crouton view dependants
	if (VPW_CroutonPoint)		VPW_CroutonPoint->DeleteDependant(this);
	if (SSCS_CroutonPoint_Up)	SSCS_CroutonPoint_Up->DeleteDependant(this);
	if (SSCS_CroutonPoint_Dn)	SSCS_CroutonPoint_Dn->DeleteDependant(this);	

	if (SSCS_Offset_Up)			SSCS_Offset_Up->DeleteDependant(this);
	if (SSCS_Offset_Dn)			SSCS_Offset_Dn->DeleteDependant(this);

	// Remove all dependants that we can 
	DeleteDependants();

	SkinControl::DestroyWindow();

	// Delete the resource
	if (m_ScreenObjectSkin) NewTek_Delete(m_ScreenObjectSkin);
}

//**************************************************************************************************************************
// Constructor
InOuts_Control::InOuts_Control(void)
{	// We set these to null incase Initialise window is called before the
	// root item is set
	VPW_InPoint=VPW_OutPoint=VPW_CroutonPoint=NULL;
	m_LastDependants=NULL;
}

//*****************************************************************************************************
// Deferred Message cllback
void InOuts_Control::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{	// Are we a default IO Control, or the standard on ?
	if (m_ItemsBeingEdited.NoItems) ReConfigure();
	/*if (!strcmp("InOuts_Control",GetTypeName(this)))
	{	if (m_ItemsBeingEdited.NoItems) ReConfigure();
		else NewTek_Delete(this);
	}*/
}