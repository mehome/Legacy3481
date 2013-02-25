
#include "stdafx.h"
#include "ProcessingVision.h"
#include "NI_VisionProcessingBase.h"
#include "VisionGoalTracker.h"
#include "VisionRinTinTinTracker.h"

UDP_Client_Interface *g_UDP_Output=NULL;
extern VisionTracker* g_pTracker[eNumTrackers];
TrackerType SelectedTracker = eGoalTracker;

//Give something cool to look at
class SineWaveMaker
{
	public:
		SineWaveMaker() : m_rho(0.0),m_rho2(0.0) {}
		void operator()(double &Sample,double &Sample2,double freq_hz=1000,double SampleRate=48000,double amplitude=1.0)
		{
			double			 theta,theta2;
			size_t index=0; //array index of buffer

			const double pi2 = 3.1415926 * 2.0;
			//Compute the angle ratio unit we are going to use
			//Multiply times pi 2 to Convert the angle ratio unit into radians
			theta = (freq_hz / SampleRate) * pi2;
			theta2 = ((freq_hz*0.5) / SampleRate) * pi2;

			Sample = sin( m_rho ) * amplitude;
			Sample2 = sin (m_rho2) * amplitude;

			//Find Y given the hypotenuse (scale) and the angle (rho)
			//Note: using sin will solve for Y, and give us an initial 0 size
			//increase our angular measurement
			m_rho += theta;
			m_rho2 += theta2;
			//bring back the angular measurement by the length of the circle when it has completed a revolution
			if ( m_rho > pi2 )
				m_rho -= pi2;
			if ( m_rho2 > pi2 )
				m_rho2 -= pi2;
		}
	private:
		double m_rho,m_rho2;
} g_TestSample;

Bitmap_Frame *NI_VisionProcessing(Bitmap_Frame *Frame, double &x_target, double &y_target, bool &have_target)
{
	if( g_pTracker[SelectedTracker] == NULL )
	{
		if( SelectedTracker == eGoalTracker )
			g_pTracker[eGoalTracker] = new VisionGoalTracker();
		if( SelectedTracker == eFrisbeTracker )
			g_pTracker[eFrisbeTracker] = new VisionRinTinTinTracker( false ); 
		if( g_pTracker[SelectedTracker] == NULL)
			return Frame;

		// quick tweaks 
		g_pTracker[SelectedTracker]->SetDisplayMode(eMasked);
		g_pTracker[SelectedTracker]->SetUseColorThreshold(false);
		g_pTracker[SelectedTracker]->SetShowBounds(true);
	}

	g_pTracker[SelectedTracker]->Profiler.start();

	g_pTracker[SelectedTracker]->GetFrame(Frame);

	// do the actual processing
	have_target = g_pTracker[SelectedTracker]->ProcessImage(x_target, y_target) > 0;

	// Return our processed image back to our outgoing frame.
	g_pTracker[SelectedTracker]->ReturnFrame(Frame);

	g_pTracker[SelectedTracker]->Profiler.stop();
	g_pTracker[SelectedTracker]->Profiler.display(L"vision:");

	return Frame;
}

extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_RGB32(Bitmap_Frame *Frame)
{
	double x_target, y_target;
	bool have_target = false;
	Frame = NI_VisionProcessing(Frame, x_target, y_target, have_target);
	//DebugOutput("X=%.2f, Y=%.2f, %s\n", x_target, y_target, have_target ? "target: yes" : "target: no");
	if (g_UDP_Output && have_target)
		(*g_UDP_Output)(x_target,y_target);

	return Frame;
}

extern "C" PROCESSINGVISION_API bool Set_VisionSettings( VisionSetting_enum VisionSetting, double value)
{
	switch( VisionSetting )
	{
		case eTrackerType:
			SelectedTracker = (TrackerType)(int)value;
			break;
		default:
			break;
	}

	return true;
}

extern "C" PROCESSINGVISION_API double Get_VisionSettings( VisionSetting_enum VisionSetting )
{
	switch( VisionSetting )
	{
		case eTrackerType:
			return (double)SelectedTracker;
			break;
		default:
			break;
	}
	return 0.0;
}

extern "C" PROCESSINGVISION_API void Callback_SmartCppDashboard_Initialize(char *IPAddress)
{
	if (IPAddress)
		g_UDP_Output=UDP_Client_Interface::GetNewInstance(IPAddress);
}

extern "C" PROCESSINGVISION_API void Callback_SmartCppDashboard_Shutdown()
{
	delete g_UDP_Output;
	g_UDP_Output=NULL;
}
