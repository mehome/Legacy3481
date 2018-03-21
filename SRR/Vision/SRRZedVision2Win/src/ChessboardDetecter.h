#pragma once

#include "stdafx.h"

class ChessboardDetecter
{
public:
	ChessboardDetecter()
		: boardSize(9, 6)
	{}

	~ChessboardDetecter() {}
	void detectBeacon(cv::Mat& view, sl::Mat* depth, sl::Mat* point_cloud);

private:
	std::vector<cv::Point2f> pointBuf;
	cv::Size boardSize;            // The size of the board -> Number of items by width and height
	cv::Mat cameraMatrix, distCoeffs;
};