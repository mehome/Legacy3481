#include "stdafx.h"

class VisionRinTinTinTracker : public VisionTracker
{
public:
	VisionRinTinTinTracker();
	VisionRinTinTinTracker( bool use_color_treshold );
	virtual ~VisionRinTinTinTracker();

	int ProcessImage(double &x, double &y);
};