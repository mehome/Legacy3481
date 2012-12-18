#ifndef __VIDEOEDITOR__
#define __VIDEOEDITOR__

#define VideoEditor_SkinRoot		"VideoEditor"
#define VideoEditor_TimeLineFile	"TimeLine.lsd"

class InOuts2_Control;

class VIDEOEDITORDLL VideoEditor :	public AutoSkinControl {
	public:
		// Constructors and Destructors
		VideoEditor();
		~VideoEditor();

		// Get the window ready (from BaseWindowClass)
		virtual void InitialiseWindow(void);
		virtual void DestroyWindow(void);
		virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

		static void OnProcessAttach();
		static void OnProcessDetach();

	protected:
		enum VideoEditorToggleButtons { NoButton, InOutButton=1, TimeLineButton=2, StoryBoardButton=4, BothButton=8 };
		// My AutoSkinControl Callbacks
		virtual void InitializeSubControls();

		//! The TimeLine we want to edit
		static TimeLine*	s_timeLine;

	private:
		// The skin
		Auto_StretchySkin *m_skin;
		char *m_Skin;
		//These are all mutually exclusive AutoSkin Controls to be placed in the client area
		VideoEditorClient_Interface *m_StoryBoardView;
		VideoEditorClient_Interface *m_TimeLineView;

		VideoEditorClient_Interface *m_SplitStoryBoardView;
		VideoEditorClient_Interface *m_SplitTimeLineView;
		SplitView *m_SplitView;

		//A way to open the InOuts
		InOuts2_Control *m_InOuts2;
		Dynamic<int> m_ToggleButton_dyn;
		VideoEditor_Crouton *m_VC; //last crouton selected to pass to inouts
		ContentInstance *m_ContentInstance; //needed to listen for deletion

		int m_ToggleButton_LastState; // Need this to correctly work w/ toggle buttons
	};

#endif //__VIDEOEDITOR__
