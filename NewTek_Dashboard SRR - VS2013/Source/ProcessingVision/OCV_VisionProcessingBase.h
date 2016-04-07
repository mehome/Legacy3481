#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "profile.h"
#include "ProcessingVision.h"

#pragma warning(disable:4800)

class VisionTracker
{
public:
	VisionTracker();
	virtual ~VisionTracker();

	// override.
	virtual int ProcessImage(double &x, double &y) = 0;

	int GetFrame(Bitmap_Frame *Frame);
	void ReturnFrame(Bitmap_Frame *Frame);

	profile Profiler;

	void rotate_90n(cv::Mat &src, cv::Mat &dst, int angle);

protected:
	cv::Mat *InputImageRGB;
	cv::Mat *rotInputImageRGB;
};