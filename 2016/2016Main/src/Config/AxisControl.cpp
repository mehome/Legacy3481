/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoftware.co
\*********************************************************************/

#include "AxisControl.h"
#include "DrivingFits.h"


void AxisControl::Update()
{
	double raw = joy->GetRawAxis(axis);

	if(raw > deadzone || raw < -deadzone)
		setVictors(linearValueEstimator(raw, deadzone)*polarity);
	else
		setVictors(0);

}
