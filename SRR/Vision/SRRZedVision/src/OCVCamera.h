#pragma once

//opencv includes
#include <opencv2/opencv.hpp>

class OCVCamera 
{
public:
	OCVCamera(const char* file);
	~OCVCamera();

	cv::Mat GrabFrame(void);

private:
	int width;
	int height;

	cv::VideoCapture capture;
	cv::Mat frame;
};