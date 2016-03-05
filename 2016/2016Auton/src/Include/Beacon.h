/*
 * Beacon.h
 *
 *  Created on: 2 Jan 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_INCLUDE_BEACON_H_
#define SRC_INCLUDE_BEACON_H_

#include <WPILib.h>

#include "Config.h"
#include "Preproc.h"

using namespace Configuration;

namespace Systems {

class Beacon final
{
public:
	Beacon(); //!< Default constructor for the beacon system
	virtual ~Beacon(); //!< Default deconstructor for the beacon system
	void Standby(); //!< Public setter method to set the beacon into standby mode
	void Ready(); //!< Public setter method to set the beacon into ready mode
	void Error(); //!< Public setter method to set the beacon into error mode, not to be mistaken for the beacons internal error mode
	void RedMode(); //!< Public setter method to set the beacon into red mode
	void BlueMode(); //!< Public setter method to set the beacon into blue mode

    void Initialize() __attribute__((deprecated(UNBOUNDED)));//!< Initializes the beacon system

private:
    bool ready=false; //!< Private boolean for setting mode to ready
    bool standby=true; //!< Private boolean for setting mode to standby
    bool error=false; //!< Private boolean for setting mode to error
    bool redMode=false; //!< Private boolean for setting mode to red
    bool blueMode=false; //!< Private boolean for setting mode to blue

    void Clear(); //!< private method to clear all private boolean switches

	DigitalOutput *beaconSignalOne = Config::Instance()->GetDOutput(CommonName::BeaconSignalOne());
	DigitalOutput *beaconSignalTwo = Config::Instance()->GetDOutput(CommonName::BeaconSignalTwo());
	DigitalOutput *beaconSignalOff = Config::Instance()->GetDOutput(CommonName::BeaconSignalThree());
};

} /* namespace Systems */



#endif /* SRC_INCLUDE_BEACON_H_ */
