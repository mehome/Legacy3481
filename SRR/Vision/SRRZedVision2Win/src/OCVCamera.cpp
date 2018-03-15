#include "stdafx.h"
#include "OCVCamera.h"

OCVCamera::OCVCamera(const char *file, OVCcamFlip Flip)
{
	flip = Flip;
	capture.open(file);

	IsOpen = capture.isOpened();
	if (IsOpen)
	{
		capture >> frame;
		width = frame.cols;
		height = frame.rows;
	}
	else
		printf("Front Camera NOT opened.\n");
}

OCVCamera::~OCVCamera() {}

cv::Mat OCVCamera::GrabFrame(void)
{
	if (capture.isOpened())
	{
		capture >> frame;
		switch (flip)
		{
			case CCW:
				cv::transpose(frame, frame);
				cv::flip(frame, frame, 0);
				break;
			case CW:
				cv::transpose(frame, frame);
				cv::flip(frame, frame, 1);
			case NONE:
				break;
		}
	}

	return frame;
}