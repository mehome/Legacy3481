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

protected:
#if 0
	// images
	Image *InputImageRGB;		// our input image
	ImageInfo SourceImageInfo;	// info about our input image
	Image *ParticleImageU8;		// 8 bit image - particle processing
	Image *WorkImageU8;			// 8 bit image - work area
	Image *ThresholdImageU8;

	// separate planes for splitting color images
	Image *Plane1;
	Image *Plane2;
	Image *Plane3;

	Image *DestinationThresImage;
#endif
};