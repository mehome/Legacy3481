#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "OCV_VisionProcessingBase.h"

using namespace cv;
#define FLIP90

VisionTracker::VisionTracker()
{	
	Profiler = new profile;
}

VisionTracker::~VisionTracker()
{
}


int VisionTracker::GetFrame(Bitmap_Frame *Frame)
{
	int success = 1;
#ifndef FLIP90
	InputImageRGB = new Mat(Frame->YRes, Frame->XRes, CV_8UC4, Frame->Memory);
	if( InputImageRGB == NULL ) success = false;
#else
	rotInputImageRGB = new Mat(Frame->YRes, Frame->XRes, CV_8UC4, Frame->Memory);
	InputImageRGB = new Mat(Frame->YRes, Frame->XRes, CV_8UC4);
	if (rotInputImageRGB != NULL)
	{
		rotate_90n(*rotInputImageRGB, *InputImageRGB, -90);
	}
	else
		success = false;
#endif

	return success;
}

void VisionTracker::ReturnFrame(Bitmap_Frame *Frame)
{	// copy image back to our frame.
	// no need to copy, for return if not flipped.
#ifdef FLIP90
	//rotate_90n(*InputImageRGB, *rotInputImageRGB, 90);
	//memcpy((void*)Frame->Memory, InputImageRGB->data, Frame->Stride * 4 * Frame->YRes);
	//Frame->XRes = Frame->YRes;
	//Frame->YRes = Frame->XRes;
	//Frame->Stride = Frame->YRes;
#endif
}

void VisionTracker::rotate_90n(cv::Mat &src, cv::Mat &dst, int angle)
{
	if (angle == 270 || angle == -90){
		// Rotate clockwise 270 degrees
		cv::transpose(src, dst);
		cv::flip(dst, dst, 0);
	}
	else if (angle == 180 || angle == -180){
		// Rotate clockwise 180 degrees
		cv::flip(src, dst, -1);
	}
	else if (angle == 90 || angle == -270){
		// Rotate clockwise 90 degrees
		cv::transpose(src, dst);
		cv::flip(dst, dst, 1);
	}
	else if (angle == 360 || angle == 0){
		if (src.data != dst.data){
			src.copyTo(dst);
		}
	}
}