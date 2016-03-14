/****************************** Header ******************************\
Summary: 	 Contains structures for holding configuration values
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/

#ifndef SRC_CONFIG_CONFIGSTRUCTS_H_
#define SRC_CONFIG_CONFIGSTRUCTS_H_

#include <string>
#include <WPILib.h>

#include "DrivingFits.h"
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
	bool allowC = true;//!< allowC determines whether clockwise rotation is allowed on the motor.
	bool allowCC = true;//!< allowC determines whether counter clockwise rotation is allowed on the motor.
	bool isReversed;//!< If the victor polarity is reversed or not.

public:
	VictorItem(Victor *victor, string name, bool isReversed = false){ this->victor = victor; this->name = name; this->isReversed = isReversed; }//!< Constructor.
	void SetAllowC(bool val){ allowC = val; }//!< Sets allowC value
	void SetAllowCC(bool val){ allowCC = val; }//!< Sets allowCC value
	void Set(double val)
	{
		if(val<0 && allowCC)
		{
			if(isReversed) victor->Set(-val);
			else victor->Set(val);
		}
		else if(val>0 && allowC)
		{
			if(isReversed) victor->Set(-val);
			else victor->Set(val);
		}
		else
			victor->Set(0);

	}//!< Calls the victor's 'Set' function and reverses the polarity if isRevers=true.

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
	DoubleSolenoid::Value GetCurrentState() { return solenoid->Get(); }//!< Returns the current value for this solenoid.
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

/*! Structure to handle analog devices allocated from the config.*/
struct AnalogIItem
{
	AnalogInput *in;//!< Pointer to the analog device.
	string name;//!< Name of the analog device.
};

/*! Structure to handle analog devices allocated from the config.*/
struct AnalogOItem
{
	AnalogOutput *out;//!< Pointer to the analog device.
	string name;//!< Name of the analog device.
};

/*! Structure to handle Relays allocated from the config.*/
struct RelayItem
{
	Relay *relay;//!< Pointer to the relay.
	string name;//!< Name of the relay.
};

struct RobotParameters
{
	double minBatterShootRange, maxBatterShootRange;
};

/*! Structure to store the configuration and control vales for driving the robot.*/
struct DriverConfig
{
			//now loading with defaults, these are overrided by the config
			DriveType type; //!< Defines the type of drive the robot will use.
			DriveFit driveFit; //!< Defines the type of fit the controls should use to keep the axii and deadzones proportional.
			int controllerSlot; //!< Defines the slot the controller is plugged into on the driver station.
			int polynomialFitPower; //!< Defines to what power the polynomial drive fit should use, the default is quadratic.
			double accelerationFactor; //!< Defines the acceleration used to prevent high sticking.
			double leftPowerMultiplier; //!< Defines a multiplier to adjust the power of the left side of the drive train.
			double rightPowerMultiplier;//!< Defines a multiplier to adjust the power of the right side of the drive train.
			int reverse_a;//!< Defines one of the buttons used to invert the drive controls.
			int reverse_b;//!< Defines one of the buttons used to invert the drive controls.

			//Tank values
			int leftAxis;//!< Defines the left axis for tank drive.
			int rightAxis;//!< Defines the right axis for tank drive.
			int leftPolarity;//!< Defines the left polarity for tank drive.
			int rightPolarity;//!< Defines the right polarity for tank drive.
			double leftDeadZone;//!< Defines the left deadzone for tank drive.
			double rightDeadZone;//!< Defines the right deadzon for tank drive.


			//Arcade values
			int arcade_turningAxis;//!< Defines the axis used for turning in arcade drive.
			int arcade_driveAxis;//!< Defines the axis used for driving (x axis, forward and back) in arcade drive.
			int arcade_turningPolarity;//!< Defines the polarity used for turning in arcade drive.
			int arcade_drivePolarity;//!< Defines the polarity used for driving in arcade drive.
			double arcade_turningDeadZone;//!< Defines the deadzone used for turning in arcade drive.
			double arcade_driveDeadZone;//!< Defines the deadzone used for driving in arcade drive.

			//Kicker values
			int kicker_left;//!< Defines the button or axis used for running a kickerwheel left.
			int kicker_right;//!< Defines the button or axis used for running a kickerwheel right.
			int kicker_LeftPolarity;//!< Defines the polarity used for running a kickerwheel left.
			int kicker_RightPolarity;//!< Defines the polarity used for running a kickerwheel right.
			double kicker_RightDeadZone;//!< Defines the deadzone used for running a kickerwheel right.
			double kicker_LeftDeadZone;//!< Defines the deadzone used for running a kickerwheel left.

			//Gear Shifting
			int shiftGear;//!< Defines the button used for shifting gear.

};

/*! Structure to store the configuration and control vales for operating the robot.*/
struct OperatorConfig
{
			int controllerSlot = -1;//!< Defines the slot the controller is plugged into on the driver station.

			//2016
			int intakeInButton = -1;//!< Defines the button used for running the intake inward.
			int intakeOutButton = -1;//!< Defines the button used for running the intake outward.
			int intakeAxis = -1;
			double intakeDeadZone = -1;
			int intakePolarity = -1;//!< Defines the polarity of the intake controls.
			double intakePowerMultiplier = -1;//!< Defines the intake power.

			int indexerInButton = -1;//!< Defines the button used for running the indexer.
			int indexerOutButton = -1;//!< Defines the button used for running the indexer.
			int indexerAxis = -1;
			double indexerDeadZone = -1;
			int indexerPolarity = -1;//!< Defines the polarity used for running the indexer.
			double indexerPowerMultiplier = -1;//!< Defines the power used for running the indexer.

			int shooterInButton = -1;
			int shooterOutButton = -1;
			int shooterAxis = -1;//!< Defines the axis used for running the shooter.
			double shooterDeadZone = -1;
		    int shooterPolarity = -1;//!< Defines the polarity used for running the shooter.
		    double shooterPowerMultiplier = -1;//!< Defines the power used for running the indexer.

			int intakeShift = -1;//!< Defines the button to toggle the intake position.
			int shooterShift = -1;//!< Defines the button to toggle the shooter position.
			int climberShift = -1;//!< Defines the button to toggle the climber position.

			int climberInButton = -1;
			int climberOutButton = -1;
			double climberDeadZone = -1;
			int climberAxis = -1;//!< Defines the button to run the climber upward at constant power.
			int climberPolarity = -1;//!< Defines the polarity used to run the climber.
			double climberPowerMultiplier = -1;//!< Defines the power used to run the climber.

		};
};


#endif /* SRC_CONFIG_CONFIGSTRUCTS_H_ */
