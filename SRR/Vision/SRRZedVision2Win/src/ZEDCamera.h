#pragma once

#include "stdafx.h"

class ZEDCamera 
{
public:
	ZEDCamera(const char* file);
	~ZEDCamera();

	cv::Mat GrabFrame(void);
	cv::Mat GrabFrameAndDapth(void);
	sl::Mat GrabDepth(void);
	cv::Mat GetNormDisparity(void);
	cv::Mat GetNormDepth(void);
	cv::Mat GetNormConfidence(void);
	void ResetCalibration(void);
	int GetGain(void);
	void SetGain(int);
	int GetExposure(void);
	void SetExposure(int);
	void saveSbSimage(std::string);
	cv::Mat slMat2cvMat(sl::Mat& input);

	bool IsOpen;

	sl::Resolution image_size;

	bool bHaveFrame;

	sl::RuntimeParameters runtime_parameters;
	int ViewID;

	int confidenceLevel;

	sl::Mat depth;

	sl::Camera* zed;

private:

	cv::Mat frame;
	cv::Mat cvDepth;
	cv::Mat cvDisparity;
	cv::Mat cvConfidence;

	sl::SELF_CALIBRATION_STATE old_self_calibration_state;
};