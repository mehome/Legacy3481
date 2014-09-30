
#include "WPILib.h"

#include "Base/src/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/src/Vec2d.h"
#include "Base/src/Misc.h"
#include "Base/src/Event.h"
#include "Base/src/EventMap.h"
#include "Base/src/Script.h"
#include "Base/src/Script.h"
#include "Common/src/Entity_Properties.h"
#include "Common/src/Physics_1D.h"
#include "Common/src/Physics_2D.h"
#include "Common/src/Entity2D.h"
#include "Common/src/Goal.h"
#include "Common/src/Ship_1D.h"
#include "Common/src/Ship.h"
#include "Common/src/Vehicle_Drive.h"
#include "Common/src/PIDController.h"
#include "Common/src/Poly.h"
#include "Common/src/AI_Base_Controller.h"

#include "Base/src/Joystick.h"
#include "Base/src/JoystickBinder.h"
#include "Common/src/InOut_Interface.h"
#include "Common/src/Robot_Control_Interface.h"
#include "Common/src/UI_Controller.h"
#include "Common/src/PIDController.h"
#include "Common/src/InOut_Interface.h"
#include "Common/src/Debug.h"
#include "Common/src/Robot_Control_Common.h"
#include "Tank_Robot.h"
#include "Servo_Robot_Control.h"

#ifdef __UsingTestingKit__

using namespace Framework::Base;

  /***********************************************************************************************************************************/
 /*														Servo_Robot_Control															*/
/***********************************************************************************************************************************/


Servo_Robot_Control::Servo_Robot_Control(bool UseSafety) :
	//m_YawControl(1),
	m_LastYawAxisSetting(0.0),
	 m_LastLeftVelocity(0.0),m_LastRightVelocity(0.0),
	m_dTime_s(0.0)
{
	//ResetPos();  may need this later
}

Servo_Robot_Control::~Servo_Robot_Control() 
{
}

void Servo_Robot_Control::Reset_Encoders()
{
	//m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Servo_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Tank_Robot_Properties *robot_props=static_cast<const Tank_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	//This will copy all the props
	m_TankRobotProps=robot_props->GetTankRobotProps();
	//m_LeftEncoder.SetReverseDirection(m_TankRobotProps.LeftEncoderReversed);
	//m_RightEncoder.SetReverseDirection(m_TankRobotProps.RightEncoderReversed);
}

double Servo_Robot_Control::RPS_To_LinearVelocity(double RPS)
{
	return RPS * m_TankRobotProps.MotorToWheelGearRatio * M_PI * m_TankRobotProps.WheelDiameter; 
}

void Servo_Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=m_LastLeftVelocity,RightVelocity=m_LastRightVelocity;
	Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", LeftVelocity,RightVelocity);
	//Dout(m_TankRobotProps.Feedback_DiplayRow, "l=%.1f r=%.1f", m_LeftEncoder.GetRate()/3.0,m_RightEncoder.GetRate()/3.0);
}

//This is kept simple and straight forward, as it should be generic enough to work with multiple robots
void Servo_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	//For now leave this disabled... should not need to script this
	Dout(2, "l=%.1f r=%.1f", LeftVoltage,RightVoltage);
	
	//first interpolate the angular velocity
	const Tank_Robot_Props &props=m_TankRobotProps;
	const double D=props.WheelDimensions.length();
	//Here we go it is finally working I just needed to take out the last division
	const Vec2d &WheelDimensions=props.WheelDimensions;
	//L is the vehicle’s wheelbase
	const double L=WheelDimensions[1];
	//W is the vehicle’s track width
	const double W=WheelDimensions[0];
	const double skid=cos(atan2(W,L));
	const double MaxSpeed=m_RobotMaxSpeed;
	const double omega = ((LeftVoltage*MaxSpeed*skid) + (RightVoltage*MaxSpeed*-skid)) * 0.5;

	double AngularVelocity=(omega / (Pi * D)) * Pi2;
	if (m_TankRobotProps.ReverseSteering)
		AngularVelocity*=-1.0;
	
	double NewAngle=m_LastYawAxisSetting+(RAD_2_DEG(AngularVelocity * m_dTime_s) * m_TankRobotProps.MotorToWheelGearRatio);

	if (NewAngle>Servo::GetMaxAngle())
		NewAngle=Servo::GetMaxAngle();
	else if (NewAngle<Servo::GetMinAngle())
		NewAngle=Servo::GetMinAngle();

	//Ensure the angle deltas of angular velocity are calibrated to servo's angles
	m_LastYawAxisSetting=NewAngle;
	//Dout(4, "a=%.2f av=%.2f",m_LastYawAxisSetting,AngularVelocity);
	//if (!IsZero(AngularVelocity))
	//	printf("a=%.2f av=%.2f\n",m_LastYawAxisSetting,AngularVelocity);

	//m_YawControl.SetAngle(m_LastYawAxisSetting);
	
	const double inv_skid=1.0/cos(atan2(W,L));
	double RCW=AngularVelocity;
	double RPS=RCW / Pi2;
	RCW=RPS * (Pi * D) * inv_skid;  //D is the turning diameter

	m_LastLeftVelocity = + RCW;
	m_LastRightVelocity = - RCW;
}

void Servo_Robot_Control::Tank_Drive_Control_TimeChange(double dTime_s)
{
	m_dTime_s=dTime_s;
}

#endif
