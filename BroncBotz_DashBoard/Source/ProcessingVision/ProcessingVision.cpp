
#include "stdafx.h"
#define HAVE_NI_VISION 0

#if HAVE_NI_VISION
	#include <nivision.h>
	#include <nimachinevision.h>
#endif
#include "ProcessingVision.h"


extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_RGB32(Bitmap_Frame *Frame)
{
#if HAVE_NI_VISION
	ImageType imageType = IMAQ_IMAGE_RGB;    // Image Type
	Image* image;		    // Image
	//Image* image2;
	void* pImageArray;

	// Create an IMAQ Vision image
	image = imaqCreateImage(imageType, 7);	// arg 2 is border
	imaqArrayToImage(image, (void*)Frame->Memory, Frame->XRes, Frame->YRes);

	pImageArray = imaqImageToArray(image, IMAQ_NO_RECT, NULL, NULL);

	if(pImageArray != NULL)
		memcpy((void*)Frame->Memory, pImageArray, Frame->Stride * 4 * Frame->YRes);

	//Test... make a green box in the center of the frame
	size_t CenterY=Frame->YRes / 2;
	size_t CenterX=Frame->XRes / 2;
	size_t LineWidthInBytes=Frame->Stride * 4;
	for (size_t y=CenterY;y<CenterY+10;y++)
	{
		for (size_t x=CenterX; x<CenterX+10; x++)
		{
			*(Frame->Memory+ (x*4 + 0) + (LineWidthInBytes * y))=0;
			*(Frame->Memory+ (x*4 + 1) + (LineWidthInBytes * y))=255;
			*(Frame->Memory+ (x*4 + 2) + (LineWidthInBytes * y))=0;
		}
	}

	imaqDispose(pImageArray);

	imaqDispose(image);
	//imaqDispose(image2);

#else

	//Test... make a green box in the center of the frame
	size_t CenterY=Frame->YRes / 2;
	size_t CenterX=Frame->XRes / 2;
	size_t LineWidthInBytes=Frame->Stride * 4;
	for (size_t y=CenterY;y<CenterY+10;y++)
	{
		for (size_t x=CenterX; x<CenterX+10; x++)
		{
			*(Frame->Memory+ (x*4 + 0) + (LineWidthInBytes * y))=0;
			*(Frame->Memory+ (x*4 + 1) + (LineWidthInBytes * y))=255;
			*(Frame->Memory+ (x*4 + 2) + (LineWidthInBytes * y))=0;
		}
	}
#endif

	return Frame;
}
