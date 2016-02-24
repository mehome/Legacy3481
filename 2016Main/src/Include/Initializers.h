/****************************** Header ******************************\
Summary: 	 Collection of methods to specifically call the initialisers
		 	 for each congruent loop
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef INCLUDE_INITIALIZERS_H_
#define INCLUDE_INITIALIZERS_H_

#include <WPILib.h>

#include "SystemsCollection.h"

using namespace Systems;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void InitializeDrive(){ SystemsCollection::Instance().drive->Initialize(); }//!< Indirectly creates an instance of the drive and calls it's Initialize function.
void InitializeBeacon(){ SystemsCollection::Instance().beacon->Initialize(); }//!< Indirectly creates an instance of the beacon system and calls it's Initialize function.
void InitializeSensors(){ SystemsCollection::Instance().sensing->Initialize(); }//!< Indirectly creates an instance of the sensing system and calls it's Initialize function.
void InitializeOperation(){ SystemsCollection::Instance().operator_->Initialize(); }//!< Indirectly creates an instance of the operator system and calls it's Initialize function.
void InitializeMPC(){ SystemsCollection::Instance().MPC->Initialize(); }//!< Indirectly creates an instance of the operator system and calls it's Initialize function.

#pragma GCC diagnostic pop

#endif /* INCLUDE_INITIALIZERS_H_ */
