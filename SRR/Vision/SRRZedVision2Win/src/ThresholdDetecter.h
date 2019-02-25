#pragma once

#include "stdafx.h"

class ThresholdDetecter
{
public:
	ThresholdDetecter(bool interactive);
	ThresholdDetecter(int3 lov, int3 high, bool interactive);
	~ThresholdDetecter();

	void detectRockSample(cv::Mat& frame, sl::Mat* point_cloud, sl::Pose* camera_pose, cv::Point mhit, bool small_display);
	void updateThresholdSettings(int key);
	void switchThresholdSettings();
	bool loadThreshold(std::string file);
	void saveThreshold(std::string file);
	void setThreshold(int3 low, int3 high);
	std::pair<int3, int3> getThreshold(void);
	void printThreshold(void);

private:
	int HSV_Range[6];
	int3 HSV_low;
	int3 HSV_high;

	std::string str_threshold_setting[6];

	enum THRESHOLD_SETTING {
		H_Low,
		H_Range = H_Low,
		H_High,
		H_Center = H_High,
		S_Low,
		S_High,
		V_Low,
		V_High
	};

	THRESHOLD_SETTING threshold_setting;
	int thresh_inc;

	cv::Scalar passcolor;
	cv::Scalar failcolor;
	cv::Mat hsv, masked, binary;

#if defined(HAVE_CUDA) && defined(USE_CUDA)
	cv::cuda::GpuMat frame_gpu, hsv_gpu, masked_gpu, binary_gpu;
	cv::Ptr<cv::cuda::Filter> blurFilter_; 
	cv::Ptr<cv::cuda::Filter> erodeFilter_;
	cv::Ptr<cv::cuda::Filter> dilateFilter_;
	cv::cuda::Stream stream;
#endif

	// countours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	// max, min values for object qaulifying
	// this is to help with narrowing the valid targets
	cv::RotatedRect objRectMax, objRectMin;
	double AreaMax, AreaMin;
	float DistMax, DistMin;
	bool MouseDown;
	bool interactive_mode;

	class foundTarget
	{
	public:
		foundTarget(sl::float4 loc)
		:	Location(loc), avgLocation(loc), conf(0), in_view(true) {}
		foundTarget& operator += ( const sl::float4 nl ) 
		{	// avarage in a new point
			avgLocation.x += nl.x; avgLocation.x /= 2;
			avgLocation.y += nl.y; avgLocation.y /= 2;
			avgLocation.z += nl.z; avgLocation.z /= 2;
		}
		// check if close enough
		bool in_range(const sl::float4 loc)
		{
			float dist = sl::float4::distance(Location, loc);
			return dist <= min_dist;
		}
	private:
		sl::float4 Location;
		sl::float4 avgLocation;
		float conf;
		bool in_view;
		const float min_dist = 0.01f;
	};
	std::vector<foundTarget> foundList;

	std::ofstream outputFile;
};