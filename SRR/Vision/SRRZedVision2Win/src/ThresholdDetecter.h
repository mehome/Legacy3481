#pragma once

#include "stdafx.h"

class ThresholdDetecter
{
public:
	ThresholdDetecter();
	ThresholdDetecter(int3 lov, int3 high);
	~ThresholdDetecter();

	void detectRockSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);
	void updateThresholdSettings(char key);
	void switchThresholdSettings();

private:
	int HSV_Range[6];
	int3 HSV_low;
	int3 HSV_high;

	std::string str_threshold_setting[6];

	enum THRESHOLD_SETTING {
		H_Low,
		H_High,
		S_Low,
		S_High,
		V_Low,
		V_High
	};

	THRESHOLD_SETTING threshold_setting;
	int thresh_inc;

	cv::Scalar color;
	cv::Mat hsv, masked;

	// countours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

};