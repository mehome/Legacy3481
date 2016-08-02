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
	cv::Mat GetNormDisparity(void);
	cv::Mat GetNormDepth(void);
	cv::Mat GetNormConfidence(void);
	void ResetCalibration(void);
	int GetGain(void);
	void SetGain(int);
	int GetExposure(void);
	void SetExposure(int);
	void saveSbSimage(std::string);

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
	cv::Mat cvDepth;
	cv::Mat cvDisparity;
	cv::Mat cvConfidence;

	sl::zed::ZED_SELF_CALIBRATION_STATUS old_self_calibration_status;
};