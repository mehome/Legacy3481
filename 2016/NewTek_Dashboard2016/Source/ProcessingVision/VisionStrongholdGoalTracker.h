#include "stdafx.h"

class VisionStrongholdGoalTracker : public VisionTracker
{
public:
	VisionStrongholdGoalTracker();
	virtual ~VisionStrongholdGoalTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);

private:
	ParticleList particleListVert;	// our results data structure
	ParticleList particleListHorz;

	struct TargetReport {
		int verticalIndex;
		int horizontalIndex;
		int Hot;
		double totalScore;
		double leftScore;
		double rightScore;
		double tapeWidthScore;
		double verticalScore;
	} target;

	int hotOrNot(TargetReport target);
};