
#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "ProcessingVision.h"
#include "NI_VisionProcessingBase.h"
#include "VisionGoalTracker.h"
#include "VisionRinTinTinTracker.h"
#include "VisionAerialAssistGoalTracker.h"
#include "VisionBallTracker.h"

//#define __Using_UDP__

#ifndef __Using_UDP__
#include "../SmartDashboard2/SmartDashboard_Import.h"
#endif

#ifdef __Using_UDP__
UDP_Client_Interface *g_UDP_Output=NULL;
#endif
extern VisionTracker* g_pTracker[eNumTrackers];
TrackerType SelectedTracker = eGoalTracker;
Dashboard_Framework_Interface *g_Framework=NULL;
TrackerType PendingTracker = eGoalTracker;
FrameWork::event frameSync;
bool g_IsTargeting=false;

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
			g_pTracker[eGoalTracker] = new VisionAerialAssistGoalTracker();
//		if( SelectedTracker == eFrisbeTracker )
//			g_pTracker[eFrisbeTracker] = new VisionRinTinTinTracker();
		if( SelectedTracker == eBallTracker )
			g_pTracker[eBallTracker] = new VisionBallTracker();
		if( g_pTracker[SelectedTracker] == NULL)
			return Frame;
	}

	g_pTracker[SelectedTracker]->Profiler.start();

	g_pTracker[SelectedTracker]->GetFrame(Frame);

	// do the actual processing
	have_target = g_pTracker[SelectedTracker]->ProcessImage(x_target, y_target) > 0;

	// Return our processed image back to our outgoing frame.
	g_pTracker[SelectedTracker]->ReturnFrame(Frame);

	g_pTracker[SelectedTracker]->Profiler.stop();
	g_pTracker[SelectedTracker]->Profiler.display(L"vision:");

	if( SelectedTracker != PendingTracker )
	{
		SelectedTracker = PendingTracker;
		frameSync.set(); 
	}

	return Frame;
}

#if 1
extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame)
{
	if (g_IsTargeting)
	{
		Bitmap_Handle *bgra_handle=g_Framework->CreateBGRA(Frame);
		Bitmap_Frame &bgra_frame=bgra_handle->frame;
		g_Framework->UYVY_to_BGRA(Frame,&bgra_frame);

		double x_target, y_target;
		bool have_target = false;
		//Note: out_frame could be UVYV if we wanted it to be... I don't want to assume its the same as &bgra_frame even though it may be
		Bitmap_Frame *out_frame;
		out_frame = NI_VisionProcessing(&bgra_frame, x_target, y_target, have_target);
		//TODO still under testing... this is experimental to see if it has better improvement
		#if 0
		//This is somewhat cheating, but we can assume there is only one instance of the tracking that will be happening
		SmartDashboard::PutNumber("desired x",x_target);
		using namespace FrameWork;
		static KalmanFilter s_KalFilter_Xtarget;
		static KalmanFilter s_KalFilter_Ytarget;
		static Averager<double,5> s_Xtarget_Averager;
		static Averager<double,5> s_Ytarget_Averager;
		static Predict_simple s_Xtarget_Predict;
		static time_type s_LastTime;
		#if 0
		static bool FirstRun=true;
		if (FirstRun)
		{
			SmartDashboard::PutNumber("lag",0.06);
			FirstRun=false;
		}
		const double lag=SmartDashboard::GetNumber("lag");
		#endif
		x_target = s_KalFilter_Xtarget(x_target);  //apply the Kalman filter
		x_target=s_Xtarget_Averager.GetAverage(x_target); //and Ricks x element averager
		y_target = s_KalFilter_Ytarget(y_target);  //apply the Kalman filter
		y_target=s_Ytarget_Averager.GetAverage(y_target); //and Ricks x element averager
		const time_type current_time=time_type::get_current_time();
		const time_type dtime_s=current_time-s_LastTime;
		s_LastTime=current_time;
		x_target=s_Xtarget_Predict(x_target,dtime_s,0.25);
		#endif

		//DebugOutput("X=%.2f, Y=%.2f, %s\n", x_target, y_target, have_target ? "target: yes" : "target: no");
		#ifdef __Using_UDP__
		if (g_UDP_Output && have_target)
			(*g_UDP_Output)(x_target,y_target);
		#else
		SmartDashboard::PutNumber("X Position",x_target);
		SmartDashboard::PutNumber("Y Position",y_target);
		#endif

		g_Framework->BGRA_to_UYVY(out_frame,Frame);
		g_Framework->DestroyBGRA(bgra_handle);
	}
	return Frame;
}

#else
extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame)
{
	#undef __TestUYVYDot__
	#define __TestBGRADot__
	#ifdef __TestUYVYDot__
	//Test... make a green box in the center of the frame
	size_t CenterY=Frame->YRes / 2;
	size_t CenterX=Frame->XRes / 2;
	size_t LineWidthInBytes=Frame->Stride * 4;
	for (size_t y=CenterY;y<CenterY+10;y++)
	{
		//http://www.mikekohn.net/file_formats/yuv_rgb_converter.php
		//yuv 149,43,21 = rgb 0,255,0
		for (size_t x=CenterX; x<CenterX+10; x++)
		{
			*(Frame->Memory+ (x*2 + 0) + (LineWidthInBytes * y))=21;
			*(Frame->Memory+ (x*2 + 1) + (LineWidthInBytes * y))=149;
			*(Frame->Memory+ (x*2 + 2) + (LineWidthInBytes * y))=43;
			*(Frame->Memory+ (x*2 + 3) + (LineWidthInBytes * y))=149;
		}
	}
	#endif
	#ifdef __TestBGRADot__
	if (g_Framework)
	{
		Bitmap_Handle *bgra_handle=g_Framework->CreateBGRA(Frame);
		Bitmap_Frame &bgra_frame=bgra_handle->frame;
		g_Framework->UYVY_to_BGRA(Frame,&bgra_frame);
		//Test... make a green box in the center of the frame
		size_t CenterY=bgra_frame.YRes / 2;
		size_t CenterX=bgra_frame.XRes / 2;
		size_t LineWidthInBytes=bgra_frame.Stride * 4;
		for (size_t y=CenterY-5;y<CenterY+5;y++)
		{
			for (size_t x=CenterX-5; x<CenterX+5; x++)
			{
				*(bgra_frame.Memory+ (x*4 + 0) + (LineWidthInBytes * y))=0;
				*(bgra_frame.Memory+ (x*4 + 1) + (LineWidthInBytes * y))=255;
				*(bgra_frame.Memory+ (x*4 + 2) + (LineWidthInBytes * y))=0;
			}
		}
		g_Framework->BGRA_to_UYVY(&bgra_frame,Frame);
		g_Framework->DestroyBGRA(bgra_handle);
	}
	#endif

	return Frame;
}
#endif

extern "C" PROCESSINGVISION_API void ResetDefaults( void )
{
	g_pTracker[SelectedTracker]->SetDefaultThreshold();
}

extern "C" PROCESSINGVISION_API bool Set_VisionSettings( VisionSetting_enum VisionSetting, double value)
{
	switch( VisionSetting )
	{
		case eTrackerType:
			PendingTracker = (TrackerType)(int)value;
			if( g_pTracker[PendingTracker] == NULL )
			{	// this is for init - no sync needed if selected tracker not started.
				if( g_pTracker[SelectedTracker] == NULL )
					SelectedTracker = PendingTracker;
				if( PendingTracker == eGoalTracker )
					g_pTracker[eGoalTracker] = new VisionAerialAssistGoalTracker();
//				if( PendingTracker == eFrisbeTracker )
//					g_pTracker[eFrisbeTracker] = new VisionRinTinTinTracker(); 
				if( PendingTracker == eBallTracker )
					g_pTracker[eBallTracker] = new VisionBallTracker();
			}
			frameSync.wait(500);	// cheezy method to make the control dialog wait so it can get the correct values after switching
			break;
		case eDisplayType:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetDisplayMode((DisplayType)(int)value);
			break;
		case eSolidMask:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetSolidMask((bool)(int)value);
			break;
		case eThresholdMode:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetThresholdMode((ThresholdColorSpace)(int)value);
			frameSync.wait(250);	// cheezy method to make the control dialog wait so it can get the correct values after switching
			break;
		case eOverlays:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetShowOverlays((bool)(int)value);
			break;
		case eAimingText:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetShowAiming((bool)(int)value);
			break;
		case eBoundsText:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetShowBounds((bool)(int)value);
			break;
		case e3PtGoal:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			if( SelectedTracker == eGoalTracker )
				g_pTracker[SelectedTracker]->Set3PtGoal((bool)(int)value);
			break;
		case eIsTargeting:
			g_IsTargeting=(value==0.0)?false:true;
			break;
		case eThresholdPlane1Min:
		case eThresholdPlane2Min:
		case eThresholdPlane3Min:
		case eThresholdPlane1Max:
		case eThresholdPlane2Max:
		case eThresholdPlane3Max:
			if( g_pTracker[SelectedTracker] == NULL )
				return false;
			g_pTracker[SelectedTracker]->SetThresholdValues(VisionSetting, (int)value);
			break;

		default:
			break;
	}

	return true;
}

extern "C" PROCESSINGVISION_API double Get_VisionSettings( VisionSetting_enum VisionSetting )
{
	if( g_pTracker[PendingTracker] == NULL )
	{
		if( PendingTracker == eGoalTracker )
			g_pTracker[eGoalTracker] = new VisionAerialAssistGoalTracker();
//		if( PendingTracker == eFrisbeTracker )
//			g_pTracker[eFrisbeTracker] = new VisionRinTinTinTracker(); 
		if( PendingTracker == eBallTracker )
			g_pTracker[eBallTracker] = new VisionBallTracker();
	}

	switch( VisionSetting )
	{
		case eTrackerType:
			return (double)SelectedTracker;
		case eDisplayType:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetDisplayMode();	
			break;
		case eThresholdMode:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetThresholdMode();
			break;
		case eSolidMask:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetSolidMask();
			break;
		case eOverlays:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetShowOverlays();
			break;
		case eAimingText:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetShowAiming();
			break;
		case eBoundsText:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetShowBounds();
			break;
		case eThresholdPlane1Min:
		case eThresholdPlane2Min:
		case eThresholdPlane3Min:
		case eThresholdPlane1Max:
		case eThresholdPlane2Max:
		case eThresholdPlane3Max:
			if( g_pTracker[SelectedTracker] != NULL )
				return g_pTracker[SelectedTracker]->GetThresholdValues(VisionSetting);
			break;
		case eIsTargeting:
			return (double)g_IsTargeting;
			break;
		default:
			break;
	}
	return 0.0;
}

extern "C" PROCESSINGVISION_API void Callback_SmartCppDashboard_Initialize(const char *IPAddress,const char *WindowTitle,Dashboard_Framework_Interface *DashboardHelper)
{
	g_Framework=DashboardHelper;
	#ifdef __Using_UDP__
	if (IPAddress)
		g_UDP_Output=UDP_Client_Interface::GetNewInstance(IPAddress);
	#else
	SmartDashboard::SetClientMode();
	SmartDashboard::SetIPAddress(IPAddress?IPAddress:"127.0.0.1");  //if null use local host
	SmartDashboard::init();
	#endif
}

extern "C" PROCESSINGVISION_API void Callback_SmartCppDashboard_Shutdown()
{
	#ifdef __Using_UDP__
	delete g_UDP_Output;
	g_UDP_Output=NULL;
	#else
	SmartDashboard::shutdown();
	#endif
}

extern "C" PROCESSINGVISION_API Plugin_Controller_Interface *Callback_CreatePluginControllerInterface()
{
	Plugin_SquareTargeting *plugin=new Plugin_SquareTargeting(Get_VisionSettings,Set_VisionSettings,ResetDefaults);
	return plugin;
}

//we must delete where we create especially in a plugin environment
extern "C" PROCESSINGVISION_API void Callback_DestroyPluginControllerInterface(Plugin_Controller_Interface *plugin)
{
	delete plugin;
}