#include "WPILib.h"
#include "tinyxml.h"
#include "Config.h"
#include <math.h>
#include "SmartDashboard/SmartDashboard.h"
/****************************** Class Header ******************************\
Class Name:  Robot
 Project:      BroncBotz 3481 2015-World Code
 Copyright (c) BroncBotz.
 All rights reserved.

 Author:	Ryan Cooper
 Email:	cooper.ryan@centaurisoft.org

 Summary: Default class, assigns Vectors888s, Controllers, and Updating
 \***************************************************************************/

class Robot: public SampleRobot {

	//Version identifier for code
	const char* Version = "1.8";
	float breakIntensity;
	Config* _config;

	int auton;
	//all components
	Config configuration;
	Joystick _driver;
	Joystick _operator;

	Victor rightDrive_1;
	Victor rightDrive_2;
	Victor leftDrive_1;
	Victor leftDrive_2;
	Victor kicker;
	Victor arm;

	DigitalInput upperLimit;
	DigitalInput lowerLimit;

	DoubleSolenoid doubleSolenoid;
	DoubleSolenoid gearShift;

	Relay compressor;

	//Encoder rightEncoder;
	//Encoder leftEncoder;

	// update every 0.005 seconds/5 milliseconds.
	double kUpdatePeriod = 0.005;

public:
	Robot() :

			//gets configuration from XML file
			configuration(), _driver(0), // Initialize driver on port 0.
			_operator(1), //initialize operator on 1

			//assign locations of each component from xml
			rightDrive_1(configuration.rightDrive_1_Value), rightDrive_2(
					configuration.rightDrive_2_Value), leftDrive_1(
					configuration.leftDrive_1_Value), leftDrive_2(
					configuration.leftDrive_2_Value), kicker(
					configuration.kicker_Value), arm(configuration.arm_Value),

			upperLimit(configuration.upperLimit), lowerLimit(
					configuration.lowerLimit),

			doubleSolenoid(configuration.solenoidPositive,
					configuration.solenoidNegative), gearShift(1, 0), compressor(
					configuration.compressorRelay)

	//	rightEncoder(6, false),
	//	leftEncoder(4, true)

	{
	}

	void RobotInit() {
		_config = new Config(Version);
		SmartDashboard::PutString("Code Version: ", Version);
		SmartDashboard::PutString("XML Version: ", _config->version);
		SmartDashboard::PutNumber("Auton#: ", 0);
	}

	void Autonomous() {

		if (SmartDashboard::GetNumber("Auton#: ") == 1) {
			kicker.SetSafetyEnabled(false);
			kicker.Set(.75);
			Wait(2.0);
			kicker.Set(0);
		}

		else if (SmartDashboard::GetNumber("Auton#: ") == 2) {
			kicker.SetSafetyEnabled(false);
			kicker.Set(-.75);
			Wait(2.0);
			kicker.Set(0);
		}

		else if (SmartDashboard::GetNumber("Auton#: ") == 3) {
			rightDrive_1.Set(.5);
			rightDrive_2.Set(.5);
			leftDrive_1.Set(-.5);
			leftDrive_2.Set(-.5);
			Wait(1.0);
			rightDrive_1.Set(0);
			rightDrive_2.Set(0);
			leftDrive_1.Set(0);
			leftDrive_2.Set(0);
		}

	}

	/**
	 * Controls
	 */
	void OperatorControl() {

		_config = new Config(Version);
		breakIntensity = _config->breakIntensity;
		SmartDashboard::PutString("XML Version: ", _config->version);
		SmartDashboard::PutNumber("Auton#: ", 0);
		gearShift.Set(DoubleSolenoid::kForward);
		compressor.Set(Relay::kOn);

		float rev = 0;
		float count = 0;

		while (IsOperatorControl() && IsEnabled()) {

			SmartDashboard::PutNumber("Drive: ",
					_driver.GetRawAxis(_config->driveAxis));
			SmartDashboard::PutNumber("Count: ", count);
			SmartDashboard::PutNumber("Right_1: ", rightDrive_1.Get());
			SmartDashboard::PutNumber("Right_2: ", rightDrive_2.Get());
			SmartDashboard::PutNumber("Left_1: ", leftDrive_1.Get());
			SmartDashboard::PutNumber("Left_2: ", leftDrive_1.Get());
			SmartDashboard::PutBoolean("  Lower Limit", lowerLimit.Get());
			SmartDashboard::PutBoolean("  Upper Limit", upperLimit.Get());

			if (_driver.GetRawAxis(_config->driveAxis)
					> _config->deadZone_Drive) {

				if (rev < _driver.GetRawAxis(_config->driveAxis)) {

					rightDrive_1.Set(rev);
					rightDrive_2.Set(rev);
					leftDrive_1.Set(-rev);
					leftDrive_2.Set(-rev);
					rev += _config->accelerationFactor;
					//rev+=.05;
					Wait(.05);
				} else {
					rightDrive_1.Set(
							(((_driver.GetRawAxis(_config->driveAxis)
									- _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					rightDrive_2.Set(
							(((_driver.GetRawAxis(_config->driveAxis)
									- _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					leftDrive_1.Set(
							(-((_driver.GetRawAxis(_config->driveAxis)
									- _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					leftDrive_2.Set(
							(-((_driver.GetRawAxis(_config->driveAxis)
									- _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					count = _driver.GetRawAxis(_config->driveAxis);
				}
			} else if (_driver.GetRawAxis(_config->driveAxis)
					< -_config->deadZone_Drive) {

				if (rev > _driver.GetRawAxis(_config->driveAxis)) {

					rightDrive_1.Set(rev);
					rightDrive_2.Set(rev);
					leftDrive_1.Set(-rev);
					leftDrive_2.Set(-rev);
					rev -= _config->accelerationFactor;
					//rev-=.05;
					Wait(.05);
				} else {
					rightDrive_1.Set(
							(((_driver.GetRawAxis(_config->driveAxis)
									+ _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					rightDrive_2.Set(
							(((_driver.GetRawAxis(_config->driveAxis)
									+ _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					leftDrive_1.Set(
							(-((_driver.GetRawAxis(_config->driveAxis)
									+ _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					leftDrive_2.Set(
							(-((_driver.GetRawAxis(_config->driveAxis)
									+ _config->deadZone_Drive)
									* pow(_config->deadZone_Drive, -1))
									* _config->powerMultiplier_Drive)
									* _config->drivePolarity);
					count = _driver.GetRawAxis(_config->driveAxis);
				}
			} else {

				//Just playing, not for real use, kinda sucky
				if (_config->breakMode == 1) {

					if (count < 0) {
						int num = fabs(count) * 10 / .1;

						for (int i = 0; i < num; i++) {
							rightDrive_1.Set(
									rightDrive_1.Get()
											+ pow(rightDrive_1.Get(), 2));
							rightDrive_2.Set(
									rightDrive_1.Get()
											+ pow(rightDrive_1.Get(), 2));
							leftDrive_1.Set(
									leftDrive_1.Get()
											- pow(leftDrive_1.Get(), 2));
							leftDrive_2.Set(
									leftDrive_1.Get()
											- pow(leftDrive_1.Get(), 2));
							SmartDashboard::PutNumber("Right_1: ",
									rightDrive_1.Get());
							SmartDashboard::PutNumber("Right_2: ",
									rightDrive_2.Get());
							SmartDashboard::PutNumber("Left_1: ",
									leftDrive_1.Get());
							SmartDashboard::PutNumber("Left_2: ",
									leftDrive_2.Get());
							Wait(.005);
						}
						rightDrive_1.Set(-count * breakIntensity);
						rightDrive_2.Set(-count * breakIntensity);
						leftDrive_1.Set(count * breakIntensity);
						leftDrive_2.Set(count * breakIntensity);
						SmartDashboard::PutNumber("Break: ",
								count * breakIntensity);
					}

					if (count > 0) {
						int num = count * 10 / .1;

						for (int i = 0; i < num; i++) {
							rightDrive_1.Set(
									rightDrive_1.Get()
											- (pow(rightDrive_1.Get(), 2)));
							rightDrive_2.Set(
									rightDrive_1.Get()
											- (pow(rightDrive_1.Get(), 2)));
							leftDrive_1.Set(
									leftDrive_1.Get()
											+ (pow(leftDrive_1.Get(), 2)));
							leftDrive_2.Set(
									leftDrive_1.Get()
											+ (pow(leftDrive_1.Get(), 2)));
							SmartDashboard::PutNumber("Right_1: ",
									rightDrive_1.Get());
							SmartDashboard::PutNumber("Right_2: ",
									rightDrive_2.Get());
							SmartDashboard::PutNumber("Left_1: ",
									leftDrive_1.Get());
							SmartDashboard::PutNumber("Left_2: ",
									leftDrive_2.Get());
							Wait(.005);
						}
						rightDrive_1.Set(-count * breakIntensity);
						rightDrive_2.Set(-count * breakIntensity);
						leftDrive_1.Set(count * breakIntensity);
						leftDrive_2.Set(count * breakIntensity);
						SmartDashboard::PutNumber("Break: ",
								count * breakIntensity);
					}

					Wait(.05);

				}
				rightDrive_1.Set(0);
				rightDrive_2.Set(0);
				leftDrive_1.Set(0);
				leftDrive_2.Set(0);
				rev = 0;
				count = 0;
			}

			if (_driver.GetRawAxis(_config->turnAxis)
					> _config->deadZone_Turning) {
				rightDrive_1.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								- _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
				rightDrive_2.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								- _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
				leftDrive_1.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								- _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
				leftDrive_2.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								- _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
			} else if (_driver.GetRawAxis(_config->turnAxis)
					< -_config->deadZone_Turning) {
				rightDrive_1.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								+ _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
				rightDrive_2.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								+ _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
				leftDrive_1.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								+ _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
				leftDrive_2.Set(
						(((_driver.GetRawAxis(_config->turnAxis)
								+ _config->deadZone_Turning)
								* pow(_config->deadZone_Turning, -1))
								* _config->powerMultiplier_Turning)
								* _config->turningPolarity);
			}

			if (_config->kickerUseAxis == 1) {
				if (_driver.GetRawAxis(_config->kickerAxis)
						> _config->deadZone_KickerWheel
						|| _driver.GetRawAxis(_config->kickerAxis)
								< -_config->deadZone_KickerWheel)
					kicker.Set(
							(_driver.GetRawAxis(_config->kickerAxis)
									* _config->powerMultiplier_KickerWheel)
									* _config->kickerPolarity);
				else
					kicker.Set(0);

			} else if (_config->kickerUseAxis == 0) {
				if (_driver.GetRawButton(_config->kickerLeftButton))
					kicker.Set(1 * -_config->powerMultiplier_KickerWheel);

				else if (_driver.GetRawButton(_config->kickerRightButton))
					kicker.Set(1 * _config->powerMultiplier_KickerWheel);

				else
					kicker.Set(0);
			}

			if (_operator.GetRawAxis(_config->armAxis) < -_config->deadZone_Arm
					&& upperLimit.Get())
				arm.Set(
						(-((_operator.GetRawAxis(_config->armAxis)
								+ _config->deadZone_Arm)
								* pow(_config->deadZone_Arm, -1))
								* _config->powerMultiplier_Arm)
								* _config->armPolarity);
			else if (_operator.GetRawAxis(_config->armAxis)
					> _config->deadZone_Arm && lowerLimit.Get())
				arm.Set(
						(-((_operator.GetRawAxis(_config->armAxis)
								- _config->deadZone_Arm)
								* pow(_config->deadZone_Arm, -1))
								* _config->powerMultiplier_Arm)
								* _config->armPolarity);
			else
				arm.Set(0);

			if (_operator.GetRawButton(_config->solenoidPositiveFire))
				doubleSolenoid.Set(DoubleSolenoid::kForward);

			else if (_operator.GetRawButton(_config->solenoidNegativeFire))
				doubleSolenoid.Set(DoubleSolenoid::kReverse);

			Wait(kUpdatePeriod); // Wait 5ms for the next update.
		}
	}

};

START_ROBOT_CLASS(Robot);
