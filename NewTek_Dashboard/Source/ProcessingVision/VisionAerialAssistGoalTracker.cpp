#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessingBase.h"
#include "VisionAerialAssistGoalTracker.h"
#include "../SmartDashboard/SmartDashboard_Import.h"

#define TAPE_WIDTH_LIMIT 50
#define VERTICAL_SCORE_LIMIT 50
#define LR_SCORE_LIMIT 50

#define IS_LEFT 1
#define IS_RIGHT 2

#undef _USE_FOR_SIM_

VisionAerialAssistGoalTracker::VisionAerialAssistGoalTracker()
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

VisionAerialAssistGoalTracker::~VisionAerialAssistGoalTracker()
{
}

void VisionAerialAssistGoalTracker::SetDefaultThreshold( void )
{
	// hsv - green
	HueRange.minValue = 75,			HueRange.maxValue = 150,
	SaturationRange.minValue = 50,	SaturationRange.maxValue = 255,
	ValueRange.minValue = 50,		ValueRange.maxValue = 250;

	// rgb - green
	RedRange.minValue = 0,		RedRange.maxValue = 188,
	GreenRange.minValue = 163,	GreenRange.maxValue = 255,
	BlueRange.minValue = 9,		BlueRange.maxValue = 255;	

	// luma - I doubt this would be a good choice - added for completeness.
	LuminanceRange.minValue = 50, LuminanceRange.maxValue = 250;
}


int VisionAerialAssistGoalTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;

	particleListVert.SetParticleParams( 0.0f, 0.0f, 0.0f, (4.0f/32), 0.0f);	// area threshold, aspect min, max
	particleListHorz.SetParticleParams( 0.0f, 0.0f, 0.0f,  0.0f,     (23.5f/4) );	// area threshold, aspect min, max

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

	VisionErrChk(GetParticles(ParticleImageU8, TRUE, particleListVert));
	VisionErrChk(GetParticles(ParticleImageU8, TRUE, particleListHorz));

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

	target.totalScore = target.leftScore = target.rightScore = target.tapeWidthScore = target.verticalScore = 0;
	target.verticalIndex = 0;

	Rect rect;
	ParticleData HotTarget;
	Point P1;
	Point P2;

	if(particleListVert.numParticles > 0)
	{
		for(int i = 0; i < particleListVert.numParticles; i++)
		{
			for(int j = 0; j < particleListHorz.numParticles; j++ )
			{
				double horizWidth, horizHeight, vertWidth, leftScore, rightScore, tapeWidthScore, verticalScore, total;

				horizWidth = particleListHorz.particleData[j].eq_rect_long_side;
				vertWidth = particleListVert.particleData[i].eq_rect_short_side;
				horizHeight = particleListHorz.particleData[j].eq_rect_short_side;

				leftScore = ratioToScore(1.2 * (particleListVert.particleData[i].bound_right - particleListHorz.particleData[j].center.x)/horizWidth);
				rightScore = ratioToScore(1.2 * (particleListHorz.particleData[j].center.x - particleListVert.particleData[i].bound_left)/horizWidth);
				tapeWidthScore = ratioToScore(vertWidth/horizHeight);
				verticalScore = ratioToScore(1-(particleListVert.particleData[i].bound_top - particleListHorz.particleData[j].center.y)/(4*horizHeight));
				total = leftScore > rightScore ? leftScore:rightScore;
				total += tapeWidthScore + verticalScore;

				// if the target is the best so far store the infomation.
				if( total > target.totalScore )
				{
					target.horizontalIndex = j;
					target.verticalIndex = i;
					target.totalScore = total;
					target.leftScore = leftScore;
					target.rightScore = rightScore;
					target.tapeWidthScore = tapeWidthScore;
					target.verticalScore = verticalScore;
				}

				// overlay some useful info
				if( m_bShowOverlays )
				{
					if( particleListVert.particleData[i].status == eOK )
					{
						// write some text to show aiming point 
						Point TextPoint;
						int fu;

						TextPoint.x = particleListVert.particleData[i].center.x;
						TextPoint.y = particleListVert.particleData[i].center.y + 20;

						if( m_bShowBoundsText )
						{
							// show size of bounding box
							sprintf_s(TextBuffer, 256, "%d, %d", particleListVert.particleData[i].bound_width, particleListVert.particleData[i].bound_height);
							imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

							// center x, y
							TextPoint.y += 16;
							sprintf_s(TextBuffer, 256, "%d, %d", particleListVert.particleData[i].center.x, particleListVert.particleData[i].center.y);
							imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

						}

						// small crosshair at center of mass
						P1.x = particleListVert.particleData[i].center.x - 6;
						P1.y = particleListVert.particleData[i].center.y;
						P2.x = particleListVert.particleData[i].center.x + 6;
						P2.y = particleListVert.particleData[i].center.y;

						imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

						P1.x = particleListVert.particleData[i].center.x;
						P1.y = particleListVert.particleData[i].center.y - 6;
						P2.x = particleListVert.particleData[i].center.x;
						P2.y = particleListVert.particleData[i].center.y + 6;

						imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );
					}

					// bounding box
					rect.top = particleListVert.particleData[i].bound_top;
					rect.left = particleListVert.particleData[i].bound_left;
					rect.height = particleListVert.particleData[i].bound_height;
					rect.width = particleListVert.particleData[i].bound_width;

					if( particleListVert.particleData[i].status == eOK )
						imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );
					else
					{
						if( m_bShowBoundsText )
						{
							float BBoxColor = COLOR_GREEN;
							if( particleListVert.particleData[i].status == eAspectFail )
								BBoxColor = COLOR_RED;
							else if( particleListVert.particleData[i].status == eAreaFail)
								BBoxColor = COLOR_CYAN;
							imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, BBoxColor);
						}
						continue;
					}

					if( particleListHorz.particleData[j].status == eOK )
					{
						// write some text to show aiming point 
						Point TextPoint;
						int fu;

						TextPoint.x = particleListHorz.particleData[j].center.x;
						TextPoint.y = particleListHorz.particleData[j].center.y + 20;

						if( m_bShowBoundsText )
						{
							// show size of bounding box
							sprintf_s(TextBuffer, 256, "%d, %d", particleListHorz.particleData[j].bound_width, particleListHorz.particleData[j].bound_height);
							imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

							// center x, y
							TextPoint.y += 16;
							sprintf_s(TextBuffer, 256, "%d, %d", particleListHorz.particleData[j].center.x, particleListHorz.particleData[j].center.y);
							imaqDrawTextOnImage(InputImageRGB, InputImageRGB, TextPoint, TextBuffer, &textOps, &fu); 

						}

						// small crosshair at center of mass
						P1.x = particleListHorz.particleData[j].center.x - 6;
						P1.y = particleListHorz.particleData[j].center.y;
						P2.x = particleListHorz.particleData[j].center.x + 6;
						P2.y = particleListHorz.particleData[j].center.y;

						imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

						P1.x = particleListHorz.particleData[j].center.x;
						P1.y = particleListHorz.particleData[j].center.y - 6;
						P2.x = particleListHorz.particleData[j].center.x;
						P2.y = particleListHorz.particleData[j].center.y + 6;

						imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );
					}

					// bounding box
					rect.top = particleListHorz.particleData[j].bound_top;
					rect.left = particleListHorz.particleData[j].bound_left;
					rect.height = particleListHorz.particleData[j].bound_height;
					rect.width = particleListHorz.particleData[j].bound_width;

					if( particleListHorz.particleData[j].status == eOK )
						imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );
					else
					{
						if( m_bShowBoundsText )
						{
							float BBoxColor = COLOR_GREEN;
							if( particleListHorz.particleData[j].status == eAspectFail )
								BBoxColor = COLOR_RED;
							else if( particleListHorz.particleData[j].status == eAreaFail)
								BBoxColor = COLOR_CYAN;
							imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, BBoxColor);
						}
						continue;
					}

				}	// show overlays
			}
		}
		// determine if target is hot
		target.Hot = hotOrNot(target);

		SmartDashboard::PutNumber("TargetHot",(double)target.Hot);

		// make an aiming box for hot targets
		if(target.Hot == IS_LEFT)
		{
			HotTarget.bound_left = particleListHorz.particleData[target.horizontalIndex].bound_left;
			HotTarget.bound_top = particleListHorz.particleData[target.horizontalIndex].bound_top;
			HotTarget.bound_right = particleListVert.particleData[target.verticalIndex].bound_right;
			HotTarget.bound_bottom = particleListVert.particleData[target.verticalIndex].bound_bottom;
			HotTarget.bound_width = HotTarget.bound_right - HotTarget.bound_left;
			HotTarget.bound_height = HotTarget.bound_bottom - HotTarget.bound_top;
			HotTarget.center.x = HotTarget.bound_left + HotTarget.bound_width/2;
			HotTarget.center.y = HotTarget.bound_top + HotTarget.bound_height/2;
			// convert to aiming system coords
			HotTarget.AimSys.x = (float)((HotTarget.center.x - (SourceImageInfo.xRes/2.0)) / (SourceImageInfo.xRes/2.0)) * (SourceImageInfo.xRes / SourceImageInfo.yRes);
			HotTarget.AimSys.y = (float)((HotTarget.center.y - (SourceImageInfo.yRes/2.0)) / (SourceImageInfo.yRes/2.0));
		}
		else if(target.Hot == IS_RIGHT)
		{
			HotTarget.bound_left = particleListVert.particleData[target.verticalIndex].bound_left;
			HotTarget.bound_top = particleListHorz.particleData[target.horizontalIndex].bound_top;
			HotTarget.bound_right = particleListHorz.particleData[target.horizontalIndex].bound_right;
			HotTarget.bound_bottom = particleListVert.particleData[target.verticalIndex].bound_bottom;
			HotTarget.bound_width = HotTarget.bound_right - HotTarget.bound_left;
			HotTarget.bound_height = HotTarget.bound_bottom - HotTarget.bound_top;
			HotTarget.center.x = HotTarget.bound_left + HotTarget.bound_width/2;
			HotTarget.center.y = HotTarget.bound_top + HotTarget.bound_height/2;
			// convert to aiming system coords
			HotTarget.AimSys.x = (float)((HotTarget.center.x - (SourceImageInfo.xRes/2.0)) / (SourceImageInfo.xRes/2.0)) * (SourceImageInfo.xRes / SourceImageInfo.yRes);
			HotTarget.AimSys.y = (float)((HotTarget.center.y - (SourceImageInfo.yRes/2.0)) / (SourceImageInfo.yRes/2.0));
		}

		// calc distance
		double RectLong = particleListVert.particleData[target.verticalIndex].eq_rect_long_side;
		double height = (double)particleListVert.particleData[target.verticalIndex].bound_height;
		height = min(height, RectLong);
		int TargetHeight = 32;	

		// TODO: recalibrate
		// Angle = arctan(vertical hight in feet * image height / (2 * vertical target hight in pixels * distance in feet)) * RADS_TO_DEG
		// vertical hight is 32 in - so 2.66 ft.  So my test was: arctan(2.66 * 480 / (2 * 121 * 6.83)  (actually, my test was half scale, so hight was 1.33)
#define VIEW_ANGLE 42.25

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

		if(m_bShowOverlays)
		{
			if(target.Hot)
			{
				// bounding box
				rect.top = HotTarget.bound_top;
				rect.left = HotTarget.bound_left;
				rect.height = HotTarget.bound_height;
				rect.width = HotTarget.bound_width;

				imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_YELLOW );

				// draw a line from target CoM to center of screen
				P1.x = HotTarget.center.x;
				P1.y = HotTarget.center.y;
				P2.x = SourceImageInfo.xRes / 2;
				P2.y = SourceImageInfo.yRes / 2;
				imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_RED );

				// small crosshair at center of mass
				P1.x = HotTarget.center.x - 6;
				P1.y = HotTarget.center.y;
				P2.x = HotTarget.center.x + 6;
				P2.y = HotTarget.center.y;

				imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

				P1.x = HotTarget.center.x;
				P1.y = HotTarget.center.y - 6;
				P2.x = HotTarget.center.x;
				P2.y = HotTarget.center.y + 6;

				imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );
			}
			else
			{
				// bounding box
				rect.top = particleListVert.particleData[target.verticalIndex].bound_top;
				rect.left = particleListVert.particleData[target.verticalIndex].bound_left;
				rect.height = particleListVert.particleData[target.verticalIndex].bound_height;
				rect.width = particleListVert.particleData[target.verticalIndex].bound_width;

				imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );

				// draw a line from target CoM to center of screen
				P1.x = particleListVert.particleData[target.verticalIndex].center.x;
				P1.y = particleListVert.particleData[target.verticalIndex].center.y;
				P2.x = SourceImageInfo.xRes / 2;
				P2.y = SourceImageInfo.yRes / 2;
				imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_RED );

				// small crosshair at center of mass
				P1.x = particleListVert.particleData[target.verticalIndex].center.x - 6;
				P1.y = particleListVert.particleData[target.verticalIndex].center.y;
				P2.x = particleListVert.particleData[target.verticalIndex].center.x + 6;
				P2.y = particleListVert.particleData[target.verticalIndex].center.y;

				imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

				P1.x = particleListVert.particleData[target.verticalIndex].center.x;
				P1.y = particleListVert.particleData[target.verticalIndex].center.y - 6;
				P2.x = particleListVert.particleData[target.verticalIndex].center.x;
				P2.y = particleListVert.particleData[target.verticalIndex].center.y + 6;

				imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );
			}
		}

		// center box
		rect.top = SourceImageInfo.yRes / 2 - 5;
		rect.left = SourceImageInfo.xRes / 2 - 5;
		rect.height = 10;
		rect.width = 10; 

		float Color = target.Hot ? COLOR_GREEN : COLOR_RED;
		imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_PAINT_VALUE, IMAQ_SHAPE_RECT, Color );

	}	// num particles > 0
	else
		success = 0;

Error:
	// Get return for x, y target values;
	if( target.totalScore > 0 )
	{
		if ( target.Hot )
		{
			x_target = (double)HotTarget.AimSys.x;
			y_target = (double)HotTarget.AimSys.y;
		}
		else
		{
			// if not hot, center on the vertical target
			x_target = (double)particleListVert.particleData[target.verticalIndex].AimSys.x;
			y_target = (double)particleListVert.particleData[target.verticalIndex].AimSys.y;
		}
	}
	else
	{
		x_target = 0.0;
		y_target = 0.0;
	}

	int error = imaqGetLastError();
	return success;
}

int VisionAerialAssistGoalTracker::hotOrNot(TargetReport target)
{
	bool isHot = true;

	isHot &= target.tapeWidthScore >= TAPE_WIDTH_LIMIT;
	isHot &= target.verticalScore >= VERTICAL_SCORE_LIMIT;
	isHot &= (target.leftScore > LR_SCORE_LIMIT) | (target.rightScore > LR_SCORE_LIMIT);

	if( isHot )
		return target.leftScore > target.rightScore ? IS_LEFT : IS_RIGHT;
	else
		return 0;
}