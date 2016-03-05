/****************************** Header ******************************\
Class Name:  MPC
Summary: 	 Model Predictive Control class
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\***************************************************************************/

#ifndef SRC_INCLUDE_MPC_H_
#define SRC_INCLUDE_MPC_H_

#include <vector>

#include "Preproc.h"
#include "MPCClient.h"

using namespace std;

/*! The MPC Service is a experimental replacement for PID, using model predictive control instead.*/
class MPCService final
{
private:
	vector<MPCClient*> clients; //!< Collection of registered MPC clients.
	void analysisLoop(); //!< Infinite loop to analyse and generate models on robot performance.

public:
	MPCService(){}//!< Default empty constructor.
	void Initialize() __attribute__((deprecated(UNBOUNDED))); //!< Initialiser function that is called from Initializers.h.
	void RegisterClient(MPCClient *client){ clients.push_back(client); } //!< Registers a class that inherits MPCClient to the MPCService.

};



#endif /* SRC_INCLUDE_MPC_H_ */
