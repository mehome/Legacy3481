#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "OCV_VisionProcessingBase.h"
#include "VisionCascadeClassifierTracker.h"
#include "../SmartDashboard/SmartDashboard_Import.h"

using namespace std;
using namespace cv;

VisionCascadeClassifierTracker::VisionCascadeClassifierTracker()
: 	mode(h_original),
	bShowImg(false),
	bCascadeLoaded(false)
{	
	hook_cascade_name = "cascade_data\\hook_cascade_cpu.xml";
	p_hook_cascade = new CascadeClassifier();
	if(p_hook_cascade)
		bCascadeLoaded = p_hook_cascade->load(hook_cascade_name);
}


VisionCascadeClassifierTracker::~VisionCascadeClassifierTracker()
{
}

// TODO: 
// * convert incomming image to Mat format for OpenCV -- implement in GetFrame().
int VisionCascadeClassifierTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;
	Mat frame;
	frameCount = 0;

//	capture >> frame;
	frameCount++;

	if (!frame.empty())
	{
		std::vector<Rect> hooks;
		Mat frame_gray;

		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		//if (mode == h_equalize)
		//{
		//	equalizeHist(frame_gray, frame_gray);
		//}

		//if (mode == h_clahe)
		//{
		//	Ptr<CLAHE> clahe = createCLAHE();
		//	clahe->setClipLimit(4);
		//	//clahe->setTilesGridSize(cv::Size(8, 8));
		//	clahe->apply(frame_gray, frame_gray);
		//}

		//-- detect hook sample
		p_hook_cascade->detectMultiScale(frame_gray, hooks, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(15, 15));

		for (size_t i = 0; i < hooks.size(); i++)
		{
	//		printf("Frame %d x: %d y: %d width: %d height %d\n", frameCount, hooks[i].x, hooks[i].y, hooks[i].width, hooks[i].height);
			Point p1(hooks[i].x, hooks[i].y);
			Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
			rectangle(frame, p1, p2, Scalar(255, 0, 255), 2, 8, 0);
		}
	}

	int c = waitKey(10);

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

