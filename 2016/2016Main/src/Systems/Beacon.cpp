/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoftware.co
\*********************************************************************/

#include "Beacon.h"
#include "Config.h"

namespace Systems {

Beacon::Beacon() { }

void Beacon::Initialize()
{
	for(;;)
	{
		if(standby)
		{
			beaconSignalOff->Set(1);
			beaconSignalOne->Set(1);
			beaconSignalTwo->Set(1);
		}
		else if(ready)
		{
			beaconSignalOff->Set(1);
			beaconSignalOne->Set(0);
			beaconSignalTwo->Set(0);
		}
		else if(redMode)
		{
			beaconSignalOff->Set(1);
			beaconSignalOne->Set(1);
			beaconSignalTwo->Set(0);
		}
		else if(blueMode)
		{
			beaconSignalOff->Set(1);
			beaconSignalOne->Set(0);
			beaconSignalTwo->Set(1);
		}
		else if(error)
		{
			beaconSignalOff->Set(0);
			beaconSignalOne->Set(1);
			beaconSignalTwo->Set(0);
			Wait(.5);
			beaconSignalOff->Set(1);
			Wait(.5);
		}
		Wait(.05);
	}

}

void Beacon::Clear()
{
	ready=false;
    standby=false;
    error=false;
    redMode=false;
    blueMode=false;
}

void Beacon::Ready()
{
	Clear();
	ready=true;
}

void Beacon::Standby()
{
	Clear();
	standby=true;
}

void Beacon::BlueMode()
{
	Clear();
	blueMode=true;
}

void Beacon::RedMode()
{
	Clear();
	redMode=true;
}

void Beacon::Error()
{
	Clear();
	error=true;
}

Beacon::~Beacon() { Standby(); }

} /* namespace Systems */
