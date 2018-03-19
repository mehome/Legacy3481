#include "stdafx.h"
#include "OCVCamera.h"

OCVCamera::OCVCamera(const char *file, OVCcamFlip Flip)
	: frame_counter(0)
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

		frame_counter++;
		//if the last frame is reached, reset the capture and the frame_counter
		if (frame_counter == capture.get(cv::CAP_PROP_FRAME_COUNT))
		{
			frame_counter = 0; //Or whatever as long as it is the same as next line
			capture.set(cv::CAP_PROP_POS_FRAMES, 0);
		}

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