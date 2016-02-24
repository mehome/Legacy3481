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

/*! Structure to handle Victors allocated from the config.*/
struct VictorItem
{
private:
	string name;//!< Name of the victor.
	Victor *victor;//!< Pointer to the actual victor.
	bool isReversed;//!< If the victor polarity is reversed or not.
public:
	VictorItem(Victor *victor, string name, bool isReversed = false){ this->victor = victor; this->name = name; this->isReversed = isReversed; }//!< Constructor.
	void Set(double val){ if(isReversed) victor->Set(-val); else victor->Set(val);}//!< Calls the victor's 'Set' function and reverses the polarity if isRevers=true.
	void Stop(){ victor->Set(0); }//!< Calls the victor's 'StopMotor' function.
	void ReverseDirection() { isReversed = true; }//!< Sets isReversed to true.
	void RestoreDirection() { isReversed = false; }//!< Sets isReversed to false.
	string GetName(){ return name; }//!< Get the name of the victor.
	int GetPolarity(){ if(isReversed) return -1; return 1; }//!< Gets the current polarity of the victor, 1 or -1.
	Victor *GetVictorDirect(){ return this->victor; }//!< Returns a pointer to the actual victor.
};

/*! Structure to handle Solenoids allocated from the config.*/
struct SolenoidItem
{
private:
	string name;//!< Name of the solenoid.
	DoubleSolenoid *solenoid;//!< Pointer to the actual solenoid.
	DoubleSolenoid::Value defaultState;//!< The default state of the solenoid, kOff, kReverse, kForward.
public:
	SolenoidItem(DoubleSolenoid *solenoid, string name, DoubleSolenoid::Value defaultState = DoubleSolenoid::kOff)
    	{this->solenoid = solenoid; this->name = name; this->defaultState = defaultState; this->solenoid->Set(defaultState); }//!< Constructor.
	void Set(DoubleSolenoid::Value state){ this->solenoid->Set(state); }//!< Sets the solenoid to the specified value.
	void SetToDefault(){ this->solenoid->Set(defaultState); }//!< Sets the solenoid back to it's default value.
	DoubleSolenoid::Value GetDefaultState(){ return defaultState; }//!< Returns the default value for this solenoid.
	string GetName(){ return name; }//!< Gets the solenoid name.
	DoubleSolenoid *GetSolenoidDirect(){ return solenoid; };//!< Returns the pointer to the actual solenoid.
};

/*! Structure to handle Encoders allocated from the config.*/
struct EncoderItem
{
	Encoder *encoder;//!< Pointer to the encoder.
	string name;//!< Name of the encoder.
};

/*! Structure to handle Didital Inputs allocated from the config.*/
struct DIItem
{
	DigitalInput *in;//!< Pointer to the digital input.
	string name;//!< Name of the digital input item.
};

/*! Structure to handle Digital Outputs allocated from the config.*/
struct DOItem
	{
		DigitalOutput *out;//!< Pointer to the digital output.
		string name;//!< Name of the digital output item.
	};

/*! Structure to handle Relays allocated from the config.*/
struct RelayItem
{
	Relay *relay;//!< Pointer to the relay.
	string name;//!< Name of the relay.
};

/*! Structure to store the configuration and control vales for driving the robot.*/
struct DriverConfig
{
			//now loading with defaults, these are overrided by the config
			DriveType type = tank; //!< Defines the type of drive the robot will use.
			DriveFit driveFit = poly; //!< Defines the type of fit the controls should use to keep the axii and deadzones proportional.
			int controllerSlot = 1; //!< Defines the slot the controller is plugged into on the driver station.
			int polynomialFitPower = 2; //!< Defines to what power the polynomial drive fit should use, the default is quadratic.
			double accelerationFactor = .2; //!< Defines the acceleration used to prevent high sticking.
			double leftPowerMultiplier = 1; //!< Defines a multiplier to adjust the power of the left side of the drive train.
			double rightPowerMultiplier = 1;//!< Defines a multiplier to adjust the power of the right side of the drive train.
			int reverse_a = 5;//!< Defines one of the buttons used to invert the drive controls.
			int reverse_b = 6;//!< Defines one of the buttons used to invert the drive controls.

			//Tank values
			int leftAxis = 1;//!< Defines the left axis for tank drive.
			int rightAxis = 5;//!< Defines the right axis for tank drive.
			int leftPolarity = 1;//!< Defines the left polarity for tank drive.
			int rightPolarity = 1;//!< Defines the right polarity for tank drive.
			double leftDeadZone = .05;//!< Defines the left deadzone for tank drive.
			double rightDeadZone = .05;//!< Defines the right deadzon for tank drive.


			//Arcade values
			int arcade_turningAxis = 0;//!< Defines the axis used for turning in arcade drive.
			int arcade_driveAxis = 1;//!< Defines the axis used for driving (x axis, forward and back) in arcade drive.
			int arcade_turningPolarity = 1;//!< Defines the polarity used for turning in arcade drive.
			int arcade_drivePolarity = 1;//!< Defines the polarity used for driving in arcade drive.
			double arcade_turningDeadZone = .05;//!< Defines the deadzone used for turning in arcade drive.
			double arcade_driveDeadZone = .05;//!< Defines the deadzone used for driving in arcade drive.

			//Kicker values
			int kicker_left = 2;//!< Defines the button or axis used for running a kickerwheel left.
			int kicker_right = 3;//!< Defines the button or axis used for running a kickerwheel right.
			int kicker_LeftPolarity = 1;//!< Defines the polarity used for running a kickerwheel left.
			int kicker_RightPolarity = -1;//!< Defines the polarity used for running a kickerwheel right.
			double kicker_RightDeadZone = .05;//!< Defines the deadzone used for running a kickerwheel right.
			double kicker_LeftDeadZone = .05;//!< Defines the deadzone used for running a kickerwheel left.

			//Gear Shifting
			int shift_Hight = 8;//!< Defines the button used for shifting into high gear.
			int shift_Low = 7;//!< Defines the button used for shifting into low gear.
};

/*! Structure to store the configuration and control vales for operating the robot.*/
struct OperatorConfig
{
			int controllerSlot = 1;//!< Defines the slot the controller is plugged into on the driver station.

			//2016
			int intakeInButton = 1;//!< Defines the button used for running the intake inward.
			int intakeOutButton = 1;//!< Defines the button used for running the intake outward.
			int intakePolarity = 1;//!< Defines the polarity of the intake controls.
			double intakePowerMultiplier = .75;//!< Defines the intake power.

			int indexerAxis = 3;//!< Defines the axis used for running the indexer.
			int indexerPolarity = 1;//!< Defines the polarity used for running the indexer.
			double indexerPowerMultiplier = .75;//!< Defines the power used for running the indexer.

			int shooterAxis = 2;//!< Defines the axis used for running the shooter.
		    int shooterPolarity = 1;//!< Defines the polarity used for running the shooter.
		    double shooterPowerMultiplier = .75;//!< Defines the power used for running the indexer.

			int intakeShift = 7;//!< Defines the button to toggle the intake position.
			int shooterShift = 8;//!< Defines the button to toggle the shooter position.
			int climberShift = 6;//!< Defines the button to toggle the climber position.

			int climberUpButton = 4;//!< Defines the button to run the climber upward at constant power.
			int climberDownButton = 1;//!< Defines the button to run the climber downward at constant power.
			int climberPolarity = 1;//!< Defines the polarity used to run the climber.
			double climberPowerMultiplier = 1;//!< Defines the power used to run the climber.
		};
};


#endif /* SRC_CONFIG_CONFIGSTRUCTS_H_ */
