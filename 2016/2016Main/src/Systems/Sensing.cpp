/*
 * Sensing.cpp
 *
 *  Created on: 3 Jan 2016
 *      Author: cooper.ryan
 */

#include <WPILib.h>

#include "Sensing.h"
#include "out.h"
#include "SystemsCollection.h"

namespace Systems {

void Sensing::Initialize()
{
	  mainLoop();
}

void Sensing::mainLoop()
{
	bool toggle = false;
	PowerDistributionPanel pdp;

	while(true)
	{

	if(pdp.GetVoltage()<9)
	{
		if(!toggle)
		{
			SystemsCollection::Instance().drive->SetLowPowerMode(true);
			out << "\n\n***Went into low power mode!***\n\n";
			DriverStation::ReportError("\n\n***Drive went into low power mode!***\n\n");
			toggle = true;
		}
	}
	else
	{
		if(toggle)
		{
			SystemsCollection::Instance().drive->SetLowPowerMode(false);
			out << "\nPower mode restored.\n";
			DriverStation::ReportError("\nPower restored to drive.\n");
			toggle = false;
		}
	}

	Wait(.005);

	}
}


};
