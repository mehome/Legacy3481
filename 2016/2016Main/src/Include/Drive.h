/****************************** Header ******************************\
Class Name:  Drive
Summary: 	 Drive train loop
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef SYSTEMS_DRIVE_H_
#define SYSTEMS_DRIVE_H_

#include <string>
#include <vector>
#include <WPILib.h>

#include "Config.h"
#include "Preproc.h"
#include "ConfigEnums.h"


using namespace std;
using namespace Configuration;

namespace Systems {

/*! Drive is the drive system of the robot, this controlls are drive realted communication to hardware.
 *  It is also where the drivers joystick is set and monitored for driving.*/
class Drive final
{
public:
	    enum DriveOrientation//!< Defines the orientation of a victor.
		{
			left=0,//!< Left side of the drive train.
			right=1,//!< Right side of the drive train.
			ALL=3//!< A general wild card for any victors used for the drive, but not orientated left or right(like a kicker).
		};

	    Drive();//!< Default constructor.
		void Tank();//!< Starts the loop for a tank based drive.
		void Kicker();//!< Reads values to set a kicker wheel, if applicable.
		void Arcade();//!< Starts the loop for an arcade based drive.
		void LeftStop();//!< Stops the left side.
		void FullStop();//!< Stops all drive train motion.
		void KickerStop();//!< Stops the kicker wheel if applicable.
		void CheckButtons();//!< Checks to see if any buttons defined in the driver config have been pressed.
		void RightStop();//!< Stops the right side.
		virtual ~Drive();//!< Virtual deconstructor.
		void SetLowPowerMode(bool val) { isLowPowerMode=val; };//!< Sets the private low power flag to put the drive train into a power conserving state.
		void AddVictor(int, DriveOrientation);//!< Adds a victor with numeral and drive orientation identifiers.
		void SetLeftDrive(double, bool rev=true);//!< Sets the left side of the drive train to a value <=1, also if it should rev upto speed.
		void SetRightDrive(double, bool rev=true);//!< Sets the right side of the drive train to a value <=1, also if it should rev upto speed.
		void SetKicker(double, int);//!< Sets the kicker to a value <=1.

		void Initialize() __attribute__((deprecated(UNBOUNDED)));//!< Initializes the setup and main loop of the drive system.


private:
	    bool p_invert, invert, flipper = true; //!< Booleans for handling the toggle between normal and inverted drive.
		bool invertCheck = false;//!< Boolean to control drive inversion.
		bool isLowPowerMode = false;//!< Private boolean flag for toggling between low and normal power mode.
		double *revPtr = NULL;//!< Pointer to double values used in rev calculations (for preventing high sticking).
		double revValLeft=0;//!< Left double value used in rev calculations (for preventing high sticking).
		double revValRight=0;//!< Right double value used in rev calculations (for preventing high sticking).
		double (*driveFit)(double, double, double);//!< Pointer function to one of the different drive fits (aka linear, poly, log).

		DrivePower drivePower;//!< Defines the size of our drive train (2,4,6 CIM). This is set by the config.

		typedef void(Drive::*drive)();//!< Point function for drive type (tank/arcade).
		typedef void(Drive::*set)(double, bool);//!< Used to set orientations within the drive system.
		void rev(double, DriveOrientation);//!< Rev function is used to prevent high sticking by adding an acceleration to controls.

		Joystick *driver;//!< Pointer to where the constructor creates the driver controller from the config values.
		DriverConfig driverConfig;//!< Pointer to where the constructor creates the driver configuration from the config values.
		vector<VictorItem*> leftDriveVictors;//!< Pointers to the left side victors stored in the config.
		vector<VictorItem*> rightDriveVictors;//!< Pointers to the right side victors stored in the config.
		VictorItem *kickerWheel;//!< Pointer to the kicker wheel victor stored in the config.
		Config *config = Config::Instance();//!< Gets and sets a pointer to the config singleton instance.
};

} /* namespace Systems */

#endif /* SYSTEMS_DRIVE_H_ */
