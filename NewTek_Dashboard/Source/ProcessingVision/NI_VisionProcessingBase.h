#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "profile.h"
#include "ProcessingVision.h"

#pragma warning(disable:4800)

#define VisionErrChk(Function) {if (!(Function)) {success = 0; FrameWork::DebugOutput("error: %d\n", imaqGetLastError()); goto Error;}}

// color values for drawing
#define COLOR_BLACK  0.0f
#define COLOR_WHITE  255 + 255 * 256.0f + 255 * 256.0f * 256.0f
#define COLOR_RED    255.0f
#define COLOR_GREEN  255 * 256.0f
#define COLOR_BLUE   255 * 256.0f * 256.0f
#define COLOR_YELLOW 255 + 255 * 256.0f
#define COLOR_MAGENT 255 + 255 * 256.0f * 256.0f
#define COLOR_CYAN   255 * 256.0f + 255 * 256.0f * 256.0f

#define IMAGE_BORDER_SIZE 7

#define THRESHOLD_IMAGE_REPLACE_VALUE 1 	

struct LineSegment
{
	PointFloat p1;
	PointFloat P2;
	double angle;
};

struct ParticleData
{
	Point center;
	PointFloat AimSys;
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
		particleData.clear();
		area_threshold = 0.8f;
		aspectMin = 0.8f;
		aspectMax = 1.4f;
	}

	void SetParticleParams( float thresh, float aspMin, float aspMax )
	{
		area_threshold = thresh;
		aspectMin = aspMin;
		aspectMax = aspMax;
	}

	int numParticles;
	float area_threshold;	// particles must fill this much of their bounding box to qualify.
	float aspectMin;		// particle bounding box must be in this range of aspect ratios.
	float aspectMax;
	std::vector<ParticleData>particleData;
};

class VisionTracker
{
public:
	VisionTracker();
	virtual ~VisionTracker();

	// override.
	virtual int ProcessImage(double &x, double &y) = 0;
	
	int GetFrame(Bitmap_Frame *Frame);
	void ReturnFrame(Bitmap_Frame *Frame);

	profile Profiler;

	// settings accessors
	// display opts
	void SetDisplayMode( DisplayType DisplayMode ) {m_DisplayMode = DisplayMode; }
	void SetShowOverlays( bool bOverlays ) { m_bShowOverlays = bOverlays; }
	void SetShowAiming( bool bAimingText ) { m_bShowAimingText = bAimingText; }
	void SetShowBounds( bool bBoundsText ) { m_bShowBoundsText = bBoundsText; }

	// particle processing opts
	void SetRejectBorderObjs( bool bRejctBorder ) { m_bRejectBorderParticles = bRejctBorder; }
	void SetUseConvexHull( bool bUseConvex )	  { m_bUseConvexHull = bUseConvex; }

	// threshold
	void SetUseColorThreshold( bool bUserColorTresh ) {m_bUseColorThreshold = bUserColorTresh; }

	// object separation
	void EnableObjectSeparation( bool bObjsSep ) {m_bObjectSeparation = bObjsSep; }

	// corner and and edge
	void SetFindCorners( bool bFindCorners ) { m_bUseFindCorners = bFindCorners; }
	void SetShowCorners( bool bShowCorners ) { if( m_bUseFindCorners ) m_bShowFindCorners = bShowCorners; }

protected:
	// option switches
	// display opts
	DisplayType m_DisplayMode;
	bool m_bShowOverlays;	
	bool m_bShowAimingText;	
	bool m_bShowBoundsText;	

	// particle processing opts
	bool m_bRejectBorderParticles;	
	bool m_bUseConvexHull;			

	// object separation
	bool m_bObjectSeparation;

	// threshold method
	ThresholdColorSpace m_ThresholdMode;
	bool m_bUseColorThreshold;

	// corner and and edge
	bool m_bUseFindCorners;		
	bool m_bShowFindCorners;	// dependent on find corners

	// images
	Image *InputImageRGB;		// our input image
	ImageInfo SourceImageInfo;	// info about our input image
	Image *ParticleImageU8;		// 8 bit image - particle processing
	Image *WorkImageU8;			// 8 bit image - work area
	Image *ThresholdImageU8;

	// separate planes for splitting color images
	Image *Plane1;
	Image *Plane2;
	Image *Plane3;

	Image *Destination;

	// TODO: change below to be pointers, add ranges for the supported modes, add a member for mode, and function to switch
	//       also add a destination image pointer to simplify supporting threshold display.
	// Color thresholding values 
	Range RedRange;
	Range GreenRange;
	Range BlueRange;

	Range HueRange;
	Range SaturationRange;
	Range ValueRange;

	Range LuminanceRange;

	Range *plane1Range;
	Range *plane2Range;
	Range *plane3Range;

	// particle filter params
	int criteriaCount;
	ParticleFilterCriteria2* particleCriteria;
	ParticleFilterOptions2 particleFilterOptions;

	// Edge finding options
	EdgeOptions2 edgeOptions;
	FindEdgeOptions2 findEdgeOptions;
	StraightEdgeOptions straightEdgeOptions;

	ParticleList particleList;	// our results data structure

	// text drawing
	char TextBuffer[256];
	DrawTextOptions textOps;

	// Edge / corner finding
	ROI *roi;

	void InitParticleFilter(MeasurementType FilterMeasureTypes[], float plower[], float pUpper[], int pCalibrated[],
		int pExclude[], int rejectMatches, int connectivity);
	int GetParticles(Image* image, int connectivity, ParticleList& particleList);

	// not used in current implementations
	int BlurImage(Image* DestImage, Image* SrcImage, ColorMode_enum DestColorMOde, ColorMode_enum SrcColorMode);
	int FindParticleCorners(Image* image, ParticleList& particleList);
	int FindEdge(Image* image, ROI* roi, RakeDirection pDirection, unsigned int pStepSize, ParticleData& particleData, int lineIndex);
};