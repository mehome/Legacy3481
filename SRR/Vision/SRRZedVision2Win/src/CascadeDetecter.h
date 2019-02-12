#pragma once

#include "stdafx.h"

enum histo_mode
{
	h_original,
	h_equalize,
	h_clahe
};


class CascadeDetecter
{
public:
	CascadeDetecter(const char* cascade_name, bool interactive);
	~CascadeDetecter();

	void detectHookSample(cv::Mat& frame, sl::Mat* point_cloud, sl::Pose* camera_pose);
	bool loadCascade(const char* cascade_name);
	bool cascadeLoaded(void) 	{ return cascade_loaded; }

private:
	histo_mode mode;
	bool bShowImg;
	bool interactive_mode;
	bool cascade_loaded;
#if defined(HAVE_CUDA) && defined(USE_CUDA)
	cv::cuda::GpuMat frame_gpu, gray_gpu, hooksBuf_gpu;
	cv::Ptr<cv::cuda::CascadeClassifier> cascade_gpu;
#endif
	cv::CascadeClassifier hook_cascade;
	std::vector<cv::Rect> hooks;
	std::string hook_cascade_name;
};