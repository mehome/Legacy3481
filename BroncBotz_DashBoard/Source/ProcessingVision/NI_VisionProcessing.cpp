#include "stdafx.h"
#include "ProcessingVision.h"

#define VisionErrChk(Function) {if (!(Function)) {success = 0; /*printf("error: %d\n", imaqGetLastError());*/ goto Error;}}

// color values for drawing
#define COLOR_BLACK  0.0f
#define COLOR_WHITE  255 + 255 * 256.0f + 255 * 256.0f * 256.0f
#define COLOR_RED    255.0f
#define COLOR_GREEN  255 * 256.0f
#define COLOR_BLUE   255 * 256.0f * 256.0f
#define COLOR_YELLOW 255 + 255 * 256.0f
#define COLOR_MAGENT 255 + 255 * 256.0f * 256.0f
#define COLOR_CYAN   255 * 256.0f + 255 * 256.0f * 256.0f

struct LineSegment
{
	PointFloat p1;
	PointFloat P2;
	double angle;
};

struct ParticleData
{
	Point center;
	int bound_left;
	int bound_top;
	int bound_right;
	int bound_bottom;
	int bound_width;
	int bound_height;
	LineSegment lines[4];
	PointFloat Intersections[4];
};

struct ParticleList
{
	ParticleList()
	{
		numParticles = 0;
		particleData = NULL;
	}

	int numParticles;
	ParticleData *particleData;
};

int ProcessImage(Image *image, ParticleList &particleList);

bool CheckAspect(ParticleList particleList, int ParticleNum, float min, float max);
int ColorThreshold(Image* image, int min1, int max1, int min2, int max2, int min3, int max3, ColorMode colorMode);
int ParticleFilter(Image* image, MeasurementType FilterMeasureTypes[], float plower[], float pUpper[], int pCalibrated[],
							  int pExclude[], int rejectMatches, int connectivity);
int GetParticles(Image* image, int connectivity, ParticleList particleList);
int FindParticleCorners(Image* image, ParticleList particleList);
int FindEdge(Image* image, ROI* roi, RakeDirection pDirection, EdgePolaritySearchMode pPolarity, unsigned int pKernelSize, unsigned int pWidth,
				  float pMinThreshold, InterpolationMethod pInterpolationType, ColumnProcessingMode pColumnProcessingMode, unsigned int pStepSize,
				  StraightEdgeSearchMode pSearchMode, ParticleData* particleData, int lineIndex, int particleNumber);


Bitmap_Frame *NI_VisionProcessing(Bitmap_Frame *Frame)
{
	ImageType imageType = IMAQ_IMAGE_RGB;   
	int ImageBorder = 7;
	ParticleList particleList;

	// Create an IMAQ Vision image, copy our frame to it.
	Image *image = imaqCreateImage(imageType, ImageBorder);
	imaqArrayToImage(image, (void*)Frame->Memory, Frame->XRes, Frame->YRes);

	// do the actual processing
	ProcessImage(image, particleList);

	// Return our processed image back to our outgoing frame.
	void *pImageArray = imaqImageToArray(image, IMAQ_NO_RECT, NULL, NULL);
	if(pImageArray != NULL)
		memcpy((void*)Frame->Memory, pImageArray, Frame->Stride * 4 * Frame->YRes);

	// TODO: use particleList data to overlay
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

	delete particleList.particleData;
	imaqDispose(pImageArray);
	imaqDispose(image);

	return Frame;
}

int ProcessImage(Image *image, ParticleList &particleList)
{
	int success = 1;

	int ImageBorder = 7;
	ImageInfo info;

#if 0
	// blue
	int redMin = 20; // -30
	int redMax = 80; // -20
	int grnMin = 50; // -30
	int grnMax = 100; // - 25
	int bluMin = 90; // -40
	int bluMax = 175;
#endif
#if 1
	// grey
	int redMin = 150;	
	int redMax = 200;	
	int grnMin = 130;	
	int grnMax = 180;	
	int bluMin = 120;	
	int bluMax = 170;	
#endif
#if 0
	// dark blue
	int redMin = 20;
	int redMax = 75;
	int grnMin = 20;
	int grnMax = 75;
	int bluMin = 0;
	int bluMax = 175;
#endif
#if 0
	// red
	int redMin = 60;
	int redMax = 255;
	int grnMin = 20;
	int grnMax = 75;
	int bluMin = 20;
	int bluMax = 75;
#endif


#if 0
	// separate planes, and do low pass.
	Image *Plane1 = imaqCreateImage(IMAQ_IMAGE_U8, ImageBorder);
	Image *Plane2 = imaqCreateImage(IMAQ_IMAGE_U8, ImageBorder);
	Image *Plane3 = imaqCreateImage(IMAQ_IMAGE_U8, ImageBorder);

	VisionErrChk(imaqExtractColorPlanes(image, IMAQ_RGB, Plane1, Plane2, Plane3)); 
	
	float kernel[] = {1,2,3,2,1,
					  2,3,3,3,2,
					  3,3,0,3,3,
					  2,3,3,3,2,
					  1,2,3,2,1};

	VisionErrChk(imaqConvolve2(Plane1, Plane1, kernel, 5, 5, 0, NULL, IMAQ_ROUNDING_MODE_OPTIMIZE)); 
	VisionErrChk(imaqConvolve2(Plane2, Plane2, kernel, 5, 5, 0, NULL, IMAQ_ROUNDING_MODE_OPTIMIZE)); 
	VisionErrChk(imaqConvolve2(Plane3, Plane3, kernel, 5, 5, 0, NULL, IMAQ_ROUNDING_MODE_OPTIMIZE)); 

//#define DISPLAY_WINDOW 0
//	// Display the image
//	imaqMoveWindow(DISPLAY_WINDOW, imaqMakePoint(0,0));
//	imaqSetWindowPalette(DISPLAY_WINDOW, /*IMAQ_PALETTE_BINARY*/ IMAQ_PALETTE_GRAY, NULL, 0);
//	imaqDisplayImage(Plane1, DISPLAY_WINDOW, TRUE);
//
//	// Display the image
//	imaqMoveWindow(DISPLAY_WINDOW + 1, imaqMakePoint(700,0));
//	imaqSetWindowPalette(DISPLAY_WINDOW + 1, /*IMAQ_PALETTE_BINARY*/ IMAQ_PALETTE_GRAY, NULL, 0);
//	imaqDisplayImage(Plane2, DISPLAY_WINDOW + 1, TRUE);
//
//	// Display the image
//	imaqMoveWindow(DISPLAY_WINDOW + 2, imaqMakePoint(0,600));
//	imaqSetWindowPalette(DISPLAY_WINDOW + 2, /*IMAQ_PALETTE_BINARY*/ IMAQ_PALETTE_GRAY, NULL, 0);
//	imaqDisplayImage(Plane3, DISPLAY_WINDOW + 2, TRUE);

	VisionErrChk(imaqReplaceColorPlanes(image, image, IMAQ_RGB, Plane1, Plane2, Plane3)); 

	imaqDispose(Plane1);
	imaqDispose(Plane2);
	imaqDispose(Plane3);
#endif

	// copy image
	// image  - used for particle operations - gets converted to 8 bit (binary).
	// image2 - used for edge detection
	// image3 - copy of original - used for overlays
	imaqGetImageInfo(image, &info);
	Image *image2 = imaqCreateImage(info.imageType, ImageBorder);
	imaqDuplicate(image2, image);

	Image *image3 = imaqCreateImage(info.imageType, ImageBorder);
	imaqDuplicate(image3, image);

	VisionErrChk(ColorThreshold(image, redMin, redMax, grnMin, grnMax, bluMin, bluMax, IMAQ_RGB));

	VisionErrChk(imaqFillHoles(image, image, true));

#if 1

	//-------------------------------------------------------------------//
	//                Advanced Morphology: Remove Objects                //
	//-------------------------------------------------------------------//

	int pKernel[] = {1,1,1,1,1,1,1,1,1};	// 3x3 kernel 
	StructuringElement structElem;
	structElem.matrixCols = 3;
	structElem.matrixRows = 3;
	structElem.hexa = TRUE;
	structElem.kernel = pKernel;

	int erosions = 2;

	// Filters particles based on their size.
	VisionErrChk(imaqSizeFilter(image, image, TRUE, erosions, IMAQ_KEEP_LARGE, &structElem));

	//-------------------------------------------------------------------//
	//                  Advanced Morphology: Convex Hull                 //
	//-------------------------------------------------------------------//

	// Computes the convex envelope for each labeled particle in the source image.
	VisionErrChk(imaqConvexHull(image, image, FALSE));	// Connectivity 4??? set to true to make con 8.

	//-------------------------------------------------------------------//
	//             Advanced Morphology: Remove Border Objects            //
	//-------------------------------------------------------------------//

	// Eliminates particles touching the border of the image.
	VisionErrChk(imaqRejectBorder(image, image, TRUE));

	// particle filter parameters
	MeasurementType FilterMeasureTypes[2] = {IMAQ_MT_BOUNDING_RECT_HEIGHT, IMAQ_MT_BOUNDING_RECT_WIDTH};
	float plower[2] = {100, 90};	// i.e., height bounds: 40 - 400, width bounds 30 - 400
	float pUpper[2] = {400,400};
	int pCalibrated[2] = {0,0};
	int pExclude[2] = {0,0};

	VisionErrChk(ParticleFilter(image, FilterMeasureTypes, plower, pUpper, 
		pCalibrated, pExclude, FALSE, TRUE));


	// Counts the number of particles in the image.
	VisionErrChk(imaqCountParticles(image, TRUE, &particleList.numParticles));

	if(particleList.numParticles > 0)
	{
		particleList.particleData = new ParticleData[particleList.numParticles];

		VisionErrChk(GetParticles(image, TRUE, particleList));

		VisionErrChk(FindParticleCorners(image2, particleList));

#if 1
		// overlay some useful info
		for(int i = 0; i < particleList.numParticles; i++)
		{
			Point P1;
			Point P2;
			Rect rect;

			float aspectMin = 0.75f;
			float aspectMax = 2.2f;

			// reject if aspect is too far out.
			if(CheckAspect(particleList, i, aspectMin, aspectMax))
				continue;

			// draw a line from target CoM to center of screen
			P1.x = particleList.particleData[i].center.x;
			P1.y = particleList.particleData[i].center.y;
			P2.x = info.xRes / 2;
			P2.y = info.yRes / 2;
			imaqDrawLineOnImage(image2, image2, IMAQ_DRAW_VALUE, P1, P2, COLOR_RED );

			// center of mass
			P1.x = particleList.particleData[i].center.x - 6;
			P1.y = particleList.particleData[i].center.y;
			P2.x = particleList.particleData[i].center.x + 6;
			P2.y = particleList.particleData[i].center.y;

			imaqDrawLineOnImage(image2, image2, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

			P1.x = particleList.particleData[i].center.x;
			P1.y = particleList.particleData[i].center.y - 6;
			P2.x = particleList.particleData[i].center.x;
			P2.y = particleList.particleData[i].center.y + 6;

			imaqDrawLineOnImage(image2, image2, IMAQ_DRAW_VALUE, P1, P2, COLOR_WHITE );

			// bounding box
			rect.top = particleList.particleData[i].bound_top;
			rect.left = particleList.particleData[i].bound_left;
			rect.height = particleList.particleData[i].bound_height;
			rect.width = particleList.particleData[i].bound_width;

			imaqDrawShapeOnImage(image2, image2, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_GREEN );

			// corner points
			for(int j = 0; j < 4; j++)
			{
				P1.x = (int)particleList.particleData[i].Intersections[j].x - 6;
				P1.y = (int)particleList.particleData[i].Intersections[j].y;
				P2.x = (int)particleList.particleData[i].Intersections[j].x + 6;
				P2.y = (int)particleList.particleData[i].Intersections[j].y;

				imaqDrawLineOnImage(image2, image2, IMAQ_DRAW_VALUE, P1, P2, COLOR_YELLOW );

				P1.x = (int)particleList.particleData[i].Intersections[j].x;
				P1.y = (int)particleList.particleData[i].Intersections[j].y - 6;
				P2.x = (int)particleList.particleData[i].Intersections[j].x;
				P2.y = (int)particleList.particleData[i].Intersections[j].y + 6;

				imaqDrawLineOnImage(image2, image2, IMAQ_DRAW_VALUE, P1, P2, COLOR_YELLOW );
			}
		}
#endif
	}
#endif
	// copy the end result 
	//imaqMask(image3, image2, image);
	imaqDuplicate(image, image2);
	imaqDispose(image2);
	imaqDispose(image3);

Error:
	int error = imaqGetLastError();
	return success;
}


bool CheckAspect(ParticleList particleList, int ParticleNum, float min, float max)
{
	// reject if aspect is too far out.
	float aspect = (float)particleList.particleData[ParticleNum].bound_width / (float)particleList.particleData[ParticleNum].bound_height;
	return (aspect < 0.8 || aspect > 2.2);
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

#define DRAW_ROI

int FindParticleCorners(Image* image, ParticleList particleList)
{
	int success = 1;

	int i;
	ROI *roi;
	ROI *roi1;
	ROI *roi2;
	ROI *roi3;

	for( i=0; i < particleList.numParticles; i++ )
	{
		// reject if aspect is too far out.
		float aspectMin = 0.75f;
		float aspectMax = 2.2f;
		if(CheckAspect(particleList, i, aspectMin, aspectMax))
			continue;

		// Creates a new, empty region of interest.
		VisionErrChk(roi = imaqCreateROI());

		Rect rect;

		// Creates a new rectangle ROI contour and adds the rectangle to the provided ROI.
		// left side
		rect.top = particleList.particleData[i].bound_top - particleList.particleData[i].bound_height/10;
		rect.left = particleList.particleData[i].bound_left - 8;
		rect.height = particleList.particleData[i].bound_height + particleList.particleData[i].bound_height/5;
		rect.width = particleList.particleData[i].bound_width/4;

		VisionErrChk(imaqAddRectContour(roi, rect));

#ifdef DRAW_ROI
		imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );
#endif

		VisionErrChk(FindEdge(image, roi, IMAQ_LEFT_TO_RIGHT, 
			IMAQ_SEARCH_FOR_RISING_EDGES, 3, 1, 25, IMAQ_BILINEAR_FIXED, 
			IMAQ_AVERAGE_COLUMNS, particleList.particleData[i].bound_height/8, IMAQ_USE_FIRST_RAKE_EDGES, particleList.particleData, 0, i));

		// Cleans up resources associated with the object
		imaqDispose(roi);

		// Creates a new, empty region of interest.
		VisionErrChk(roi1 = imaqCreateROI());

		// Creates a new rectangle ROI contour and adds the rectangle to the provided ROI.
		// top side
		rect.top = particleList.particleData[i].bound_top - 8;
		rect.left = particleList.particleData[i].bound_left - particleList.particleData[i].bound_width/10;
		rect.height = particleList.particleData[i].bound_height/4;
		rect.width = particleList.particleData[i].bound_width + particleList.particleData[i].bound_width/5;

		VisionErrChk(imaqAddRectContour(roi1, rect));

#ifdef DRAW_ROI
		imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );
#endif

		VisionErrChk(FindEdge(image, roi1, IMAQ_TOP_TO_BOTTOM, 
			IMAQ_SEARCH_FOR_RISING_EDGES, 3, 1, 25, IMAQ_BILINEAR_FIXED, 
			IMAQ_AVERAGE_COLUMNS, particleList.particleData[i].bound_width/8, IMAQ_USE_FIRST_RAKE_EDGES, particleList.particleData, 1, i));

		// Cleans up resources associated with the object
		imaqDispose(roi1);

		// Creates a new, empty region of interest.
		VisionErrChk(roi2 = imaqCreateROI());

		// Creates a new rectangle ROI contour and adds the rectangle to the provided ROI.
		// right side
		rect.top = particleList.particleData[i].bound_top - particleList.particleData[i].bound_height/10;
		rect.left = particleList.particleData[i].bound_right - particleList.particleData[i].bound_width/4 + 8;
		rect.height = particleList.particleData[i].bound_height + particleList.particleData[i].bound_height/5;
		rect.width = particleList.particleData[i].bound_width/4;

		VisionErrChk(imaqAddRectContour(roi2, rect));

#ifdef DRAW_ROI
		imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );
#endif

		VisionErrChk(FindEdge(image, roi2, IMAQ_RIGHT_TO_LEFT, 
			IMAQ_SEARCH_FOR_RISING_EDGES, 3, 1, 25, IMAQ_BILINEAR_FIXED, 
			IMAQ_AVERAGE_COLUMNS, particleList.particleData[i].bound_height/8, IMAQ_USE_FIRST_RAKE_EDGES, particleList.particleData, 2, i));

		// Cleans up resources associated with the object
		imaqDispose(roi2);

		// Creates a new, empty region of interest.
		VisionErrChk(roi3 = imaqCreateROI());

		// Creates a new rectangle ROI contour and adds the rectangle to the provided ROI.
		// bottom side
		rect.top = particleList.particleData[i].bound_bottom - particleList.particleData[i].bound_height/4 + 8;
		rect.left = particleList.particleData[i].bound_left - particleList.particleData[i].bound_width/10;
		rect.height = particleList.particleData[i].bound_height/4;
		rect.width = particleList.particleData[i].bound_width + particleList.particleData[i].bound_width/5;

		VisionErrChk(imaqAddRectContour(roi3, rect));

#ifdef DRAW_ROI
		imaqDrawShapeOnImage(image, image, rect, IMAQ_DRAW_VALUE, IMAQ_SHAPE_RECT, COLOR_CYAN );
#endif

		VisionErrChk(FindEdge(image, roi3, IMAQ_BOTTOM_TO_TOP, 
			IMAQ_SEARCH_FOR_RISING_EDGES, 3, 1, 25, IMAQ_BILINEAR_FIXED, 
			IMAQ_AVERAGE_COLUMNS, particleList.particleData[i].bound_width/8, IMAQ_USE_FIRST_RAKE_EDGES, particleList.particleData, 3, i));

		// Cleans up resources associated with the object
		imaqDispose(roi3);

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
// Function Name: ColorThreshold
//
// Description  : Thresholds a color image.
//
// Parameters   : image      -  Input image
//                min1       -  Minimum range for the first plane
//                max1       -  Maximum range for the first plane
//                min2       -  Minimum range for the second plane
//                max2       -  Maximum range for the second plane
//                min3       -  Minimum range for the third plane
//                max3       -  Maximum range for the third plane
//                colorMode  -  Color space in which to perform the threshold
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int ColorThreshold(Image* image, int min1, int max1, int min2, int max2, int min3, int max3, ColorMode colorMode)
{
	int success = 1;
	Image* thresholdImage;
	Range plane1Range;
	Range plane2Range;
	Range plane3Range;

	//-------------------------------------------------------------------//
	//                          Color Threshold                          //
	//-------------------------------------------------------------------//

	// Creates an 8 bit image for the thresholded image.
	VisionErrChk(thresholdImage = imaqCreateImage(IMAQ_IMAGE_U8, 7));

	// Set the threshold range for the 3 planes.
	plane1Range.minValue = min1;
	plane1Range.maxValue = max1;
	plane2Range.minValue = min2;
	plane2Range.maxValue = max2;
	plane3Range.minValue = min3;
	plane3Range.maxValue = max3;

	// Thresholds the color image.
	VisionErrChk(imaqColorThreshold(thresholdImage, image, 1, colorMode, &plane1Range, &plane2Range, &plane3Range));

	// TODO: use this to make a copy of the original
	// Copies the threshold image in the souce image.
	VisionErrChk(imaqDuplicate(image, thresholdImage));

Error:
	imaqDispose(thresholdImage);

	return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: ParticleFilter
//
// Description  : Filters particles based on their morphological measurements.
//
// Parameters   : image          -  Input image
//                FilterMeasureTypes     -  Morphological measurement that the function
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
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int ParticleFilter(Image* image, MeasurementType FilterMeasureTypes[], float plower[], float pUpper[],
							  int pCalibrated[], int pExclude[], int rejectMatches, int connectivity)
{
	int success = 1;
	int i;
	ParticleFilterOptions2 particleFilterOptions;
	int numParticles;


	//-------------------------------------------------------------------//
	//                          Particle Filter                          //
	//-------------------------------------------------------------------//

	int criteriaCount = sizeof(FilterMeasureTypes) / sizeof(FilterMeasureTypes[0]);
	ParticleFilterCriteria2* particleCriteria = new ParticleFilterCriteria2[criteriaCount];

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

		// Filters particles based on their morphological measurements.
		VisionErrChk(imaqParticleFilter4(image, image, particleCriteria, criteriaCount, &particleFilterOptions, NULL, &numParticles));
	}

Error:
	delete[] particleCriteria;

	return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: GetParticles
//
// Description  : Computes the number of particles detected in a binary image and
//                a 2D array of requested measurements about the particle.
//
// Parameters   : image                      -  Input image
//                connectivity               -  Set this parameter to 1 to use
//                                              connectivity-8 to determine
//                                              whether particles are touching.
//                                              Set this parameter to 0 to use
//                                              connectivity-4 to determine
//                                              whether particles are touching.
//				  particleData*				 -  Array of ParticleData values.
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int GetParticles(Image* image, int connectivity, ParticleList particleList)
{
	int success = 1;
	int i;

	//-------------------------------------------------------------------//
	//                         Particle Analysis                         //
	//-------------------------------------------------------------------//

	for (i = 0 ; i < particleList.numParticles ; i++)
	{
		double measurementValue;

		// Computes the requested pixel measurements about the particle.
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_X, &measurementValue));
		particleList.particleData[i].center.x = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_Y, &measurementValue));
		particleList.particleData[i].center.y = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_LEFT, &measurementValue));
		particleList.particleData[i].bound_left = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_TOP, &measurementValue));
		particleList.particleData[i].bound_top = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_RIGHT, &measurementValue));
		particleList.particleData[i].bound_right = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_BOTTOM, &measurementValue));
		particleList.particleData[i].bound_bottom = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_WIDTH, &measurementValue));
		particleList.particleData[i].bound_width = (int)measurementValue;
		VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_BOUNDING_RECT_HEIGHT, &measurementValue));
		particleList.particleData[i].bound_height = (int)measurementValue;
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
//                pPolarity              -
//                pKernelSize            -
//                pWidth                 -
//                pMinThreshold          -
//                pInterpolationType     -
//                pColumnProcessingMode  -
//                pStepSize              -  Number of pixels that separates two
//                                          consecutive search lines.
//                pSearchMode            -
//                particleData           -  Internal Data structure
//                lineIndex              -  index for line information (one each side)
//				  particleNumber		 -  the particle we are processing.
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int FindEdge(Image* image, ROI* roi, RakeDirection pDirection, EdgePolaritySearchMode pPolarity, unsigned int pKernelSize, unsigned int pWidth,
						 float pMinThreshold, InterpolationMethod pInterpolationType, ColumnProcessingMode pColumnProcessingMode, unsigned int pStepSize,
						 StraightEdgeSearchMode pSearchMode, ParticleData* particleData, int lineIndex, int particleNumber)
{
	int success = TRUE;
	EdgeOptions2 edgeOptions;
	FindEdgeOptions2 findEdgeOptions;
	StraightEdgeOptions straightEdgeOptions;
	FindEdgeReport* findEdgeReport = NULL;
	unsigned int visionInfo;


	//-------------------------------------------------------------------//
	//                         Find Straight Edge                        //
	//-------------------------------------------------------------------//

	edgeOptions.polarity = pPolarity;
	edgeOptions.kernelSize = pKernelSize;
	edgeOptions.width = pWidth;
	edgeOptions.minThreshold = pMinThreshold;
	edgeOptions.interpolationType = pInterpolationType;
	edgeOptions.columnProcessingMode = pColumnProcessingMode;

	findEdgeOptions.direction = pDirection;
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
	straightEdgeOptions.searchMode = pSearchMode;
	straightEdgeOptions.minScore = 10;
	straightEdgeOptions.maxScore = 1000;
	straightEdgeOptions.orientation = 0;
	straightEdgeOptions.angleRange = 45;
	straightEdgeOptions.angleTolerance = 1;
	straightEdgeOptions.stepSize = pStepSize;
	straightEdgeOptions.minSignalToNoiseRatio = 0;
	straightEdgeOptions.minCoverage = 25;
	straightEdgeOptions.houghIterations = 5;

	// Locates a straight edge in the rectangular search area.
	VisionErrChk(findEdgeReport = imaqFindEdge2(image, roi, NULL, NULL, &findEdgeOptions, &straightEdgeOptions));

	// ////////////////////////////////////////
	// Store the results in the data structure.
	// ////////////////////////////////////////

	// Check if the image is calibrated.
	VisionErrChk(imaqGetVisionInfoTypes(image, &visionInfo));

	if (findEdgeReport->numStraightEdges > 0)
	{
		particleData[particleNumber].lines[lineIndex].p1.x = findEdgeReport->straightEdges->straightEdgeCoordinates.start.x;
		particleData[particleNumber].lines[lineIndex].p1.y = findEdgeReport->straightEdges->straightEdgeCoordinates.start.y;
		particleData[particleNumber].lines[lineIndex].P2.x = findEdgeReport->straightEdges->straightEdgeCoordinates.end.x;
		particleData[particleNumber].lines[lineIndex].P2.y = findEdgeReport->straightEdges->straightEdgeCoordinates.end.y;
		particleData[particleNumber].lines[lineIndex].angle = findEdgeReport->straightEdges->angle;
	}

Error:
	// Disposes the edge report
	imaqDispose(findEdgeReport);

	return success;
}
