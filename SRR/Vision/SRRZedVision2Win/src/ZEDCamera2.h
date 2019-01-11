#pragma once

#include "stdafx.h"

class ZEDCamera2 
{
public:
	ZEDCamera2();
	ZEDCamera2(const char* file);
	~ZEDCamera2();

	// the grab thread
	void operator()();

	sl::ERROR_CODE GrabFrameAndDapth(void);
	sl::ERROR_CODE GrabDepth(void);
	void ResetCalibration(void);
	void updateCameraSettings(int key);
	void switchCameraSettings(void);
	void printCameraSettings(void);
	bool loadSettings(void);
	void saveSettings(void);
	void saveSbSimage(std::string);
	cv::Mat slMat2cvMat(sl::Mat& input);
	void close(void);

	bool IsOpen;

	sl::Resolution image_size;

	bool bHaveFrame;

	sl::RuntimeParameters runtime_parameters;
	int ViewID;

	int confidenceLevel;

	sl::Camera* zed;

private:

	sl::Mat zedFrame;
	sl::Mat depth;
	sl::Mat point_cloud;
	cv::Mat frame;

	std::queue<sl::Mat>depth_queue;
	std::queue<sl::Mat>frame_queue;
	std::queue<sl::Mat>pointcl_queue;

	const int max_queue_aize = 6;

	// need exit and sync

	sl::SELF_CALIBRATION_STATE old_self_calibration_state;

	// Sample variables
	sl::CAMERA_SETTINGS camera_settings_ = sl::CAMERA_SETTINGS_BRIGHTNESS;
	std::string str_camera_settings = "BRIGHTNESS";
	int step_camera_setting = 1;
};