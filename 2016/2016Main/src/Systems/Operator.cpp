/****************************** Header ******************************\
Author(s):	Ryan Cooper, Dylan Watson, Sophie He
Email:	cooper.ryan@centaurisoft.org, dylantrwatson@gmail.com,
		binru.he@centaurisoftware.co
\*********************************************************************/

#include <WPILib.h>

#include "Operator.h"
#include "MPCClient.h"
#include "LoopChecks.h"
#include "DrivingFits.h"
#include "ButtonControl.h"
#include "SystemsCollection.h"

namespace Systems {

Operator::Operator()
{
	SingleEncoder = true;
	SingleTarget = config->GetEncoder(CommonName::shooterEncoder());
	operatorConfig = &config->_OperatorConfig;
	operator_ = new Joystick(operatorConfig->controllerSlot);
}

void Operator::CorrectionMultiplier(double multiplier)
{
	shooterPowerMultiplyer = multiplier;
}

void Operator::Initialize()
{
	config->GetControlItem(ControlName::indexer())->SetController(operator_);
	config->GetControlItem(ControlName::intake())->SetController(operator_);
	config->GetControlItem(ControlName::climber())->SetController(operator_);
	config->GetControlItem(ControlName::shooter())->SetController(operator_);
	config->GetControlItem(ControlName::intakeShift())->SetController(operator_);
	config->GetControlItem(ControlName::climberShift())->SetController(operator_);
	config->GetControlItem(ControlName::shooterShift())->SetController(operator_);

	while(_IsTeleoporated())
	{
		if(config->GetAnalogInputDevice(CommonName::IntakePressurePad())->GetAverageVoltage() < 2 ||
				config->GetControlItem(ControlName::shooter())->IsRunning())
		{
			config->GetControlItem(ControlName::indexer())->SetOnlyForward(false);
			config->GetControlItem(ControlName::intake())->SetOnlyForward(false);
			config->GetControlItem(ControlName::indexer())->Update();
			config->GetControlItem(ControlName::intake())->Update();
		}
		else
		{
			config->GetControlItem(ControlName::indexer())->SetOnlyForward(true);
			config->GetControlItem(ControlName::intake())->SetOnlyForward(true);
			config->GetControlItem(ControlName::indexer())->Update();
			config->GetControlItem(ControlName::intake())->Update();
		}

		config->GetControlItem(ControlName::climber())->Update();
		config->GetControlItem(ControlName::shooter())->Update();
		config->GetControlItem(ControlName::intakeShift())->Update();
		config->GetControlItem(ControlName::climberShift())->Update();
		config->GetControlItem(ControlName::shooterShift())->Update();
		Wait(.01);
	}
}

Operator::~Operator()
{
	delete operator_;
	delete operatorConfig;
	delete config;
}

} /* namespace Systems */
