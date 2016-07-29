#include "OCVCamera.h"

OCVCamera::OCVCamera(const char *file)
{
	capture.open(file);
	if (!capture.isOpened())
		printf("Camera NOT opened.\n");
}

OCVCamera::~OCVCamera() {}

cv::Mat OCVCamera::GrabFrame(void)
{
	if (capture.isOpened())
	{
//		std::cout << "do grab" << std::endl;
		if (capture.grab())
		{
//			std::cout << "do retrieve" << std::endl;
			capture.retrieve(frame);
		}

		//capture >> frame;
		//cv::transpose(frame, frame);
		//cv::flip(frame, frame, 0); //transpose+flip(1)=CW			
	}

	return frame;
}