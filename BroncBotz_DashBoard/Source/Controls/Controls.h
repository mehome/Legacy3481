#pragma once
#include "../Dashboard/Dashboard_Interfaces.h"

#ifdef CONTROLS_EXPORTS
#define CONTROLS_API __declspec(dllexport)
#else
#define CONTROLS_API __declspec(dllimport)
#endif

extern "C"
{
	//callbacks
	CONTROLS_API void Callback_SmartCppDashboard_Initialize(Dashboard_Controller_Interface *controller,DLGPROC gWinProc);
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
		virtual size_t GetDialogResource() const =0;
		virtual LPARAM GetInstance() const =0;
		virtual const wchar_t * const GetTitlePrefix() const =0;
		virtual long Dispatcher(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		HWND        m_hDlg;                     // HWND of Dialog.
    private:
        int OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam );
		bool m_IsClosing;
};
