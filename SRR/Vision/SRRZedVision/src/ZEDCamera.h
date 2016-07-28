#pragma once

//ZED Includes
#include <zed/Camera.hpp>
#include <zed/utils/GlobalDefine.hpp>

class ZEDCamera 
{
public:
	ZEDCamera(const char* file);
	~ZEDCamera();

	cv::Mat GrabFrame(void);

	cv::Mat GrabFrameAndDapth(void);

	sl::zed::Mat GrabDepth(void);

	int width;
	int height;
	bool bNoFrame;

	sl::zed::SENSING_MODE dm_type;
	int ViewID;

	int confidenceLevel;

	sl::zed::Mat depth;

	sl::zed::Camera* zed;

private:

	cv::Mat frame;

	sl::zed::ZED_SELF_CALIBRATION_STATUS old_self_calibration_status;
};