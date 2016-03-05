/*
 * MPC.h
 *
 *  Created on: 3 Jan 2016
 *      Author: cooper.ryan
 */

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
