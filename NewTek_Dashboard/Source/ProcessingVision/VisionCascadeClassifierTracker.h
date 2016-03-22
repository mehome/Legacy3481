#include "stdafx.h"

using namespace std;
using namespace cv;

class VisionCascadeClassifierTracker : public VisionTracker
{
public:
	VisionCascadeClassifierTracker();
	virtual ~VisionCascadeClassifierTracker();

	int ProcessImage(double &x, double &y);

private:
	CascadeClassifier hook_cascade;

	string hook_cascade_name;

	enum histo_mode mode;
	bool bShowImg;
	int frameCount;
	bool bCascadeLoaded;
};
