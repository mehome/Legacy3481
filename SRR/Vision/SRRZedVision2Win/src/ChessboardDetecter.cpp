#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"
#include "ChessboardDetecter.h"

ChessboardDetecter::ChessboardDetecter(bool interactive)
	:	boardSize(9, 6),
		interactive_mode(interactive)
{}

ChessboardDetecter::~ChessboardDetecter() {}


// use calibration target for a beacon.
void ChessboardDetecter::detectBeacon(cv::Mat& view, sl::Mat* depth, sl::Mat* point_cloud)
{
	bool found = cv::findChessboardCorners(view, boardSize, pointBuf,
		CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

	if (found)                // If done with success,
	{
		// improve the found corners' coordinate accuracy for chessboard
		cv::Mat viewGray;
		cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
		cv::cornerSubPix(viewGray, pointBuf, cv::Size(11, 11),
			cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

		/// min point
		cv::Point2f minp((float)view.cols, (float)view.rows);

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
		if(interactive_mode)
			cv::drawChessboardCorners(view, boardSize, cv::Mat(pointBuf), found);

		if (point_cloud != NULL)
		{
			sl::float4 point3D;
			// Get the 3D point cloud values for pixel 
			point_cloud->getValue((size_t)center.x, (size_t)center.y, &point3D);

			float Distance = sqrt(point3D.x*point3D.x + point3D.y*point3D.y + point3D.z*point3D.z);

			if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
			{
				SmartDashboard::PutNumber("X Position", point3D.x);
				SmartDashboard::PutNumber("Y Position", point3D.y);
				SmartDashboard::PutNumber("Z Position", point3D.z);
				SmartDashboard::PutNumber("Distance", Distance);
			}
		}
		else if (depth != NULL)
		{
			float Distance;
			depth->getValue((size_t)center.x, (size_t)center.y, &Distance);

			if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
			{
				SmartDashboard::PutNumber("X Position", center.x);
				SmartDashboard::PutNumber("Y Position", center.y);
				SmartDashboard::PutNumber("Distance", Distance);
			}
		}
		else
		{
			float x_pos = (center.x - view.cols / 2) / (view.cols / 2);
			float y_pos = (center.y - view.rows / 2) / (view.rows / 2);
			float ht = (float)(maxp.y - minp.y) / view.rows;
			SmartDashboard::PutNumber("X Position", x_pos);
			SmartDashboard::PutNumber("Y Position", y_pos);
			SmartDashboard::PutNumber("Height", ht);
		}
	}
}
