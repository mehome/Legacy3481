/*
 * BaseDefense.h
 *
 *  Created on: Feb 22, 2016
 *      Author: Lucas Romier
 */

#ifndef SRC_AUTON_DEFENSES_BASEDEFENSE_H_
#define SRC_AUTON_DEFENSES_BASEDEFENSE_H_

class BaseDefense : Auton{
public:
	BaseDefense();
	virtual ~BaseDefense();
	virtual void Start() = 0;
};

#endif /* SRC_AUTON_DEFENSES_BASEDEFENSE_H_ */
