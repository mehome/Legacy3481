#include <WPILib.h>
#include "Auton.h"

void Auton::Start(){

	SmartDashboard::PutString("", "0: XDefense\n1: YDefense\n2:ZDefense");
	int auton = SmartDashboard::GetNumber("Defense", 0);
	if(auton == 0){
		xDefense().Start();
	}else if(auton == 1){
		yDefense().Start();
	}else if(auton == 2){
		zDefense().Start();
	}
}
