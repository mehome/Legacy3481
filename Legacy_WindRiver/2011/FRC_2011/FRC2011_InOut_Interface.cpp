#undef  __EncoderHack__
#define  __ShowPotentiometerReadings__
#undef  __ShowEncoderReadings__
#undef  __ShowRollerReadings__

#if defined(__ShowEncoderReadings__) || defined(__ShowPotentiometerReadings__) || defined(__ShowRollerReadings__)
#define __ShowLCD__
#endif

#include "WPILib.h"

#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2011_Robot.h"
#include "Common/InOut_Interface.h"
#include "TankDrive/Tank_Robot_Control.h"
#include "TankDrive/Servo_Robot_Control.h"
#include "Common/Debug.h"
#include "FRC2011_InOut_Interface.h"


  /***********************************************************************************************************************************/
 /*														FRC_2011_Robot_Control														*/
/***********************************************************************************************************************************/

void FRC_2011_Robot_Control::ResetPos()
{
	m_Compress.Stop();
	//Allow driver station to control if they want to run the compressor
	if (DriverStation::GetInstance()->GetDigitalIn(7))
	{
		printf("RobotControl reset compressor\n");
		m_Compress.Start();
	}
}

FRC_2011_Robot_Control::FRC_2011_Robot_Control(bool UseSafety) :
	m_TankRobotControl(UseSafety),m_pTankRobotControl(&m_TankRobotControl),
	m_ArmMotor(5),m_RollerMotor(6),m_Compress(5,2),
	m_OnRist(5),m_OffRist(6),m_OnClaw(3),m_OffClaw(4),m_OnDeploy(2),m_OffDeploy(1),
	m_Potentiometer(1)
	#ifndef __2011_TestCamera__
	,m_Camera(NULL)
	#endif
{
	ResetPos();
	#ifndef __2011_TestCamera__
	//Should not need to wait, fixed in 2012
	//Wait(10.000);
	m_Camera=&AxisCamera::GetInstance();
	#endif
}

FRC_2011_Robot_Control::~FRC_2011_Robot_Control() 
{
	m_Compress.Stop();
	#ifndef __2011_TestCamera__
	m_Camera=NULL;  //We don't own this, but I do wish to treat it like we do
	#endif
}

void FRC_2011_Robot_Control::Reset_Arm(size_t index)
{
	m_KalFilter_Arm.Reset();
}


void FRC_2011_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
	#ifdef __2011_TestCamera__
	m_Camera.CameraProcessing_TimeChange(dTime_s);
	#endif
}

void FRC_2011_Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2011_Robot_Properties *robot_props=static_cast<const FRC_2011_Robot_Properties *>(props);
	assert(robot_props);
	m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
}


//const double c_Arm_DeadZone=0.150;  //was 0.085 for cut off
const double c_Arm_DeadZone=0.085;  //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

double FRC_2011_Robot_Control::GetArmCurrentPosition(size_t index)
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


void FRC_2011_Robot_Control::UpdateVoltage(size_t index,double Voltage)
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


void FRC_2011_Robot_Control::CloseSolenoid(size_t index,bool Close)
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


