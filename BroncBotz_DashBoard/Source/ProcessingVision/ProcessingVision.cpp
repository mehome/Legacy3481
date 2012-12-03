
#include "stdafx.h"
#include "ProcessingVision.h"


extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_RGB32(Bitmap_Frame *Frame)
{
#if 1
	Frame = NI_VisionProcessing(Frame);
#else

	//Test... make a green box in the center of the frame
	size_t CenterY=Frame->YRes / 2;
	size_t CenterX=Frame->XRes / 2;
	size_t LineWidthInBytes=Frame->Stride * 4;
	for (size_t y=CenterY-5;y<CenterY+5;y++)
	{
		for (size_t x=CenterX-5; x<CenterX+5; x++)
		{
			*(Frame->Memory+ (x*4 + 0) + (LineWidthInBytes * y))=0;
			*(Frame->Memory+ (x*4 + 1) + (LineWidthInBytes * y))=255;
			*(Frame->Memory+ (x*4 + 2) + (LineWidthInBytes * y))=0;
		}
	}
#endif

	return Frame;
}
