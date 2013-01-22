
#include "stdafx.h"
#include "ProcessingVision.h"
#undef  __UseSampleExample__

UDP_Client_Interface *g_UDP_Output=NULL;

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


extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_RGB32(Bitmap_Frame *Frame)
{
#ifndef __UseSampleExample__
	double x_target, y_target;
	Frame = NI_VisionProcessing(Frame, x_target, y_target);
	//DebugOutput("X=%.2f,Y=%.2f\n",x_target,y_target);
	if (g_UDP_Output)
		(*g_UDP_Output)(x_target,y_target);
#else

	//Test... make a green box in the center of the frame
	size_t CenterY=Frame->YRes / 2;
	size_t CenterX=Frame->XRes / 2;
	size_t LineWidthInBytes=Frame->Stride * 4;
	for (size_t y=CenterY-5;y<CenterY+5;y++)
	{
		for (size_t x=CenterX-5; x<CenterX+5; x++)
		{
			*(Frame->Memory+ (x*4 + 0) + (LineWidthInBytes * y))=0;
			*(Frame->Memory+ (x*4 + 1) + (LineWidthInBytes * y))=255;
			*(Frame->Memory+ (x*4 + 2) + (LineWidthInBytes * y))=0;
		}
	}
	double X,Y;
	g_TestSample(X,Y);
	//DebugOutput("X=%.2f,Y=%.2f\n",X,Y);
	if (g_UDP_Output)
		(*g_UDP_Output)(X,Y);
#endif

	return Frame;
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
