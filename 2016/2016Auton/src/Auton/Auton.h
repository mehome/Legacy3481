/****************************** Header ******************************\
 *
 *  Created on: 20 Jan 2016
 *      Author: wtason.dylan
 */

#ifndef SRC_AUTON_AUTON_H_
#define SRC_AUTON_AUTON_H_

#include "Config.h"
#include "AHRS.h"
#include "XDefense.h"
#include "YDefense.h"
#include "ZDefense.h"
#include "BaseDefense.h"

using namespace Configuration;

static inline void setLeftDrive(double valInput){
	Config::Instance()->GetVictor(CommonName::leftDrive_1())->Set(valInput);
	Config::Instance()->GetVictor(CommonName::leftDrive_2())->Set(valInput);
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::leftDrive_3())->Set(valInput);
}

static inline void setRightDrive(double valInput){
	Config::Instance()->GetVictor(CommonName::rightDrive_1())->Set(valInput);
	Config::Instance()->GetVictor(CommonName::rightDrive_2())->Set(valInput);
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::rightDrive_3())->Set(valInput);
}

static inline void stopLeftDrive(){
	Config::Instance()->GetVictor(CommonName::leftDrive_1())->StopMotor();
	Config::Instance()->GetVictor(CommonName::leftDrive_2())->StopMotor();
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::leftDrive_3())->StopMotor();
}

static inline void stopRightDrive(){
	Config::Instance()->GetVictor(CommonName::rightDrive_1())->StopMotor();
	Config::Instance()->GetVictor(CommonName::rightDrive_2())->StopMotor();
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::rightDrive_3())->StopMotor();
}

static inline void fullStop()
{
	stopRightDrive();
	stopLeftDrive();
}

class Auton{

public:
		AHRS *ahrs;
		Ultrasonic *ultra;

		XDefense *xDefense;
		YDefense *yDefense;
		ZDefense *zDefense;

	Auton()
    {
		try { ahrs = new AHRS(SPI::Port::kMXP); }
		catch (...)
		{
			std::string err_string = "Error instantiating navX MXP:  ";
			DriverStation::ReportError(err_string.c_str());
		}

		xDefense = new XDefense();
		yDefense = new YDefense();
		zDefense = new ZDefense();

		// Ports for Ultrasonic (InputWire, OutputWire)
		ultra = new Ultrasonic(8,9);
		ultra->SetAutomaticMode(true);
		}

	void Start();
};

#endif /* SRC_AUTON_AUTON_H_ */
