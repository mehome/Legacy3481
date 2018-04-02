#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"
#include "CascadeDetecter.h"

CascadeDetecter::CascadeDetecter(const char* cascade_name)
	:	mode(h_original),
		bShowImg(false)
{
#if defined(HAVE_CUDA)
	cascade_gpu = cv::cuda::CascadeClassifier::create(cascade_name);
#else
	loadCascade(cascade_name);
#endif
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

#if defined(HAVE_CUDA)
	cv::cuda::GpuMat frame_gpu, gray_gpu, hooksBuf_gpu;
	frame_gpu.upload(frame);
	cv::cuda::cvtColor(frame_gpu, gray_gpu, cv::COLOR_BGR2GRAY);

	cascade_gpu->setFindLargestObject(true);
	cascade_gpu->setScaleFactor(1.2);
	cascade_gpu->setMinNeighbors(4);

	cascade_gpu->detectMultiScale(gray_gpu, hooksBuf_gpu);
	cascade_gpu->convert(hooksBuf_gpu, hooks);

#else
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
	hook_cascade.detectMultiScale(frame_gray, hooks, 1.3, 4, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, cv::Size(20, 30));
#endif

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
