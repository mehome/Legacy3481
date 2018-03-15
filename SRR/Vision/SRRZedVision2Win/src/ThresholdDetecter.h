#pragma once

#include "stdafx.h"

class ThresholdDetecter
{
public:
	ThresholdDetecter();
	ThresholdDetecter(int3 lov, int3 high);
	~ThresholdDetecter();

	void detectRockSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud, cv::Point mhit, bool small_display);
	void updateThresholdSettings(int key);
	void switchThresholdSettings();
	bool loadThreshold(std::string file);
	void saveThreshold(std::string file);
	void setThreshold(int3 low, int3 high);
	std::pair<int3, int3> getThreshold(void);
	void printThreshold(void);

private:
	int HSV_Range[6];
	int3 HSV_low;
	int3 HSV_high;

	std::string str_threshold_setting[6];

	enum THRESHOLD_SETTING {
		H_Low,
		H_Range = H_Low,
		H_High,
		H_Center = H_High,
		S_Low,
		S_High,
		V_Low,
		V_High
	};

	THRESHOLD_SETTING threshold_setting;
	int thresh_inc;

	cv::Scalar passcolor;
	cv::Scalar failcolor;
	cv::Mat hsv, masked;

	// countours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	// max, min rect values
	// this is to help with narrowing the valid targets
	cv::RotatedRect objRectMax, objRectMin;
	float DistMax, DistMin;
};