#include "WPILib.h"
#include "nivision.h"


#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2012_Robot.h"
#include "FRC2012_Camera.h"
#include "Common/InOut_Interface.h"
#include "TankDrive/Tank_Robot_Control.h"
//#include "TankDrive/Servo_Robot_Control.h"
//#include "Common/Debug.h"
//#include "FRC2012_InOut_Interface.h"

  /***********************************************************************************************************************************/
 /*															FrameProcessing															*/
/***********************************************************************************************************************************/
void FrameProcessing::TestMinMax(MonoImage &image,BYTE &TestMin,BYTE &TestMax)
{
	TestMin=TestMax=0x80;
	MonoImage &cr=image;
	;
	for (int y=0; y<cr.GetHeight(); y++)
	{
		for (int x=0; x<cr.GetWidth(); x++)
		{
			//pixel_ycbcr_u8 pixel=cr(x,y);
			//BYTE color=PixelColor(pixel);
			//BYTE color=imaqGetPixel(cr,);
			BYTE color=1;
			if (color<TestMin)
				TestMin=color;
			if (color>TestMax)
				TestMax=color;
		}
	}
}


void FrameProcessing::operator()(HSLImage &hsl_image,TestChorminance color,double AspectRatio)
{
	#if 0
	MonoImage  *ColorPlane;
	if (color==eTestPr)
		ColorPlane = image.GetRedPlane();
	else 
		ColorPlane = image.GetBluePlane();
	#endif
	
	#if 0
	{
		BYTE TestMin,TestMax;
		TestMinMax(*ColorPlane,TestMin,TestMax);
		printf("\r min=%d max=%d           ",TestMin,TestMax);
	}
	#endif

	//IMAQ_FUNC int    IMAQ_STDCALL imaqGetPixel(const Image* image, Point pixel, PixelValue* value);
	//Image Information functions
	static bool first_run=false;
	if (!first_run)
	{
		first_run=true;
		Image *img=hsl_image.GetImaqImage();
		unsigned int bitDepth;
		imaqGetBitDepth(img, &bitDepth);
		printf("bitdepth=%d\n",bitDepth);
	}
	
}
  /***********************************************************************************************************************************/
 /*													FRC_2012_CameraProcessing														*/
/***********************************************************************************************************************************/

FRC_2012_CameraProcessing::FRC_2012_CameraProcessing() : m_Camera(NULL),m_LastTime(0.0)
{
	//Default will use the team ip 10.34.81.11  with 11
	m_Camera=&AxisCamera::GetInstance();
	m_Camera->WriteResolution(AxisCamera::kResolution_320x240);
	//m_Camera->WriteResolution(AxisCamera::kResolution_640x480);
	//m_Camera->WriteCompression(20);
	//m_Camera->WriteBrightness(0);
	
	switch(m_Camera->GetResolution())
	{
	case AxisCamera::kResolution_640x480:	m_Xres=640,m_Yres=480;	break;
	case AxisCamera::kResolution_640x360:	m_Xres=640,m_Yres=360;	break;
	case AxisCamera::kResolution_320x240:	m_Xres=320,m_Yres=240;	break;
	case AxisCamera::kResolution_160x120:	m_Xres=160,m_Yres=120;	break;
	}
}

FRC_2012_CameraProcessing::~FRC_2012_CameraProcessing()
{
	m_Camera=NULL;  //We don't own this, but I do wish to treat it like we do
}

void FRC_2012_CameraProcessing::CameraProcessing_TimeChange(double dTime_s)
{
	if (m_Camera->IsFreshImage())
	{
		double time=GetTime() - m_LastTime;
		//Throttle down the iterations to a reasonable workload
		if (time>=1001.0/30000.0)
		{
			m_LastTime=GetTime();
			//printf("\rCamera Time %f           ",time);
			
			HSLImage *image=m_Camera->GetImage();
			m_FrameProcessor(*image);
			printf("\rCamera Time %f           ",GetTime() - m_LastTime);
			delete image;
		}
	}
}

