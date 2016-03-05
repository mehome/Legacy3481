/****************************** Header ******************************\
 *
 *  Created on: 20 Jan 2016
 *      Author: wtason.dylan
 */

#ifndef SRC_AUTON_AUTON_H_
#define SRC_AUTON_AUTON_H_

#include "Config.h"
#include "AHRS.h"

using namespace Configuration;

/*! Interface for autonomous functions to set the left side of the drive train to a value <=1 or >=-1.*/
static inline void setLeftDrive(double valInput){
	Config::Instance()->GetVictor(CommonName::leftDrive_1())->Set(valInput);
	Config::Instance()->GetVictor(CommonName::leftDrive_2())->Set(valInput);
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::leftDrive_3())->Set(valInput);
}

/*! Interface for autonomous functions to set the right side of the drive train to a value <=1 or >=-1.*/
static inline void setRightDrive(double valInput){
	Config::Instance()->GetVictor(CommonName::rightDrive_1())->Set(valInput);
	Config::Instance()->GetVictor(CommonName::rightDrive_2())->Set(valInput);
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::rightDrive_3())->Set(valInput);
}

/*! Interface for autonomous functions to stop the left side of the drive train.*/
static inline void stopLeftDrive(){
	Config::Instance()->GetVictor(CommonName::leftDrive_1())->Stop();
	Config::Instance()->GetVictor(CommonName::leftDrive_2())->Stop();
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::leftDrive_3())->Stop();
}

/*! Interface for autonomous functions to stop the right side of the drive train.*/
static inline void stopRightDrive(){
	Config::Instance()->GetVictor(CommonName::rightDrive_1())->Stop();
	Config::Instance()->GetVictor(CommonName::rightDrive_2())->Stop();
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::rightDrive_3())->Stop();
}

/*! Interface for autonomous functions to stop the entire drive train.*/
static inline void fullStop()
{
	stopRightDrive();
	stopLeftDrive();
}

class Auton
{
private:
	AHRS *ahrs;
	Ultrasonic *ultra;
public:
	/*! Default constructor that also initialises the AHRS class for the NavX.*/
	Auton()
    {
		try { ahrs = new AHRS(SPI::Port::kMXP); }
		catch (std::exception ex )
		{
			std::string err_string = "Error instantiating navX MXP:  ";
			err_string += ex.what();
			DriverStation::ReportError(err_string.c_str());
		}
		ultra = new Ultrasonic(8,9);
		ultra->SetAutomaticMode(true);
    }
	void Start();
};

#endif /* SRC_AUTON_AUTON_H_ */
