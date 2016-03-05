/*
 * ConfigEnums.h
 *
 *  Created on: 2 Jan 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_CONFIG_CONFIGENUMS_H_
#define SRC_CONFIG_CONFIGENUMS_H_

	enum DrivePower
	{
		fourCIM = 4,
		fiveCIM = 5,
		sixCIM = 6
	};

	enum DriveType
	{
		tank = 0,
		arcade = 1
	};

	enum DriveFit
	{
		linear=0,
		quadratic=1,
		exponential=2,
		poly=3
	};



#endif /* SRC_CONFIG_CONFIGENUMS_H_ */
