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
	  /*try
	  {
		   ahrs = new AHRS(SPI::Port::kMXP);
		   out << "Initialised NavX!";
	  }
	  catch (std::exception ex )
	  {
	       std::string err_string = "Error instantiating navX MXP:  ";
	       err_string += ex.what();
	       DriverStation::ReportError(err_string.c_str());
	       out << "Error instantiating navX MXP";
	   }*/

	  mainLoop();
}

void Sensing::mainLoop()
{
	bool toggle = false;
	PowerDistributionPanel pdp;
	float drop = 12.45;

	while(true)
	{

	if(pdp.GetVoltage()<drop)
	{
		if(!toggle)
		{
			drop+=.5;
			SystemsCollection::Instance().drive.SetLowPowerMode(true);
			out << "\n\n***Went into low power mode!***\n\n";
			DriverStation::ReportError("\n\n***Drive went into low power mode!***\n\n");
			toggle = true;
		}
	}
	else
	{
		if(toggle)
		{
			drop-=.5;
			SystemsCollection::Instance().drive.SetLowPowerMode(false);
			out << "\nPower mode restored.\n";
			DriverStation::ReportError("\nPower restored to drive.\n");
			toggle = false;
		}
	}

	Wait(.005);

	}
}


};
