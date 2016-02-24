/****************************** Header ******************************\
Class Name:  Operator
Summary: 	 Operator loop
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef SYSTEMS_OPERATOR_H_
#define SYSTEMS_OPERATOR_H_

#include "Config.h"
#include "Preproc.h"
#include "MPCClient.h"
#include "ConfigStructs.h"

using namespace Configuration;

namespace Systems {

/*! Operator handles all the mechanics of the robots operation, this is where the operators
 * controls are monitored and acted upon.*/
class Operator final : public MPCClient
{
public:
	Operator();//!< Constructor.
	virtual ~Operator(); //!< Destructor.
	void Initialize() __attribute__((deprecated(UNBOUNDED)));//!< Initializes the setup and main loop of the operation system.
	void CorrectionMultiplier(double multiplier);//!< Implements the 'CorrectionMultiplier' pure virtual function required by MPCClient.

private:
	double shooterPowerMultiplyer = 1;
	Joystick *operator_; //!< Pointer to the operators controller.
	OperatorConfig *operatorConfig; //!< Pointer to the operators configuration struct.
	Config *config = Config::Instance(); //!< Pointer to the single instance of the config class.
};

} /* namespace Systems */

#endif /* SYSTEMS_OPERATOR_H_ */
