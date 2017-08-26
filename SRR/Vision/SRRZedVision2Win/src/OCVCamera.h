#pragma once

#include "stdafx.h"

enum OVCcamFlip
{
	NONE,
	CW,
	CCW
};

class OCVCamera 
{
public:
	OCVCamera(const char* file, OVCcamFlip Flip = NONE);
	~OCVCamera();

	cv::Mat GrabFrame(void);

	bool IsOpen;

private:
	int width;
	int height;

	OVCcamFlip flip;

	cv::VideoCapture capture;
	cv::Mat frame;
};