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

		// Scripting Commands
		virtual bool ExecuteCommand(char *FunctionName,						// The string representation of the command to execute
									ScriptExecutionStack &InputParameters,	// The set of input parameters to your function
									ScriptExecutionStack &OutputParameters,	// You can use this to access variables as well
									char *&Error);							// If you supported the command but failed for some reason
																			// return a user string here

		static void OnProcessAttach();
		static void OnProcessDetach();

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
		static TimeLine*	s_timeLine;

		VideoEditor_Interface* m_layout;

	private:
		// Override this function to create different types of editors
		virtual VideoEditor_Interface* CreateVideoEditor(unsigned color) = 0;

		// The skin
		Auto_StretchySkin *m_skin;
		char *m_Skin;

		//A way to open the InOuts
		InOuts2_Control *m_InOuts2;
		Dynamic<int> m_ToggleButton_dyn;
	};

class VIDEOEDITORDLL TimeLine_Editor : public VideoEditor
{
private:
	virtual VideoEditor_Interface* CreateVideoEditor(unsigned color);
};

class VIDEOEDITORDLL StoryBoard_Editor : public VideoEditor
{
private:
	virtual VideoEditor_Interface* CreateVideoEditor(unsigned color);
};

#endif //__VIDEOEDITOR__
