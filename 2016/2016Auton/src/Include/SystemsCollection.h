/****************************** Header ******************************\
Class Name:  SystemsCollection
Summary: 	 Holds and instance of each major system, this is a singleton
			 as we [should] only have one instance of each system, I cannot
			 think of any scenario where another instance should be created.
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef SYSTEMS_SYSTEMSCOLLECTION_H_
#define SYSTEMS_SYSTEMSCOLLECTION_H_

#include "Drive.h"
#include "Beacon.h"
#include "Sensing.h"
#include "Operator.h"
#include "Singleton.h"

#ifdef CRIO
#include "Pneumatics.h"
#endif

namespace Systems {

class SystemsCollection final : public Singleton<SystemsCollection>
{
	friend class Singleton<SystemsCollection>;
public:
	SystemsCollection(){}
	~SystemsCollection(){}

	Drive drive = Drive();
	Beacon beacon = Beacon();
	Sensing sensing = Sensing();
	Operator operator_ = Operator();

#ifdef CRIO
	Pneumatics pneumatics = Pneumatics();
#endif

};

} /* namespace Systems */

#endif /* SYSTEMS_SYSTEMSCOLLECTION_H_ */
