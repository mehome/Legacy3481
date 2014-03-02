#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessingBase.h"
#include "VisionBallTracker.h"
#include "../SmartDashboard/SmartDashboard_Import.h"

VisionBallTracker::VisionBallTracker()
{
	m_ThresholdMode = eThreshRGB;

	SetDefaultThreshold();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 

	switch(m_ThresholdMode)
	{
	case eThreshRGB:
		plane1Range = &RedRange;
		plane2Range = &GreenRange;
		plane3Range = &BlueRange;
		break;
	case eThreshHSV:
		plane1Range = &HueRange;
		plane2Range = &SaturationRange;
		plane3Range = &ValueRange;
		break;
	case eThreshLuma:
		plane1Range = &LuminanceRange;
		break;
	}

	SetBallThreshold( m_bBallColor );

	particleList.SetParticleParams( 0.0f, 0.8f, 1.2f, 1.02f );	// area threshold, aspect min, max, circularity max
	FirstPassParticleList.SetParticleParams( 0.0f, 0.0f, 0.0f, 0.0f );	// area threshold, aspect min, max, circularity max

	EnableObjectSeparation( true );
	EnableObjectJoin( true );

	// particle filter parameters
	MeasurementType FilterMeasureTypes[] = {IMAQ_MT_BOUNDING_RECT_WIDTH, IMAQ_MT_BOUNDING_RECT_HEIGHT};
	float plower[] = {90, 90};	
	float pUpper[] = {639, 479};
	int pCalibrated[] = {0,0};
	int pExclude[] = {0,0};

	criteriaCount = sizeof(FilterMeasureTypes) / sizeof(FilterMeasureTypes[0]);

	InitParticleFilter(FilterMeasureTypes, plower, pUpper, pCalibrated, pExclude, FALSE, TRUE);
}

VisionBallTracker::~VisionBallTracker()
{
}

void VisionBallTracker::SetDefaultThreshold( void )
{
	// hsv ????
	HueRange.minValue = 75,			HueRange.maxValue = 150,
	SaturationRange.minValue = 0,	SaturationRange.maxValue = 255,
	ValueRange.minValue = 130,		ValueRange.maxValue = 255;

	// rgb
	RedRange.minValue = 130,	RedRange.maxValue = 255,
	GreenRange.minValue = 130,	GreenRange.maxValue = 255,
	BlueRange.minValue = 130,		BlueRange.maxValue = 255;	

	// luma - I doubt this would be a good choice - added for completeness.
	LuminanceRange.minValue = 143, LuminanceRange.maxValue = 255;
}

void VisionBallTracker::SetBallThreshold(bool bColorSet)
{

}

int VisionBallTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;

	// we want the qualifying target lowest on the screen.
	// (closest target)
	int max_y = 0;
	int index = 0;

	SetBallThreshold( m_bBallColor );

	//-----------------------------------------------------------------//
	//  Threshold                                                      //
	//-----------------------------------------------------------------//

	if( m_DisplayMode == eThreshold )
		DestinationThresImage = ThresholdImageU8;
	else
		DestinationThresImage = ParticleImageU8;

	switch (m_ThresholdMode)
	{
		case eThreshRGB:
			VisionErrChk(imaqColorThreshold(DestinationThresImage, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_RGB, plane1Range, plane2Range, plane3Range));
			break;
		case eThreshHSV:
			VisionErrChk(imaqColorThreshold(DestinationThresImage, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_HSV, plane1Range, plane2Range, plane3Range));
			break;
		case eThreshLuma:
			// Extracts the luminance plane
			VisionErrChk(imaqExtractColorPlanes(InputImageRGB, IMAQ_HSL, NULL, NULL, DestinationThresImage));
			// Thresholds the image.
			VisionErrChk(imaqThreshold(DestinationThresImage, DestinationThresImage, (float)plane1Range->minValue, (float)plane1Range->maxValue, TRUE, THRESHOLD_IMAGE_REPLACE_VALUE));
			break;
	}

	// Fills holes in particles.
	VisionErrChk(imaqFillHoles(ParticleImageU8, DestinationThresImage, TRUE));

	//-------------------------------------------------------------------//
	//                Advanced Morphology: Remove Objects                //
	//-------------------------------------------------------------------//

	int pKernel[] = {1,1,1,
				     1,1,1,
					 1,1,1};	// 3x3 kernel
	StructuringElement structElem;
	structElem.matrixCols = 3;
	structElem.matrixRows = 3;
	structElem.hexa = FALSE;
	structElem.kernel = pKernel;

	int erosions = 4;

	// Filters particles based on their size.
	VisionErrChk(imaqSizeFilter(ParticleImageU8, ParticleImageU8, FALSE, erosions, IMAQ_KEEP_LARGE, &structElem));

	// this process breaks objects apart at narrow borders
	if( m_bObjectSeparation )
	{
		//-------------------------------------------------------------------//
		//                  Advanced Morphology: Danielsson                  //
		//-------------------------------------------------------------------//

		// Creates a very accurate distance map based on the Danielsson distance algorithm.
		VisionErrChk(imaqDanielssonDistance(WorkImageU8, ParticleImageU8));

		//-------------------------------------------------------------------//
		//                       Lookup Table: Equalize                      //
		//-------------------------------------------------------------------//
		// Calculates the histogram of the image and redistributes pixel values across
		// the desired range to maintain the same pixel value distribution.
		VisionErrChk(imaqEqualize(WorkImageU8, WorkImageU8, 0, 255, NULL));

		//-------------------------------------------------------------------//
		//                             Watershed                             //
		//-------------------------------------------------------------------//
		int zoneCount;
		VisionErrChk(imaqWatershedTransform(WorkImageU8, WorkImageU8, TRUE, &zoneCount));


		//-------------------------------------------------------------------//
		//                          Basic Morphology                         //
		//-------------------------------------------------------------------//

		// Sets the structuring element.
		int pKernel1[9] = {0,1,0,
						   1,1,1,
						   0,1,0};
		StructuringElement structElem1;
		structElem1.matrixCols = 3;
		structElem1.matrixRows = 3;
		structElem1.hexa = FALSE;
		structElem1.kernel = pKernel1;

		// Applies multiple morphological transformation to the binary image.
		for (int i = 0 ; i < 3 ; i++)
		{
			VisionErrChk(imaqMorphology(WorkImageU8, WorkImageU8, IMAQ_ERODE, &structElem1));
		}

		//-------------------------------------------------------------------//
		//                       Operators: Mask Image                       //
		//-------------------------------------------------------------------//

		// Masks the image
		VisionErrChk(imaqMask(ParticleImageU8, ParticleImageU8, WorkImageU8));
	}

	int numParticles = 0;

	// Filters particles based on their morphological measurements.
	// gets rid of smaller objects
	VisionErrChk(imaqParticleFilter4(ParticleImageU8, ParticleImageU8, particleCriteria, criteriaCount, &particleFilterOptions, NULL, &numParticles));

	if( m_bJoinObjects )
	{
		// this will help to identify a circular object
		VisionErrChk(imaqConvexHull(WorkImageU8, ParticleImageU8, FALSE));	// Connectivity 4??? set to true to make con 8.

		// get bounding boxes for these before we dilate.
		// so we can get the correct size of the ball.
		VisionErrChk(GetParticles(WorkImageU8, TRUE, FirstPassParticleList));

		// Erode the image to help remove noise
		int pKernel2[25] = {0,1,1,1,0,
			1,1,1,1,1,
			1,1,1,1,1,
			1,1,1,1,1,
			0,1,1,1,0};
		structElem.matrixCols = 5;
		structElem.matrixRows = 5;
		structElem.hexa = FALSE;
		structElem.kernel = pKernel2;

		// Applies multiple morphological transformation to the binary image.
		for (int i = 0 ; i < 3 ; i++)
		{
			VisionErrChk(imaqMorphology(ParticleImageU8, ParticleImageU8, IMAQ_ERODE, &structElem));
		}

		// Now dialate it a bunch of times using the same kernal. hopefully, the images will merge.
		for (int i = 0 ; i < 16 ; i++)
		{
			VisionErrChk(imaqMorphology(ParticleImageU8, ParticleImageU8, IMAQ_DILATE, &structElem));
		}
	}

	VisionErrChk(imaqConvexHull(ParticleImageU8, ParticleImageU8, FALSE));	// Connectivity 4??? set to true to make con 8.

	// we retest for a possibly bisected ball.
	VisionErrChk(GetParticles(ParticleImageU8, TRUE, particleList));

	if( m_DisplayMode == eThreshold )
	{
		imaqMask(InputImageRGB, InputImageRGB, ThresholdImageU8);
	}

	if(particleList.numParticles > 0)
	{
		if( m_DisplayMode == eMasked )
		{
			if( m_bShowSolidMask )
			{
				PixelValue px_val;
				px_val.rgb = IMAQ_RGB_YELLOW;
				imaqFillImage(InputImageRGB, px_val, ParticleImageU8);
			}
			imaqMask(InputImageRGB, InputImageRGB, ParticleImageU8);	// mask image onto InputImageRGB
		}

		Rect rect;

		if( m_bShowBoundsText )
		{
			for(int i = 0; i < FirstPassParticleList.numParticles; i++)
			{
				// bounding box
				rect.top = FirstPassParticleList.particleData[i].bound_top;
				rect.left = FirstPassParticleList.particleData[i].bound_left;
				rect.height = FirstPassParticleList.particleData[i].bound_height;
				rect.width = FirstPassParticleList.particleData[i].bound_width;

				imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_WHITE );
			}
		}

		// overlay some useful info
		for(int i = 0; i < particleList.numParticles; i++)
		{
			Point P1;
			Point P2;

			// track lowest center y (image coords, top is zero.)
			if( particleList.particleData[i].center.y > max_y &&
				particleList.particleData[i].status == eOK )
			{
				max_y = particleList.particleData[i].center.y;
				index = i;
			}

			if( m_bShowOverlays )
			{
				if( particleList.particleData[i].status == eOK )
				{
					// write some text to show aiming point 
					Point TextPoint;
					int fu;

					TextPoint.x = particleList.particleData[i].center.x;
					TextPoint.y = particleList.particleData[i].center.y + 50;

					if( m_bShowAimingText )
					{
						sprintf_s(TextBuffer, 256, "%f, %f", particleList.particleData[i].AimSys.x, particleList.particleData[i].AimSys.y);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 
						TextPoint.y += 16;
					}

					if( m_bShowBoundsText )
					{
						// show size of bounding box
						sprintf_s(TextBuffer, 256, "%d, %d", particleList.particleData[i].bound_width, particleList.particleData[i].bound_height);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

						// center x, y
						TextPoint.y += 16;
						sprintf_s(TextBuffer, 256, "%d, %d", particleList.particleData[i].center.x, particleList.particleData[i].center.y);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

						// aspect (don't have info for area ratio...)
						TextPoint.y += 16;
						sprintf_s(TextBuffer, 256, "%f", (float)particleList.particleData[i].bound_width / (float)particleList.particleData[i].bound_height); 
														//	 ,(float)particleList.particleData[i].bound_width * (float)particleList.particleData[i].bound_height);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

					}

					// draw a line from target CoM to center of screen
					P1.x = particleList.particleData[i].center.x;
					P1.y = particleList.particleData[i].center.y;
					P2.x = SourceImageInfo.xRes / 2;
					P2.y = SourceImageInfo.yRes / 2;
					imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_RED );

					// small crosshair at center of mass
					P1.x = particleList.particleData[i].center.x - 6;
					P1.y = particleList.particleData[i].center.y;
					P2.x = particleList.particleData[i].center.x + 6;
					P2.y = particleList.particleData[i].center.y;

					imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

					P1.x = particleList.particleData[i].center.x;
					P1.y = particleList.particleData[i].center.y - 6;
					P2.x = particleList.particleData[i].center.x;
					P2.y = particleList.particleData[i].center.y + 6;

					imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );
				}

				// bounding box
				rect.top = particleList.particleData[i].bound_top;
				rect.left = particleList.particleData[i].bound_left;
				rect.height = particleList.particleData[i].bound_height;
				rect.width = particleList.particleData[i].bound_width;

				if( particleList.particleData[i].status == eOK )
				{
					imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );
				}
				else
				{
					if( m_bShowBoundsText )
					{
						float BBoxColor = COLOR_GREEN;
						if( particleList.particleData[i].status == eAspectFail )
							BBoxColor = COLOR_RED;
						else if( particleList.particleData[i].status == eAreaFail)
							BBoxColor = COLOR_CYAN;
						else if( particleList.particleData[i].status == eCircularityFail)
							BBoxColor = COLOR_YELLOW;
						imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, BBoxColor );
					}
				}
			}	// show overlays

			if( particleList.particleData[i].status == eOK )
			{
				int ballSize_top = 999999;
				int ballSize_left = 999999;
				int ballSize_bottom = 0;
				int ballSize_right = 0;

				// find the actual size
				for(int x = 0; x < FirstPassParticleList.numParticles; x++)
				{	// find the first pass items within the bounds of this object
					if( (FirstPassParticleList.particleData[x].bound_top >= particleList.particleData[i].bound_top) &&
						(FirstPassParticleList.particleData[x].bound_left >= particleList.particleData[i].bound_left) &&
						(FirstPassParticleList.particleData[x].bound_bottom <= particleList.particleData[i].bound_bottom) &&
						(FirstPassParticleList.particleData[x].bound_right <= particleList.particleData[i].bound_right) )
					{
						if( FirstPassParticleList.particleData[x].bound_top < ballSize_top )
							ballSize_top = FirstPassParticleList.particleData[x].bound_top;
						if( FirstPassParticleList.particleData[x].bound_left < ballSize_left )
							ballSize_left = FirstPassParticleList.particleData[x].bound_left;
						if( FirstPassParticleList.particleData[x].bound_bottom > ballSize_bottom )
							ballSize_bottom = FirstPassParticleList.particleData[x].bound_bottom;
						if( FirstPassParticleList.particleData[x].bound_right > ballSize_right )
							ballSize_right = FirstPassParticleList.particleData[x].bound_right;
					}
				}

				if(ballSize_top != 999999 && ballSize_left != 999999)
				{
					rect.top = ballSize_top;
					rect.left = ballSize_left;
					rect.height = ballSize_bottom - ballSize_top;
					rect.width = ballSize_right - ballSize_left;

					if(m_bShowOverlays)
						imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );

					// calc distance
					int TargetHeight = 25;	

					// TODO: recalibrate
					// Angle = arctan(vertical hight in feet * image height / (2 * vertical target hight in pixels * distance in feet)) * RADS_TO_DEG
					// vertical hight is 32 in - so 2.66 ft.  So my test was: arctan(2.66 * 480 / (2 * 121 * 6.83)  (actually, my test was half scale, so hight was 1.33)
#define VIEW_ANGLE 33

					double Distance = SourceImageInfo.yRes * TargetHeight / (rect.height * 12 * 2 * tan(VIEW_ANGLE * M_PI/(180*2)));

					SmartDashboard::PutNumber("TargetDistance", Distance);

					if( m_bShowBoundsText )
					{
						Point TextPoint;
						int fu;

						TextPoint.x = 40;
						TextPoint.y = 40;

						// show size of bounding box
						sprintf_s(TextBuffer, 256, "  Distance: %.2f", Distance);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 
					}	
				}
			}
		}	// particle loop

		// center box
		rect.top = SourceImageInfo.yRes / 2 - 5;
		rect.left = SourceImageInfo.xRes / 2 - 5;
		rect.height = 10;
		rect.width = 10; 

		imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_PAINT_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );
	}	// num particles > 0
	else
		success = 0;

Error:
	// Get return for x, y target values;
	if( max_y > 0 )
	{
		x_target = (double)particleList.particleData[index].AimSys.x;
		y_target = (double)particleList.particleData[index].AimSys.y;
	}
	else
	{
		x_target = 0.0;
		y_target = 0.0;
	}

	int error = imaqGetLastError();
	return success;
}

