#pragma config(Hubs,  S1, HTMotor,  HTMotor,  HTMotor,  HTMotor)
#pragma config(Motor,  motorA,          motorA,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorB,          motorB,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorC,          motorC,        tmotorNormal, PIDControl, encoder)
#include "JoystickDriver.c"

task main()
{
  while(1 == 1)
  {
    getJoystickSettings(joystick);
    if (joystick.joy1_Buttons == 1)
    {
    motor[motorA] =100;
    motor[motorC] = 100;
  }

  if (joystick.joy1_Buttons == 2)
  {
    motor[motorA] = -100;
    motor[motorC] = -100;
    wait1Msec(1000);
    motor[motorA] = 0;
    motor[motorC] = 0;
  }
  if (joystick.joy1_Buttons == 4)
  {
  motor[motorA] =0;
    motor[motorC] =0;
  }
  }
}
