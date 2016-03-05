/*
 * ConfigStructs.h
 *
 *  Created on: 2 Jan 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_CONFIG_CONFIGSTRUCTS_H_
#define SRC_CONFIG_CONFIGSTRUCTS_H_

#include <string>
#include <WPILib.h>

#include "ConfigEnums.h"

using namespace std;

namespace Configuration
{

struct VictorItem
{
	Victor *victor;
	string name;
};

struct SolenoidItem
{
	DoubleSolenoid *solenoid;
	string name;
};

struct EncoderItem
{
	Encoder *encoder;
	string name;
};

struct DIItem
{
	DigitalInput *in;
	string name;
};

struct DOItem
	{
		DigitalOutput *out;
		string name;
	};

struct RelayItem
{
	Relay *relay;
	string name;
};

struct DriverConfig
{
			//now loading with defaults, these are overrided by the config
			DriveType type = tank;
			DriveFit driveFit = poly;
			int controllerSlot = 1;
			int polynomialFitPower = 2;
			bool breakMode = false;
			double breakTime = 0;
			double breakIntensity = 0;
			double accelerationFactor = .2;
			double leftPowerMultiplier = 1;
			double rightPowerMultiplier = 1;
			int highGear = 8;
			int lowGear = 7;

			//Tank values
			int leftAxis = 1;
			int rightAxis = 5;
			int leftPolarity = 1;
			int rightPolarity = 1;
			double leftDeadZone = .05;
			double rightDeadZone = .05;


			//Arcade values
			int arcade_turningAxis = 0;
			int arcade_driveAxis = 1;
			int arcade_turningPolarity = 1;
			int arcade_drivePolarity = 1;
			double arcade_turningDeadZone = .05;
			double arcade_driveDeadZone = .05;

			//Kicker values
			int kicker_left = 2;
			int kicker_right = 3;
			int kicker_LeftPolarity = 1;
			int kicker_RightPolarity = -1;
			double kicker_RightDeadZone = .05;
			double kicker_LeftDeadZone = .05;
};

struct OperatorConfig
{
			int controllerSlot = 1;

			int redModeButton = 1;
			int blueModeButton = 2;

			//2015
			int armAxis = 0;
			int armPolarity = 1;
			double armDeadZone = .05;
			double armPwerMultiplier= 1;
			int manipulatorButton = 6;
			int wingsButton = 5;

		};
};


#endif /* SRC_CONFIG_CONFIGSTRUCTS_H_ */
