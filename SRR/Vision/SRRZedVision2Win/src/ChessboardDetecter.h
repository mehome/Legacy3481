#pragma once

#include "stdafx.h"

class ChessboardDetecter
{
public:
	ChessboardDetecter(bool interactive);
	~ChessboardDetecter();

	void detectBeacon(cv::Mat& view, sl::Mat* point_cloud, sl::Pose* camera_pose);

private:
	bool interactive_mode;
	std::vector<cv::Point2f> pointBuf;
	cv::Size boardSize;            // The size of the board -> Number of items by width and height
	cv::Mat cameraMatrix, distCoeffs;
};