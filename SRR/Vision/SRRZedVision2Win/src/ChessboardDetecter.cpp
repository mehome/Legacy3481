
#include "stdafx.h"

int count = 0;
std::vector<cv::Point2f> pointBuf;

float GetDistanceAtPoint(sl::Mat depth, int x, int y);

// use calibration target for a beacon.
void detectBeacon(cv::Mat view, sl::Mat depth)
{
	cv::Size boardSize(9, 6);            // The size of the board -> Number of items by width and height
	cv::Mat cameraMatrix, distCoeffs;
	cv::Size imageSize;

	imageSize = view.size();  // Format input image.

	bool found = cv::findChessboardCorners(view, boardSize, pointBuf,
		CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

//	std::cout << "findcorners returned " << found << std::endl;
	if (found)                // If done with success,
	{
		// improve the found corners' coordinate accuracy for chessboard
		cv::Mat viewGray;
		cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
		cv::cornerSubPix(viewGray, pointBuf, cv::Size(11, 11),
			cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

		/// min point
		cv::Point2f minp((float)imageSize.width, (float)imageSize.height);

		/// max point
		cv::Point2f maxp(0, 0);

		/// center
		cv::Point2f center;

		for (int i = 0; i < pointBuf.size(); i++)
		{
			if (pointBuf[i].x < minp.x) minp.x = pointBuf[i].x;
			if (pointBuf[i].y < minp.y) minp.y = pointBuf[i].y;
			if (pointBuf[i].x > maxp.x) maxp.x = pointBuf[i].x;
			if (pointBuf[i].y > maxp.y) maxp.y = pointBuf[i].y;
		}
		center.x = minp.x + (maxp.x - minp.x) / 2;
		center.y = minp.y + (maxp.y - minp.y) / 2;

		// Draw the corners.
		cv::drawChessboardCorners(view, boardSize, cv::Mat(pointBuf), found);

		float Distance = GetDistanceAtPoint(depth, (int)center.x, (int)center.y);
		std::cout << "beacon found at " << center.x << ", " << center.y << " distance: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
	}
}
