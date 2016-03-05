/****************************** Header ******************************\
Class Name:  ControlName
Summary: 	 Self returning type to force user conformity with specific
		     string input methods.
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/

#ifndef SRC_INCLUDE_CONTROLNAME_H_
#define SRC_INCLUDE_CONTROLNAME_H_

#include<string>

using namespace std;

/*! ControlName is a self contained class that used to force users to search/select physical
 * components by a ControlName instead of by a string, this assures there are no misspellings*/
class ControlName final
{
public:
	inline ControlName() { } //!< Defeult constructor.
	operator string() const { return this->name; } //!< Overrides the string method to return its private name.
	inline ControlName(string name) { this->name = name; } //!< Parameterised constructed to set the string name.

	static inline ControlName intake() { return ControlName("intake"); }
	static inline ControlName indexer() { return ControlName("indexer"); }
	static inline ControlName shooter() { return ControlName("shooter"); }
	static inline ControlName climber() { return ControlName("climber"); }

	static inline ControlName shooterShift() { return ControlName("shooterShift"); }
	static inline ControlName climberShift() { return ControlName("climberShift"); }
	static inline ControlName intakeShift() { return ControlName("intakeShift"); }
	static inline ControlName gearShift() { return ControlName("gearShift"); }

private:
	string name; //!< Private string that stores the name of the linked control.

};

#endif /* SRC_INCLUDE_CONTROLNAME_H_ */
