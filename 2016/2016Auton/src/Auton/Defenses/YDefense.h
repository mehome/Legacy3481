/*
 * YDefense.h
 *
 *  Created on: Feb 22, 2016
 *      Author: Lucas Romier
 */

#ifndef SRC_AUTON_DEFENSES_YDEFENSE_H_
#define SRC_AUTON_DEFENSES_YDEFENSE_H_
#include "BaseDefense.h"

class YDefense : BaseDefense{
public:
	YDefense(){};
	virtual ~YDefense();
	virtual void Start();
};

#endif /* SRC_AUTON_DEFENSES_YDEFENSE_H_ */
