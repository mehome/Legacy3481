
#include "stdafx.h"

int H_low = 122;
int H_high = 155;
int S_low = 50;
int S_high = 255;
int V_low = 90;
int V_high = 255;

int ThreshInc = 10;

void detectRockSample(cv::Mat frame, sl::Mat depth)
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

#if 0	// TODO: depends on camera
		float Distance = GetDistanceAtPoint(depth, (int)mc[i].x, (int)mc[i].y);
#endif

		if ((contourArea(contours[i]) > 150) &&
			(minRect[i].size.width > 10) &&
			(minRect[i].size.height > 10))
		{
#if 0		// TODO: fix reporting
			std::cout << "rock found at " << mc[i].x << ", " << mc[i].y << " distance: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
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
