#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"
#include "ThresholdDetecter.h"

ThresholdDetecter::ThresholdDetecter()
	: thresh_inc(10),
	threshold_setting(H_Low),
	passcolor(0, 255, 0),
	failcolor(0, 0, 255),
	objRectMin(cv::Point2f(0, 0), cv::Size(1280, 720), 180),
	objRectMax(cv::Point2f(0, 0), cv::Size(20, 20), -180),
	AreaMin(1280*720), AreaMax(0),
	DistMin(64), DistMax(0),
	MouseDown(false)
{
}

ThresholdDetecter::ThresholdDetecter(int3 low, int3 high)
	: thresh_inc(10),
	threshold_setting(H_Low),
	passcolor(0, 255, 0),
	failcolor(0, 0, 255),
	objRectMin(cv::Point2f(0, 0), cv::Size(1280, 720), 180),
	objRectMax(cv::Point2f(0, 0), cv::Size(20, 20), -180),
	AreaMin(1280 * 720), AreaMax(250),
	DistMin(64), DistMax(0),
	MouseDown(false)
{
	// original values
	HSV_low = low;
	HSV_high = high;
	// working values
	HSV_Range[H_Low] = HSV_low.x;
	HSV_Range[S_Low] = HSV_low.y;
	HSV_Range[V_Low] = HSV_low.z;
	HSV_Range[H_High] = HSV_high.x;
	HSV_Range[S_High] = HSV_high.y;
	HSV_Range[V_High] = HSV_high.z;

	str_threshold_setting[H_Range] = "THRESHOLD HUE RANGE";
	str_threshold_setting[H_Center] = "THRESHOLD HUE CENTER";
	str_threshold_setting[S_Low] = "THRESHOLD SATURATION LOW";
	str_threshold_setting[S_High] = "THRESHOLD SATURATION HIGH";
	str_threshold_setting[V_Low] = "THRESHOLD VALUE LOW";
	str_threshold_setting[V_High] = "THRESHOLD VALUE HIGH";
};

ThresholdDetecter::~ThresholdDetecter() {}

void ThresholdDetecter::setThreshold(int3 low, int3 high)
{
	HSV_Range[H_Low] = low.x;
	HSV_Range[S_Low] = low.y;
	HSV_Range[V_Low] = low.z;
	HSV_Range[H_High] = high.x;
	HSV_Range[S_High] = high.y;
	HSV_Range[V_High] = high.z;
}

std::pair<int3, int3> ThresholdDetecter::getThreshold(void)
{
	int3 low, high;
	low.x = HSV_Range[H_Low];
	low.y = HSV_Range[S_Low];
	low.z = HSV_Range[V_Low];
	high.x = HSV_Range[H_High];
	high.y = HSV_Range[S_High];
	high.z = HSV_Range[V_High];
	std::pair<int3, int3> retval(low, high);
	return retval;
}

void ThresholdDetecter::printThreshold(void)
{
	std::cout << "Threshold range" << std::endl;
	std::cout << "H: " << HSV_Range[H_Low] << " - " << HSV_Range[H_High] << std::endl;
	std::cout << "S: " << HSV_Range[S_Low] << " - " << HSV_Range[S_High] << std::endl;
	std::cout << "V: " << HSV_Range[V_Low] << " - " << HSV_Range[V_High] << std::endl;
}

void ThresholdDetecter::detectRockSample(cv::Mat& frame, sl::Mat* depth, sl::Mat* point_cloud, cv::Point mhit, bool small_display)
{
	cv::Mat binary;
	cv::Mat hsv, masked;

	//convert the img from color to hsv
	cv::cvtColor(frame, hsv, CV_BGR2HSV);

	cv::GaussianBlur(hsv, hsv, cv::Size(5, 5), 0, 0);

	//process the image - threshold
	cv::inRange(hsv, cv::Scalar(HSV_Range[H_Low], HSV_Range[S_Low], HSV_Range[V_Low]), cv::Scalar(HSV_Range[H_High], HSV_Range[S_High], HSV_Range[V_High]), binary);

	int erosion_size = 3;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		cv::Point(erosion_size, erosion_size));
	// this eliminates small artifacts.
	cv::erode(binary, binary, element);
	cv::dilate(binary, binary, element);

	/// Find contours
	cv::findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	// mask and display   
	frame.copyTo(masked, binary);

	/// rotated rectangles 
	std::vector<cv::RotatedRect> minRect(contours.size());

	if (mhit.x != -1 && mhit.y != -1)
		std::cout << "-------------- " << contours.size() << " countours ---------------------------------------------" << std::endl;

	for (int i = 0; i< contours.size(); i++)
	{
		/// Find the rotated rectangles for each contour
		minRect[i] = cv::minAreaRect(cv::Mat(contours[i]));

#ifdef BUTTON_TARGET_PICKER 
		// Is the middle Mouse button down?
		if (mhit.x != -1 && mhit.y != -1)
		{
			if (!MouseDown)
			{	// reset
				objRectMax.center = cv::Point2f(0, 0);
				objRectMax.size = cv::Size(0, 0);
				objRectMax.angle = -180;
				objRectMin.center = cv::Point2f(0, 0);
				objRectMin.size = cv::Size(1280, 720);
				objRectMin.angle = 180;
				AreaMax = 0;
				AreaMin = 1280 * 720;
				DistMax = 0;
				DistMin = 64;
				MouseDown = true;
				std::cout << "min max reset." << std::endl;
			}

			// First, see if it's in the bounding box for this rect
			cv::Rect bounds(minRect[i].boundingRect());
			if ((mhit.x > bounds.x) && (mhit.x < (bounds.x + bounds.width)) &&
				(mhit.y > bounds.y) && (mhit.y < (bounds.y + bounds.height)))
			{
				int margin = 10;
				// now get value ranges
				if (minRect[i].angle < objRectMin.angle) objRectMin.angle = minRect[i].angle - margin;
				if (minRect[i].angle > objRectMax.angle) objRectMax.angle = minRect[i].angle + margin;
				if (minRect[i].size.width < objRectMin.size.width) objRectMin.size.width = minRect[i].size.width - margin;
				if (minRect[i].size.width > objRectMax.size.width) objRectMax.size.width = minRect[i].size.width + margin;
				if (minRect[i].size.height < objRectMin.size.height) objRectMin.size.height = minRect[i].size.height - margin;
				if (minRect[i].size.height > objRectMax.size.height) objRectMax.size.height = minRect[i].size.height + margin;
				if (contourArea(contours[i]) < AreaMin) AreaMin = contourArea(contours[i]) - margin*10;
				if (contourArea(contours[i]) > AreaMax) AreaMax = contourArea(contours[i]) + margin*10;

				std::cout << "HIT" << std::endl;
				std::cout << "idx: " << i << " angle: " << minRect[i].angle << std::endl;
				std::cout << "area: " << contourArea(contours[i]) << std::endl;
				std::cout << "width: " << minRect[i].size.width << std::endl;
				std::cout << "height: " << minRect[i].size.height << std::endl << std::endl << std::endl;

				std::cout << "min angle: " << objRectMin.angle << " max angle: " << objRectMax.angle << std::endl;
				std::cout << "min area: " << AreaMin << " max area: " << AreaMax << std::endl;
				std::cout << "min width: " << objRectMin.size.width << " max width: " << objRectMax.size.width << std::endl;
				std::cout << "min height: " << objRectMin.size.height << " max height: " << objRectMax.size.height << std::endl << std::endl;
				if (point_cloud != NULL)
				{
					sl::float4 point3D;
					// Get the 3D point cloud values for pixel 
					point_cloud->getValue((size_t)minRect[i].center.x, (size_t)minRect[i].center.y, &point3D);

					if ((!isnan(point3D.x) && point3D.x != sl::TOO_CLOSE && point3D.x != sl::TOO_FAR) &&
						(!isnan(point3D.y) && point3D.y != sl::TOO_CLOSE && point3D.y != sl::TOO_FAR) &&
						(!isnan(point3D.z) && point3D.z != sl::TOO_CLOSE && point3D.z != sl::TOO_FAR))
					{
						float Distance = sqrt(point3D.x*point3D.x + point3D.y*point3D.y + point3D.z*point3D.z);
						if (Distance < DistMin) DistMin = Distance;
						if (Distance > DistMax) DistMax = Distance;
					}
				}
				else if (depth != NULL)
				{
					float Distance;
					depth->getValue((size_t)minRect[i].center.x, (size_t)minRect[i].center.y, &Distance);

					if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
					{
						if (Distance < DistMin) DistMin = Distance;
						if (Distance > DistMax) DistMax = Distance;
					}
				}
			}
		}
		else
			MouseDown = false;

		if (/*(minRect[i].angle >= objRectMin.angle) && (minRect[i].angle <= objRectMax.angle) &&*/
			(contourArea(contours[i]) >= AreaMin) && (contourArea(contours[i]) <= AreaMax) &&
			(minRect[i].size.width >= objRectMin.size.width) && (minRect[i].size.width <= objRectMax.size.width) &&
			(minRect[i].size.height >= objRectMin.size.height) && (minRect[i].size.height <= objRectMax.size.height))
#else
		if ((contourArea(contours[i]) > 250) &&
			(minRect[i].size.width > 20) &&
			(minRect[i].size.height > 20))
#endif
		{
			if (point_cloud != NULL)
			{
				sl::float4 point3D;
				// Get the 3D point cloud values for pixel 
				point_cloud->getValue((size_t)minRect[i].center.x, (size_t)minRect[i].center.y, &point3D);

				if ((!isnan(point3D.x) && point3D.x != sl::TOO_CLOSE && point3D.x != sl::TOO_FAR) &&
					(!isnan(point3D.y) && point3D.y != sl::TOO_CLOSE && point3D.y != sl::TOO_FAR) &&
					(!isnan(point3D.z) && point3D.z != sl::TOO_CLOSE && point3D.z != sl::TOO_FAR))
				{
					float Distance = sqrt(point3D.x*point3D.x + point3D.y*point3D.y + point3D.z*point3D.z);

					SmartDashboard::PutNumber("X Position", point3D.x);
					SmartDashboard::PutNumber("Y Position", point3D.y);
					SmartDashboard::PutNumber("Z Position", point3D.z);
					SmartDashboard::PutNumber("Distance", Distance);
				}
			}
			else if (depth != NULL)
			{
				float Distance;
				depth->getValue((size_t)minRect[i].center.x, (size_t)minRect[i].center.y, &Distance);

				if (Distance != sl::OCCLUSION_VALUE && Distance != sl::TOO_CLOSE && Distance != sl::TOO_FAR)
				{
					SmartDashboard::PutNumber("X Position", minRect[i].center.x);
					SmartDashboard::PutNumber("Y Position", minRect[i].center.y);
					SmartDashboard::PutNumber("Distance", Distance);
				}
			}
			else
			{
				SmartDashboard::PutNumber("X Position", minRect[i].center.x);
				SmartDashboard::PutNumber("Y Position", minRect[i].center.y);
			}

			/// Draw contours
			cv::drawContours(frame, contours, i, passcolor, 2, 8, hierarchy, 0, cv::Point());
			cv::circle(frame, minRect[i].center, 4, passcolor, -1, 8, 0);
			// rotated rectangle
			cv::Point2f rect_points[4]; minRect[i].points(rect_points);
			for (int j = 0; j < 4; j++)
				cv::line(frame, rect_points[j], rect_points[(j + 1) % 4], passcolor, 1, 8);
		}
		else
		{
			/// Draw contours
			cv::drawContours(frame, contours, i, failcolor, 2, 8, hierarchy, 0, cv::Point());
			cv::circle(frame, minRect[i].center, 4, failcolor, -1, 8, 0);
			// rotated rectangle
			cv::Point2f rect_points[4]; minRect[i].points(rect_points);
			for (int j = 0; j < 4; j++)
				cv::line(frame, rect_points[j], rect_points[(j + 1) % 4], failcolor, 1, 8);
		}
	}

	if (small_display)
		resize(masked, masked, cv::Size((int)masked.cols / 2, (int)masked.rows / 2));

	cv::imshow("Masked", masked);
}

/**
This function updates threshold settings
**/
void ThresholdDetecter::updateThresholdSettings(int key) {

	// Keyboard shortcuts
	switch (key) {

		// Switch to the next camera parameter
	case 'S':
		switchThresholdSettings();
		break;

		// Increase camera settings value 
	case '>':
		switch (threshold_setting) {
		case H_Range:
		{
			int range = HSV_Range[H_High] - HSV_Range[H_Low];
			int center = HSV_Range[H_High] - range / 2;
			range += thresh_inc;
			if (range > 255) range = 255;
			std::cout << str_threshold_setting[threshold_setting] << ": " << range << std::endl;
			HSV_Range[H_High] = center + range / 2;
			HSV_Range[H_Low] = center - range / 2;
			break;
		}
		case H_Center:
		{
			int range = HSV_Range[H_High] - HSV_Range[H_Low];
			int center = HSV_Range[H_High] - range / 2;
			center += thresh_inc;
			if (center + range / 2 > 255) center = 255 - range / 2;
			std::cout << str_threshold_setting[threshold_setting] << ": " << center << std::endl;
			HSV_Range[H_High] = center + range / 2;
			HSV_Range[H_Low] = center - range / 2;
			break;
		}
		default:
			HSV_Range[threshold_setting] += thresh_inc;
			if (threshold_setting % 2 == 0)
			{
				if (HSV_Range[threshold_setting] > HSV_Range[threshold_setting + 1])
					HSV_Range[threshold_setting] = HSV_Range[threshold_setting + 1];
			}
			else
			{
				if (HSV_Range[threshold_setting] > 255)
					HSV_Range[threshold_setting] = 255;
			}

			std::cout << str_threshold_setting[threshold_setting] << ": " << HSV_Range[threshold_setting] << std::endl;
			break;
		}
		break;
		// Decrease camera settings value 
	case '<':
		switch (threshold_setting) {
		case H_Range:
		{
			int range = HSV_Range[H_High] - HSV_Range[H_Low];
			int center = HSV_Range[H_High] - range / 2;
			range -= thresh_inc;
			if (range < 0) range = 0;
			std::cout << str_threshold_setting[threshold_setting] << ": " << range << std::endl;
			HSV_Range[H_High] = center + range / 2;
			HSV_Range[H_Low] = center - range / 2 - 1;
			break;
		}
		case H_Center:
		{
			int range = HSV_Range[H_High] - HSV_Range[H_Low];
			int center = HSV_Range[H_High] - range / 2;
			center -= thresh_inc;
			if (center - range / 2 < 0) center = 0 + range / 2;
			std::cout << str_threshold_setting[threshold_setting] << ": " << center << std::endl;
			HSV_Range[H_High] = center + range / 2;
			HSV_Range[H_Low] = center - range / 2 - 1;
			break;
		}
		default:
			HSV_Range[threshold_setting] -= thresh_inc;
			if (threshold_setting % 2 == 0)
			{
				if (HSV_Range[threshold_setting] < 0)
					HSV_Range[threshold_setting] = 0;
			}
			else
			{
				if (HSV_Range[threshold_setting] < HSV_Range[threshold_setting - 1])
					HSV_Range[threshold_setting] = HSV_Range[threshold_setting - 1];
			}
			std::cout << str_threshold_setting[threshold_setting] << ": " << HSV_Range[threshold_setting] << std::endl;
			break;
		}
		break;

	case 'i':
		if (thresh_inc < 10)
			thresh_inc++;
		else
			thresh_inc += 5;
		if (thresh_inc > 20) thresh_inc = 20;
		std::cout << "threshold_inc: " << thresh_inc << std::endl;
		break;

	case 'I':
		if (thresh_inc < 10)
			thresh_inc--;
		else
			thresh_inc -= 5;
		if (thresh_inc <= 2) thresh_inc = 2;
		std::cout << "threshold_inc: " << thresh_inc << std::endl;
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

	case 'Z':
		std::cout << "Full range" << std::endl;
		HSV_Range[H_Low] = 0;
		HSV_Range[S_Low] = 0;
		HSV_Range[V_Low] = 0;
		HSV_Range[H_High] = 255;
		HSV_Range[S_High] = 255;
		HSV_Range[V_High] = 255;
		break;

	case 0x00700000:	saveThreshold("slot1"); break;
	case 0x00710000:	saveThreshold("slot2"); break;
	case 0x00720000:	saveThreshold("slot3"); break;
	case 0x00730000:	saveThreshold("slot4"); break;
	
	case 0x00740000:	loadThreshold("slot1"); break;
	case 0x00750000:	loadThreshold("slot2"); break;
	case 0x00760000:	loadThreshold("slot3"); break;
	case 0x00770000:	loadThreshold("slot4"); break;

	}
}

/**
This function toggles between threshold settings
**/
void ThresholdDetecter::switchThresholdSettings() {
	switch (threshold_setting) {
	case H_Range:
		threshold_setting = H_Center;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case H_Center:
		threshold_setting = S_Low;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case S_Low:
		threshold_setting = S_High;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case S_High:
		threshold_setting = V_Low;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case V_Low:
		threshold_setting = V_High;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;

	case V_High:
		threshold_setting = H_Range;
		std::cout << "Threshold Settings: " << str_threshold_setting[threshold_setting] << std::endl;
		break;
	}
}

/**
/ This function loads threshold settings from one of 4 slots
**/
bool ThresholdDetecter::loadThreshold(std::string file)
{
	bool ret = false;
	std::string line;
	std::string::size_type sz;   // alias of size_t

	std::ifstream myfile(file);
	if (myfile.is_open())
	{
		for (int i = 0; i < 6; i++)
		{
			if(getline(myfile, line))
				HSV_Range[i] = std::stoi(line, &sz);
		}
		myfile.close();
		std::cout << "HSV " << file << " loaded." << std::endl;

		ret = true;
	}
	else std::cout << "Unable to open " << file << std::endl;

	return ret;
}

/**
/ This function saves threashold settings into one of 4 slots
**/
void ThresholdDetecter::saveThreshold(std::string file)
{
	std::ofstream myfile;
	myfile.open(file);
	for (int i = 0; i < 6; i++)
		myfile << HSV_Range[i] << std::endl;
	myfile.close();
	std::cout << "HSV " << file << " saved." << std::endl;
}



