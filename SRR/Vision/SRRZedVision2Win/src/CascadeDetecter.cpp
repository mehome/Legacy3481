#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"

enum histo_mode
{
	h_original,
	h_equalize,
	h_clahe
};

float GetDistanceAtPoint(sl::Mat depth, int x, int y);

enum histo_mode mode = h_original;

bool bShowImg = false;

cv::CascadeClassifier hook_cascade;

std::vector<cv::Rect> hooks;

/**
* @function detectHookSample
*/
void detectHookSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud)
{
	cv::Mat frame_gray;

	cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
	if (mode == h_original && bShowImg)
		cv::imshow("original gray", frame_gray);

	if (mode == h_equalize)
	{
		cv::equalizeHist(frame_gray, frame_gray);
		if (bShowImg)
			cv::imshow("equalized", frame_gray);
	}

	if (mode == h_clahe)
	{
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		//clahe->setTilesGridSize(cv::Size(8, 8));
		clahe->apply(frame_gray, frame_gray);
		if (bShowImg)
			cv::imshow("clahe", frame_gray);
	}

	//-- detect hook sample
	hook_cascade.detectMultiScale(frame_gray, hooks, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(15, 15));

	int height = 0;
	int XRes = frame_gray.cols;
	int YRes = frame_gray.rows;

	for (size_t i = 0; i < hooks.size(); i++)
	{
		float Distance = GetDistanceAtPoint(depth, hooks[i].x, hooks[i].y);
		std::cout << "hook found at: " << hooks[i].x << ", " << hooks[i].y << " Dist: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;

		cv::Point p1(hooks[i].x, hooks[i].y);
		cv::Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
		rectangle(frame, p1, p2, cv::Scalar(255, 0, 255), 2, 8, 0);

#if 0
		float4 point3D;
		// Get the 3D point cloud values for pixel (i,j)
		point_cloud.getValue(i, j, &point3D);
		float x = point3D.x;
		float y = point3D.y;
		float z = point3D.z;
		float color = point3D.w;

		float distance = sqrt(point3D.x*point3D.x + point3D.y*point3D.y + point3D.z*point3D.z);
#endif

		double x_target = hooks[i].x + hooks[i].width / 2;
		double y_target = hooks[i].y + hooks[i].height / 2;
		SmartDashboard::PutNumber("X Position", x_target);
		SmartDashboard::PutNumber("Y Position", y_target);
	}
}
