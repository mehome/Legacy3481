#include "stdafx.h"

class VisionCascadeClassifierTracker : public VisionTracker
{
public:
	VisionCascadeClassifierTracker();
	virtual ~VisionCascadeClassifierTracker();

	int ProcessImage(double &x, double &y);

private:
#ifdef OCV_READY
	void detectAndDisplay(Mat frame);

	CascadeClassifier hook_cascade;

	String hook_cascade_name;
	string window_name;

	std::string filename;
	enum histo_mode mode;
	bool bShowImg;
	int frameCount;
	bool bCascadeLoaded;
#endif
};
