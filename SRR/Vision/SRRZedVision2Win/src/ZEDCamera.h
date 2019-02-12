#pragma once

#include "stdafx.h"

class ZEDCamera 
{
public:
	ZEDCamera();
	ZEDCamera(const char* file);
	~ZEDCamera();

	sl::Mat GetFrame(void);
	sl::Mat GetDepth(void);
	sl::Mat GetPointCloud(void);
	cv::Mat GetView(void);
	sl::Pose GetPose(void);
	bool HaveFrame(void);
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

	sl::RuntimeParameters runtime_parameters;
	int ViewID;

private:

	void transformPose(sl::Transform &pose, float tx);

	int confidenceLevel;

	sl::Camera* zed;

	sl::Pose camera_pose;

	sl::Mat zedFrame;
	sl::Mat depth;
	sl::Mat point_cloud;
	cv::Mat frame;

	std::queue<sl::Mat>depth_queue;
	std::queue<sl::Mat>frame_queue;
	std::queue<sl::Mat>pointcl_queue;
	std::queue<sl::Pose>pose_queue;

	const int max_queue_aize = 100;
	bool quit;

	std::thread grab_thread;
	void grab_run();

	// need exit and sync

	sl::SELF_CALIBRATION_STATE old_self_calibration_state;

	// Sample variables
	sl::CAMERA_SETTINGS camera_settings_ = sl::CAMERA_SETTINGS_BRIGHTNESS;
	std::string str_camera_settings = "BRIGHTNESS";
	int step_camera_setting = 1;
};