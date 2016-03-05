/****************************** Header ******************************\
Class Name:  ButtonControl : ControlItem
Summary: 	 Control that uses buttons
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/

#ifndef SRC_CONFIG_BUTTONCONTROL_H_
#define SRC_CONFIG_BUTTONCONTROL_H_

#include "ControlItem.h"
#include "ControlName.h"

/*! ButtonControl is a class to control a component of the robot with buttons on the joystick.*/
class ButtonControl final : public ControlItem
{
private:
	bool isPneumatic;//!< Defines whether or not this is being used for pnaumatics
	double multiplier;//!< Same multiplier pulled from config for the respective component
	int button_A, button_B;//!< Same button assignments pulled from config for the respective component

public:
	/*! Default constructor.*/
	ButtonControl(ControlName name, ControlItemType type, int button_a, int button_b, int polarity, double multiplier, bool isPneumatic)
		{ button_A = button_a; button_B = button_b; this->multiplier = multiplier; this->name = name; this->type = type;
			this->polarity = polarity; this->isPneumatic = isPneumatic; };

	virtual void Update();//!< Override of the pure virtual Update method in the ControlItem abstract class
	virtual ~ButtonControl(){};//!< Virtual destructor
};

#endif /* SRC_CONFIG_BUTTONCONTROL_H_ */
