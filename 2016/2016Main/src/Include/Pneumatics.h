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

/*! The pneumatics class handles all related tasks for running the pneumatics system.
 * This class is deprecated as pneumatics is all automated with the roborio. This class
 * now just holds static functions for pneumatic related tasks.*/
class Pneumatics final
{
public:
	//WARNING: This function is no longer required at startup,
	//because default positions for solenoids can be set in the configuration file.
	static void SetStartGear()__attribute__((deprecated(DEPRICATED))){Config::Instance()->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kForward);}//!< Sets the drive gears to the start default position.
};

} /* namespace Systems */

#endif /* SYSTEMS_PNEUMATICS_H_ */
