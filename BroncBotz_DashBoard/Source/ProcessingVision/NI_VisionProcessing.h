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
	float aspect;
	float area;
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

class VisionTracker
{
public:
	VisionTracker();
	~VisionTracker();

	int ProcessImage(double &x, double &y);
	int GetFrame(Bitmap_Frame *Frame);
	void RetrieveFrame(Bitmap_Frame *Frame);

	profile Profiler;

private:
	Image *InputImageRGB;
	Image *ParticleImageU8;

	ParticleList particleList;

	bool CheckAspect(float aspect, float min, float max);
	int ColorThreshold(Image* DestImage, Image* SrcImage, int min1, int max1, int min2, int max2, int min3, int max3, ColorMode colorMode);
	int ParticleFilter(Image* image, MeasurementType FilterMeasureTypes[], float plower[], float pUpper[], int pCalibrated[],
		int pExclude[], int rejectMatches, int connectivity);
	int GetParticles(Image* image, int connectivity, ParticleList particleList);
	int FindParticleCorners(Image* image, ParticleList particleList);
	int FindEdge(Image* image, ROI* roi, RakeDirection pDirection, EdgePolaritySearchMode pPolarity, unsigned int pKernelSize, unsigned int pWidth,
		float pMinThreshold, InterpolationMethod pInterpolationType, ColumnProcessingMode pColumnProcessingMode, unsigned int pStepSize,
		StraightEdgeSearchMode pSearchMode, ParticleData* particleData, int lineIndex, int particleNumber);
};