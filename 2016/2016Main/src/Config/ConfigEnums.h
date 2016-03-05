/*
 * ConfigEnums.h
 *
 *  Created on: 2 Jan 2016
 *      Author: cooper.ryan
 */

#ifndef SRC_CONFIG_CONFIGENUMS_H_
#define SRC_CONFIG_CONFIGENUMS_H_

	/*! Enumeration for storing the drive train configuration.*/
	enum DrivePower
	{
		fourCIM = 4, //!< 4 CIM drive train.
		fiveCIM = 5, //!< 5 CIM drive train.
		sixCIM = 6 //!< 6 CIM drive train.
	};

	/*! Enumeration for storing the drive train control type.*/
	enum DriveType
	{
		tank = 0, //!< Tank drive.
		arcade = 1 //!< Arcade drive.
	};

	/*! Enumeration for storing the drive train control method.*/
	enum DriveFit
	{
		linear=0, //!< Linear fit.
		quadratic=1, //!< Quadratic fit.
		exponential=2, //!< Exponential fit.
		poly=3 //!< Polynomial fit.
	};



#endif /* SRC_CONFIG_CONFIGENUMS_H_ */
