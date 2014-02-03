#include "stdafx.h"

class VisionAerialAssistGoalTracker : public VisionTracker
{
public:
	VisionAerialAssistGoalTracker();
	virtual ~VisionAerialAssistGoalTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);

private:
	ParticleList particleList;	// our results data structure
};