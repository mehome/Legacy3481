#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"
#include "CascadeDetecter.h"

float GetDistanceAtPoint(sl::Mat depth, size_t x, size_t y);

histo_mode mode = h_original;

bool bShowImg = false;

cv::CascadeClassifier hook_cascade;

std::vector<cv::Rect> hooks;

CascadeDetecter::CascadeDetecter(const char* cascade_name)
	:	mode(h_original),
		bShowImg(false)
{
	loadCascade(cascade_name);
}

CascadeDetecter::~CascadeDetecter() {}

bool CascadeDetecter::loadCascade(const char* cascade_Name)
{
	hook_cascade_name = cascade_Name;
	return cascade_loaded = hook_cascade.load(hook_cascade_name);
}

void CascadeDetecter::detectHookSample(cv::Mat& frame, sl::Mat* depth, sl::Mat* point_cloud)
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

	// TODO: because it's possible to detect multiple objects, we need a method to eliminate false positives5
	for (size_t i = 0; i < hooks.size(); i++)
	{
		// draw a rect around the detected object
		cv::Point p1(hooks[i].x, hooks[i].y);
		cv::Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
		rectangle(frame, p1, p2, cv::Scalar(255, 0, 255), 2, 8, 0);

		// get our pixel target center
		size_t x_target = hooks[i].x + hooks[i].width / 2;
		size_t y_target = hooks[i].y + hooks[i].height / 2;

		if (point_cloud != NULL)
		{
			sl::float4 point3D;
			// Get the 3D point cloud values for pixel 
			point_cloud->getValue(x_target, y_target, &point3D);

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
			depth->getValue(x_target, y_target, &Distance);

			if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
			{
				SmartDashboard::PutNumber("X Position", (double)x_target);
				SmartDashboard::PutNumber("Y Position", (double)y_target);
				SmartDashboard::PutNumber("Distance", Distance);
			}
		}
		else
		{
			SmartDashboard::PutNumber("X Position", (double)x_target);
			SmartDashboard::PutNumber("Y Position", (double)y_target);
		}
	}
}


#if 1
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

	// TODO: because it's possible to detect multiple objects, we need a method to eliminate false positives5
	for (size_t i = 0; i < hooks.size(); i++)
	{
		// draw a rect around the detected object
		cv::Point p1(hooks[i].x, hooks[i].y);
		cv::Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
		rectangle(frame, p1, p2, cv::Scalar(255, 0, 255), 2, 8, 0);

		// get our pixel target center
		size_t x_target = hooks[i].x + hooks[i].width / 2;
		size_t y_target = hooks[i].y + hooks[i].height / 2;

#ifdef USE_POINT_CLOUD 
		sl::float4 point3D;
		// Get the 3D point cloud values for pixel 
		point_cloud.getValue(x_target, y_target, &point3D);

		float Distance = sqrt(point3D.x*point3D.x + point3D.y*point3D.y + point3D.z*point3D.z);

		if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
		{
			//std::cout << "hook found at: " << point3D.x << ", " << point3D.y << ", " << point3D.z << " Dist: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
			SmartDashboard::PutNumber("X Position", point3D.x);
			SmartDashboard::PutNumber("Y Position", point3D.y);
			SmartDashboard::PutNumber("Z Position", point3D.z);
			SmartDashboard::PutNumber("Distance", Distance);
		}
#else
		float Distance = GetDistanceAtPoint(depth, x_target, y_target);

		if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
		{
			//std::cout << "hook found at: " << x_target << ", " << y_target << " Dist: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
			SmartDashboard::PutNumber("X Position", x_target);
			SmartDashboard::PutNumber("Y Position", y_target);
			SmartDashboard::PutNumber("Distance", Distance);
		}
#endif
	}
}
#endif