/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#include "Pneumatics.h"
#include "Config.h"

namespace Systems {

Pneumatics::Pneumatics() { }

#ifdef CRIO
void Pneumatics::Initialize()
{

	Config::Instance()->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kForward);

	override = false;

	for(;;)
	{


		if(compressorSwitch->Get()==0 && !override && !isSuspended)
			compressorRelay->Set(Relay::kForward);
		else if(override)
		{
			compressorRelay->Set(Relay::kOff);
			return;//terminate unused loop
		}
		else
			compressorRelay->Set(Relay::kOff);
	}
}
#endif

#ifdef CRIO
void Pneumatics::ForceComprssorShutOff()
{
	override = true;
	cout << "\nThe compressor was forcibly shut off" << endl;
}

void Pneumatics::SuspendInOffPosition()
{
	isSuspended = true;
}

void Pneumatics::Resume()
{
	isSuspended = false;
}
#endif

Pneumatics::~Pneumatics() { }

} /* namespace Systems */
