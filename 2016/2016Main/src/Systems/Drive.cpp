/****************************** Header ******************************\
Author(s):	Ryan Cooper, Dylan Watson, Sophie He
Email:	cooper.ryan@centaurisoft.org, dylantrwatson@gmail.com,
		binru.he@centaurisoftware.co
\*********************************************************************/

#include <WPILib.h>

#include "Drive.h"
#include "Preproc.h"
#include "DrivingFits.h"
#include "ControlItem.h"
#include "ConfigEnums.h"
#include "ConfigStructs.h"

namespace Systems {

Drive::Drive() {
	drivePower = config->_DrivePower;
	driverConfig = config->_DriverConfig;

switch(drivePower){
case DrivePower::fourCIM:
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_1()));
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_2()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_1()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_2()));
	break;
case DrivePower::fiveCIM:
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_1()));
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_2()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_1()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_2()));
	kickerWheel = config->GetVictor(CommonName::kickerWheel());
	break;
case DrivePower::sixCIM:
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_1()));
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_2()));
	leftDriveVictors.push_back(config->GetVictor(CommonName::leftDrive_3()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_1()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_2()));
	rightDriveVictors.push_back(config->GetVictor(CommonName::rightDrive_3()));
	break;
}

driver = new Joystick(driverConfig.controllerSlot);
config->GetControlItem(ControlName::gearShift())->SetController(driver);

	if(driverConfig.driveFit==DriveFit::linear)
		driveFit=linearValueEstimator;
	else if(driverConfig.driveFit==DriveFit::exponential)
		driveFit=exponentialValueEstimator;
	else
		driveFit = polynomialValueEstimator;
}

void Drive::Initialize()
{
	drive d;
	if(driverConfig.type==DriveType::tank)
		d=&Drive::Tank;
	else
		d=&Drive::Arcade;

	for(;;)
	{
		p_invert = invert;
		invert = (driver->GetRawButton(driverConfig.reverse_a) && driver->GetRawButton(driverConfig.reverse_b));

		if(invert && !p_invert)
			invertCheck = invertCheck ? false : true;

		PTR_TO_MEMBER(*this,d)();

		Wait(.005);
	}
}

//high-stick prevention assigned to Dylan, this works, but is nasty.
void Drive::rev(double input, DriveOrientation orientaion)
{
	set s;

#pragma GCC diagnostic ignored "-Wswitch"
	switch(orientaion)
			{
			case DriveOrientation::left:
			s=&Drive::SetLeftDrive;
			revPtr=&revValLeft;
			break;
			case DriveOrientation::right:
			s=&Drive::SetRightDrive;
			revPtr=&revValRight;
			break;
			}

	int plyer=1;
	if(input<0)
		plyer=-1;
	input = dabs(input);

	double accel=driverConfig.accelerationFactor;
	REV://? forgive me.
	accel+=accel/2;
	if((*revPtr)<input)
	{
		if(orientaion==DriveOrientation::ALL)
		{
			SetLeftDrive((*revPtr)*plyer, false);
			SetRightDrive((*revPtr)*plyer, false);
		}
		else
		PTR_TO_MEMBER(*this,s)((*revPtr)*plyer, false);
		(*revPtr)+=accel;
		Wait(.05);
		goto REV;
	}
	else
		return;
}

void Drive::SetLeftDrive(double val, bool rev)
{

	double pw=1;

	if(isLowPowerMode)
		pw=.55;

	if(rev)
	this->rev(val, DriveOrientation::left);
	if(val>.96)
		val=1;
	else if(val<-.96)
		val=-1;

	SmartDashboard::PutNumber("Left value:",(val)*driverConfig.leftPowerMultiplier*pw);
	for(unsigned int i=0; i<leftDriveVictors.size();i++)
		(*leftDriveVictors[i]).Set((val)*driverConfig.leftPowerMultiplier*pw);
}

void Drive::SetRightDrive(double val, bool rev)
{
	double pw=1;

	if(isLowPowerMode)
		pw=.55;


	if(rev)
	this->rev(val,  DriveOrientation::right);

	if(val>.96)
		val=1;
	else if(val<-.96)
		val=-1;

	SmartDashboard::PutNumber("Right value:",(val)*driverConfig.rightPowerMultiplier*pw);
	for(unsigned int i=0; i<rightDriveVictors.size();i++)
		(*rightDriveVictors[i]).Set((val)*driverConfig.rightPowerMultiplier*pw);
}

void Drive::SetKicker(double val, int pol)
{
	if(val>.96)
		val=1;
	else if(val<-.96)
		val=-1;
		(*kickerWheel).Set((val)*pol);
}

void Drive::LeftStop()
{
	for(unsigned int i=0; i<leftDriveVictors.size();i++)
		(*leftDriveVictors[i]).Stop();
}

void Drive::RightStop()
{
	for(unsigned int i=0; i<rightDriveVictors.size();i++)
			(*rightDriveVictors[i]).Stop();
}

void Drive::KickerStop()
{
	(*kickerWheel).Stop();
}

void Drive::FullStop()
{
	LeftStop();
	RightStop();
	KickerStop();
}

void Drive::CheckButtons()
{

	config->GetControlItem(ControlName::gearShift())->Update();

	/*if(invertCheck)
	{
		if(flipper)
		{
			int tmp = driverConfig.leftAxis;
			driverConfig.leftAxis = driverConfig.rightAxis;
			driverConfig.rightAxis = tmp;
			flipper = false;

			for(unsigned int i=0; i<leftDriveVictors.size();i++)
				(*leftDriveVictors[i]).ReverseDirection();

			for(unsigned i=0; i<rightDriveVictors.size();i++)
				(*rightDriveVictors[i]).ReverseDirection();
		}
	}
	else
	{
		if(!flipper)
		{
			int tmp = driverConfig.leftAxis;
			driverConfig.leftAxis = driverConfig.rightAxis;
			driverConfig.rightAxis = tmp;
			flipper = true;

			for(unsigned int i=0; i<leftDriveVictors.size();i++)
				(*leftDriveVictors[i]).RestoreDirection();

			for(unsigned i=0; i<rightDriveVictors.size();i++)
				(*rightDriveVictors[i]).RestoreDirection();
		}
	}*/

}

void Drive::Kicker()
{
	if((driver->GetRawAxis(driverConfig.kicker_left) > driverConfig.kicker_LeftDeadZone))
		SetKicker(-driveFit(driver->GetRawAxis(driverConfig.kicker_left),driverConfig.kicker_LeftDeadZone, driverConfig.polynomialFitPower), driverConfig.kicker_LeftPolarity);
	else if ((driver->GetRawAxis(driverConfig.kicker_right) > driverConfig.kicker_RightDeadZone))
		SetKicker(-driveFit(driver->GetRawAxis(driverConfig.kicker_right),driverConfig.kicker_RightDeadZone, driverConfig.polynomialFitPower), driverConfig.kicker_RightPolarity);
	else
		KickerStop();
}

void Drive::Tank()
{
	if((driver->GetRawAxis(driverConfig.leftAxis) > driverConfig.leftDeadZone))
			SetLeftDrive(-driveFit(driver->GetRawAxis(driverConfig.leftAxis),driverConfig.leftDeadZone, driverConfig.polynomialFitPower));
	else if(driver->GetRawAxis(driverConfig.leftAxis) < -driverConfig.leftDeadZone)
		SetLeftDrive(-driveFit(driver->GetRawAxis(driverConfig.leftAxis),driverConfig.leftDeadZone, driverConfig.polynomialFitPower));
	else
	{
			revValLeft=0;
			LeftStop();
	}

	if((driver->GetRawAxis(driverConfig.rightAxis) > driverConfig.rightDeadZone))
		SetRightDrive(driveFit(driver->GetRawAxis(driverConfig.rightAxis),driverConfig.rightDeadZone, driverConfig.polynomialFitPower));
	else if(driver->GetRawAxis(driverConfig.rightAxis) < -driverConfig.rightDeadZone)
		SetRightDrive(driveFit(driver->GetRawAxis(driverConfig.rightAxis),driverConfig.rightDeadZone, driverConfig.polynomialFitPower));
	else
	{
		revValRight=0;
		RightStop();
	}

	if(drivePower==DrivePower::fiveCIM)
		Kicker();
	CheckButtons();
}

void Drive::Arcade()
{
	if((driver->GetRawAxis(driverConfig.arcade_driveAxis) > driverConfig.arcade_driveDeadZone) || (driver->GetRawAxis(driverConfig.arcade_driveAxis) < -driverConfig.arcade_driveDeadZone)
			|| (driver->GetRawAxis(driverConfig.arcade_turningAxis) > driverConfig.arcade_turningDeadZone) || (driver->GetRawAxis(driverConfig.arcade_turningAxis) < -driverConfig.arcade_turningDeadZone))
	{
		SetLeftDrive(-driveFit(driver->GetRawAxis(driverConfig.arcade_driveAxis)+driver->GetRawAxis(driverConfig.arcade_turningAxis),driverConfig.leftDeadZone, driverConfig.polynomialFitPower));
		SetRightDrive(driveFit(driver->GetRawAxis(driverConfig.arcade_driveAxis)-driver->GetRawAxis(driverConfig.arcade_turningAxis),driverConfig.rightDeadZone, driverConfig.polynomialFitPower));
	}
	else
		FullStop();
	if(drivePower==DrivePower::fiveCIM)
		Kicker();
	CheckButtons();
}

/*double Drive::LeftDrivePassthrough(double p_in)
{
	return driveFit(p_in, driverConfig.leftDeadZone, driverConfig.polynomialFitPower);
}

double Drive::RightDrivePassthrough(double p_in)
{
	return driveFit(p_in, driverConfig.rightDeadZone, driverConfig.polynomialFitPower);
}*/

Drive::~Drive()
{
	delete driver;

	for(unsigned int i=0; i<leftDriveVictors.size();i++)
			delete leftDriveVictors[i];

	for(unsigned int i=0; i<rightDriveVictors.size();i++)
				delete rightDriveVictors[i];

	delete kickerWheel;
	delete config;
}

} /* namespace Systems */
