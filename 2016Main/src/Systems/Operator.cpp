/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#include <WPILib.h>

#include "Operator.h"
#include "MPCClient.h"
#include "DrivingFits.h"
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
	double shooterAxis, indexerAxis;
	bool intakeIn, intakeOut, climberUp, climberDown;
	bool intakeShiftStatus = false, climberShiftStatus = false , shooterShiftStatus = false;
	bool p_intakeShift, p_climberShift, p_shooterShift;
	bool intakeShift, climberShift, shooterShift;

	for(;;)
	{
		p_intakeShift = intakeShift;
		p_climberShift = climberShift;
		p_shooterShift = shooterShift;

		indexerAxis = operator_->GetRawAxis(operatorConfig->indexerAxis);
		intakeShift = operator_->GetRawButton(operatorConfig->intakeShift);
		climberShift = operator_->GetRawButton(operatorConfig->climberShift);
		shooterShift = operator_->GetRawButton(operatorConfig->shooterShift);
		shooterAxis = operator_->GetRawAxis(operatorConfig->shooterAxis);
	    intakeIn = operator_->GetRawButton(operatorConfig->intakeInButton);
		intakeOut = operator_->GetRawButton(operatorConfig->intakeOutButton);
		climberUp = operator_->GetRawButton(operatorConfig->climberUpButton);
		climberDown = operator_->GetRawButton(operatorConfig->climberDownButton);


		if(intakeShift && !p_intakeShift)
			intakeShiftStatus = intakeShiftStatus ? false : true;

		if(intakeShiftStatus)
		{
			if(config->GetSolenoid(CommonName::intakeShift())->GetDefaultState() == DoubleSolenoid::kForward)
				config->GetSolenoid(CommonName::intakeShift())->Set(DoubleSolenoid::kReverse);
			else if(config->GetSolenoid(CommonName::intakeShift())->GetDefaultState() == DoubleSolenoid::kReverse)
				config->GetSolenoid(CommonName::intakeShift())->Set(DoubleSolenoid::kForward);
			else
				config->GetSolenoid(CommonName::intakeShift())->Set(DoubleSolenoid::kOff);
		}
		else
			config->GetSolenoid(CommonName::intakeShift())->SetToDefault();

		if(climberShift && !p_climberShift)
			climberShiftStatus = climberShiftStatus ? false : true;

		if(climberShiftStatus)
		{
			if(config->GetSolenoid(CommonName::climberShift())->GetDefaultState() == DoubleSolenoid::kForward)
				config->GetSolenoid(CommonName::climberShift())->Set(DoubleSolenoid::kReverse);
			else if(config->GetSolenoid(CommonName::climberShift())->GetDefaultState() == DoubleSolenoid::kReverse)
				config->GetSolenoid(CommonName::climberShift())->Set(DoubleSolenoid::kForward);
			else
				config->GetSolenoid(CommonName::climberShift())->Set(DoubleSolenoid::kOff);
		}
		else
			config->GetSolenoid(CommonName::climberShift())->SetToDefault();


		if(shooterShift && !p_shooterShift)
			shooterShiftStatus = shooterShiftStatus ? false : true;

		if(shooterShiftStatus)
		{
			if(config->GetSolenoid(CommonName::shooterShift())->GetDefaultState() == DoubleSolenoid::kForward)
				config->GetSolenoid(CommonName::shooterShift())->Set(DoubleSolenoid::kReverse);
			else if(config->GetSolenoid(CommonName::shooterShift())->GetDefaultState() == DoubleSolenoid::kReverse)
				config->GetSolenoid(CommonName::shooterShift())->Set(DoubleSolenoid::kForward);
			else
				config->GetSolenoid(CommonName::shooterShift())->Set(DoubleSolenoid::kOff);
		}
		else
			config->GetSolenoid(CommonName::shooterShift())->SetToDefault();

		if(shooterAxis>.05)
		{
			config->GetVictor(CommonName::shooter_1())->Set(operatorConfig->shooterPolarity*shooterAxis*operatorConfig->shooterPowerMultiplier);
		    config->GetVictor(CommonName::shooter_2())->Set(operatorConfig->shooterPolarity*shooterAxis*operatorConfig->shooterPowerMultiplier);
		}
		else
		{
			config->GetVictor(CommonName::shooter_1())->Stop();
		    config->GetVictor(CommonName::shooter_2())->Stop();
		}

		if(intakeIn)
		{
			config->GetVictor(CommonName::intakeMotor())->Set(operatorConfig->intakePowerMultiplier*operatorConfig->intakePolarity);
			config->GetVictor(CommonName::indexerMotor())->Set(operatorConfig->intakePowerMultiplier*operatorConfig->indexerPolarity);
		}
		else if(intakeOut)
		{
		    config->GetVictor(CommonName::intakeMotor())->Set(-operatorConfig->intakePowerMultiplier*operatorConfig->intakePolarity);
		    config->GetVictor(CommonName::indexerMotor())->Set(-operatorConfig->intakePowerMultiplier*operatorConfig->indexerPolarity);
		}
		else if(indexerAxis>.05)
			config->GetVictor(CommonName::indexerMotor())->Set(operatorConfig->indexerPowerMultiplier*indexerAxis*operatorConfig->indexerPowerMultiplier);
		else
		{
			config->GetVictor(CommonName::intakeMotor())->Stop();
			config->GetVictor(CommonName::indexerMotor())->Stop();
		}

		SmartDashboard::PutNumber("climberVal: ",operatorConfig->climberPowerMultiplier);
		SmartDashboard::PutBoolean("climberDown: ",climberDown);

		if(climberUp)
		{
			config->GetVictor(CommonName::climber_1())->Set(operatorConfig->climberPowerMultiplier*operatorConfig->climberPolarity);
			config->GetVictor(CommonName::climber_2())->Set(operatorConfig->climberPowerMultiplier*operatorConfig->climberPolarity);
		}
		else if(climberDown)
		{
			config->GetVictor(CommonName::climber_1())->Set(-operatorConfig->climberPowerMultiplier*operatorConfig->climberPolarity);
			config->GetVictor(CommonName::climber_2())->Set(-operatorConfig->climberPowerMultiplier*operatorConfig->climberPolarity);
		}
		else
		{
			config->GetVictor(CommonName::climber_1())->Stop();
		    config->GetVictor(CommonName::climber_2())->Stop();
		}


		Wait(.005);
	}
}

Operator::~Operator()
{
	// TODO Auto-generated destructor stub
}

} /* namespace Systems */
