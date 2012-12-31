#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "profile.h"
#include "ProcessingVision.h"

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
		aspectMax = 1.33333333f;
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
	~VisionTracker();

	int ProcessImage(double &x, double &y);
	int GetFrame(Bitmap_Frame *Frame);
	void ReturnFrame(Bitmap_Frame *Frame);

	profile Profiler;

private:
	// images
	Image *InputImageRGB;		// our input image
	ImageInfo SourceImageInfo;	// info about our input image
	Image *ParticleImageU8;		// 8 bit image - particle processing

	// separate planes for splitting color images
	Image *Plane1;
	Image *Plane2;
	Image *Plane3;

	// Color thresholding values // TODO: add UI to set these
	Range plane1Range;
	Range plane2Range;
	Range plane3Range;

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

	int BlurImage(Image* DestImage, Image* SrcImage, ColorMode_enum DestColorMOde, ColorMode_enum SrcColorMode);
	void InitParticleFilter(MeasurementType FilterMeasureTypes[], float plower[], float pUpper[], int pCalibrated[],
		int pExclude[], int rejectMatches, int connectivity);
	int GetParticles(Image* image, int connectivity, ParticleList& particleList);
	int FindParticleCorners(Image* image, ParticleList& particleList);
	int FindEdge(Image* image, ROI* roi, RakeDirection pDirection, unsigned int pStepSize, ParticleData& particleData, int lineIndex);
};