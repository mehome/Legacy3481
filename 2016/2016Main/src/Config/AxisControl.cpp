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
	{
		if(raw > deadzone && !onlyReverse)
			setVictors(linearValueEstimator(raw, deadzone)*polarity);
		else if(raw < -deadzone && !onlyForward)
			setVictors(linearValueEstimator(raw, deadzone)*polarity);
		isRunning = true;
	}
	else
	{
		setVictors(0);
		isRunning = false;
	}

}
