#pragma once

#include "stdafx.h"

class ZEDCamera 
{
public:
	ZEDCamera(const char* file);
	~ZEDCamera();

	sl::ERROR_CODE GrabFrameAndDapth(void);
	sl::ERROR_CODE GrabDepth(void);
	sl::ERROR_CODE GetNormDepth(void);
	sl::ERROR_CODE GetNormConfidence(void);
	void ResetCalibration(void);
	void updateCameraSettings(int key);
	void switchCameraSettings();
	void saveSbSimage(std::string);
	cv::Mat slMat2cvMat(sl::Mat& input);

	bool IsOpen;

	sl::Resolution image_size;

	bool bHaveFrame;

	sl::RuntimeParameters runtime_parameters;
	int ViewID;

	int confidenceLevel;

	sl::Mat depth;
	sl::Mat point_cloud;
	cv::Mat frame;
	cv::Mat cvConfidence;
	cv::Mat cvNormDepth;

	sl::Camera* zed;

private:

	sl::Mat zedFrame;
	sl::Mat Disparity;
	sl::Mat Confidence;

	sl::SELF_CALIBRATION_STATE old_self_calibration_state;

	// Sample variables
	sl::CAMERA_SETTINGS camera_settings_ = sl::CAMERA_SETTINGS_BRIGHTNESS;
	std::string str_camera_settings = "BRIGHTNESS";
	int step_camera_setting = 1;
};