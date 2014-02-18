#include "stdafx.h"

class VisionAerialAssistGoalTracker : public VisionTracker
{
public:
	VisionAerialAssistGoalTracker();
	virtual ~VisionAerialAssistGoalTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);

private:
	ParticleList particleListVert;	// our results data structure
	ParticleList particleListHorz;

	struct TargetReport {
		int verticalIndex;
		int horizontalIndex;
		bool Hot;
		double totalScore;
		double leftScore;
		double rightScore;
		double tapeWidthScore;
		double verticalScore;
	} target;

	bool hotOrNot(TargetReport target);
};