#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessingBase.h"
#include "VisionCascadeClassifierTracker.h"
#include "../SmartDashboard/SmartDashboard_Import.h"

#ifdef OCV_READY
// cascade classifier
#include "OpenNI.h"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;
#endif

VisionCascadeClassifierTracker::VisionCascadeClassifierTracker()
#ifdef OCV_READY
: 	histo_mode mode(h_original),
	bShowImg(false),
	bCascadeLoaded(false)
{	
	hook_cascade_name = "data\\SRR Samples\\cascades\\hook_cascade_cpu.xml";
	window_name = "Capture - sample detection";
	//-- 1. Load the cascades
	bCascadLoaded = hook_cascade.load(hook_cascade_name);
}
#else
{}
#endif


VisionCascadeClassifierTracker::~VisionCascadeClassifierTracker()
{
}

void VisionCascadeClassifierTracker::SetDefaultThreshold( void )
{
	// hsv - green
	HueRange.minValue = 0,			HueRange.maxValue = 255,
	SaturationRange.minValue = 0,	SaturationRange.maxValue = 255,
	ValueRange.minValue = 0,		ValueRange.maxValue = 255;

	// rgb - green
	RedRange.minValue = 0,		RedRange.maxValue = 255,
	GreenRange.minValue = 0,	GreenRange.maxValue = 255,
	BlueRange.minValue = 0,		BlueRange.maxValue = 255;	

	// luma - I doubt this would be a good choice - added for completeness.
	LuminanceRange.minValue = 0, LuminanceRange.maxValue = 255;
}

// TODO: 
// * work out OpenCV lib requirements
// * convert incomming image to Mat format for OpenCV -- implement in GetFrame().
// * restructure - nuetral base class for general and interface, new subclasses for NI vs OCV.
// * Modify detection function... can probably move that code to main processing func.
int VisionCascadeClassifierTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;
#ifdef OCV_READY
	VideoCapture capture;
	Mat frame;
	frameCount = 0;

	//-- 2. Read the video stream
	//capture.open("http://ctetrick.no-ip.org/videostream.asf?user=guest&pwd=watchme&resolution=32");
	// note for ip webcam, use http://192.168.0.101:8080/video?dummy=param.mjpeg
	// video - "..\Video\grass_sun2.mp4"
	capture.open(filename.c_str());
	if (capture.isOpened())
	{
		for (;;)
		{
			capture >> frame;
			frameCount++;

			//-- 3. Apply the classifier to the frame
			if (!frame.empty())
			{
				detectAndDisplay(frame);
			}

			int c = waitKey(10);
			if ((char)c == 'c') { break; }
		}
	}
#endif
	// Get return for x, y target values;

	if( true )
	{
		x_target = 0.0;	// TODO: add values
		y_target = 0.0;
	}
	else
	{
		x_target = 0.0;
		y_target = 0.0;
	}

	return success;
}

#ifdef OCV_READY
void VisionCascadeClassifierTracker::detectAndDisplay(Mat frame)
{
	std::vector<Rect> hooks;
	Mat frame_gray;

	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	if (mode == h_original && bShowImg)
		imshow("original gray", frame_gray);

	if (mode == h_equalize)
	{
		equalizeHist(frame_gray, frame_gray);
		if (bShowImg)
			imshow("equalized", frame_gray);
	}

	if (mode == h_clahe)
	{
		Ptr<CLAHE> clahe = createCLAHE();
		clahe->setClipLimit(4);
		//clahe->setTilesGridSize(cv::Size(8, 8));
		clahe->apply(frame_gray, frame_gray);
		if (bShowImg)
			imshow("clahe", frame_gray);
	}

	//-- detect hook sample
	hook_cascade.detectMultiScale(frame_gray, hooks, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(15, 15));

	for (size_t i = 0; i < hooks.size(); i++)
	{
		printf("Frame %d x: %d y: %d width: %d height %d\n", frameCount, hooks[i].x, hooks[i].y, hooks[i].width, hooks[i].height);
		Point p1(hooks[i].x, hooks[i].y);
		Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
		rectangle(frame, p1, p2, Scalar(255, 0, 255), 2, 8, 0);
	}

	//-- Show what you got
	if (bShowImg)
		imshow(window_name, frame);
}
#endif
