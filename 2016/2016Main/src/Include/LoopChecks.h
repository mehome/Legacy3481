/*
 * LoopChecks.h
 *
 *  Created on: 14 Mar 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_INCLUDE_LOOPCHECKS_H_
#define SRC_INCLUDE_LOOPCHECKS_H_

#include <WPILib.h>

inline static bool _IsAutononomous()
{
	return DriverStation::GetInstance().IsAutonomous() && DriverStation::GetInstance().IsEnabled();
}

inline static bool _IsTeleoporated()
{
	return DriverStation::GetInstance().IsOperatorControl() && DriverStation::GetInstance().IsEnabled();
}


#endif /* SRC_INCLUDE_LOOPCHECKS_H_ */
