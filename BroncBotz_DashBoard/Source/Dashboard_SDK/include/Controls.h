#pragma once

#ifdef CONTROLS_EXPORTS
#define CONTROLS_API __declspec(dllexport)
#else
#define CONTROLS_API __declspec(dllimport)
#endif

extern "C"
{
	//simply pass Callback_Initialize() to here
	SmartCppDashboard_Initialize(Dashboard_Controller_Interface *controller);
	void SmartCppDashboard_Run ();											// run the filter graph
	void SmartCppDashboard_Stop ();											// stop filter graph
	void SmartCppDashboard_Pause ();										// pause filter graph
	void SmartCppDashboard_Seek (double start, double end,  bool scrubbing =false);	// seek to start/stop positions (in seconds)
	void SmartCppDashboard_SetRate (int rate);									// set the play speed  (as percentage of normal)

	//callbacks
	CONTROLS_API void Callback_Initialize(Dashboard_Controller_Interface *controller);
	/// Populate your own menu items to be appended to the default menu
	/// \param StartingOffset you must start your enum entries with this starting offset (this ensures it will not conflict with the existing entries)
	CONTROLS_API void Callback_SmartCppDashboard_AddMenuItems (HMENU hPopupMenu,size_t StartingOffset);
	/// the value here will be any of your items starting with zero if they were selected (i.e. the starting offset is subtracted)
	CONTROLS_API void Callback_SmartCppDashboard_On_Selection(int selection);
};
