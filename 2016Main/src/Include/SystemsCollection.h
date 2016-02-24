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

#include "MPC.h"
#include "Drive.h"
#include "Beacon.h"
#include "Sensing.h"
#include "Operator.h"
#include "Singleton.h"

namespace Systems {

/*! SystemsCollection is a singleton that stores the active instances of all required independently run systems.*/
class SystemsCollection final : public Singleton<SystemsCollection>
{
	friend class Singleton<SystemsCollection>;
public:
	SystemsCollection(){}//!< Default constructor.
	~SystemsCollection(){ } //!< Destructor.

	Drive *drive = new Drive(); //!< Creates and instance of the Drive system.
	Beacon *beacon = new Beacon();//!< Creates and instance of the Beacon system.
	Sensing *sensing = new Sensing();//!< Creates and instance of the Sensing system.
	MPCService *MPC = new MPCService();//!< Creates and instance of the MPC system.
	Operator *operator_ = new Operator();//!< Creates and instance of the Operator system.

	//testing to try and fix twitch bug, maybe these still run because we are using a singleton?
	void Reset()
	{
		delete drive;
		delete beacon;
		delete sensing;
		delete MPC;
		delete operator_;

		drive = new Drive();
		beacon = new Beacon();
		sensing = new Sensing();
		MPC = new MPCService();
		operator_ = new Operator();
	}
};

} /* namespace Systems */

#endif /* SYSTEMS_SYSTEMSCOLLECTION_H_ */
