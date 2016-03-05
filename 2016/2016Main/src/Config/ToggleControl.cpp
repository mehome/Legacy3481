/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoftware.co
\*********************************************************************/

#include "ToggleControl.h"

void ToggleControl::Update()
{
	p_action = action;
	action = joy->GetRawButton(button);

	if(action && !p_action)
		status = status ? false : true;

	if(status)
	{
		if(isPneumatic)
			forwardPneumatics();
		else if(isDOI)
			setDigitalOutputs(1);
		else
			runVictors();

		isFirstRun = false;
	}
	else
	{
		if(!isFirstRun)
		{
			if(isPneumatic)
				reversePneumatics();
			else if(isDOI)
				setDigitalOutputs(0);
			else
				stopVictors();
		}
	}
}

void ToggleControl::reversePneumatics()
{

	for(int i=0; i<(int)m_solenoids.size();i++)
	{
		if(m_solenoids[i]->GetCurrentState()==DoubleSolenoid::kOff){}
		else
		m_solenoids[i]->Set(DoubleSolenoid::kReverse);
	}
}

void ToggleControl::forwardPneumatics()
{
	setSolenoids(DoubleSolenoid::kForward);
}

void ToggleControl::runVictors()
{
	setVictors(multiplier);
}

void ToggleControl::stopVictors()
{
	setVictors(0);
}
