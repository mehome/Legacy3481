#include "stdafx.h"

class VisionBallTracker : public VisionTracker
{
public:
	VisionBallTracker();
	virtual ~VisionBallTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);
};