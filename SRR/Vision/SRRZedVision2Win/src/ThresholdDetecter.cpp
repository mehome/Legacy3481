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
	:	thresh_inc(10),
		threshold_setting(H_Low)
{
}

ThresholdDetecter::ThresholdDetecter(int3 low, int3 high)
	: thresh_inc(10),
	threshold_setting(H_Low)
{	// original values
	HSV_low = low;
	HSV_high = high;
	// working values
	HSV_Range[H_Low] = HSV_low.x;
	HSV_Range[S_Low] = HSV_low.y;
	HSV_Range[V_Low] = HSV_low.z;
	HSV_Range[H_High] = HSV_high.x;
	HSV_Range[S_High] = HSV_high.y;
	HSV_Range[V_High] = HSV_high.z;

	str_threshold_setting[H_Low] = "THRESHOLD HUE LOW";
	str_threshold_setting[H_High] = "THRESHOLD HUE HIGH";
	str_threshold_setting[S_Low] = "THRESHOLD SATURATION LOW";
	str_threshold_setting[S_High] = "THRESHOLD SATURATION HIGH";
	str_threshold_setting[V_Low] = "THRESHOLD VALUE LOW";
	str_threshold_setting[V_High] = "THRESHOLD VALUE HIGH";
};



ThresholdDetecter::~ThresholdDetecter()
{	// original values (wide open)
	HSV_low.x = HSV_low.y = HSV_low.z = 0;
	HSV_high.x = HSV_high.y = HSV_high.z = 255;
	// working values
	HSV_Range[H_Low] = HSV_low.x;
	HSV_Range[S_Low] = HSV_low.y;
	HSV_Range[V_Low] = HSV_low.z;
	HSV_Range[H_High] = HSV_high.x;
	HSV_Range[S_High] = HSV_high.y;
	HSV_Range[V_High] = HSV_high.z;

	str_threshold_setting[H_Low] = "THRESHOLD HUE LOW";
	str_threshold_setting[H_High] = "THRESHOLD HUE HIGH";
	str_threshold_setting[S_Low] = "THRESHOLD SATURATION LOW";
	str_threshold_setting[S_High] = "THRESHOLD SATURATION HIGH";
	str_threshold_setting[V_Low] = "THRESHOLD VALUE LOW";
	str_threshold_setting[V_High] = "THRESHOLD VALUE HIGH";
}

void ThresholdDetecter::detectRockSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud)
{

}

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
		HSV_Range[threshold_setting] += thresh_inc;
		std::cout << str_threshold_setting[threshold_setting] << ": " << HSV_Range[threshold_setting] << std::endl;
		break;

		// Decrease camera settings value 
	case '<':
		HSV_Range[threshold_setting] -= thresh_inc;
		std::cout << str_threshold_setting[threshold_setting] << ": " << HSV_Range[threshold_setting] << std::endl;
		break;

		// Reset to default parameters
	case 'R':
		std::cout << "Reset HSV settings to default" << std::endl;
		HSV_Range[H_Low] = HSV_low.x;
		HSV_Range[S_Low] = HSV_low.y;
		HSV_Range[V_Low] = HSV_low.z;
		HSV_Range[H_High] = HSV_high.x;
		HSV_Range[S_High] = HSV_high.y;
		HSV_Range[V_High] = HSV_high.z;
		break;
	}
}

/**
This function toggles between threshold settings
**/
void ThresholdDetecter::switchThresholdSettings() {
	switch (threshold_setting) {
	case H_Low:
		threshold_setting = S_Low;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case S_Low:
		threshold_setting = V_Low;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case V_Low:
		threshold_setting = H_High;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case H_High:
		threshold_setting = S_High;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case S_High:
		threshold_setting = V_High;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case V_High:
		threshold_setting = H_Low;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;
	}
}

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
