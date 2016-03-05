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

	static inline CommonName gearShift() { return CommonName("gearShift"); }

	static inline CommonName compressorSwitch() { return CommonName("compressorSwitch"); }
	static inline CommonName Compressor() { return CommonName("Compressor"); }

	static inline CommonName BeaconSignalOne() { return CommonName("beaconOne"); }
	static inline CommonName BeaconSignalTwo() { return CommonName("beaconTwo"); }
	static inline CommonName BeaconSignalThree() { return CommonName("beaconOffState"); }

private:
	string name; //!< Private string that stores the name of the linked device.

};

#endif /* SRC_INCLUDE_COMMONNAME_H_ */
