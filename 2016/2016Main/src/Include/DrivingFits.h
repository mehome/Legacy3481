/****************************** Header ******************************\
Summary: 	 Contains static methods for making proportional
				curves from joystick and deadzone
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\***************************************************************************/

#ifndef SRC_INCLUDE_DRIVINGFITS_H_
#define SRC_INCLUDE_DRIVINGFITS_H_


/*! Function that returns -1 if the input is less than 0, or 1 if greater than 0. */
inline int sign(double x)
{
	if(x<0)
		return -1;
	else return 1;
}

/*! Function that returns an output proportional (linearly) to the axis and deadzone inputs */
inline double linearValueEstimator(double x, double dz, double na=0)
	{
		return ((fabs(x)-dz)*pow((1-dz),-1))*sign(x);
	}

/*! Function that returns an output proportional (quadratically) to the axis and deadzone inputs */
inline double polynomialValueEstimator(double x, double dz, double power = 2)
	{
			double value = (pow((fabs(x)-dz),power)*pow((1-dz),-power))*sign(x);
			if(value>1)
				return 1;
			else if(value<-1)
				return -1;
			return value;
	}

/*! Function that returns an output proportional (exponentially) to the axis and deadzone inputs */
inline double exponentialValueEstimator(double x, double dz, double power = 2)
	{
			double value = (pow(pow(2,1/(1-dz)),(fabs(x)-dz))-1)*sign(x);
			if(value>1)
				return 1;
			else if(value<-1)
				return -1;
			return value;
	}

#endif /* SRC_INCLUDE_DRIVINGFITS_H_ */
