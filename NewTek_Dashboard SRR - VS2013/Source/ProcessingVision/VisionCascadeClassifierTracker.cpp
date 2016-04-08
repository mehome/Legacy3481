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
	bShowImg(true),
	bCascadeLoaded(false),
	frameCount(0)
{	
	hook_cascade_name = "cascade_data\\hook_cascade_cpu.xml";
	p_hook_cascade = new CascadeClassifier();
	if(p_hook_cascade)
		bCascadeLoaded = p_hook_cascade->load(hook_cascade_name);
}


VisionCascadeClassifierTracker::~VisionCascadeClassifierTracker()
{
}

int VisionCascadeClassifierTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;

	const double Aspect = (double)InputImageRGB->cols / (double)InputImageRGB->rows;

	std::vector<Rect> hooks;
	Mat frame_gray;
	int height;
	int XRes = InputImageRGB->cols;
	int YRes = InputImageRGB->rows;

	frameCount++;

	if (bCascadeLoaded && !InputImageRGB->empty())
	{
		cvtColor(*InputImageRGB, frame_gray, COLOR_BGR2GRAY);

		if (mode == h_equalize)
		{
			equalizeHist(frame_gray, frame_gray);
		}

		if (mode == h_clahe)
		{
			Ptr<CLAHE> clahe = createCLAHE();
			clahe->setClipLimit(4);
			//clahe->setTilesGridSize(cv::Size(8, 8));
			clahe->apply(frame_gray, frame_gray);
		}
		if (bShowImg)
			imshow("gray", frame_gray);

		//-- detect hook sample
		p_hook_cascade->detectMultiScale(frame_gray, hooks, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(15, 15));

		x_target = 0.0;
		y_target = 0.0;
		height = 0;

		for (size_t i = 0; i < hooks.size(); i++)
		{
			Point p1(hooks[i].x, hooks[i].y);
			Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
			rectangle(*InputImageRGB, p1, p2, Scalar(255, 0, 255), 2, 8, 0);

			// TODO: need to work out how to pick when there are multiple objects found.
			double center_x = (double)(hooks[i].x + hooks[i].width / 2);
			double center_y = (double)(hooks[i].y + hooks[i].height / 2);

			// convert to aiming system coords
			x_target = (double)((center_x - (XRes / 2.0)) / (XRes / 2.0)) * Aspect;
			y_target = (double)((center_y - (YRes / 2.0)) / (YRes / 2.0));

			height = hooks[i].height;
		}
		if (bShowImg)
			imshow("input", *InputImageRGB);
	}

	int c = waitKey(10);

	// Angle = arctan(vertical hight in feet * image height / (2 * vertical target hight in pixels * distance in feet)) * RADS_TO_DEG

	//#define VIEW_ANGLE 42.25
#define VIEW_ANGLE 47
	double Distance = 0.0;
	if (height != 0)
	{
		int TargetHeight = 9; // inches - actual sample is about 8 inches tall, but trained cascade images are taller.
		Distance = (double)(YRes * TargetHeight) / (height * 12 * 2 * tan(VIEW_ANGLE * M_PI / (180 * 2)));
		SmartDashboard::PutNumber("TargetDistance", Distance);
	}
	else
		SmartDashboard::PutNumber("TargetDistance", 0);

	wchar_t debugstr[1024];
	swprintf(debugstr, L"x_target %f y_target %f distance %f\n", x_target, y_target, Distance);
	OutputDebugString(debugstr);

	return success;
}

