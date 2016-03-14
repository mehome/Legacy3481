/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoftware.co
\*********************************************************************/

#include <WPILib.h>

#include "out.h"
#include "Config.h"
#include "ExTimer.h"
#include "Sensing.h"
#include "CommonName.h"
#include "SystemsCollection.h"

namespace Systems {

void Sensing::Initialize()
{
	  mainLoop();
}

void Sensing::mainLoop()
{
	double range;
	bool toggle = false;
	ExTimer voltageRegTimer(5);
	PowerDistributionPanel pdp;

	double max = Config::Instance()->_RobotParameters.maxBatterShootRange;
	double min = Config::Instance()->_RobotParameters.minBatterShootRange;

	//ultra = new Ultrasonic(Config::Instance()->GetDOutput(CommonName::UltraSonicOutput()),
						//	Config::Instance()->GetDInput(CommonName::UltraSonicInput()));

	//ultra->SetAutomaticMode(true);

	while(true)
	{

		if(ledRingEnabled)
			Config::Instance()->GetDOutput(CommonName::VideoLEDRelay())->Set(1);
		else
			Config::Instance()->GetDOutput(CommonName::VideoLEDRelay())->Set(0);

		if(Config::Instance()->GetControlItem(ControlName::shooter())->IsRunning())
			ledRingEnabled = true;
		else
			ledRingEnabled = false;

/*		range = ultra->GetRangeMM();

		if(range < max && range > min && !Config::Instance()->)
		{
			SystemsCollection::Instance().beacon->Ready();
		}
		else if(range < max && range > min)
		{
			SystemsCollection::Instance().beacon->BlueMode();
		}
		else
			SystemsCollection::Instance().beacon->RedMode();
*/


		if(!Config::Instance()->GetDInput(CommonName::ClimberLimitUp())->Get())
		{
			Config::Instance()->GetVictor(CommonName::climber_1())->SetAllowC(false);
			Config::Instance()->GetVictor(CommonName::climber_2())->SetAllowC(false);
		}
		else
		{
			Config::Instance()->GetVictor(CommonName::climber_1())->SetAllowC(true);
			Config::Instance()->GetVictor(CommonName::climber_2())->SetAllowC(true);
		}

		if(!Config::Instance()->GetDInput(CommonName::ClimberLimitDown())->Get())
		{
			Config::Instance()->GetVictor(CommonName::climber_1())->SetAllowCC(false);
			Config::Instance()->GetVictor(CommonName::climber_2())->SetAllowCC(false);
		}
		else
		{
			Config::Instance()->GetVictor(CommonName::climber_1())->SetAllowCC(true);
			Config::Instance()->GetVictor(CommonName::climber_2())->SetAllowCC(true);
		}

	if(pdp.GetVoltage()<7.5)
	{
		if(!toggle && !voltageRegTimer.IsRunning())
		{
			SystemsCollection::Instance().drive->SetLowPowerMode(true);
			out << "\n\n***Went into low power mode!***\n\n";
			DriverStation::ReportError("\n\n***Drive went into low power mode!***\n\n");
			toggle = true;
			voltageRegTimer.Start();
		}
	}
	else
	{
		if(toggle && voltageRegTimer.HasExpired())
		{
			SystemsCollection::Instance().drive->SetLowPowerMode(false);
			out << "\nPower mode restored.\n";
			DriverStation::ReportError("\nPower restored to drive.\n");
			toggle = false;
			voltageRegTimer.Renew();
		}
	}

	Wait(.005);

	}
}


};
