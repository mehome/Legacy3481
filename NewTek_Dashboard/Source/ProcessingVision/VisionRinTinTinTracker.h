#include "stdafx.h"

class VisionRinTinTinTracker : public VisionTracker
{
public:
	VisionRinTinTinTracker();
	virtual ~VisionRinTinTinTracker();

	int ProcessImage(double &x, double &y);
	void SetDefaultThreshold(void);

private:
	ParticleList particleList;	// our results data structure
};