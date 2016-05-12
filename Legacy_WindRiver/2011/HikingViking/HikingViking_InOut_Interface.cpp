#define  __ShowPotentiometerReadings__
#undef  __ShowEncoderReadings__
#undef  __ShowRollerReadings__

#if defined(__ShowEncoderReadings__) || defined(__ShowPotentiometerReadings__) || defined(__ShowRollerReadings__)
#define __ShowLCD__
#endif

#include "WPILib.h"

#include "stdafx.h"
#include "Robot_Tester.h"

#include "HikingViking_Robot.h"
#include "Common/InOut_Interface.h"
#include "TankDrive/Servo_Robot_Control.h"
#include "TankDrive/Tank_Robot_Control.h"
#include "HikingViking_InOut_Interface.h"


  /***********************************************************************************************************************************/
 /*														HikingViking_Robot_Control													*/
/***********************************************************************************************************************************/

void HikingViking_Robot_Control::ResetPos()
{
	m_Compress.Stop();
	//Allow driver station to control if they want to run the compressor
	if (DriverStation::GetInstance()->GetDigitalIn(7))
	{
		printf("RobotControl reset compressor\n");
		m_Compress.Start();
	}
}

HikingViking_Robot_Control::HikingViking_Robot_Control(bool UseSafety) :
	m_TankRobotControl(UseSafety),m_pTankRobotControl(&m_TankRobotControl),
	m_ArmMotor(5),m_RollerMotor(6),m_Compress(5,2),
	m_OnRist(5),m_OffRist(6),m_OnClaw(3),m_OffClaw(4),m_OnDeploy(2),m_OffDeploy(1),
	m_Potentiometer(1)
{
	ResetPos();
}

HikingViking_Robot_Control::~HikingViking_Robot_Control() 
{
	m_Compress.Stop();
}

void HikingViking_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case HikingViking_Robot::eArm:
			m_KalFilter_Arm.Reset();
			//m_Potentiometer.ResetPos();
			break;
	}
}


void HikingViking_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
}

void HikingViking_Robot_Control::Initialize(const Entity_Properties *props)
{
	const HikingViking_Robot_Properties *robot_props=static_cast<const HikingViking_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotProps=*robot_props;  //save a copy
	m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
}


//const double c_Arm_DeadZone=0.150;  //was 0.085 for cut off
const double c_Arm_DeadZone=0.085;  //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

double HikingViking_Robot_Control::GetRotaryCurrentPorV(size_t index)
{	
	double result=0.0;

	switch (index)
	{
		case HikingViking_Robot::eArm:
		{
			const HikingViking_Robot_Props &props=m_RobotProps.GetHikingVikingRobotProps();

			double raw_value = (double)m_Potentiometer.GetAverageValue();
			raw_value = m_KalFilter_Arm(raw_value);  //apply the Kalman filter
			raw_value=m_ArmAverager.GetAverage(raw_value); //and Ricks x element averager
			//Note the value is inverted with the negative operator
			double PotentiometerRaw_To_Arm;
			{
				const int RawRangeHalf=512;
				PotentiometerRaw_To_Arm=((raw_value / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
				PotentiometerRaw_To_Arm*=props.PotentiometerToArmRatio;  //convert to arm's gear ratio
			}
			result=(-PotentiometerRaw_To_Arm) + m_RobotProps.GetArmProps().GetRotaryProps().PotentiometerOffset;
			SmartDashboard::PutNumber("ArmAngle",RAD_2_DEG(result));
			const double height= (sin(result)*props.ArmLength)+props.GearHeightOffset;
			SmartDashboard::PutNumber("Height",height*3.2808399);
			
			//I may keep these on as they should be useful feedback
			#ifdef __ShowPotentiometerReadings__
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %.1fft %.1fin", RAD_2_DEG(ret),height*3.2808399,height*39.3700787);
			lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %f %.1fft ", RAD_2_DEG(result),height,height*3.2808399);
			//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: Pot=%.1f ", raw_value);
			#endif
			
			//Now to convert to the motor gear ratio as this is what we work in
			result*=props.ArmToGearRatio;
		}
	}
	return result;
}


void HikingViking_Robot_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	switch (index)
	{
		case HikingViking_Robot::eArm:
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
			SmartDashboard::PutNumber("ArmVoltage",Voltage);
		}
			break;
		case HikingViking_Robot::eRollers:
			m_RollerMotor.Set(Voltage);
			#ifdef __ShowRollerReadings__
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "RollerVolt=%f ", Voltage);
			#endif
			SmartDashboard::PutNumber("RollerVoltage",Voltage);
			break;
	}
}


void HikingViking_Robot_Control::CloseSolenoid(size_t index,bool Close)
{
	//virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
	//virtual void ReleaseLazySusan(bool Release) {m_LazySusan.SetAngle(Release?Servo::GetMaxAngle():Servo::GetMinAngle());}

	switch (index)
	{
		case HikingViking_Robot::eDeployment:
			printf("CloseDeploymentDoor=%d\n",Close);
			m_OnDeploy.Set(Close),m_OffDeploy.Set(!Close);
			SmartDashboard::PutBoolean("Deployment",Close);
			break;
		case HikingViking_Robot::eClaw:
			printf("CloseClaw=%d\n",Close);
			m_OnClaw.Set(Close),m_OffClaw.Set(!Close);
			SmartDashboard::PutBoolean("Claw",Close);
			break;
		case HikingViking_Robot::eRist:
			printf("CloseRist=%d\n",Close);
			m_OnRist.Set(Close),m_OffRist.Set(!Close);
			SmartDashboard::PutBoolean("Wrist",Close);
			break;
	}
}


