
#include "stdafx.h"
#include "ProcessingVision.h"
#include "NI_VisionProcessingBase.h"
#include "VisionGoalTracker.h"
#include "VisionRinTinTinTracker.h"

UDP_Client_Interface *g_UDP_Output=NULL;
extern VisionTracker* g_pTracker[eNumTrackers];
TrackerType SelectedTracker = eGoalTracker;
Dashboard_Framework_Interface *g_Framework=NULL;

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

#if 1
extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame)
{
	Bitmap_Handle *bgra_handle=g_Framework->CreateBGRA(Frame);
	Bitmap_Frame &bgra_frame=bgra_handle->frame;
	g_Framework->UYVY_to_BGRA(Frame,&bgra_frame);

	double x_target, y_target;
	bool have_target = false;
	//Note: out_frame could be UVYV if we wanted it to be... I don't want to assume its the same as &bgra_frame even though it may be
	Bitmap_Frame *out_frame;
	out_frame = NI_VisionProcessing(&bgra_frame, x_target, y_target, have_target);
	//DebugOutput("X=%.2f, Y=%.2f, %s\n", x_target, y_target, have_target ? "target: yes" : "target: no");
	if (g_UDP_Output && have_target)
		(*g_UDP_Output)(x_target,y_target);

	g_Framework->BGRA_to_UYVY(out_frame,Frame);
	g_Framework->DestroyBGRA(bgra_handle);

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

extern "C" PROCESSINGVISION_API void Callback_SmartCppDashboard_Initialize(char *IPAddress,Dashboard_Framework_Interface *DashboardHelper)
{
	g_Framework=DashboardHelper;
	if (IPAddress)
		g_UDP_Output=UDP_Client_Interface::GetNewInstance(IPAddress);
}

extern "C" PROCESSINGVISION_API void Callback_SmartCppDashboard_Shutdown()
{
	delete g_UDP_Output;
	g_UDP_Output=NULL;
}
