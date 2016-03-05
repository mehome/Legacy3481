/****************************** Header ******************************\
Class Name:  AxisControl : ControlItem
Summary: 	 Control that uses an axis
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/


#ifndef AXISCONTROL_H_
#define AXISCONTROL_H_

#include "ControlItem.h"
#include "ControlName.h"

/*! AxisControl is a class to control a component of the robot with an axis on the joystick.*/
class AxisControl final : public ControlItem
{
private:
	int axis;//!< Defines the axis to use
	double multiplier, deadzone; //!<multiplier and deadzones are the same values pulled from the config

public:
	AxisControl(){};
	/*! Default constructor.*/
	AxisControl(ControlName name, ControlItemType type, int axis, int polarity, double deadzone, double multiplier = 1)
		{ this->name = name; this->axis = axis; this->polarity = polarity;
			this->deadzone = deadzone; this->multiplier = multiplier; this->type = type; };

	virtual void Update();//!< Override of the pure virtual Update method in the ControlItem abstract class
	virtual ~AxisControl(){};//!< Virtual destructor
};

#endif /* AXISCONTROL_H_ */
