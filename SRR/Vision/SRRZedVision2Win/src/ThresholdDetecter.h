#pragma once

#include "stdafx.h"

class ThresholdDetecter
{
public:
	ThresholdDetecter();
	ThresholdDetecter(int3 lov, int3 high);
	~ThresholdDetecter();

	void detectRockSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);

private:
	int3 HSVLow;
	int3 HSVHigh;

	int ThreshInc = 10;
};