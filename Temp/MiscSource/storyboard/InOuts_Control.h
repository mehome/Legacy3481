#ifndef __InOuts_Control__
#define __InOuts_Control__

//*****************************************************************************************************
// Dependants ID numbers
#define InOuts_Control_Dependant_TotalClipLength		1000
#define InOuts_Control_Dependant_StretchDuration		1001
#define InOuts_Control_Dependant_InPoint				1002
#define InOuts_Control_Dependant_OutPoint				1003
#define InOuts_Control_Dependant_CroutonPoint			1004
#define InOuts_Control_Dependant_OffSetPoint			1005
#define InOuts_Control_Dependant_Generic				1006
#define InOuts_Control_Dependant_Alias					1007

//*****************************************************************************************************
class SBDLL InOuts_Control : public SkinControl
{	private:	//*****************************************************************************************************
				// The skin being used
					ScreenObjectSkin *m_ScreenObjectSkin;

				//*****************************************************************************************************
				// The framerate for the source footage
					Dynamic<float>	TotalClipLength;
					Dynamic<float>	NveTotalClipLength;
					Dynamic<float>	Zero,PlusOne,MinusOne;
					Dynamic<float>	BigNo;

					// Are we editing audio or video
					unsigned	GetAVFlag(void);
					bool		EditingAudio(void);
					bool		EditingVideo(void);

					// Setup the text items
					void SetupTextBoxes(void);
					float GetFrameRateToUse(void);

				//*****************************************************************************************************
				// In Points !
					UtilLib_Edit		*m_UtilLib_Edit_In;
					TextItem			m_TextItem_In;

					VideoPreviewWindow	*VPW_InPoint;
					SkinControl_SubControl_SliderButton *SSCS_InPoint_Up,*SSCS_InPoint_Dn;

				//*****************************************************************************************************
				// Out Points !
					UtilLib_Edit		*m_UtilLib_Edit_Out;
					TextItem			m_TextItem_Out;

					VideoPreviewWindow	*VPW_OutPoint;
					SkinControl_SubControl_SliderButton *SSCS_OutPoint_Up,*SSCS_OutPoint_Dn;

				//*****************************************************************************************************
				// Crouton Position
					UtilLib_Edit		*m_UtilLib_Edit_Crouton;
					TextItem			m_TextItem_Crouton;

					VideoPreviewWindow	*VPW_CroutonPoint;
					SkinControl_SubControl_SliderButton *SSCS_CroutonPoint_Up,*SSCS_CroutonPoint_Dn;

				//*****************************************************************************************************
				// Clip Duration
					UtilLib_Edit		*m_UtilLib_Edit_Duration;
					TextItem			m_TextItem_Duration;
					Dynamic<float>		DurationPoint;

					bool SetDuration(Dynamic<float> *In,Dynamic<float> *Out,float &Duration,float ClipLength);

				//*****************************************************************************************************
				// Crouton Duration
					UtilLib_Edit	*m_UtilLib_Edit_CroutonDuration;
					TextItem		m_TextItem_CroutonDuration;
					Dynamic<float>	CroutonDurationPoint;

				//*****************************************************************************************************
				// Clip OffSet
					UtilLib_Edit	*m_UtilLib_Edit_OffSet;
					TextItem		m_TextItem_OffSet;

					SkinControl_SubControl_SliderButton *SSCS_Offset_Up,*SSCS_Offset_Dn;

				//*****************************************************************************************************
				// The title
					UtilLib_Edit	*m_UtilLib_Edit_Title;
					TextItem		m_Title;

				//*****************************************************************************************************
				// The list of items being edited
				tList<StoryBoard_Crouton*> m_ItemsBeingEdited;

				// Reconfigure the first entry
				void ReConfigure(void);

				// Deal with dependants
				StoryBoard_Crouton*	m_LastDependants;
				void DeleteDependants(void);
				void AddDependants(StoryBoard_Crouton* Item);

				//*****************************************************************************************************
				// Put the correct things in all the undo buffers
				void DoUndo(void);

	public:		//*****************************************************************************************************
				// Get the window ready
				virtual void InitialiseWindow(void);
				virtual void DestroyWindow(void);

				//*****************************************************************************************************
				// The dynamic callbacks
				virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

				//*****************************************************************************************************
				// Control the list of items that are being edited at the current time
				// This item will return some undefined data in the SelectedItems tList.
				void InOuts_Control_Set(tList<StoryBoard_Crouton*> *SelectedItems);

				// Should this window be closed ?
				bool InOuts_Control_AnyEntriesLeft(void);

				//*****************************************************************************************************
				// Deferred Message cllback
				virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

				//*****************************************************************************************************
				// Constructor
				InOuts_Control(void);

				//*****************************************************************************************************
				// Put the list of GUIDs used into a list
				void SaveGUIDs(tList<GUID> *ListOfGUIDs);

				// A friend
				friend VideoPreviewWindow;
};

class SBDLL InOuts_Control_Default : public InOuts_Control
{};

#endif