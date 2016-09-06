#include "stdafx.h"
#include "OCVCamera.h"

OCVCamera::OCVCamera(const char *file)
{
	capture.open(file);

	IsOpen = capture.isOpened();
	if (!IsOpen)
		printf("Camera NOT opened.\n");
}

OCVCamera::~OCVCamera() {}

cv::Mat OCVCamera::GrabFrame(void)
{
	if (capture.isOpened())
	{
		capture >> frame;
		cv::transpose(frame, frame);
		cv::flip(frame, frame, 0); //transpose+flip(1)=CW			
	}

	return frame;
}