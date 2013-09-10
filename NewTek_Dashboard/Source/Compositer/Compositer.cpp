#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "Compositer.h"
#include "../SmartDashboard2/SmartDashboard_Import.h"

Dashboard_Framework_Interface *g_Framework=NULL;
FrameWork::event frameSync;

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

extern "C" COMPOSITER_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame)
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
	if (SmartDashboard::GetBoolean("Edit Position"))
	{
		static size_t Test=0;
		if (Test++>30)
		{
			FrameWork::DebugOutput("Test!\n");
			Test=0;
		}
	}
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



extern "C" COMPOSITER_API void Callback_SmartCppDashboard_Initialize(char *IPAddress,Dashboard_Framework_Interface *DashboardHelper)
{
	g_Framework=DashboardHelper;
	SmartDashboard::SetClientMode();
	SmartDashboard::SetIPAddress(IPAddress);
	SmartDashboard::init();
	SmartDashboard::PutBoolean("Edit Position",false);
}

extern "C" COMPOSITER_API void Callback_SmartCppDashboard_Shutdown()
{
	SmartDashboard::shutdown();

}
