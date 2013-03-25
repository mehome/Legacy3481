#include "stdafx.h"

class VisionGoalTracker : public VisionTracker
{
public:
	VisionGoalTracker();
	virtual ~VisionGoalTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);
	void Set3PtGoalAspect(bool);
};