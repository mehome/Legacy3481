#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"
#include "ThresholdDetecter.h"

int H_low = 122;
int H_high = 155;
int S_low = 50;
int S_high = 255;
int V_low = 90;
int V_high = 255;

int ThreshInc = 10;

float GetDistanceAtPoint(sl::Mat depth, size_t x, size_t y);

ThresholdDetecter::ThresholdDetecter()
	: ThreshInc(10)
{
	HSVLow.x = 0; HSVHigh.y = 0; HSVHigh.z = 0;
	HSVHigh.x = 255; HSVHigh.y = 255; HSVHigh.z = 255;
}

ThresholdDetecter::ThresholdDetecter(int3 low, int3 high)
	:	ThreshInc(10),
		HSVLow(low),
		HSVHigh(high)
{
}

ThresholdDetecter::~ThresholdDetecter()
{
}

void ThresholdDetecter::detectRockSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud)
{

}

#if 0
/**
This function updates threshold settings
**/
void ThresholdDetecter::updateThresholdSettings(char key) {

	// Keyboard shortcuts
	switch (key) {

		// Switch to the next camera parameter
	case 'S':
		switchThresholdSettings();
		break;

		// Increase camera settings value 
	case '>':
		current_value = zed->getCameraSettings(camera_settings_);
		zed->setCameraSettings(camera_settings_, current_value + step_camera_setting);
		std::cout << str_camera_settings << ": " << current_value + step_camera_setting << std::endl;
		break;

		// Decrease camera settings value 
	case '<':
		current_value = zed->getCameraSettings(camera_settings_);
		if (current_value >= 1) {
			zed->setCameraSettings(camera_settings_, current_value - step_camera_setting);
			std::cout << str_camera_settings << ": " << current_value - step_camera_setting << std::endl;
		}
		break;

		// Reset to default parameters
	case 'r':
		std::cout << "Reset all settings to default" << std::endl;
		zed->setCameraSettings(sl::CAMERA_SETTINGS_BRIGHTNESS, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_CONTRAST, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_HUE, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_SATURATION, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_GAIN, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_WHITEBALANCE, -1, true);
		break;
	}
}

/**
This function toggles between threshold settings
**/
void ThresholdDetecter::switchThresholdSettings() {
	switch (camera_settings_) {
	case sl::CAMERA_SETTINGS_BRIGHTNESS:
		camera_settings_ = sl::CAMERA_SETTINGS_CONTRAST;
		str_camera_settings = "Contrast";
		std::cout << "Camera Settings: CONTRAST" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_CONTRAST:
		camera_settings_ = sl::CAMERA_SETTINGS_HUE;
		str_camera_settings = "Hue";
		std::cout << "Camera Settings: HUE" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_HUE:
		camera_settings_ = sl::CAMERA_SETTINGS_SATURATION;
		str_camera_settings = "Saturation";
		std::cout << "Camera Settings: SATURATION" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_SATURATION:
		camera_settings_ = sl::CAMERA_SETTINGS_GAIN;
		str_camera_settings = "Gain";
		std::cout << "Camera Settings: GAIN" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_GAIN:
		camera_settings_ = sl::CAMERA_SETTINGS_EXPOSURE;
		str_camera_settings = "Exposure";
		std::cout << "Camera Settings: EXPOSURE" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_EXPOSURE:
		camera_settings_ = sl::CAMERA_SETTINGS_WHITEBALANCE;
		str_camera_settings = "White Balance";
		step_camera_setting = 100;
		std::cout << "Camera Settings: WHITE BALANCE" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_WHITEBALANCE:
		camera_settings_ = sl::CAMERA_SETTINGS_BRIGHTNESS;
		str_camera_settings = "Brightness";
		std::cout << "Camera Settings: BRIGHTNESS" << std::endl;
		break;
	}
}
#endif

void detectRockSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud)
{
	cv::Scalar color = cv::Scalar(255, 0, 255);

	cv::Mat hsv, binary, masked;

	//convert the img from color to hsv
	cv::cvtColor(frame, hsv, CV_BGR2HSV);

	//cv::blur(hsv, hsv, cv::Size(3,3));
	cv::GaussianBlur(hsv, hsv, cv::Size(5, 5), 0, 0);

	//process the image - threshold
	cv::inRange(hsv, cv::Scalar(H_low, S_low, V_low), cv::Scalar(H_high, S_high, V_high), binary);

	int erosion_size = 3;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		cv::Point(erosion_size, erosion_size));
	// this eliminates small artifacts.
	cv::erode(binary, binary, element);
	cv::dilate(binary, binary, element);

	// countours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	/// Find contours
	cv::findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	// mask and display   
	frame.copyTo(masked, binary);

	/// moments
	std::vector<cv::Moments> mu(contours.size());
	/// mass centers
	std::vector<cv::Point2f> mc(contours.size());
	/// rotated rectangles 
	std::vector<cv::RotatedRect> minRect(contours.size());

	for (int i = 0; i< contours.size(); i++)
	{
		/// Get the moments
		mu[i] = moments(contours[i], false);
		///  Get the mass centers:
		mc[i] = cv::Point2f((float)(mu[i].m10 / mu[i].m00), (float)(mu[i].m01 / mu[i].m00));
		/// Find the rotated rectangles for each contour
		minRect[i] = cv::minAreaRect(cv::Mat(contours[i]));

		if ((contourArea(contours[i]) > 150) &&
			(minRect[i].size.width > 10) &&
			(minRect[i].size.height > 10))
		{
#ifdef USE_POINT_CLOUD 
			sl::float4 point3D;
			// Get the 3D point cloud values for pixel 
			point_cloud.getValue((size_t)mc[i].x, (size_t)mc[i].y, &point3D);

			float Distance = sqrt(point3D.x*point3D.x + point3D.y*point3D.y + point3D.z*point3D.z);

			if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
			{
				std::cout << "rock found at: " << point3D.x << ", " << point3D.y << ", " << point3D.z << " Dist: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
				SmartDashboard::PutNumber("X Position", point3D.x);
				SmartDashboard::PutNumber("Y Position", point3D.y);
				SmartDashboard::PutNumber("Z Position", point3D.z);
				SmartDashboard::PutNumber("Distance", Distance);
			}
#else
			float Distance = GetDistanceAtPoint(depth, (size_t)mc[i].x, (size_t)mc[i].y);

			if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
			{
				std::cout << "rock found at " << mc[i].x << ", " << mc[i].y << " distance: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
				SmartDashboard::PutNumber("X Position", mc[i].x);
				SmartDashboard::PutNumber("Y Position", mc[i].y);
				SmartDashboard::PutNumber("Distance", Distance);
			}
#endif
			/// Draw contours
			cv::drawContours(frame, contours, i, color, 2, 8, hierarchy, 0, cv::Point());
			cv::circle(frame, mc[i], 4, color, -1, 8, 0);
			// rotated rectangle
			cv::Point2f rect_points[4]; minRect[i].points(rect_points);
			for (int j = 0; j < 4; j++)
				cv::line(frame, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
		}
	}

	cv::imshow("Maksed", masked);
}
