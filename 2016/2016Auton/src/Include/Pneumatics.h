/****************************** Header ******************************\
Class Name:  Pneumatics
Summary: 	 Pneumatics loop (compressor functions)
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef SYSTEMS_PNEUMATICS_H_
#define SYSTEMS_PNEUMATICS_H_

#include <WPILib.h>

#include "Config.h"
#include "Preproc.h"

using namespace Configuration;

namespace Systems {

class Pneumatics final
{
public:
	Pneumatics();
	void Resume();
	virtual ~Pneumatics();

#ifdef CRIO
    void Initialize() __attribute__((deprecated(UNBOUNDED)));
	void SuspendInOffPosition()  __attribute__ ((deprecated(LEGACY)));
	void ForceComprssorShutOff() __attribute__ ((deprecated(LEGACY)));
#endif

	static void SetStartGear(){Config::Instance()->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kForward);}

private:
#ifdef CRIO
	bool override = false;
	bool isSuspended = false;
	Relay *compressorRelay = Config::Instance()->GetRelay(CommonName::Compressor());
	DigitalInput *compressorSwitch = Config::Instance()->GetDInput(CommonName::compressorSwitch());
#endif

};

} /* namespace Systems */

#endif /* SYSTEMS_PNEUMATICS_H_ */
