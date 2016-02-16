/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/

#include <WPILib.h>

#include "Drive.h"
#include "Preproc.h"
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

double linearValueEstimator(double, double, double);
double polynomialValueEstimator(double, double, double);
double exponentialValueEstimator(double, double, double);

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
		PTR_TO_MEMBER(*this,d)();
}

void Drive::rev(double input, DriveOrientation orientaion)
{
	double dabs(double in);
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
	REV:
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
	for(int i=0; i<4/2;i++)
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
	for(int i=0; i<4/2;i++)
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
	SetLeftDrive(0);
}

void Drive::KickerStop()
{
	SetKicker(0,0);
}

void Drive::RightStop()
{
	SetRightDrive(0);
}

void Drive::FullStop()
{
	SetLeftDrive(0);
	SetRightDrive(0);
	SetKicker(0,0);
}

void Drive::CheckButtons()
{
	if(driver->GetRawButton(driverConfig.highGear))
		config->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kForward);
	else if(driver->GetRawButton(driverConfig.lowGear))
			config->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kReverse);
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

double Drive::LeftDrivePassthrough(double p_in)
{
	return driveFit(p_in, driverConfig.leftDeadZone, driverConfig.polynomialFitPower);
}

double Drive::RightDrivePassthrough(double p_in)
{
	return driveFit(p_in, driverConfig.rightDeadZone, driverConfig.polynomialFitPower);
}

double dabs(double in)
{
	if(in<0)
	return in=in*-1;
	return in;
}

int sign(double x)
{
	if(x<0)
		return -1;
	else return 1;
}

double linearValueEstimator(double x, double dz, double na=0)
	{
		return ((dabs(x)-dz)*pow((1-dz),-1))*sign(x);
	}

double polynomialValueEstimator(double x, double dz, double power = 2)
	{
			double value = (pow((dabs(x)-dz),power)*pow((1-dz),-power))*sign(x);
			if(value>1)
				return 1;
			else if(value<-1)
				return -1;
			return value;
	}

double exponentialValueEstimator(double x, double dz, double power = 2)
	{
			double value = (pow(pow(2,1/(1-dz)),(dabs(x)-dz))-1)*sign(x);
			if(value>1)
				return 1;
			else if(value<-1)
				return -1;
			return value;
	}

Drive::~Drive() { }

} /* namespace Systems */
