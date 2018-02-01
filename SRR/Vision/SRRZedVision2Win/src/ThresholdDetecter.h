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
	int3 HSVLow;
	int3 HSVHigh;

	int ThreshInc = 10;

	enum THRESHOLD_SETTING {
		H_Low,
		H_High,
		S_Low,
		S_High,
		V_Low,
		V_High
	};

	THRESHOLD_SETTING treshold_setting = H_Low;
	std::string str_threshold_setting = "THRESHOLD HUE LOW";
};