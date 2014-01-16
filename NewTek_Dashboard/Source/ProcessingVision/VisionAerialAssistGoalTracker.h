#include "stdafx.h"

class VisionAerialAssistGoalTracker : public VisionTracker
{
public:
	VisionAerialAssistGoalTracker();
	virtual ~VisionAerialAssistGoalTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);
	void Set3PtGoalAspect(bool);
};