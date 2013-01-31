#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessing.h"

extern VisionTracker* g_pTracker;

Bitmap_Frame *NI_VisionProcessing(Bitmap_Frame *Frame, double &x_target, double &y_target)
{
	if( g_pTracker == NULL )
	{
		g_pTracker = new VisionTracker();
		if( g_pTracker == NULL)
			return Frame;
	}

	// quick tweaks 
	g_pTracker->SetUseMasking(false);

	g_pTracker->Profiler.start();

	g_pTracker->GetFrame(Frame);

	// do the actual processing
	g_pTracker->ProcessImage(x_target, y_target);

	// Return our processed image back to our outgoing frame.
	g_pTracker->ReturnFrame(Frame);

	g_pTracker->Profiler.stop();
	g_pTracker->Profiler.display(L"vision:");

	return Frame;
}

VisionTracker::VisionTracker()
	: criteriaCount( 0 ), particleCriteria( NULL ),
	  m_bUseMasking( false ), m_bShowOverlays( true ),
	  m_bShowAimingText( true ), m_bShowBoundsText( false ),
	  m_bRejectBorderParticles( true ), m_bUseConvexHull( false ),
	  m_bUseFindCorners( false ), m_bShowFindCorners( false )
{	
	plane1Range.minValue = 100, plane1Range.maxValue = 200,	// red	- These values are grey - used in our sample video.
	plane2Range.minValue = 100, plane2Range.maxValue = 210, // green
	plane3Range.minValue = 100, plane3Range.maxValue = 210;	// blue

	Profiler = new profile;
	InputImageRGB = imaqCreateImage(IMAQ_IMAGE_RGB, IMAGE_BORDER_SIZE);
	ParticleImageU8 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);

	// separate planes (for noise filter)
	Plane1 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);
	Plane2 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);
	Plane3 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);

	// particle filter parameters
	MeasurementType FilterMeasureTypes[] = {IMAQ_MT_BOUNDING_RECT_WIDTH, IMAQ_MT_BOUNDING_RECT_HEIGHT};
	float plower[] = {20, 20};	
	float pUpper[] = {200, 200};
	int pCalibrated[] = {0,0};
	int pExclude[] = {0,0};

	criteriaCount = sizeof(FilterMeasureTypes) / sizeof(FilterMeasureTypes[0]);

	InitParticleFilter(FilterMeasureTypes, plower, pUpper, pCalibrated, pExclude, FALSE, TRUE);

	// text drawing setup
	strcpy_s(textOps.fontName, 32, "Arial");
	textOps.fontSize = 12;
	textOps.bold = false;
	textOps.italic = false;
	textOps.underline = false;
	textOps.strikeout = false;
	textOps.textAlignment = IMAQ_CENTER;
	textOps.fontColor = IMAQ_WHITE;

	// Edge / corner finding
	roi = imaqCreateROI();

	// Edge finding options
	edgeOptions.polarity = IMAQ_SEARCH_FOR_RISING_EDGES;
	edgeOptions.kernelSize = 3;
	edgeOptions.width = 1;
	edgeOptions.minThreshold = 25;
	edgeOptions.interpolationType = IMAQ_BILINEAR_FIXED;
	edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;

	findEdgeOptions.showSearchArea = TRUE;
	findEdgeOptions.showSearchLines = TRUE;
	findEdgeOptions.showEdgesFound = TRUE;
	findEdgeOptions.showResult = TRUE;
	findEdgeOptions.searchAreaColor = IMAQ_RGB_GREEN;
	findEdgeOptions.searchLinesColor = IMAQ_RGB_BLUE;
	findEdgeOptions.searchEdgesColor = IMAQ_RGB_YELLOW;
	findEdgeOptions.resultColor = IMAQ_RGB_RED;
	findEdgeOptions.overlayGroupName = NULL;
	findEdgeOptions.edgeOptions = edgeOptions;

	straightEdgeOptions.numLines = 1;
	straightEdgeOptions.searchMode = IMAQ_USE_FIRST_RAKE_EDGES;
	straightEdgeOptions.minScore = 10;
	straightEdgeOptions.maxScore = 1000;
	straightEdgeOptions.orientation = 0;
	straightEdgeOptions.angleRange = 45;
	straightEdgeOptions.angleTolerance = 1;
	straightEdgeOptions.minSignalToNoiseRatio = 0;
	straightEdgeOptions.minCoverage = 25;
	straightEdgeOptions.houghIterations = 5;
}

VisionTracker::~VisionTracker()
{
	imaqDispose(InputImageRGB);
	imaqDispose(ParticleImageU8);

	delete[] particleCriteria;

	imaqDispose(roi);

	imaqDispose(Plane1);
	imaqDispose(Plane2);
	imaqDispose(Plane3);
}

int VisionTracker::GetFrame(Bitmap_Frame *Frame)
{
	int success = 1;

	// Copy our frame to an NI image.
	success = imaqArrayToImage(InputImageRGB, (void*)Frame->Memory, Frame->XRes, Frame->YRes);
	imaqGetImageInfo(InputImageRGB, &SourceImageInfo);
	return success;
}

void VisionTracker::ReturnFrame(Bitmap_Frame *Frame)
{	// copy NI image back to our frame.
	void *pImageArray = imaqImageToArray(InputImageRGB, IMAQ_NO_RECT, NULL, NULL);
	if(pImageArray != NULL)
	{
		memcpy((void*)Frame->Memory, pImageArray, Frame->Stride * 4 * Frame->YRes);
		imaqDispose(pImageArray);
	}
}

int VisionTracker::ProcessImage(double &x_target, double &y_target)
{
	int success = 1;

	//-----------------------------------------------------------------//
	//  Color threshold and optional noise filter                      //
	//-----------------------------------------------------------------//

	// color threshold
	VisionErrChk(imaqColorThreshold(ParticleImageU8, InputImageRGB, THRESHOLD_IMAGE_REPLACE_VALUE, IMAQ_RGB, &plane1Range, &plane2Range, &plane3Range));

	//-------------------------------------------------------------------//
	//     Advanced Morphology: particle filtering functions             //
	//-------------------------------------------------------------------//

	// fill holes
	VisionErrChk(imaqFillHoles(ParticleImageU8, ParticleImageU8, true));

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


int VisionTracker::BlurImage(Image* DestImage, Image* SrcImage, ColorMode_enum DestColorMOde, ColorMode_enum SrcColorMode)
{
	int success = 1;

	VisionErrChk(imaqExtractColorPlanes(SrcImage, SrcColorMode, Plane1, Plane2, Plane3)); 

	int krows = 5;
	int kcols = 5;

	// simple averaging convolution
	float kernel[] = {1,1,1,1,1,
			 		  1,1,1,1,1,
					  1,1,1,1,1,
					  1,1,1,1,1,
					  1,1,1,1,1};

	VisionErrChk(imaqConvolve2(Plane1, Plane1, kernel, krows, kcols, 0, NULL, IMAQ_ROUNDING_MODE_OPTIMIZE)); 
	VisionErrChk(imaqConvolve2(Plane2, Plane2, kernel, krows, kcols, 0, NULL, IMAQ_ROUNDING_MODE_OPTIMIZE)); 
	VisionErrChk(imaqConvolve2(Plane3, Plane3, kernel, krows, kcols, 0, NULL, IMAQ_ROUNDING_MODE_OPTIMIZE)); 

	// recombine planes
	VisionErrChk(imaqReplaceColorPlanes(DestImage, SrcImage, DestColorMOde, Plane1, Plane2, Plane3)); 

Error:

	return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: InitParticleFilter
//
// Description  : Sets up particle filters
//
// Parameters   : FilterMeasureTypes     -  Morphological measurement that the function
//                                  uses for filtering.
//                plower         -  Lower bound of the criteria range.
//                pUpper         -  Upper bound of the criteria range.
//                pCalibrated    -  Whether to take a calibrated measurement or not.
//                pExclude       -  TRUE indicates that a match occurs when the
//                                  value is outside the criteria range.
//                rejectMatches  -  Set this parameter to TRUE to transfer only
//                                  those particles that do not meet all the criteria.
//                                  Set this parameter to FALSE to transfer only those
//                                  particles that meet all the criteria to the destination.
//                connectivity   -  Set this parameter to 1 to use connectivity-8
//                                  to determine whether particles are touching.
//                                  Set this parameter to 0 to use connectivity-4
//                                  to determine whether particles are touching.
//
////////////////////////////////////////////////////////////////////////////////
void VisionTracker::InitParticleFilter(MeasurementType FilterMeasureTypes[], float plower[], float pUpper[],
							  int pCalibrated[], int pExclude[], int rejectMatches, int connectivity)
{
	int i;

	//-------------------------------------------------------------------//
	//                          Particle Filter                          //
	//-------------------------------------------------------------------//

	delete[] particleCriteria;	// remove any old data
	particleCriteria = new ParticleFilterCriteria2[criteriaCount];

	if (criteriaCount > 0)
	{
		// Fill in the ParticleFilterCriteria2 structure.
		for (i = 0 ; i < criteriaCount ; i++)
		{
			particleCriteria[i].parameter = FilterMeasureTypes[i];
			particleCriteria[i].lower = plower[i];
			particleCriteria[i].upper = pUpper[i];
			particleCriteria[i].calibrated = pCalibrated[i];
			particleCriteria[i].exclude = pExclude[i];
		}

		particleFilterOptions.rejectMatches = rejectMatches;
		particleFilterOptions.rejectBorder = 0;
		particleFilterOptions.fillHoles = TRUE;
		particleFilterOptions.connectivity8 = connectivity;
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: GetParticles
//
// Description  : Computes the number of particles detected in a binary image and
//                extracts requested measurements about the particle.
//
// Parameters   : image                      -  Input image
//                connectivity               -  Set this parameter to 1 to use
//                                              connectivity-8 to determine
//                                              whether particles are touching.
//                                              Set this parameter to 0 to use
//                                              connectivity-4 to determine
//                                              whether particles are touching.
//				  particleData&				 -  Array of ParticleData values.
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int VisionTracker::GetParticles(Image* image, int connectivity, ParticleList& particleList)
{
	int success = 1;

	const float XRes = (float)SourceImageInfo.xRes;
	const float YRes = (float)SourceImageInfo.yRes;
	const float Aspect = XRes / YRes;

	// remove any old values
	particleList.particleData.clear();
	particleList.numParticles = 0;

	//-------------------------------------------------------------------//
	//                         Particle Analysis                         //
	//-------------------------------------------------------------------//

	// Counts the number of particles in the image
	int numParticles;
	VisionErrChk(imaqCountParticles(ParticleImageU8, TRUE, &numParticles));

	for (int i = 0 ; i < numParticles ; i++)
	{
		double bound_left;
		double bound_right;
		double bound_top;
		double bound_bottom;
		double area;
		double center_x;
		double center_y;

		// Computes the requested pixel measurements about the particle.

		// bounding box
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_LEFT, &bound_left));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_TOP, &bound_top));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_RIGHT, &bound_right));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_BOTTOM, &bound_bottom));

		double bound_width = bound_right - bound_left;
		double bound_height = bound_bottom - bound_top;
		double aspect = bound_width / bound_height;
		double bound_area = bound_width * bound_height;

		// if aspect is not in range, skip it.
		if( aspect < particleList.aspectMin || aspect > particleList.aspectMax)
			continue;

		// particle area
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_AREA, &area));

		// if particle area doesn't fill bounding area enough skip it.
		if( bound_area > 0 && (area / bound_area < particleList.area_threshold) )
			continue;

		// all good, fill the values and add new entry to the list.
		ParticleData newParticle;
		particleList.numParticles++;

		newParticle.bound_left = (int)bound_left;
		newParticle.bound_top = (int)bound_top;
		newParticle.bound_right = (int)bound_right;
		newParticle.bound_bottom = (int)bound_bottom;

		// bounding box size
		newParticle.bound_width = (int)bound_width;
		newParticle.bound_height = (int)bound_height;

		// center of particle
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_X, &center_x));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_Y, &center_y));

		newParticle.center.x = (int)center_x;
		newParticle.center.y = (int)center_y;

		// convert to aiming system coords
		newParticle.AimSys.x = (float)((center_x - (XRes/2.0)) / (XRes/2.0)) * Aspect;
		newParticle.AimSys.y = (float)((center_y - (YRes/2.0)) / (YRes/2.0));

		particleList.particleData.push_back(newParticle);
	}

Error:

	return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: FindParticleCorners
//
// Description  : Locates and returns corner points of (assumed rectagular) particle.
//
// Parameters   : image			- Input image
//				: partleList	- particle data array. 
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////

int VisionTracker::FindParticleCorners(Image* image, ParticleList& particleList)
{
	int success = 1;

	float area_threshold = 0.8f;

	for(int i=0; i < particleList.numParticles; i++ )
	{
		Rect rect;
		ContourID cid;

		// left side
		rect.top = particleList.particleData[i].bound_top - particleList.particleData[i].bound_height/10;
		rect.left = particleList.particleData[i].bound_left - 8;
		rect.height = particleList.particleData[i].bound_height + particleList.particleData[i].bound_height/5;
		rect.width = particleList.particleData[i].bound_width/4;

		cid = imaqAddRectContour(roi, rect);
		VisionErrChk(cid);

		VisionErrChk(FindEdge(image, roi, IMAQ_LEFT_TO_RIGHT, particleList.particleData[i].bound_height/8, particleList.particleData[i], 0));

		if( m_bShowFindCorners )
			imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );

		// remove the contour
		VisionErrChk(imaqRemoveContour(roi, cid));

		// top side
		rect.top = particleList.particleData[i].bound_top - 8;
		rect.left = particleList.particleData[i].bound_left - particleList.particleData[i].bound_width/10;
		rect.height = particleList.particleData[i].bound_height/4;
		rect.width = particleList.particleData[i].bound_width + particleList.particleData[i].bound_width/5;

		cid = imaqAddRectContour(roi, rect);
		VisionErrChk(cid);

		VisionErrChk(FindEdge(image, roi, IMAQ_TOP_TO_BOTTOM, particleList.particleData[i].bound_width/8, particleList.particleData[i], 1));

		if( m_bShowFindCorners )
			imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );

		// remove the contour
		VisionErrChk(imaqRemoveContour(roi, cid));

		// right side
		rect.top = particleList.particleData[i].bound_top - particleList.particleData[i].bound_height/10;
		rect.left = particleList.particleData[i].bound_right - particleList.particleData[i].bound_width/4 + 8;
		rect.height = particleList.particleData[i].bound_height + particleList.particleData[i].bound_height/5;
		rect.width = particleList.particleData[i].bound_width/4;

		cid = imaqAddRectContour(roi, rect);
		VisionErrChk(cid);

		VisionErrChk(FindEdge(image, roi, IMAQ_RIGHT_TO_LEFT, particleList.particleData[i].bound_height/8, particleList.particleData[i], 2));

		if( m_bShowFindCorners )
			imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );

		// remove the contour
		VisionErrChk(imaqRemoveContour(roi, cid));

		// bottom side
		rect.top = particleList.particleData[i].bound_bottom - particleList.particleData[i].bound_height/4 + 8;
		rect.left = particleList.particleData[i].bound_left - particleList.particleData[i].bound_width/10;
		rect.height = particleList.particleData[i].bound_height/4;
		rect.width = particleList.particleData[i].bound_width + particleList.particleData[i].bound_width/5;

		cid = imaqAddRectContour(roi, rect);
		VisionErrChk(cid);

		VisionErrChk(FindEdge(image, roi, IMAQ_BOTTOM_TO_TOP, particleList.particleData[i].bound_width/8, particleList.particleData[i], 3));

		if( m_bShowFindCorners )
			imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );

		// remove the contour
		VisionErrChk(imaqRemoveContour(roi, cid));

		// get intersection points.
		VisionErrChk(imaqGetIntersection(particleList.particleData[i].lines[0].p1,
			particleList.particleData[i].lines[0].P2,
			particleList.particleData[i].lines[1].p1,
			particleList.particleData[i].lines[1].P2,
			&particleList.particleData[i].Intersections[0]))

			VisionErrChk(imaqGetIntersection(particleList.particleData[i].lines[1].p1,
			particleList.particleData[i].lines[1].P2,
			particleList.particleData[i].lines[2].p1,
			particleList.particleData[i].lines[2].P2,
			&particleList.particleData[i].Intersections[1]))

			VisionErrChk(imaqGetIntersection(particleList.particleData[i].lines[2].p1,
			particleList.particleData[i].lines[2].P2,
			particleList.particleData[i].lines[3].p1,
			particleList.particleData[i].lines[3].P2,
			&particleList.particleData[i].Intersections[2]))

			VisionErrChk(imaqGetIntersection(particleList.particleData[i].lines[3].p1,
			particleList.particleData[i].lines[3].P2,
			particleList.particleData[i].lines[0].p1,
			particleList.particleData[i].lines[0].P2,
			&particleList.particleData[i].Intersections[3]))
	}

Error:

	return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: FindEdge
//
// Description  : Locates a straight edge in a rectangular search area.
//
// Parameters   : image                  -  Input image
//                roi                    -  Region of interest
//                pDirection             -
//                pStepSize              -  Number of pixels that separates two
//                                          consecutive search lines.
//                particleData           -  Internal Data structure
//                lineIndex              -  index for line information (one each side)
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int VisionTracker::FindEdge(Image* image, ROI* roi, RakeDirection pDirection, unsigned int pStepSize, ParticleData& particleData, int lineIndex)
{
	int success = TRUE;
	FindEdgeReport* findEdgeReport = NULL;

	//-------------------------------------------------------------------//
	//                         Find Straight Edge                        //
	//-------------------------------------------------------------------//

	findEdgeOptions.direction = pDirection;
	straightEdgeOptions.stepSize = pStepSize;

	// Locates a straight edge in the rectangular search area.
	VisionErrChk(findEdgeReport = imaqFindEdge2(image, roi, NULL, NULL, &findEdgeOptions, &straightEdgeOptions));

	// Store the results in the data structure.
	if (findEdgeReport->numStraightEdges > 0)
	{
		particleData.lines[lineIndex].p1.x = findEdgeReport->straightEdges->straightEdgeCoordinates.start.x;
		particleData.lines[lineIndex].p1.y = findEdgeReport->straightEdges->straightEdgeCoordinates.start.y;
		particleData.lines[lineIndex].P2.x = findEdgeReport->straightEdges->straightEdgeCoordinates.end.x;
		particleData.lines[lineIndex].P2.y = findEdgeReport->straightEdges->straightEdgeCoordinates.end.y;
		particleData.lines[lineIndex].angle = findEdgeReport->straightEdges->angle;
	}

Error:
	// Disposes the edge report
	imaqDispose(findEdgeReport);

	return success;
}
