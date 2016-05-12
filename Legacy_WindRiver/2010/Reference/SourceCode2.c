#pragma config(Hubs,  S1, HTMotor,  HTServo,  none,     none)
#pragma config(Sensor, S2,     touchSensor,         sensorTouch)
#pragma config(Sensor, S3,     lightSensor,         sensorLightActive)
#pragma config(Sensor, S4,     sonarSensor,         sensorSONAR)
#pragma config(Motor,  mtr_S1_C1_1,     motorD,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  mtr_S1_C1_2,     motorE,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor, mtr_S1_C2_1,      motorF,        tmotorNormal, PIDControl, encoder)
#include "JoystickDriver.c"


void initializeRobot()
{
  // Place code here to sinitialize servos to starting positions.
  // Sensors are automatically configured and setup by ROBOTC. They may need a brief time to stabilize.
  initializeDriveMotors();

  return;
}

task main()
{

  waitForStart();


    getJoystickSettings(joystick);

    if(joystick.joy1_y1 < 30 && joystick.joy1_y1 > -30)
    {
      motor[motorD] = 0;
      motor[motorF] = 0;
    }
    else
    {
      motor[motorD] = joystick.joy1_y1/2;
      motor[motorE] = joystick.joy1_y1/2;
    }

    if(joystick.joy1_y2 < 30 && joystick.joy1_y2 > -30)
    {
      motor[motorF] = 0;
    }
    else
    {
      motor[motorF] = -joystick.joy1_y2/2;
    }
  }
