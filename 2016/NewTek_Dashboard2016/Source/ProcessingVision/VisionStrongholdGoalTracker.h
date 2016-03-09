#include "stdafx.h"

class VisionStrongholdGoalTracker : public VisionTracker
{
public:
	VisionStrongholdGoalTracker();
	virtual ~VisionStrongholdGoalTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);

private:
	ParticleList particleList;	// our results data structure
};