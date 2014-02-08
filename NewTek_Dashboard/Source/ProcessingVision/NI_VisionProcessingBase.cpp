#include "stdafx.h"
#include "ProcessingVision.h"
#include "profile.h"
#include "NI_VisionProcessingBase.h"

VisionTracker::VisionTracker()
	: criteriaCount( 0 ), particleCriteria( NULL ), m_bObjectSeparation( false ),
	  m_DisplayMode( eNormal ), m_bShowOverlays( true ), m_bShowSolidMask( false ),
	  m_bShowAimingText( true ), m_bShowBoundsText( false ), m_ThresholdMode( eThreshRGB ),
	  m_bRejectBorderParticles( true ), m_bUseConvexHull( false ), m_b3PtGoal( false ), m_bBallColor( false ),
	  m_bUseFindCorners( false ), m_bShowFindCorners( false ), m_bJoinObjects( false )
{	
	Profiler = new profile;
	InputImageRGB = imaqCreateImage(IMAQ_IMAGE_RGB, IMAGE_BORDER_SIZE);
	ParticleImageU8 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);
	WorkImageU8 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);
	ThresholdImageU8 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);

	// separate planes (for noise filter)
	Plane1 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);
	Plane2 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);
	Plane3 = imaqCreateImage(IMAQ_IMAGE_U8, IMAGE_BORDER_SIZE);

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
	imaqDispose(ThresholdImageU8);

	delete[] particleCriteria;

	imaqDispose(roi);

	imaqDispose(WorkImageU8);
	imaqDispose(Plane1);
	imaqDispose(Plane2);
	imaqDispose(Plane3);
}

void VisionTracker::SetThresholdMode( ThresholdColorSpace mode ) 
{
	m_ThresholdMode = mode; 
	switch( mode )
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
			plane1Range = plane2Range = plane2Range = &LuminanceRange;
			break;
	}
}

void VisionTracker::SetThresholdValues( VisionSetting_enum whichVal, int value )
{
	switch( whichVal )
	{
		case eThresholdPlane1Min:
			plane1Range->minValue = value;
			break;
		case eThresholdPlane2Min:
			plane2Range->minValue = value;
			break;
		case eThresholdPlane3Min:
			plane3Range->minValue = value;
			break;
		case eThresholdPlane1Max:
			plane1Range->maxValue = value;
			break;
		case eThresholdPlane2Max:
			plane2Range->maxValue = value;
			break;
		case eThresholdPlane3Max:
			plane3Range->maxValue = value;
			break;
	}
}

int VisionTracker::GetThresholdValues( VisionSetting_enum whichVal )
{
	switch( whichVal )
	{
	case eThresholdPlane1Min:
		return plane1Range->minValue;
	case eThresholdPlane2Min:
		return plane2Range->minValue;
	case eThresholdPlane3Min:
		return plane3Range->minValue;
	case eThresholdPlane1Max:
		return plane1Range->maxValue;
	case eThresholdPlane2Max:
		return plane2Range->maxValue;
	case eThresholdPlane3Max:
		return plane3Range->maxValue;
	default:
		return 0;
	}
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


double VisionTracker::ratioToScore(double ratio)
{
	double score = (max(0, min(100*(1-fabs(1-ratio)),100)));
	return score;
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
	VisionErrChk(imaqCountParticles(image, TRUE, &numParticles));

	for (int i = 0 ; i < numParticles ; i++)
	{
		double bound_left;
		double bound_right;
		double bound_top;
		double bound_bottom;
		double area;
		double center_x;
		double center_y;
		double eq_long_side;
		double eq_short_side;
		eStatus status = eOK;

		// Computes the requested pixel measurements about the particle.

		// bounding box
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_LEFT, &bound_left));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_TOP, &bound_top));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_RIGHT, &bound_right));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_BOTTOM, &bound_bottom));

		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_EQUIVALENT_RECT_LONG_SIDE, &eq_long_side));
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_EQUIVALENT_RECT_SHORT_SIDE, &eq_short_side));

		double bound_width = bound_right - bound_left;
		double bound_height = bound_bottom - bound_top;
		double aspect = bound_width / bound_height;
		double bound_area = bound_width * bound_height;

		DOUT("p=%d  width=%f height=%f aspect=%f\n", i, bound_width, bound_height, aspect);

#if 1
		// if aspect is not in range, skip it.
		if( particleList.aspectMin > 0 && particleList.aspectMax > 0)
		{
			if( aspect < particleList.aspectMin || aspect > particleList.aspectMax)
			{
				DOUT("rejected - min %f < asp %f < max %f\n", particleList.aspectMin, aspect, particleList.aspectMax);
				status = eAspectFail;
			}
		}
		else if( particleList.ideal_horz_asp > 0 && particleList.ideal_vert_asp > 0 )
		{
			if( bound_width > bound_height)
			{
				if(ratioToScore((eq_long_side/eq_short_side)/particleList.ideal_horz_asp) < 50)
				{
					DOUT("rejected - vert asp score %f < 50\n", ratioToScore((eq_long_side/eq_short_side)/particleList.ideal_horz_asp));
					status = eAspectFail;
				}
			}
			else
			{
				if(ratioToScore((eq_short_side/eq_long_side)/particleList.ideal_vert_asp) < 50)
				{
					DOUT("rejected - vert asp score %f < 50\n", ratioToScore((eq_short_side/eq_long_side)/particleList.ideal_vert_asp));
					status = eAspectFail;
				}
			}
		}
#endif
		if(status == eOK && particleList.circularity_limit > 0)
		{
			double circularity;
			VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_HEYWOOD_CIRCULARITY_FACTOR, &circularity));
			DOUT("circularity: %f\n", circularity);
			// if not round enough
			if(circularity > particleList.circularity_limit)
			{
				DOUT("rejected - circularity factor too high %f\n", circularity);
				status = eCircularityFail;
			}
		}
#if 1
		if(status == eOK && particleList.area_threshold > 0)
		{
			// particle area
			VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_AREA, &area));

			// if particle area fills too much bounding area enough skip it.
			if( bound_area > 0 && (area / bound_area < particleList.area_threshold) )
			{
				DOUT("rejected - area ratio %f < threshold %f\n", area / bound_area, particleList.area_threshold);
				status = eAreaFail;
			}
		}
#endif
		// all good, fill the values and add new entry to the list.
		ParticleData newParticle;
		particleList.numParticles++;

		newParticle.status = status;
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
