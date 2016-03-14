/****************************** Header ******************************\
Class Name:  CommonName
Summary: 	 Self returning type to force user conformity with specific
		     string input methods.
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/

#ifndef SRC_INCLUDE_COMMONNAME_H_
#define SRC_INCLUDE_COMMONNAME_H_

#include<string>

using namespace std;

/*! CommonName is a self contained class that used to force users to search/select physical
 * components by a CommonName instead of by a string, this assures there are no misspellings*/
class CommonName final
{
public:
	inline CommonName() { } //!< Defeult constructor.
	operator string() const { return this->name; } //!< Overrides the string method to return its private name.
	inline CommonName(string name) { this->name = name; } //!< Parameterised constructed to set the string name.

	static inline CommonName rightDrive_1() { return CommonName("rightDrive_1"); }
	static inline CommonName rightDrive_2() { return CommonName("rightDrive_2"); }
	static inline CommonName rightDrive_3() { return CommonName("rightDrive_3"); }
	static inline CommonName leftDrive_1() { return CommonName("leftDrive_1"); }
	static inline CommonName leftDrive_2() { return CommonName("leftDrive_2"); }
	static inline CommonName leftDrive_3() { return CommonName("leftDrive_3"); }
	static inline CommonName kickerWheel() { return CommonName("kickerWheel"); }

	static inline CommonName leftEncoder() { return CommonName("leftEncoder"); }
	static inline CommonName rightEncoder() { return CommonName("rightEncoder"); }
	static inline CommonName shooterEncoder() { return CommonName("shooterEncoder"); }

	static inline CommonName climber_1() { return CommonName("climber_1"); }
    static inline CommonName climber_2() { return CommonName("climber_2"); }

    static inline CommonName indexerMotor() { return CommonName("indexerMotor"); }
    static inline CommonName intakeMotor() { return CommonName("intakeMotor"); }


	static inline CommonName shooter_1() { return CommonName("shooter_1"); }
    static inline CommonName shooter_2() { return CommonName("shooter_2"); }

	static inline CommonName gearShift() { return CommonName("gearShift"); }
	static inline CommonName intakeShift() { return CommonName("intakeShift"); }
	static inline CommonName shooterShift() { return CommonName("shooterShift"); }
	static inline CommonName climberShift() { return CommonName("climberShift"); }

	static inline CommonName BeaconSignalOne() { return CommonName("beaconOne"); }
	static inline CommonName BeaconSignalTwo() { return CommonName("beaconTwo"); }
	static inline CommonName BeaconSignalThree() { return CommonName("beaconOffState"); }

	static inline CommonName UltraSonicInput() { return CommonName("ultraSonicInput"); }
	static inline CommonName UltraSonicOutput() { return CommonName("ultraSonicOutput"); }

	static inline CommonName ClimberLimitUp() { return CommonName("climberLimitUp"); }
	static inline CommonName ClimberLimitDown() { return CommonName("climberLimitDown"); }

	static inline CommonName VideoLEDRelay() { return CommonName("videoLEDRelay"); }
	static inline CommonName IntakePressurePad() { return CommonName("intakePressurePad"); }

private:
	string name; //!< Private string that stores the name of the linked device.

};

#endif /* SRC_INCLUDE_COMMONNAME_H_ */
