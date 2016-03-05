/*
 * BaseDefense.h
 *
 *  Created on: Feb 22, 2016
 *      Author: Lucas Romier
 */

#ifndef SRC_AUTON_DEFENSES_BASEDEFENSE_H_
#define SRC_AUTON_DEFENSES_BASEDEFENSE_H_

#include "AHRS.h"
#include <WPILib.h>

class BaseDefense{
public:
	BaseDefense(){
		try { ahrs = new AHRS(SPI::Port::kMXP); }
				catch (...)
				{
					std::string err_string = "Error instantiating navX MXP:  ";
					DriverStation::ReportError(err_string.c_str());
				}
		ultra = new Ultrasonic(8,9);
		ultra->SetAutomaticMode(true);
	};
	virtual ~BaseDefense();
	virtual void Start() = 0;
protected:
	AHRS *ahrs;
	Ultrasonic *ultra;
};

#endif /* SRC_AUTON_DEFENSES_BASEDEFENSE_H_ */
