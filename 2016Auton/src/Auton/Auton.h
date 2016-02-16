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


static void setLeftDrive(double valInput){
	Config::Instance()->GetVictor(CommonName::leftDrive_1())->Set(valInput);
	Config::Instance()->GetVictor(CommonName::leftDrive_2())->Set(valInput);
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::leftDrive_3())->Set(valInput);
}

static void setRightDrive(double valInput){
	Config::Instance()->GetVictor(CommonName::rightDrive_1())->Set(valInput);
	Config::Instance()->GetVictor(CommonName::rightDrive_2())->Set(valInput);
	if(Config::Instance()->_DrivePower == DrivePower::sixCIM)
		Config::Instance()->GetVictor(CommonName::rightDrive_3())->Set(valInput);
}

class Auton
{
private:
	AHRS *ahrs;
public:
	Auton(){}
	void Start();
};

#endif /* SRC_AUTON_AUTON_H_ */
