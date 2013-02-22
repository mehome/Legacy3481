#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessingBase.h"
#include "VisionGoalTracker.h"

VisionGoalTracker::VisionGoalTracker()
{	
	// hsv - green
	plane1Range.minValue = 75, plane1Range.maxValue = 150,	// Hue	
	plane2Range.minValue = 50, plane2Range.maxValue = 255, // Saturation
	plane3Range.minValue = 50, plane3Range.maxValue = 250;	// Value

	// rgb - green
//  plane1Range.minValue = 0, plane1Range.maxValue = 188,	// red
//	plane2Range.minValue = 163, plane2Range.maxValue = 255, // green
//	plane3Range.minValue = 9, plane3Range.maxValue = 255;	// blue

	particleList.SetParticleParams( 0.8f, 0.8f, 1.4f );	// area threshold, aspect min, max

	// particle filter parameters
	MeasurementType FilterMeasureTypes[] = {IMAQ_MT_BOUNDING_RECT_WIDTH, IMAQ_MT_BOUNDING_RECT_HEIGHT};
	float plower[] = {20, 20};	
	float pUpper[] = {200, 200};
	int pCalibrated[] = {0,0};
	int pExclude[] = {0,0};

	criteriaCount = sizeof(FilterMeasureTypes) / sizeof(FilterMeasureTypes[0]);

	InitParticleFilter(FilterMeasureTypes, plower, pUpper, pCalibrated, pExclude, FALSE, TRUE);
}

VisionGoalTracker::~VisionGoalTracker()
{
}

int VisionGoalTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;

	//-----------------------------------------------------------------//
	//  Color threshold and optional noise filter                      //
	//-----------------------------------------------------------------//

	// color threshold
	if( m_bShowThreshold )
	{
		//	VisionErrChk(imaqColorThreshold(ThresholdImageU8, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_RGB, &plane1Range, &plane2Range, &plane3Range));
		VisionErrChk(imaqColorThreshold(ThresholdImageU8, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_HSV, &plane1Range, &plane2Range, &plane3Range));

		// fill holes
		VisionErrChk(imaqFillHoles(ParticleImageU8, ThresholdImageU8, true));
	}
	else
	{
		//	VisionErrChk(imaqColorThreshold(ParticleImageU8, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_RGB, &plane1Range, &plane2Range, &plane3Range));
		VisionErrChk(imaqColorThreshold(ParticleImageU8, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_HSV, &plane1Range, &plane2Range, &plane3Range));

		// fill holes
		VisionErrChk(imaqFillHoles(ParticleImageU8, ParticleImageU8, true));
	}

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

	// we want the qualifying target highest on the screen.
	// (most valuable target)
	int min_y = SourceImageInfo.yRes + 1;
	int index = 0;

	if( m_bShowOverlays )
	{
		VisionErrChk(GetParticles(ParticleImageU8, TRUE, particleList));

		if( m_bShowThreshold )
		{
			imaqMask(InputImageRGB, InputImageRGB, ThresholdImageU8);
		}

		if(particleList.numParticles > 0)
		{
			if( m_bUseFindCorners )
				VisionErrChk(FindParticleCorners(InputImageRGB, particleList));

			if( m_bUseMasking )
				imaqMask(InputImageRGB, InputImageRGB, ParticleImageU8);	// mask image onto InputImageRGB

			// overlay some useful info
			for(int i = 0; i < particleList.numParticles; i++)
			{
				Point P1;
				Point P2;
				Rect rect;

				// track highest center x
				if( particleList.particleData[i].center.y < min_y )
				{
					min_y = particleList.particleData[i].center.y;
					index = i;
				}

				// write some text to show aiming point 
				Point TextPoint;
				int fu;

				if( m_bShowAimingText )
				{
					TextPoint.x = particleList.particleData[i].center.x;
					TextPoint.y = particleList.particleData[i].center.y + 20;
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

				// bounding box
				rect.top = particleList.particleData[i].bound_top;
				rect.left = particleList.particleData[i].bound_left;
				rect.height = particleList.particleData[i].bound_height;
				rect.width = particleList.particleData[i].bound_width;

				imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );

				// center box
				rect.top = SourceImageInfo.yRes / 2 - 5;
				rect.left = SourceImageInfo.xRes / 2 - 5;
				rect.height = 10;
				rect.width = 10; 

				imaqDrawShapeOnImage(InputImageRGB, InputImageRGB, rect, IMAQ_PAINT_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );
				
				if( m_bUseFindCorners && m_bShowFindCorners )
				{
					// corner points
					for(int j = 0; j < 4; j++)
					{
						P1.x = (int)particleList.particleData[i].Intersections[j].x - 6;
						P1.y = (int)particleList.particleData[i].Intersections[j].y;
						P2.x = (int)particleList.particleData[i].Intersections[j].x + 6;
						P2.y = (int)particleList.particleData[i].Intersections[j].y;

						imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_YELLOW );

						P1.x = (int)particleList.particleData[i].Intersections[j].x;
						P1.y = (int)particleList.particleData[i].Intersections[j].y - 6;
						P2.x = (int)particleList.particleData[i].Intersections[j].x;
						P2.y = (int)particleList.particleData[i].Intersections[j].y + 6;

						imaqDrawLineOnImage(InputImageRGB, InputImageRGB, IMAQ_DRAW_VALUE, P1, P2, COLOR_YELLOW );
					}
				}
			}	// particle loop
		}	// num particles > 0
		else
			success = 0;
	}	// show overlays

Error:
	// Get return for x, y target values;
	if( min_y < SourceImageInfo.yRes + 1 )
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
