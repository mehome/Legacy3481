#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "OCV_VisionProcessingBase.h"

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
#if 0
	// Copy our frame to an NI image.
	success = imaqArrayToImage(InputImageRGB, (void*)Frame->Memory, Frame->XRes, Frame->YRes);
	imaqGetImageInfo(InputImageRGB, &SourceImageInfo);
#endif
	return success;
}

void VisionTracker::ReturnFrame(Bitmap_Frame *Frame)
{	// copy image back to our frame.
#if 0
	void *pImageArray = imaqImageToArray(InputImageRGB, IMAQ_NO_RECT, NULL, NULL);
	if(pImageArray != NULL)
	{
		memcpy((void*)Frame->Memory, pImageArray, Frame->Stride * 4 * Frame->YRes);
		imaqDispose(pImageArray);
	}
#endif
}

