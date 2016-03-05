/****************************** Header ******************************\
Class Name:  ToggleControl : ControlItem
Summary: 	 Control that uses an toggle button
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef SRC_CONFIG_TOGGLECONTROL_H_
#define SRC_CONFIG_TOGGLECONTROL_H_

#include "ControlItem.h"
#include "ControlName.h"

/*! ToggleControl is a class to control a component of the robot with a toggle button on the joystick.*/
class ToggleControl final : public ControlItem
{
protected:
	int button;//!< Button value from config for respective control.
	bool isPneumatic, isDOI, isFirstRun = true;//!< Runtime parameters that define the type of physical component controlled.
	double multiplier;//!< Multiplier pulled from config for respective control if a multiplier is defined.
	bool status = false, p_action = false, action = false;//!< Toggle state booleans.

	void reversePneumatics();//!< Reverses all pneumatics owned by the control.
	void forwardPneumatics();//!< Puts all the pnaumatics woned by the control to the forward potition.
	void runVictors();//!< Runs the victors owned by the control. Note this is mono directional toggle.
	void stopVictors();//!< Stops all victors owned by the control. Note this is mono directional toggle.

public:
	/*! Default constructor.*/
	ToggleControl(ControlName name, ControlItemType type, int button, double multiplier = 1, bool isPneumaticControl = false, bool isDOControl = false)
		{ isPneumatic = isPneumaticControl; this->button = button; this->multiplier = multiplier;
			isDOI = isDOControl; this->name = name; this->type = type; };

	virtual void Update();//!< Override of the pure virtual Update method in the ControlItem abstract class
	virtual ~ToggleControl(){};//!< Virtual destructor
};

#endif /* SRC_CONFIG_TOGGLECONTROL_H_ */
