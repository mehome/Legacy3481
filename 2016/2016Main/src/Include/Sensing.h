/****************************** Header ******************************\
Class Name:  Sensing
Summary: 	 Controls sensors and how to robot responds to them
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/


#ifndef SRC_INCLUDE_SENSING_H_
#define SRC_INCLUDE_SENSING_H_

#include <WPILib.h>

#include "out.h"
//#include "AHRS.h"
#include "Preproc.h"

namespace Systems {

/*! The sensing class is used to monitor sensors or other robot systems,
 * this is where the power monitor is handled, which looks at the PDB and its
 * current voltage levels, if they drop too low the this class changes the robot into
 * a low power mode to prevent brown-out.*/
class Sensing final
{
private:
	Out out; //!< Instance of Out for writing to log and screen.
	//AHRS *ahrs;
	void mainLoop(); //!< The main loop for the Sensing system.

public:
	Sensing(){}//!< Default constructor.
    ~Sensing(){} //!< Destructor.
	void Initialize() __attribute__((deprecated(UNBOUNDED)));//!< Initialises the mainLoop for the sensing system.
};

};
#endif /* SRC_INCLUDE_SENSING_H_ */
