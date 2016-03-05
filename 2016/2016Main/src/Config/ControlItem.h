/****************************** Header ******************************\
Class Name:  ControlItem
Summary: 	 Base abstract class for all control classes
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef SRC_CONFIG_CONTROLITEM_H_
#define SRC_CONFIG_CONTROLITEM_H_

#include <Vector>
#include <String>
#include <WPILib.h>

#include "ControlName.h"
#include "ConfigStructs.h"

using namespace std;
using namespace Configuration;

/*! Enumeration for defining the type of control.*/
enum ControlItemType
{
	buttonControl,
	toggleControl,
	axisControl
};

/*! Abstract class that all Control classes inherit from.*/
class ControlItem
{
protected:
	int polarity;//!< Pulls the polarity for the control if used or defined.
	Joystick *joy;//!< Pointer to the joystick to use.
	ControlName name;//!< Defines the name of the control.
	ControlItemType type;//!< Defines the type of control.
	vector<VictorItem *> m_controllers;//!< Vector for pointers to the contol's respective VictorItems.
	vector<SolenoidItem *> m_solenoids;//!< Vector for pointers to the contol's respective SolenoidItems.
	vector<DOItem *> m_digitalOutputs;//!< Vector for pointers to the contol's respective DOItems.

	/*! Sets all victors owned by the control to the specified value.*/
	void setVictors(double value)
	{
		for(int i=0; i<(int)m_controllers.size();i++)
			m_controllers[i]->Set(value);
	}

	/*! Sets all solenoids owned by the control to the specified value.*/
	void setSolenoids(DoubleSolenoid::Value value)
	{
		for(int i=0; i<(int)m_solenoids.size();i++)
			m_solenoids[i]->Set(value);
	}

	/*! Sets all digital outputs owned by the control to the specified value.*/
	void setDigitalOutputs(int val)
	{
			for(int i=0; i<(int)m_digitalOutputs.size();i++)
				m_digitalOutputs[i]->out->Set(val);
	}

public:
	ControlItem(){};//!< Default constructor
	virtual void Update() = 0;//!< Pure virtual Update method that all Control classes are required to override.
    void SetController(Joystick *joy){ this->joy = joy; };//!< Sets the controller to access for checking control values.
    void AddVictorItem(VictorItem *item){ m_controllers.push_back(item); };//!< Adds a victor to the Control's motor controller vecotor.
    void AddSolenoidItem(SolenoidItem *item){ m_solenoids.push_back(item); };//!< Adds a solenoid to the Control's double solenoid vector.
    void AddDOItem(DOItem *item){ m_digitalOutputs.push_back(item); };//!< Adds a DigitalOutput to the Control's digital output vector.
    string GetName(){ return name; };//!< Returns the name of the control.
    virtual ~ControlItem(){};
};

#endif /* SRC_CONFIG_CONTROLITEM_H_ */
