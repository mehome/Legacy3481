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
	CascadeDetecter(const char* cascade_name);
	~CascadeDetecter();

	void detectHookSample(cv::Mat& frame, sl::Mat* depth, sl::Mat* point_cloud);
	bool loadCascade(const char* cascade_name);
	bool cascadeLoaded(void) 	{ return cascade_loaded; }

private:
	histo_mode mode;
	bool bShowImg;
	bool cascade_loaded;
#if defined(HAVE_CUDA) && !defined(OLDSCHOOL)
	cv::Ptr<cv::cuda::CascadeClassifier> cascade_gpu;
#endif
	cv::CascadeClassifier hook_cascade;
	std::vector<cv::Rect> hooks;
	std::string hook_cascade_name;
};