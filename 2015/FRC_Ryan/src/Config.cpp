#include "tinyxml.h"
#include <Config.h>

/****************************** Class Header ******************************\
Class Name:  Config
 Project:      BroncBotz 3481 2015-World Code
 Copyright (c) BroncBotz.
 All rights reserved.

 Email:	cooper.ryan@centaurisoft.org

 Summary: Contains parsing for XML configuration file
 \***************************************************************************/

Config::Config() {
	TiXmlDocument doc("/robot.xml");
	bool loadOkay = doc.LoadFile();

	if (!loadOkay) {
		printf("\nError reading xml config...\n");
	} else {
		TiXmlHandle hDoc(&doc);

#ifndef Get_Vector_Assignments
		rightDrive_1_Value =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Victors").FirstChild(
								"DriveTrain").FirstChild("rightDrive_1").ToElement()->Attribute(
								"channel"));
		rightDrive_2_Value =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Victors").FirstChild(
								"DriveTrain").FirstChild("rightDrive_2").ToElement()->Attribute(
								"channel"));
		leftDrive_1_Value =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Victors").FirstChild(
								"DriveTrain").FirstChild("leftDrive_1").ToElement()->Attribute(
								"channel"));
		leftDrive_2_Value =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Victors").FirstChild(
								"DriveTrain").FirstChild("leftDrive_2").ToElement()->Attribute(
								"channel"));
		kicker_Value =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Victors").FirstChild(
								"DriveTrain").FirstChild("kickerDrive").ToElement()->Attribute(
								"channel"));
		arm_Value =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Victors").FirstChild(
								"Arm").FirstChild("arm_1").ToElement()->Attribute(
								"channel"));
#endif

#ifndef Get_DIO
		upperLimit = atoi(
				hDoc.FirstChild("Robot").FirstChild("DIO").FirstChild(
						"dartLimits").ToElement()->Attribute("upperChannel"));
		lowerLimit = atoi(
				hDoc.FirstChild("Robot").FirstChild("DIO").FirstChild(
						"dartLimits").ToElement()->Attribute("lowerChannel"));
#endif

#ifndef Get_Compressor
		compressorRelay =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Compressor").ToElement()->Attribute(
								"relay"));
		compressorLimit =
				atoi(
						hDoc.FirstChild("Robot").FirstChild("Compressor").ToElement()->Attribute(
								"limit"));
#endif

#ifndef Get_Solenoids
		solenoidPositive = atoi(
				hDoc.FirstChild("Robot").FirstChild("Solenoids").FirstChild(
						"manipulator").ToElement()->Attribute("positiveFlow"));
		solenoidNegative = atoi(
				hDoc.FirstChild("Robot").FirstChild("Solenoids").FirstChild(
						"manipulator").ToElement()->Attribute("negativeFlow"));
#endif
	}
}

Config::Config(const char* Version) {
	TiXmlDocument doc("/robot.xml");
	bool loadOkay = doc.LoadFile();

	if (!loadOkay) {
		printf("\nError reading xml config...\n");
	} else {
		TiXmlHandle hDoc(&doc);
		//get and print version numbers for XML and code
		version = hDoc.FirstChild("Version").ToElement()->Attribute("version");
		printf("\n");
		printf("XML Version: ");
		printf(version);
		printf("\n");
		printf("Code Version: ");
		printf(Version);
		printf("\n");

#ifndef Get_DeadZones
		deadZone_Drive = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("deadZone"));
		deadZone_Drivel =
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("deadZone");
		deadZone_KickerWheel =
				atof(
						hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
								"Kicker").FirstChild("kickerAxis").ToElement()->Attribute(
								"deadZone"));
		deadZone_Turning = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"turning").ToElement()->Attribute("deadZone"));
		deadZone_Arm = atof(
				hDoc.FirstChild("Controls").FirstChild("Operator").FirstChild(
						"arm").ToElement()->Attribute("deadZone"));
#endif

#ifndef Get_Power_Multipliers
		powerMultiplier_Drive = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("powerMultiplier"));
		powerMultiplier_KickerWheel = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"Kicker").ToElement()->Attribute("powerMultiplier"));
		powerMultiplier_Turning = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"turning").ToElement()->Attribute("powerMultiplier"));
		powerMultiplier_Arm = atof(
				hDoc.FirstChild("Controls").FirstChild("Operator").FirstChild(
						"arm").ToElement()->Attribute("powerMultiplier"));
#endif

#ifndef Get_Axis_Button_Assignments
		driveAxis = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("axis"));
		accelerationFactor = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("accelerationFactor"));
		breakMode = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("breakMode"));
		breakTime = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("breakTime"));
		breakIntensity = atof(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("breakIntensity"));
		kickerAxis =
				atoi(
						hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
								"Kicker").FirstChild("kickerAxis").ToElement()->Attribute(
								"axis"));
		kickerUseAxis = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"Kicker").ToElement()->Attribute("useAxis"));
		kickerLeftButton =
				atoi(
						hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
								"Kicker").FirstChild("kickerBtn").ToElement()->Attribute(
								"leftButton"));
		kickerRightButton =
				atoi(
						hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
								"Kicker").FirstChild("kickerBtn").ToElement()->Attribute(
								"rightButton"));
		turnAxis = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"turning").ToElement()->Attribute("axis"));
		armAxis = atoi(
				hDoc.FirstChild("Controls").FirstChild("Operator").FirstChild(
						"arm").ToElement()->Attribute("axis"));
		/*solenoidPositiveFire = atoi(
				hDoc.FirstChild("Controls").FirstChild("Operator").FirstChild(
						"manipulator").ToElement()->Attribute("positiveFlow"));
		solenoidNegativeFire = atoi(
				hDoc.FirstChild("Controls").FirstChild("Operator").FirstChild(
						"manipulator").ToElement()->Attribute("negativeFlow"));*/

		drivePolarity = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"drive").ToElement()->Attribute("polarity"));
		turningPolarity = atoi(
				hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
						"turning").ToElement()->Attribute("polarity"));
		kickerPolarity =
				atoi(
						hDoc.FirstChild("Controls").FirstChild("Driver").FirstChild(
								"Kicker").FirstChild("kickerAxis").ToElement()->Attribute(
								"polarity"));
		armPolarity = atoi(
				hDoc.FirstChild("Controls").FirstChild("Operator").FirstChild(
						"arm").ToElement()->Attribute("polarity"));
#endif

	}
}

Config::~Config() {
}

