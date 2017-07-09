#pragma once

//opencv includes
#include "stdafx.h"

class OCVCamera 
{
public:
	OCVCamera(const char* file);
	~OCVCamera();

	cv::Mat GrabFrame(void);

	bool IsOpen;

private:
	int width;
	int height;

	cv::VideoCapture capture;
	cv::Mat frame;
};