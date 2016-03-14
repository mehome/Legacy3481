/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoftware.co
\*********************************************************************/

#include <WPILib.h>

#include "ButtonControl.h"

void ButtonControl::Update()
{
	if(!isPneumatic)
	{
		if(joy->GetRawButton(button_A) && !onlyReverse)
		{
			setVictors(multiplier);
			isRunning = true;
		}
		else if(joy->GetRawButton(button_B) && !onlyForward)
		{
			setVictors(-multiplier);
			isRunning = true;
		}
		else
		{
			setVictors(0);
			isRunning = false;
		}
	}
	else
	{
		if(joy->GetRawButton(button_A))
		{
			setSolenoids(DoubleSolenoid::kForward);
			isRunning = true;
		}
		else if(joy->GetRawButton(button_B))
		{
			setVictors(DoubleSolenoid::kReverse);
			isRunning = true;
		}
		else
			isRunning = false;

	}
}
