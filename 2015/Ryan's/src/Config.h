/*
 * Config.h
 *
 *  Created on: 24 Mar 2015
 *      Author: Ryan
 */

#ifndef RYAN_S_SRC_CONFIG_H_
#define RYAN_S_SRC_CONFIG_H_

class Config {


public:

	    unsigned int rightDrive_1_Value;
		unsigned int rightDrive_2_Value;
		unsigned int leftDrive_1_Value;
		unsigned int leftDrive_2_Value;
		unsigned int kicker_Value;
		unsigned int arm_Value;

		const char* deadZone_Drivel;

		float deadZone_Drive;
		float deadZone_Turning;
		float deadZone_KickerWheel;
		float deadZone_Arm;

		float powerMultiplier_Drive;
		float powerMultiplier_Turning;
		float powerMultiplier_KickerWheel;
		float powerMultiplier_Arm;

		int driveAxis;
		int turnAxis;
		int kickerAxis;
		int armAxis;
		int kickerLeftButton;
		int kickerRightButton;
		int kickerUseAxis;
		int solenoidPositiveFire;
		int solenoidNegativeFire;

		int solenoidPositive;
		int solenoidNegative;

		int upperLimit;
		int lowerLimit;

		int compressorRelay;
		int compressorLimit;

		int drivePolarity;
		int turningPolarity;
		int armPolarity;
		int kickerPolarity;

		int breakMode;

		const char * version;

	Config(const char* Version);
	Config();
	virtual ~Config();
};

#endif /* RYAN_S_SRC_CONFIG_H_ */
