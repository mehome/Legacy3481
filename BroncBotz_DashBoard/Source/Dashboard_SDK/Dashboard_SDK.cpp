#include "stdafx.h"

#include "../Dashboard/Dashboard_Interfaces.h"
#include "../Dashboard/Dashboard.h"

extern "C"
{
	void SmartCppDashboard_Run ()
	{
		if (GetControllerInterface()) GetControllerInterface()->Run();
	}
	void SmartCppDashboard_Stop ()
	{
		if (GetControllerInterface()) GetControllerInterface()->Stop();
	}
	void SmartCppDashboard_Pause ()
	{
		if (GetControllerInterface()) GetControllerInterface()->Pause();
	}
	void SmartCppDashboard_Seek (double start, double end,  bool scrubbing =false)
	{
		if (GetControllerInterface()) GetControllerInterface()->Seek(start,end,scrubbing);
	}
	void SmartCppDashboard_SetRate (int rate)
	{
		if (GetControllerInterface()) GetControllerInterface()->SetRate(rate);
	}
};
