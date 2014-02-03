#include "stdafx.h"

class VisionBallTracker : public VisionTracker
{
public:
	VisionBallTracker();
	virtual ~VisionBallTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);

private:
	ParticleList particleList;	// our results data structure
	ParticleList FirstPassParticleList;
};