#undef  __EncoderHack__
#undef  __ShowPotentiometerReadings__
#undef  __ShowEncoderReadings__
#undef  __ShowRollerReadings__

#if defined(__ShowEncoderReadings__) || defined(__ShowPotentiometerReadings__) || defined(__ShowRollerReadings__)
#define __ShowLCD__
#endif

#include "WPILib.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Common/Entity_Properties.h"
#include "Common/Physics_1D.h"
#include "Common/Physics_2D.h"
#include "Common/Entity2D.h"
#include "Common/Goal.h"
#include "Common/Ship_1D.h"
#include "Common/Ship.h"
#include "Common/Robot_Tank.h"
#include "Common/AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2011_Robot.h"
#include "InOut_Interface.h"

using namespace Framework::Base;

  /***********************************************************************************************************************************/
 /*														Driver_Station_Joystick														*/
/***********************************************************************************************************************************/

size_t Driver_Station_Joystick::GetNoJoysticksFound() 
{
	return m_NoJoysticks;
}

bool Driver_Station_Joystick::read_joystick (size_t nr, JoyState &Info)
{
	//First weed out numbers not in range
	int Number=(int)nr;
	Number-=m_StartingPort;
	bool ret=false;
	nr++;  //DOH the number selection is cardinal!  :(
	if ((Number>=0) && (Number<m_NoJoysticks))
	{
		memset(&Info,0,sizeof(JoyState));  //zero the memory
		//The axis selection is also ordinal
		Info.lX=m_ds->GetStickAxis(nr,1);
		Info.lY=m_ds->GetStickAxis(nr,2);
		Info.lZ=m_ds->GetStickAxis(nr,3);
		Info.lRx=m_ds->GetStickAxis(nr,4);
		Info.lRy=m_ds->GetStickAxis(nr,5);
		Info.ButtonBank[0]=m_ds->GetStickButtons(nr);
		ret=true;
	}
	return ret;
}

Driver_Station_Joystick::Driver_Station_Joystick(int NoJoysticks,int StartingPort) : m_NoJoysticks(NoJoysticks), m_StartingPort(StartingPort)
{
	m_ds = DriverStation::GetInstance();
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName="Joystick_1";
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=true;
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	common.ProductName="Joystick_2";
	m_JoyInfo.push_back(common);
	common.ProductName="Joystick_3";
	m_JoyInfo.push_back(common);
	common.ProductName="Joystick_4";
	m_JoyInfo.push_back(common);
}

Driver_Station_Joystick::~Driver_Station_Joystick()
{
}

  /***********************************************************************************************************************************/
 /*															Robot_Control															*/
/***********************************************************************************************************************************/

void Robot_Control::SetSafety(bool UseSafety)
{
	if (UseSafety)
	{
		//I'm giving a whole second before the timeout kicks in... I do not want false positives!
		m_RobotDrive.SetExpiration(1.0);
		m_RobotDrive.SetSafetyEnabled(true);
	}
	else
		m_RobotDrive.SetSafetyEnabled(false);
}


void Robot_Control::ResetPos()
{
	m_Compress.Stop();
	//Allow driver station to control if they want to run the compressor
	if (DriverStation::GetInstance()->GetDigitalIn(7))
	{
		printf("RobotControl reset compressor\n");
		m_Compress.Start();
	}
}

Robot_Control::Robot_Control(bool UseSafety) :
	m_1(1),m_2(2),m_3(3),m_4(4),
	m_RobotDrive(&m_1,&m_2,&m_3,&m_4),
	//m_RobotDrive(1,2,3,4),  //default Jaguar instantiation
	m_ArmMotor(5),m_RollerMotor(6),m_Compress(5,2),
	m_OnRist(5),m_OffRist(6),m_OnClaw(3),m_OffClaw(4),m_OnDeploy(2),m_OffDeploy(1),
	m_LeftEncoder(3,4),m_RightEncoder(1,2),
	m_Potentiometer(1),m_Camera(NULL)
{
	ResetPos();
	SetSafety(UseSafety);
	const double EncoderPulseRate=(1.0/360.0);
	m_LeftEncoder.SetDistancePerPulse(EncoderPulseRate),m_RightEncoder.SetDistancePerPulse(EncoderPulseRate);
	m_LeftEncoder.Start(),m_RightEncoder.Start();
	//Seems like it doesn't matter how long I wait I'll get the exception, this is probably that fix they were talking about
	//fortunately it doesn't effect any functionality
	//Wait(10.000);
	m_Camera=&AxisCamera::GetInstance();
}

Robot_Control::~Robot_Control() 
{
	m_LeftEncoder.Stop(),m_RightEncoder.Stop();  //TODO Move for autonomous mode only
	m_RobotDrive.SetSafetyEnabled(false);
	m_Compress.Stop();
	m_Camera=NULL;  //We don't own this, but I do wish to treat it like we do
}

void Robot_Control::Reset_Arm()
{
	m_KalFilter_Arm.Reset();
}

void Robot_Control::Reset_Encoders()
{
	m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Robot_Control::TimeChange(double dTime_s)
{
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
}

void Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2011_Robot_Properties *robot_props=static_cast<const FRC_2011_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
}

void Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=0.0,RightVelocity=0.0;
	double LeftRate=m_LeftEncoder.GetRate();
	//LeftRate=m_KalFilter_EncodeLeft(LeftRate);
	double RightRate=m_RightEncoder.GetRate();
	//RightRate=m_KalFilter_EncodeRight(RightRate);
	LeftVelocity=FRC_2011_Robot::RPS_To_LinearVelocity(LeftRate);
	RightVelocity=FRC_2011_Robot::RPS_To_LinearVelocity(RightRate);
	#ifdef __EncoderHack__
	LeftVelocity=RightVelocity;  //Unfortunately the left encoder is not working remove once 
	#endif
	#ifdef __ShowEncoderReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->PrintfLine(DriverStationLCD::kUser_Line4, "l=%.1f r=%.1f", LeftVelocity,RightVelocity);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line4, "l=%.1f r=%.1f", m_LeftEncoder.GetRate()/3.0,m_RightEncoder.GetRate()/3.0);	
	#endif
}

//This is kept simple and straight forward, as it should be generic enough to work with multiple robots
void Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	#if 0
	float right=DriverStation::GetInstance()->GetAnalogIn(1) - 1.0;
	float left=DriverStation::GetInstance()->GetAnalogIn(2) - 1.0;
	m_RobotDrive.SetLeftRightMotorOutputs(right,left);
	return;
	#endif

	#ifdef __ShowEncoderReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->PrintfLine(DriverStationLCD::kUser_Line3, "l=%.1f r=%.1f", LeftVoltage,RightVoltage);
	//printf("l=%.1f r=%.1f\n", LeftVoltage,RightVoltage);
	#endif

	m_RobotDrive.SetLeftRightMotorOutputs((float)(LeftVoltage),(float)(RightVoltage));
}

//const double c_Arm_DeadZone=0.150;  //was 0.085 for cut off
const double c_Arm_DeadZone=0.085;  //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

double Robot_Control::GetArmCurrentPosition()
{	
	double raw_value = (double)m_Potentiometer.GetAverageValue();
	//raw_value = m_KalFilter_Arm(raw_value);  //apply the Kalman filter
	//Note the value is inverted with the negative operator
	double ret=-FRC_2011_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(raw_value);
	//I may keep these on as they should be useful feedback
	#ifdef __ShowPotentiometerReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	double height=FRC_2011_Robot::Robot_Arm::Arm_AngleToHeight_m(ret);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %.1fft %.1fin", RAD_2_DEG(ret),height*3.2808399,height*39.3700787);
	lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %f %.1fft ", RAD_2_DEG(ret),height,height*3.2808399);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: Pot=%.1f ", raw_value);
	#endif
	return ret;
}




  /***********************************************************************************************************************************/
 /*														Robot_Control_2011															*/
/***********************************************************************************************************************************/

void Robot_Control_2011::UpdateVoltage(size_t index,double Voltage)
{
	switch (index)
	{
		case FRC_2011_Robot::eArm:
		{
			#if 0
			float ToUse=DriverStation::GetInstance()->GetAnalogIn(1) - 1.0;
			m_ArmMotor.Set(ToUse);
			return;
			#endif
			
			//Note: client code needs to check the levels are correct!
			m_ArmMotor.Set(Voltage);  //always the same velocity for both!
			#ifdef __ShowPotentiometerReadings__
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "ArmVolt=%f ", Voltage);
			#endif
		}
			break;
		case FRC_2011_Robot::eRollers:
			m_RollerMotor.Set(Voltage);
			#ifdef __ShowRollerReadings__
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "RollerVolt=%f ", Voltage);
			#endif
			break;
	}
}


void Robot_Control_2011::CloseSolenoid(size_t index,bool Close)
{
	//virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
	//virtual void ReleaseLazySusan(bool Release) {m_LazySusan.SetAngle(Release?Servo::GetMaxAngle():Servo::GetMinAngle());}

	switch (index)
	{
		case FRC_2011_Robot::eDeployment:
			printf("CloseDeploymentDoor=%d\n",Close);
			m_OnDeploy.Set(Close),m_OffDeploy.Set(!Close);
			break;
		case FRC_2011_Robot::eClaw:
			printf("CloseClaw=%d\n",Close);
			m_OnClaw.Set(Close),m_OffClaw.Set(!Close);
			break;
		case FRC_2011_Robot::eRist:
			printf("CloseRist=%d\n",Close);
			m_OnRist.Set(Close),m_OffRist.Set(!Close);
			break;
	}
}






