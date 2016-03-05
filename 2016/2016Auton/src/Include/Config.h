/****************************** Header ******************************\
 *
 *  Created on: 2 Jan 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_INCLUDE_CONFIG_H_
#define SRC_INCLUDE_CONFIG_H_

#include <string>
#include <vector>
#include <WPILib.h>

#include "CommonName.h"
#include "ConfigStructs.h"
#include "ConfigEnums.h"
#include "pugixml.h"
#include "Preproc.h"
#include "out.h"

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
#ifdef ROBORIO
		void Load(const char*);//!< Loads the configuration from XML markup.
#endif
#ifdef CRIO
		void Load();//!< Load the configuration for CRIO.
#endif
		static Config* Instance();//!< Returns the instance of the config singleton.
		 ~Config(){ instanceFlag = false; };//!< Deconstructor that sets the instance flag to false.

		vector<DIItem> DIs;//!< Collection of all DigitalInput devices.
		vector<DOItem> DOs;//!< Collection of all DigitalOutput devices.
		vector<RelayItem> Relays;//!< Collection of all Relays.
		vector<VictorItem> Victors;//!< Collection of all Victors.
		vector<EncoderItem> Encoders;//!< Collection of all Encoders.
		vector<SolenoidItem> Solenoids;//!< Collection of all Solenoids.

		DrivePower _DrivePower;//!< Defines the DrivePower.
		DriverConfig _DriverConfig;//!< Defines the Driver's configuration.
		OperatorConfig _OperatorConfig;//!< Defines the Operator's configuration.

		Relay *GetRelay(CommonName name);//!< Searches for a relay that has been allocated by the config, returns the relay if found.
		Victor *GetVictor(CommonName name);//!< Searches for a victor that has been allocated by the config, returns the victor if found.
		DigitalInput *GetDInput(CommonName name);//!< Searches for a digital input device that has been allocated by the config, returns the device if found.
		DigitalOutput *GetDOutput(CommonName name);//!< Searches for a digital output device that has been allocated by the config, returns the device if found.
		DoubleSolenoid *GetSolenoid(CommonName name);//!< Searches for a double solenoid that has been allocated by the config, returns the solenoid if found.

	private:
		 Out out;//!< Gets an instance of the out redirector.
		 bool loaded;//!< private boolean to set whether the config has loaded correctly.
		 static Config *single;//!< This is a static instantiation of config for use as a singleton.
		 static bool instanceFlag;//!< Flag for controlling the singleton.

#ifdef ROBORIO
		 void loadValues(xml_document&);//!< If the XML file has been found, continue loading all the components
#endif
#ifdef CRIO
		 void loadValues();//!< emulates the ROBORIO version of this function for the CRIO
#endif

		 Config(){ loaded=false; _DrivePower=DrivePower::fourCIM; };//!< Private constructor that has default values
	};

}
#endif /* SRC_INCLUDE_CONFIG_H_ */
