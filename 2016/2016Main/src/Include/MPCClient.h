/****************************** Header ******************************\
Class Name:  MPCClient.h
Summary: 	 
Project:     FRC%202016%20CPP
Copyright (c) .
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/

#ifndef SRC_INCLUDE_MPCCLIENT_H_
#define SRC_INCLUDE_MPCCLIENT_H_

#include <vector>

#include "WPILib.h"

using namespace std;

/*! The MPC Client is a class designed to be inherited by classes that
 * are to be registers with the MPC service.*/
class MPCClient
{
public:
	string name; //!< Name of the client.
	Encoder *Left; //!< Pointer to where the left encoder is allocated.
	Encoder *Right; //!< Pointer to where the right encoder is allocated.
	Encoder *SingleTarget; //!< Pointer to where the encoder is allocated, should there only be one encoder.
	double Target; //!< Target revolutions.
	double samplingModel, predefinedModel = 1; //!< Variables to store model data.
	double *left, *right, *singleIn; //!< Pointers to variables that store model data for reading.
	bool SingleEncoder, checkMe; //!< Booleans to specify whether the client uses a single encoder, and MPCService should check the clients status.
	virtual void CorrectionMultiplier(double multiplier)=0; //!< Pure virtual function to be implemented in the client that inherits this class.
	virtual ~MPCClient(){}; //!< Virtual destructor.
};

#endif /* SRC_INCLUDE_MPCCLIENT_H_ */
