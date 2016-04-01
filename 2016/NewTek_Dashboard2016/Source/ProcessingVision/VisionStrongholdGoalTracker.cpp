#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessingBase.h"
#include "VisionStrongholdGoalTracker.h"
#include "../SmartDashboard/SmartDashboard_Import.h"

#define TAPE_WIDTH_LIMIT 50
#define VERTICAL_SCORE_LIMIT 50
#define LR_SCORE_LIMIT 50

#define IS_LEFT 1
#define IS_RIGHT 2

#undef _USE_FOR_SIM_

VisionStrongholdGoalTracker::VisionStrongholdGoalTracker()
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

	SetRejectBorderObjs(false);
	SetUseConvexHull(false);

#ifdef _USE_FOR_SIM_
	SetUseConvexHull(true);
#endif
	// particle filter parameters
	MeasurementType FilterMeasureTypes[] = {IMAQ_MT_AREA};
	float plower[] = {150};	
	float pUpper[] = {65535};
	int pCalibrated[] = {0};
	int pExclude[] = {0};

	criteriaCount = sizeof(FilterMeasureTypes) / sizeof(FilterMeasureTypes[0]);

	InitParticleFilter(FilterMeasureTypes, plower, pUpper, pCalibrated, pExclude, FALSE, TRUE);
}

VisionStrongholdGoalTracker::~VisionStrongholdGoalTracker()
{
}

void VisionStrongholdGoalTracker::SetDefaultThreshold( void )
{
	// hsv - green
	HueRange.minValue = 75,			HueRange.maxValue = 150,
	SaturationRange.minValue = 50,	SaturationRange.maxValue = 255,
	ValueRange.minValue = 50,		ValueRange.maxValue = 250;

	// rgb - green
	RedRange.minValue = 99,	RedRange.maxValue = 255,
	GreenRange.minValue = 191,	GreenRange.maxValue = 255,
	BlueRange.minValue = 193,	BlueRange.maxValue = 255;	

	// luma - I doubt this would be a good choice - added for completeness.
	LuminanceRange.minValue = 50, LuminanceRange.maxValue = 250;
}


int VisionStrongholdGoalTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;

	particleList.SetParticleParamsThresh( 0.255f, 0.385f, (12.0f/12), (20.0f/10.75) );	// area threshold min, max, aspect min, max

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


	// filter small particles
	int pKernel[] = {1,1,1,
					 1,1,1,
					 1,1,1};	// 3x3 kernel 
	StructuringElement structElem;
	structElem.matrixCols = 3;
	structElem.matrixRows = 3;
	structElem.hexa = TRUE;
	structElem.kernel = pKernel;

	int erosions = 2;

	// Filters particles based on their size.
	VisionErrChk(imaqSizeFilter(ParticleImageU8, ParticleImageU8, TRUE, erosions, IMAQ_KEEP_LARGE, &structElem));

	// Eliminates particles touching the border of the image.
	if( m_bRejectBorderParticles )
		VisionErrChk(imaqRejectBorder(ParticleImageU8, ParticleImageU8, TRUE));

	int numParticles = 0;

	// Filters particles based on their morphological measurements.
	VisionErrChk(imaqParticleFilter4(ParticleImageU8, ParticleImageU8, particleCriteria, criteriaCount, &particleFilterOptions, NULL, &numParticles));

	// Computes the convex envelope for each labeled particle in the source image.
	if( m_bUseConvexHull )
		VisionErrChk(imaqConvexHull(ParticleImageU8, ParticleImageU8, FALSE));	// Connectivity 4??? set to true to make con 8.

	VisionErrChk(GetParticles(ParticleImageU8, TRUE, particleList));

	if( m_DisplayMode == eThreshold )
	{
		imaqMask(InputImageRGB, InputImageRGB, ThresholdImageU8);
	}

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
	Point P1;
	Point P2;

	int index = -1;

	if(particleList.numParticles > 0)
	{
		for(int i = 0; i < particleList.numParticles; i++)
		{
			// overlay some useful info
			if( m_bShowOverlays )
			{
				if( particleList.particleData[i].status == eOK )
				{
					// write some text to show aiming point 
					Point TextPoint;
					int fu;

					TextPoint.x = particleList.particleData[i].center.x;
					TextPoint.y = particleList.particleData[i].center.y + 20;

					if( m_bShowBoundsText )
					{
						// show size of bounding box
						sprintf_s(TextBuffer, 256, "%d, %d", particleList.particleData[i].bound_width, particleList.particleData[i].bound_height);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

						// center x, y
						TextPoint.y += 16;
						sprintf_s(TextBuffer, 256, "%d, %d", particleList.particleData[i].center.x, particleList.particleData[i].center.y);
						imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

					}

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
					imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );
				else
				{
					if( m_bShowBoundsText )
					{
						float BBoxColor = COLOR_GREEN;
						if( particleList.particleData[i].status == eAspectFail )
							BBoxColor = COLOR_RED;
						else if( particleList.particleData[i].status == eAreaFail)
							BBoxColor = COLOR_CYAN;
						imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, BBoxColor);
					}
					continue;
				}
			}	// show overlays

			if( particleList.particleData[i].status == eOK )
			{
				index = i;

				// calc distance
				double RectLong = particleList.particleData[i].eq_rect_long_side;
				double height = (double)particleList.particleData[i].bound_height;
				height = min(height, RectLong);
				int TargetHeight = 12;	

				// TODO: recalibrate
				// Angle = arctan(vertical hight in feet * image height / (2 * vertical target hight in pixels * distance in feet)) * RADS_TO_DEG
				// vertical hight is 32 in - so 2.66 ft.  So my test was: arctan(2.66 * 480 / (2 * 121 * 6.83)  (actually, my test was half scale, so hight was 1.33)
//#define VIEW_ANGLE 42.25
#define VIEW_ANGLE 60

				double Distance = SourceImageInfo.yRes * TargetHeight / (height * 12 * 2 * tan(VIEW_ANGLE * M_PI/(180*2)));

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

		// center box
		rect.top = SourceImageInfo.yRes / 2 - 5;
		rect.left = SourceImageInfo.xRes / 2 - 5;
		rect.height = 10;
		rect.width = 10; 

		float Color = (index != -1) ? COLOR_GREEN : COLOR_RED;
		imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_PAINT_VALUE, IMAQ_SHAPE_RECT, Color );

		if( index != -1 )
		{
			// draw a line from target CoM to center of screen
			P1.x = particleList.particleData[index].center.x;
			P1.y = particleList.particleData[index].center.y;
			P2.x = SourceImageInfo.xRes / 2;
			P2.y = SourceImageInfo.yRes / 2;
			imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_RED );
		}
	}	// num particles > 0
	else
		success = 0;

Error:
	// Get return for x, y target values;
	if( index != -1 )
	{
		// if not hot, center on the vertical target
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

