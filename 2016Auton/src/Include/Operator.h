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
#include "ConfigStructs.h"

using namespace Configuration;

namespace Systems {

class Operator final
{
public:
	Operator();
	virtual ~Operator();
	void Initialize() __attribute__((deprecated(UNBOUNDED)));

private:
	Joystick *operator_;
	OperatorConfig *operatorConfig;
	Config *config = Config::Instance();
};

} /* namespace Systems */

#endif /* SYSTEMS_OPERATOR_H_ */
