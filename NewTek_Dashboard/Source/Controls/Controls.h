#pragma once
#include "../Dashboard/Dashboard_Interfaces.h"
#include "../ProcessingVision/Plugin_Control_Interface.h"

#ifdef CONTROLS_EXPORTS
#define CONTROLS_API __declspec(dllexport)
#else
#define CONTROLS_API __declspec(dllimport)
#endif

extern "C"
{
	//callbacks
	CONTROLS_API void Callback_SmartCppDashboard_Initialize(Dashboard_Controller_Interface *controller,DLGPROC gWinProc);
	CONTROLS_API void CallBack_SmartCppDashboard_Initialize_Plugin(Plugin_Controller_Interface *plugin);
	CONTROLS_API void Callback_SmartCppDashboard_StartedStreaming(HWND pParent);
	CONTROLS_API void Callback_SmartCppDashboard_Shutdown();
	/// Populate your own menu items to be appended to the default menu
	/// \param StartingOffset you must start your enum entries with this starting offset (this ensures it will not conflict with the existing entries)
	CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset);
	/// the value here will be any of your items starting with zero if they were selected (i.e. the starting offset is subtracted)
	CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection,HWND pParent);
};

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

void DebugOutput(const char *format, ... );

#define TBM_GETPOS              (WM_USER)
#define TBM_SETPOS              (WM_USER+5)

class DialogBase : public MessageBase_Interface
{
    public:
        DialogBase();
        virtual ~DialogBase(void);

		// Runs the Dialog.
        virtual bool Run(HWND pParent);
		void OnEndDialog(void);

	protected:
		//This method probably belongs to file controls, but I can see other dialogs wanting to use it
		BOOL getopenfile(wchar_t *dest,wchar_t *filename,const wchar_t *defpath,const wchar_t *defext,const wchar_t *inputprompt,const wchar_t *filter,BOOL musthave);
		//Easier to use version here
		/// \ret true if a file was picked (successful)
		bool getopenfilename(
			const wchar_t *inputprompt,			///What the file requester shows in the window title
			std::wstring &Output,				///The full path and filename of file selected
			BOOL musthave,						///For reading existing files this should be true... for writing new files this should be false
			const wchar_t *defaultPath=NULL,	///This may be ignored as windows will remember where you were for you
			const wchar_t *defaultExt=NULL);

		virtual size_t GetDialogResource() const =0;
		virtual LPARAM GetInstance() const =0;
		virtual const wchar_t * const GetTitlePrefix() const =0;
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		HWND        m_hDlg;                     // HWND of Dialog.
    private:
        int OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam );
		bool m_IsClosing;
};
