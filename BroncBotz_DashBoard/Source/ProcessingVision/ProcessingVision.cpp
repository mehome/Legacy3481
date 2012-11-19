
#include "stdafx.h"
#include "ProcessingVision.h"


extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_RGB24(Bitmap_Frame *Frame)
{
	#if 1
	//Test... make a green box in the center of the frame
	size_t CenterY=Frame->YRes / 2;
	size_t CenterX=Frame->XRes / 2;
	size_t LineWidthInBytes=Frame->Stride * 3;
	for (size_t y=CenterY;y<CenterY+10;y++)
	{
		for (size_t x=CenterX; x<CenterX+10; x++)
		{
			*(Frame->Memory+ (x*3 + 0) + (LineWidthInBytes * y))=0;
			*(Frame->Memory+ (x*3 + 1) + (LineWidthInBytes * y))=255;
			*(Frame->Memory+ (x*3 + 2) + (LineWidthInBytes * y))=0;
		}
	}
	return Frame;
	#else
	return Frame;
	#endif
}
