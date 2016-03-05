/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#include <WPILib.h>

#include "Operator.h"
#include "SystemsCollection.h"

namespace Systems {

Operator::Operator()
{
	operatorConfig = &config->_OperatorConfig;
	operator_ = new Joystick(operatorConfig->controllerSlot);
}

void Operator::Initialize()
{
	for(;;)
	{

		//beacon controls
		if(operator_->GetRawButton(operatorConfig->blueModeButton))
			SystemsCollection::Instance().beacon.BlueMode();
		if(operator_->GetRawButton(operatorConfig->redModeButton))
			SystemsCollection::Instance().beacon.RedMode();

		Wait(.005);
	}
}

Operator::~Operator()
{
	// TODO Auto-generated destructor stub
}

} /* namespace Systems */
