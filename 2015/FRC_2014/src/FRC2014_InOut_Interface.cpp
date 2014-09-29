#if 0
#include "WPILib.h"

#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2014_Robot.h"
#include "Common/InOut_Interface.h"
#include "Drive/Tank_Robot_Control.h"
#include "Drive/Servo_Robot_Control.h"
#include "Common/Debug.h"
#include "FRC2014_InOut_Interface.h"

#ifdef __DebugLUA__
#define __ShowLCD__
#endif

#undef __DisableMotorControls__
#undef  __EnablePrintfDumps__
#undef __DisableCompressor__

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2014_Robot_Control::ResetPos()
{
	#ifndef __DisableCompressor__
	//Enable this code if we have a compressor 
	m_Compress.Stop();
	//Allow driver station to control if they want to run the compressor
	if (DriverStation::GetInstance()->GetDigitalIn(8))
	{
		printf("RobotControl reset compressor\n");
		m_Compress.Start();
	}
	#endif
}

enum VictorSlotList
{
	eVictor_NoZeroUsed,
	eVictor_RightMotor1,		//1Y Used in InOut_Interface
	eVictor_RightMotor2,		//2  Used in InOut_Interface
	eVictor_LeftMotor1,			//3Y Used in InOut_Interface
	eVictor_LeftMotor2,			//4  Used in InOut_Interface
	eVictor_Helix,				//5
	eVictor_IntakeDeployment,	//6  aka flippers (one motor)
	eVictor_Rollers,			//7y
	eVictor_PowerWheel_First,	//8  The slower wheel
	eVictor_PowerWheel_Second,	//9  The fast motor
	eVictor_IntakeMotor			//10 The transitional motor (tied to rollers)
};
enum RelaySlotList
{
	eRelay_NoZeroUsed,
	eRelay_Compressor=8  //put at the end
};

//No more than 14 cardinal!
enum DigitalIO_SlotList
{
	eDigitalIO_NoZeroUsed,
	eEncoder_DriveRight_A,
	eEncoder_DriveRight_B,
	eEncoder_DriveLeft_A,
	eEncoder_DriveLeft_B,
	eEncoder_IntakeDeployment_A,
	eEncoder_IntakeDeployment_B,
	eEncoder_PowerWheel_First_A,
	eEncoder_PowerWheel_First_B,
	eEncoder_PowerWheel_Second_A,
	eEncoder_PowerWheel_Second_B,
	eSensor_Intake_DeployedLimit,
	eLimit_Compressor=14
};

//Note: If any of these are backwards simply switch the on/off order here only!
enum SolenoidSlotList
{
	eSolenoid_NoZeroUsed,
	eSolenoid_EngageDrive_On,
	eSolenoid_EngageDrive_Off,
	eSolenoid_EngageLiftWinch_On,
	eSolenoid_EngageLiftWinch_Off,
	eSolenoid_EngageDropWinch_On,
	eSolenoid_EngageDropWinch_Off,
	eSolenoid_EngageFirePiston_On,
	eSolenoid_EngageFirePiston_Off,
};

//Note: the order of the initialization list must match the way they are in the class declaration, so if the slots need to change, simply
//change them in the enumerations
FRC_2014_Robot_Control::FRC_2014_Robot_Control(bool UseSafety) :
	m_TankRobotControl(UseSafety),
	#ifdef __UsingTestingKit__
	m_PitchAxis(2),m_TurretAxis(1),
	#else
	m_PitchAxis(2,6),m_TurretAxis(2,5),
	#endif
	m_pTankRobotControl(&m_TankRobotControl),
	//Victors--------------------------------
	m_drive_1(2,eVictor_RightMotor1),m_drive_2(2,eVictor_RightMotor2),
	m_PowerWheel_First_Victor(eVictor_PowerWheel_First),m_PowerWheel_Second_Victor(eVictor_PowerWheel_Second),
	m_Helix_Victor(eVictor_Helix),
	m_IntakeMotor_Victor(eVictor_IntakeMotor),m_Rollers_Victor(eVictor_Rollers),m_IntakeDeployment_Victor(eVictor_IntakeDeployment),
	m_Compress(eLimit_Compressor,eRelay_Compressor),
	//Solenoids------------------------------
	m_EngageDrive(eSolenoid_EngageDrive_On,eSolenoid_EngageDrive_Off),
	m_EngageLiftWinch(eSolenoid_EngageLiftWinch_On,eSolenoid_EngageLiftWinch_Off),
	m_EngageDropWinch(eSolenoid_EngageDropWinch_On,eSolenoid_EngageDropWinch_Off),
	m_EngageFirePiston(eSolenoid_EngageFirePiston_On,eSolenoid_EngageFirePiston_Off),
	//Sensors----------------------------------
	m_IntakeDeployment_Encoder(eEncoder_IntakeDeployment_A,eEncoder_IntakeDeployment_B,false,CounterBase::k4X),
	m_PowerWheel_First_Encoder(eEncoder_PowerWheel_First_A,eEncoder_PowerWheel_First_B),
	m_PowerWheel_Second_Encoder(eEncoder_PowerWheel_Second_A,eEncoder_PowerWheel_Second_B),
	m_Intake_DeployedLimit(eSensor_Intake_DeployedLimit),
	//other----------------------------------
	//m_PowerWheelAverager(0.5),
	m_PowerWheel_PriorityAverager(10,0.30),
	m_IntakeDeploymentOffset(DEG_2_RAD(90.0)),
	m_IsDriveEngaged(true)
{
	//TODO set the SetDistancePerPulse() for turret
	ResetPos();
	const double EncoderPulseRate=(1.0/360.0);
	m_IntakeDeployment_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_PowerWheel_First_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_PowerWheel_Second_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_IntakeDeployment_Encoder.Start();
	m_PowerWheel_First_Encoder.Start();
	m_PowerWheel_Second_Encoder.Start();
	m_PowerWheelFilter.Reset();
}

FRC_2014_Robot_Control::~FRC_2014_Robot_Control() 
{
	//m_Compress.Stop();
	m_IntakeDeployment_Encoder.Stop();
	m_PowerWheel_First_Encoder.Stop();
	m_PowerWheel_Second_Encoder.Stop();
}

void FRC_2014_Robot_Control::Reset_Rotary(size_t index)
{
	//we probably will not need Kalman filters
	//m_KalFilter_Arm.Reset();
}

void FRC_2014_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
	//if (GetBoolSensorState(FRC_2014_Robot::eIntake_DeployedLimit_Sensor))
	//{
	//	m_IntakeDeployment_Encoder.Reset();
	//	m_IntakeDeploymentOffset=0.0;
	//}
}

void FRC_2014_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
	
	//Note: this will be NULL when Low Gear comes through here!
	const FRC_2014_Robot_Properties *robot_props=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy
		//Not sure I need to this since I got the copy... we'll see
		//m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
	}
	//CloseSolenoid(FRC_2014_Robot::eFirePiston);
}

//NOTE: for now, never pulse the spike relays, there is too much debate on the damage that will cause
Relay::Value TranslateToRelay(double Voltage)
{
	Relay::Value ret=Relay::kOff;  //*NEVER* want both on!
	const double Threshold=0.08;  //This value is based on dead voltage for arm... feel free to adjust, but keep high enough to avoid noise
	
	if (Voltage>Threshold)
		ret=Relay::kForward;
	else if (Voltage<-Threshold)
		ret=Relay::kReverse;
	return ret;
}

void FRC_2014_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	#ifndef __DisableMotorControls__
	//switch (index)
	//{
	//}
	#endif
}

void FRC_2014_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) 
{
	const Tank_Robot_Props &TankRobotProps=m_RobotProps.GetTankRobotProps();
	if (!TankRobotProps.ReverseSteering)
	{
		m_drive_1.Set((float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
		m_drive_2.Set(-(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
	}
	else
	{
		m_drive_1.Set((float)RightVoltage * TankRobotProps.VoltageScalar_Right);
		m_drive_2.Set(-(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
	}
	m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}

void FRC_2014_Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) 
{
	m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);
	//For climb states... use only one encoder as a safety precaution
	if (!m_IsDriveEngaged)
		RightVelocity=LeftVelocity;
}

bool FRC_2014_Robot_Control::GetBoolSensorState(size_t index)
{
	bool ret=false;
	//switch (index)
	//{
	//case FRC_2014_Robot::eIntake_DeployedLimit_Sensor:
	//	ret= m_Intake_DeployedLimit.Get()!=0;
	//	break;
	//default:
	//	assert (false);
	//}
	return ret;
}

double FRC_2014_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	//switch (index)
	//{
	//}
	
	return result;
}

void FRC_2014_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	//const char * const SolenoidState=Open?"Engaged":"Disengaged";
	//Translate the open state into a value for double solenoid
	//DoubleSolenoid::Value value=Open ? DoubleSolenoid::kForward : DoubleSolenoid::kReverse;
	//switch (index)
	//{
	//}
}

bool FRC_2014_Robot_Control::GetIsSolenoidOpen(size_t index) const
{
	bool ret=false;
	//switch (index)
	//{
	//}
	return ret;
}

void FRC_2014_Robot_Control::Reset_Servo(size_t index)
{

	 //may want to just center these
	//switch (index)
	//{
	//}
}

double FRC_2014_Robot_Control::GetServoAngle(size_t index)
{
	double result=0.0;
	//switch (index)
	//{
	//}
	return result;
}

void FRC_2014_Robot_Control::SetServoAngle(size_t index,double radians)
{
	//switch (index)
	//{
	//}
}

#endif

