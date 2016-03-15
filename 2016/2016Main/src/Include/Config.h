/****************************** Header ******************************\
Class Name:  Config
Summary: 	 Handles the entire configuration for the robot
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\***************************************************************************/

#ifndef SRC_INCLUDE_CONFIG_H_
#define SRC_INCLUDE_CONFIG_H_

#include <string>
#include <vector>
#include <WPILib.h>

#include "pugixml.h"
#include "Preproc.h"
#include "CommonName.h"
#include "ControlName.h"
#include "ControlItem.h"
#include "ConfigStructs.h"

using namespace pugi;
using namespace std;

namespace Configuration
{

/*! Config loads and stores the robot configuration from the XML markup, this class is a
 *  singleton, assuring that all systems/classes/methods can access and read the configuration.*/
	class Config final
	{
	public:
		bool IsLoaded();//!< Returns whether the config has loaded or not.
		void Load(const char*);//!< Loads the configuration from XML markup.
		void BuildControlSchema();

		static Config* Instance();//!< Returns the instance of the config singleton.
		 ~Config(){ instanceFlag = false; };//!< Deconstructor that sets the instance flag to false.

		vector<DIItem> DIs;//!< Collection of all DigitalInput devices.
		vector<DOItem> DOs;//!< Collection of all DigitalOutput devices.
		vector<RelayItem> Relays;//!< Collection of all Relays.
		vector<VictorItem> Victors;//!< Collection of all Victors.
		vector<EncoderItem> Encoders;//!< Collection of all Encoders.
		vector<SolenoidItem> Solenoids;//!< Collection of all Solenoids.
		vector<AnalogIItem> AnalogInputDevices;//!< Collection of all analog input devices.
		vector<AnalogOItem> AnalogOutputDevices;//!< Collection of all analog output devices.
		vector<ControlItem*> Controls;//!< Collection of all ControlItems.

		DrivePower _DrivePower;//!< Defines the DrivePower.
		DriverConfig _DriverConfig;//!< Defines the Driver's configuration.
		OperatorConfig _OperatorConfig;//!< Defines the Operator's configuration.
		RobotParameters _RobotParameters;//!< Defines extra parameters used by the robot program.

		Relay *GetRelay(CommonName name);//!< Searches for a relay that has been allocated by the config, returns the relay if found.
		VictorItem *GetVictor(CommonName name);//!< Searches for a victor that has been allocated by the config, returns the victor if found.
		DigitalInput *GetDInput(CommonName name);//!< Searches for a digital input device that has been allocated by the config, returns the device if found.
		DigitalOutput *GetDOutput(CommonName name);//!< Searches for a digital output device that has been allocated by the config, returns the device if found.
		SolenoidItem *GetSolenoid(CommonName name);//!< Searches for a double solenoid that has been allocated by the config, returns the solenoid if found.
		Encoder *GetEncoder(CommonName name);//!< Searches for a Encoder that has been allocated by the config, returns the Encoder if found.
		ControlItem *GetControlItem(ControlName name);//!< Searches for a ControlItem that has been allocated by the config, returns the Control if found.
		AnalogInput *GetAnalogInputDevice(CommonName name);//!< Searches for a Analog input device that has been allocated by the config, returns the device if found.
		AnalogOutput *GetAnalogOutputDevice(CommonName name);//!< Searches for a Analog input device that has been allocated by the config, returns the device if found.

		bool IsAutonEnabled(){ return autonEnabled; }
		bool QuickLoad(){ return quickLoad; }
	private:
		 bool autonEnabled = false;
		 bool quickLoad;//!< Boolean to control quick loading mode (not having to restart robot code to apply config changes).
		 bool loaded;//!< private boolean to set whether the config has loaded correctly.
		 static Config *single;//!< This is a static instantiation of config for use as a singleton.
		 static bool instanceFlag;//!< Flag for controlling the singleton.

		 void loadValues(xml_document&);//!< If the XML file has been found, continue loading all the components

		 Config(){ loaded=false; _DrivePower=DrivePower::fourCIM; };//!< Private constructor that has default values
	};

}
#endif /* SRC_INCLUDE_CONFIG_H_ */
