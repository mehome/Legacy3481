/*
 * Sensing.h
 *
 *  Created on: 3 Jan 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_INCLUDE_SENSING_H_
#define SRC_INCLUDE_SENSING_H_

#include <WPILib.h>

#include "out.h"
#include "AHRS.h"
#include "Preproc.h"

namespace Systems {

class Sensing final
{
private:
	Out out;
	AHRS *ahrs;
	void mainLoop();

public:
	Sensing(){}
    virtual ~ Sensing(){}
	void Initialize() __attribute__((deprecated(UNBOUNDED)));
};

};
#endif /* SRC_INCLUDE_SENSING_H_ */
