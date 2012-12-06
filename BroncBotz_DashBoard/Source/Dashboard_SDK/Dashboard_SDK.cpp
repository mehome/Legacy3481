#include "stdafx.h"

#include "../Dashboard/Dashboard_Interfaces.h"
#include "../Dashboard/Dashboard.h"

//class SDK_Interface
//{
//public:
//	SDK_Interface()
//	{
//		m_PlugIn=GetModuleHandle(NULL);
//		if (m_PlugIn)
//		{
//			m_GetControllerInterface=(function_GetControllerInterface) GetProcAddress(m_PlugIn,"GetControllerInterface");
//			assert(m_GetControllerInterface);
//			m_ControllerInterface=(*m_GetControllerInterface)();
//			assert(m_ControllerInterface);
//		}
//	}
//	void Run() {m_ControllerInterface->Run();}
//	void Stop() {m_ControllerInterface->Stop();}
//	void Pause() {m_ControllerInterface->Pause();}
//	void Seek(double start, double end, bool scrubbing) {m_ControllerInterface->Seek(start,end,scrubbing);}
//	void SetRate(int rate) {m_ControllerInterface->SetRate(rate);}
//protected:
//private:
//	HMODULE m_PlugIn;
//	typedef Dashboard_Controller_Interface * (*function_GetControllerInterface)(void);
//	function_GetControllerInterface m_GetControllerInterface;
//	Dashboard_Controller_Interface *m_ControllerInterface;
//} *g_SDK;

Dashboard_Controller_Interface *g_SDK;

extern "C"
{
	void SmartCppDashboard_Initialize(Dashboard_Controller_Interface *controller)
	{
		g_SDK=controller;
	}
	void SmartCppDashboard_Run ()
	{
		g_SDK->Run();
	}
	void SmartCppDashboard_Stop ()
	{
		g_SDK->Stop();
	}
	void SmartCppDashboard_Pause ()
	{
		g_SDK->Pause();
	}
	void SmartCppDashboard_Seek (double start, double end,  bool scrubbing =false)
	{
		g_SDK->Seek(start,end,scrubbing);
	}
	void SmartCppDashboard_SetRate (int rate)
	{
		g_SDK->SetRate(rate);
	}
};


