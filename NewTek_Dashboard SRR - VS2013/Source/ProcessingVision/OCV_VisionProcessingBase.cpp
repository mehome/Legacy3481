#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "OCV_VisionProcessingBase.h"

using namespace cv;

VisionTracker::VisionTracker()
{	
	Profiler = new profile;
}

VisionTracker::~VisionTracker()
{
}


int VisionTracker::GetFrame(Bitmap_Frame *Frame)
{
	int success = 1;
	InputImageRGB = new Mat(Frame->YRes, Frame->XRes, CV_8UC4, Frame->Memory);
	if( InputImageRGB == NULL ) success = false;
	return success;
}

void VisionTracker::ReturnFrame(Bitmap_Frame *Frame)
{	// copy image back to our frame.
	// no need to copy, for return.
#if 0
	void *pImageArray = imaqImageToArray(InputImageRGB, IMAQ_NO_RECT, NULL, NULL);
	if(pImageArray != NULL)
	{
		memcpy((void*)Frame->Memory, pImageArray, Frame->Stride * 4 * Frame->YRes);
		imaqDispose(pImageArray);
	}
#endif
}

