#ifndef __STORYBOARDEDITOR__
#define __STORYBOARDEDITOR__

#define StoryBoard_Editor_SkinRoot		"VideoEditor/StoryBoard"

class InOuts2_Control;

class VIDEOEDITORDLL StoryBoard_Editor :	public VideoEditorClient_Interface,public BaseLoadSave_UiConfig {
	public:
		// Constructors and Destructors
		StoryBoard_Editor();
		~StoryBoard_Editor();

		// Get the window ready (from BaseWindowClass)
		virtual void InitialiseWindow(void);
		virtual void DestroyWindow(void);

		// Scripting Commands
		virtual bool ExecuteCommand(char *FunctionName,						// The string representation of the command to execute
									ScriptExecutionStack &InputParameters,	// The set of input parameters to your function
									ScriptExecutionStack &OutputParameters,	// You can use this to access variables as well
									char *&Error);							// If you supported the command but failed for some reason
																			// return a user string here
		virtual bool Persitance_ShouldBeUsed_Load(void) {return false;}
		virtual bool Persitance_ShouldBeUsed_Save(void) {return false;}
		virtual bool BaseLoadSave_UiConfig_ShouldISaveOrLoad(void) {return false;}

		virtual void VideoEditorClient_SetTimeLine(TimeLine *timeline);
		virtual VideoEditor_Interface *VideoEditorClient_GetLayout() {return m_layout;}

	protected:
		Dynamic<float>	m_scrollBarVar;
		Dynamic<float>	m_scrollBarMin;
		Dynamic<float>	m_scrollBarMax;
		Dynamic<float>	m_scrollBarWidth;


		Dynamic<float>	m_widthBarVar;

		// ScreenObjects we own for controls
		ScreenObject_BitmapFile	m_VsliderBitmaps[4];
		// The controls we need to keep handles on
		UtilLib_Slider*		m_Vslider;
		// My AutoSkinControl Callbacks
		virtual void InitializeSubControls();

		//! The TimeLine we want to edit
		TimeLine*	m_timeLine;

		VideoEditor_Interface* m_layout;

	private:
		// Override this function to create different types of editors
		virtual VideoEditor_Interface* CreateVideoEditor(unsigned color);

		// The skin
		Auto_StretchySkin *m_skin;
		char *m_Skin;
	};

#endif //__STORYBOARDEDITOR__
