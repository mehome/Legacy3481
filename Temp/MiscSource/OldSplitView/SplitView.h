#ifndef __SPLITVIEW__
#define __SPLITVIEW__

#define SplitView_SkinRoot		"VideoEditor/SplitView"

class InOuts2_Control;

class VIDEOEDITORDLL SplitView_Control :public UtilLib_Button {
	public:
		virtual void	MouseLButtonClick(long Flags,long x,long y);
		virtual void	MouseLButtonRelease(long Flags,long x,long y);
		virtual void	MouseMoved(long Flags,long x,long y);

		SplitView_Control();
		void SetChildWindows(HWND upper,HWND lower) {m_UpperChild=upper;m_LowerChild=lower;}
		void UpdateChildRects();

	private:
		HWND m_UpperChild;
		HWND m_LowerChild;
		long m_OffsetY;

	};

class VIDEOEDITORDLL SplitView :	public VideoEditorClient_Interface,public BaseLoadSave_UiConfig {
	public:
		// Constructors and Destructors
		SplitView();
		~SplitView();

		// Get the window ready (from BaseWindowClass)
		virtual void InitialiseWindow(void);
		virtual void DestroyWindow(void);

		virtual void VideoEditorClient_SetTimeLine(TimeLine *timeline);
		virtual VideoEditor_Interface *VideoEditorClient_GetLayout() {return m_layout;}
		virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

		virtual bool Persitance_ShouldBeUsed_Load(void) {return false;}
		virtual bool Persitance_ShouldBeUsed_Save(void) {return false;}
		virtual bool BaseLoadSave_UiConfig_ShouldISaveOrLoad(void) {return false;}

		void SetChildWindows(HWND upper,HWND lower);

	protected:
		// My AutoSkinControl Callbacks
		virtual void InitializeSubControls();

		//! The TimeLine we want to edit
		TimeLine*	m_timeLine;

		VideoEditor_Interface* m_layout;

	private:
		SplitView_Control *m_SplitView_Button;  //needed to pass the child windows to control
		// The skin
		Auto_StretchySkin *m_skin;
		char *m_Skin;
	};

#endif //__SPLITVIEW__
