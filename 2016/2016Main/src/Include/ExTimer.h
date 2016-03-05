/*
 * ExTimer.h
 *
 *  Created on: 4 Mar 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_INCLUDE_EXTIMER_H_
#define SRC_INCLUDE_EXTIMER_H_

#include <time.h>

/*! ExTimer is an expiration timer used to check if a specified amount of time has passed.*/
class ExTimer
{
private:
	double timeout;//!< Defines the amount of time before the timer expires.
	time_t endTime;//!< Creates the time that is the mode comparison.
	bool isRunning = false;//!< Defines if the timer is currently in use.

public:
	ExTimer(double waitTime){ timeout = waitTime; }//!< Default constructor.
	bool IsRunning(){ return isRunning; }//!< Returns if the time is currently in use.
	void Start(){ isRunning = true; endTime = time(NULL)+timeout; };//!< Starts the timer.
	bool HasExpired(){ if(endTime < time(NULL)){ return true; } return false; }//!< Checks to see if the timer has expired, and returns respectively.
	void Renew(){ isRunning = false; }//!< Renews the timer for use again.
};


#endif /* SRC_INCLUDE_EXTIMER_H_ */
