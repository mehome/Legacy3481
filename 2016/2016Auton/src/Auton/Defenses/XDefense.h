/*
 * XDefense.h
 *
 *  Created on: Feb 22, 2016
 *      Author: Lucas Romier
 */

#ifndef SRC_AUTON_DEFENSES_XDEFENSE_H_
#define SRC_AUTON_DEFENSES_XDEFENSE_H_
#include "BaseDefense.h"

class XDefense : BaseDefense{
public:
	XDefense(){};
	virtual ~XDefense();
	virtual void Start();
};

#endif /* SRC_AUTON_DEFENSES_XDEFENSE_H_ */
