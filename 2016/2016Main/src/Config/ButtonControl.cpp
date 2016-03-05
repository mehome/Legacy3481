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
		if(joy->GetRawButton(button_A))
			setVictors(multiplier);
		else if(joy->GetRawButton(button_B))
			setVictors(-multiplier);
		else setVictors(0);
	}
	else
	{
		if(joy->GetRawButton(button_A))
		{
			setSolenoids(DoubleSolenoid::kForward);
		}
		else if(joy->GetRawButton(button_B))
		{
			setVictors(DoubleSolenoid::kReverse);
		}

	}
}
