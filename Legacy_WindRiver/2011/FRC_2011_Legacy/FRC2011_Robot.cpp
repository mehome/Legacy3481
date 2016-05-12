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
#include "Common/AI_Base_Controller.h"
#include "Common/Robot_Tank.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2011_Robot.h"

#undef __UseTestKitArmRatios__
const bool c_UsingArmLimits=true;
const double PI=M_PI;

using namespace Framework::Base;
using namespace std;

const double c_OptimalAngleUp_r=DEG_2_RAD(44.3);  //70
const double c_OptimalAngleDn_r=DEG_2_RAD(58.0);  //50
const double c_ArmLength_m=1.8288;  //6 feet

#ifndef __UseTestKitArmRatios__
const double c_ArmToGearRatio=72.0/28.0;
#else
const double c_ArmToGearRatio=54.0/12.0;
#endif

const double c_GearToArmRatio=1.0/c_ArmToGearRatio;

#ifndef __UseTestKitArmRatios__
const double c_PotentiometerToArmRatio=c_GearToArmRatio * (60.0/36.0);
const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
//const double c_PotentiometerToGearRatio=60.0/32.0;
//const double c_PotentiometerToArmRatio=c_PotentiometerToGearRatio * c_GearToArmRatio;
#else
const double c_PotentiometerToArmRatio=36.0/54.0;
const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
#endif

const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Claw														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Claw::Robot_Claw(const char EntityName[],Robot_Control_Interface *robot_control) :
	Ship_1D(EntityName),m_RobotControl(robot_control),m_Grip(false),m_Squirt(false)
{
}

void FRC_2011_Robot::Robot_Claw::TimeChange(double dTime_s)
{
	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Grip ^ m_Squirt)
		SetCurrentLinearAcceleration(m_Grip?ACCEL:-BRAKE);

	__super::TimeChange(dTime_s);
	//send out the voltage
	double CurrentVelocity=m_Physics.GetVelocity();
	double Voltage=CurrentVelocity/MAX_SPEED;

	//Clamp range
	if (Voltage>0.0)
	{
		if (Voltage>1.0)
			Voltage=1.0;
	}
	else if (Voltage<0.0)
	{
		if (Voltage<-1.0)
			Voltage=-1.0;
	}
	else
		Voltage=0.0;  //is nan case

	m_RobotControl->UpdateVoltage(eRollers,Voltage);
}

void FRC_2011_Robot::Robot_Claw::CloseClaw(bool Close)
{
	m_RobotControl->CloseSolenoid(eClaw,Close);
}

void FRC_2011_Robot::Robot_Claw::Grip(bool on)
{
	m_Grip=on;
}

void FRC_2011_Robot::Robot_Claw::Squirt(bool on)
{
	m_Squirt=on;
}

void FRC_2011_Robot::Robot_Claw::BindAdditionalEventControls(bool Bind)
{
	Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Claw_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2011_Robot::Robot_Claw::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Claw_Close"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Claw::CloseClaw);
		em->EventOnOff_Map["Claw_Grip"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Claw::Grip);
		em->EventOnOff_Map["Claw_Squirt"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Claw::Squirt);
	}
	else
	{
		em->EventValue_Map["Claw_SetCurrentVelocity"].Remove(*this, &FRC_2011_Robot::Robot_Claw::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Claw_Close"]  .Remove(*this, &FRC_2011_Robot::Robot_Claw::CloseClaw);
		em->EventOnOff_Map["Claw_Grip"]  .Remove(*this, &FRC_2011_Robot::Robot_Claw::Grip);
		em->EventOnOff_Map["Claw_Squirt"]  .Remove(*this, &FRC_2011_Robot::Robot_Claw::Squirt);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Arm::Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Ship_1D(EntityName),m_RobotControl(robot_control),
	//m_PIDController(0.5,1.0,0.0),
	//m_PIDController(1.0,0.5,0.0),
	m_PIDController(1.0,1.0/8.0,0.0),
	//m_PIDController(1.0,1.0/2.0,0.0),
	m_LastPosition(0.0),m_CalibratedScaler(1.0),m_LastTime(0.0),
	m_UsingPotentiometer(false),  //to be safe
	m_VoltageOverride(false)
{
	m_UsingPotentiometer=true;  //for testing on AI simulator (unless I make a control for this)
}

void FRC_2011_Robot::Robot_Arm::Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props)
{
	m_LastPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;
	__super::Initialize(em,props);
	const Ship_1D_Properties *ship=static_cast<const Ship_1D_Properties *>(props);
	assert(ship);
	m_MaxSpeedReference=ship->GetMaxSpeed();
	m_PIDController.SetInputRange(-m_MaxSpeedReference,m_MaxSpeedReference);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDController.SetOutputRange(-m_MaxSpeedReference*tolerance,m_MaxSpeedReference*tolerance);
	m_PIDController.Enable();
	m_CalibratedScaler=MAX_SPEED;
}

double FRC_2011_Robot::Robot_Arm::AngleToHeight_m(double Angle_r)
{
	return (sin(Angle_r*c_GearToArmRatio)*c_ArmLength_m)+c_GearHeightOffset;
}
double FRC_2011_Robot::Robot_Arm::Arm_AngleToHeight_m(double Angle_r)
{
	return (sin(Angle_r)*c_ArmLength_m)+c_GearHeightOffset;
}

double FRC_2011_Robot::Robot_Arm::HeightToAngle_r(double Height_m)
{
	return asin((Height_m-c_GearHeightOffset)/c_ArmLength_m) * c_ArmToGearRatio;
}

double FRC_2011_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(double raw)
{
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=c_PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}


void FRC_2011_Robot::Robot_Arm::TimeChange(double dTime_s)
{
	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated
	double PotentiometerVelocity; //increased scope for debugging dump
	
	//Update the position to where the potentiometer says where it actually is
	if (m_UsingPotentiometer)
	{
		if (m_LastTime!=0.0)
		{
			double LastSpeed=fabs(m_Physics.GetVelocity());  //This is last because the time change has not happened yet
			double NewPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;

			//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
			double Displacement=NewPosition-m_LastPosition;
			PotentiometerVelocity=Displacement/m_LastTime;
			double PotentiometerSpeed=fabs(PotentiometerVelocity);

			double control=0.0;
			control=-m_PIDController(LastSpeed,PotentiometerSpeed,dTime_s);
			m_CalibratedScaler=MAX_SPEED+control;

			//DOUT5("pSpeed=%f cal=%f Max=%f",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);
			//printf("\rpSp=%f cal=%f Max=%f                 ",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);

			SetPos_m(NewPosition);
			m_LastPosition=NewPosition;
		}
		m_LastTime=dTime_s;
	}
	else
	{
		//Test potentiometer readings without applying to current position (disabled by default)
		m_RobotControl->GetArmCurrentPosition();
		//This is only as a sanity fix for manual mode... it should be this already (I'd assert if I could)
		//MAX_SPEED=m_CalibratedScaler=1.0;
	}
	__super::TimeChange(dTime_s);
	double CurrentVelocity=m_Physics.GetVelocity();
	
	//TODO this should be fixed omit this code once confirmed
	//Unfortunately something happened when the wires got crossed during the texas round up, now needing to reverse the voltage
	//This was also reversed for the testing kit.  We apply reverse on current velocity for squaring operation to work properly, and
	//must not do this in the interface, since that will support next year's robot.
	//CurrentVelocity*=-1.0; 
	
	double Voltage=CurrentVelocity/m_CalibratedScaler;

	if (!m_VoltageOverride)
	{
		//Clamp range, PID (i.e. integral) controls may saturate the amount needed
		if (Voltage>0.0)
		{
			if (Voltage>1.0)
				Voltage=1.0;
		}
		else if (Voltage<0.0)
		{
			if (Voltage<-1.0)
				Voltage=-1.0;
		}
		else
			Voltage=0.0;  //is nan case
	}
	else
	{
		Voltage=0.0;
		m_PIDController.ResetI(m_MaxSpeedReference * -0.99);  //clear error for I for better transition back
	}

	#if 0
	Voltage*=Voltage;  //square them for more give
	//restore the sign
	if (CurrentVelocity<0)
		Voltage=-Voltage;
	#endif

	#if 0
	if (Voltage!=0.0)
	{
		double PosY=m_LastPosition;
		if (!m_VoltageOverride)
			printf("v=%f y=%f p=%f e=%f d=%f cs=%f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,fabs(CurrentVelocity)-fabs(PotentiometerVelocity),m_CalibratedScaler);
		else
			printf("v=%f y=%f VO p=%f e=%f d=%f cs=%f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,fabs(CurrentVelocity)-fabs(PotentiometerVelocity),m_CalibratedScaler);
	}
	#endif

	m_RobotControl->UpdateVoltage(eArm,Voltage);
	//Show current height (only in AI Tester)
	#if 0
	double Pos_m=GetPos_m();
	double height=AngleToHeight_m(Pos_m);
	if (!m_VoltageOverride)
		DOUT4("Arm=%f Angle=%f %fft %fin",CurrentVelocity,RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
	else
		DOUT4("VO Arm=%f Angle=%f %fft %fin",CurrentVelocity,RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
	#endif
}

void FRC_2011_Robot::Robot_Arm::PosDisplacementCallback(double posDisplacement_m)
{
	m_VoltageOverride=false;
	//note 0.02 is fine for arm without claw
	if ((m_UsingPotentiometer)&&(!GetLockShipToPosition())&&(fabs(posDisplacement_m)<0.20))
		m_VoltageOverride=true;
}

void FRC_2011_Robot::Robot_Arm::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	if (m_UsingPotentiometer)
	{
		m_PIDController.Reset();
		m_RobotControl->Reset_Arm();
		double NewPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;
		Stop();
		SetPos_m(NewPosition);
		m_LastPosition=NewPosition;
	}
}

void FRC_2011_Robot::Robot_Arm::SetPotentiometerSafety(double Value)
{
	//printf("\r%f       ",Value);
	if (Value < -0.8)
	{
		if (m_UsingPotentiometer)
		{
			//first disable it
			m_UsingPotentiometer=false;
			//Now to reset stuff
			printf("Disabling potentiometer\n");
			//m_PIDController.Reset();
			ResetPos();
			//This is no longer necessary
			//MAX_SPEED=m_MaxSpeedReference;
			m_LastPosition=0.0;
			m_CalibratedScaler=MAX_SPEED;
			m_LastTime=0.0;
			m_UsingRange=false;
		}
	}
	else
	{
		if (!m_UsingPotentiometer)
		{
			m_UsingPotentiometer=true;
			//setup the initial value with the potentiometers value
			printf("Enabling potentiometer\n");
			ResetPos();
			m_UsingRange=true;
			m_CalibratedScaler=MAX_SPEED;
		}
	}
}

double ArmHeightToBack(double value)
{
	const double Vertical=PI/2.0*c_ArmToGearRatio;
	return Vertical + (Vertical-value);
}

double FRC_2011_Robot::Robot_Arm::GetPosRest()
{
	return HeightToAngle_r(-0.02);
}
void FRC_2011_Robot::Robot_Arm::SetPosRest()
{
	SetIntendedPosition(GetPosRest()  );
}
void FRC_2011_Robot::Robot_Arm::SetPos0feet()
{
	//SetIntendedPosition( HeightToAngle_r(0.02) );
	SetIntendedPosition( HeightToAngle_r(-0.4) ); //about 31 - 39 inches with raptor claw
}
void FRC_2011_Robot::Robot_Arm::SetPos3feet()
{
	//Not used, but kept for reference
	//SetIntendedPosition(ArmHeightToBack( HeightToAngle_r(1.143)) );
	//SetIntendedPosition(HeightToAngle_r(0.9144));  //actual
	//SetIntendedPosition(HeightToAngle_r(0.80001));  //31.5 inches
	//SetIntendedPosition(HeightToAngle_r(0.94800));  //36 inches
	SetIntendedPosition(HeightToAngle_r(0.812842));  //67 inches with raptor claw
}
void FRC_2011_Robot::Robot_Arm::SetPos6feet()
{
	//SetIntendedPosition( HeightToAngle_r(1.8288) );  //actual
	//SetIntendedPosition( HeightToAngle_r(1.7018) );  //67 inches
	//SetIntendedPosition( HeightToAngle_r(1.08712) );  //42.8 inches
	//SetIntendedPosition( HeightToAngle_r(0.71000) );  //72 inches with wrist up
	SetIntendedPosition( HeightToAngle_r(2.193113) );  //104.5 inches 9 side raptor claw
}
void FRC_2011_Robot::Robot_Arm::SetPos9feet()
{
	//SetIntendedPosition( HeightToAngle_r(2.7432) );  //actual
	//SetIntendedPosition( HeightToAngle_r(2.6543) ); //104.5 inches
	SetIntendedPosition( HeightToAngle_r(2.715686) ); //112 inches middle raptor claw
}
void FRC_2011_Robot::Robot_Arm::CloseRist(bool Close)
{
	m_RobotControl->CloseSolenoid(eRist,Close);
}

void FRC_2011_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Arm_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2011_Robot::Robot_Arm::SetPotentiometerSafety);
		
		em->Event_Map["Arm_SetPosRest"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPosRest);
		em->Event_Map["Arm_SetPos0feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos9feet);
		em->EventOnOff_Map["Arm_Rist"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::CloseRist);
	}
	else
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Arm_SetPotentiometerSafety"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPotentiometerSafety);

		em->Event_Map["Arm_SetPosRest"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPosRest);
		em->Event_Map["Arm_SetPos0feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos9feet);
		em->EventOnOff_Map["Arm_Rist"]  .Remove(*this, &FRC_2011_Robot::Robot_Arm::CloseRist);
	}
}

  /***********************************************************************************************************************************/
 /*															FRC_2011_Robot															*/
/***********************************************************************************************************************************/
FRC_2011_Robot::FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders) : 
	Robot_Tank(EntityName), m_RobotControl(robot_control), m_Arm(EntityName,robot_control), m_Claw(EntityName,robot_control),
	//m_PIDController_Left(1.0,1.0,0.25),	m_PIDController_Right(1.0,1.0,0.25),
	m_PIDController_Left(1.0,1.0,0.0),	m_PIDController_Right(1.0,1.0,0.0),
	//m_PIDController_Left(0.0,0.0,0.0),	m_PIDController_Right(0.0,0.0,0.0),
	m_UsingEncoders(UseEncoders),m_VoltageOverride(false),m_UseDeadZoneSkip(true)
{
	//m_UsingEncoders=true; //testing
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=1.0;
}

void FRC_2011_Robot::Initialize(Framework::Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const FRC_2011_Robot_Properties *RobotProps=static_cast<const FRC_2011_Robot_Properties *>(props);
	m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
	m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);

	const double OutputRange=MAX_SPEED*0.875;  //create a small range
	const double InputRange=20.0;  //create a large enough number that can divide out the voltage and small enough to recover quickly
	m_PIDController_Left.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Left.SetOutputRange(-InputRange,OutputRange);
	m_PIDController_Left.Enable();
	m_PIDController_Right.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Right.SetOutputRange(-InputRange,OutputRange);
	m_PIDController_Right.Enable();
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
}
void FRC_2011_Robot::ResetPos()
{
	__super::ResetPos();
	m_Arm.ResetPos();
	m_Claw.ResetPos();
	m_RobotControl->Reset_Encoders();
	m_PIDController_Left.Reset(),m_PIDController_Right.Reset();
	//ensure teleop has these set properly
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
	m_UseDeadZoneSkip=true;
}

void FRC_2011_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->TimeChange(dTime_s);
	if (m_UsingEncoders)
	{
		double Encoder_LeftVelocity,Encoder_RightVelocity;
		m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);

		double LeftVelocity=GetLeftVelocity();
		double RightVelocity=GetRightVelocity();

		double control_left=0.0,control_right=0.0;
		//only adjust calibration when both velocities are in the same direction, or in the case where the encoder is stopped which will
		//allow the scaler to normalize if it need to start up again.
		if (((LeftVelocity * Encoder_LeftVelocity) > 0.0) || IsZero(Encoder_LeftVelocity) )
		{
			control_left=-m_PIDController_Left(fabs(LeftVelocity),fabs(Encoder_LeftVelocity),dTime_s);
			m_CalibratedScaler_Left=MAX_SPEED+control_left;
		}
		if (((RightVelocity * Encoder_RightVelocity) > 0.0) || IsZero(Encoder_RightVelocity) )
		{
			control_right=-m_PIDController_Right(fabs(RightVelocity),fabs(Encoder_RightVelocity),dTime_s);
			m_CalibratedScaler_Right=MAX_SPEED+control_right;
		}

		//Adjust the engaged max speed to avoid the PID from overflow lockup
		//ENGAGED_MAX_SPEED=(m_CalibratedScaler_Left+m_CalibratedScaler_Right) / 2.0;
		//DOUT5("p=%f e=%f d=%f cs=%f",RightVelocity,Encoder_RightVelocity,RightVelocity-Encoder_RightVelocity,m_CalibratedScaler_Right);
		//printf("\rcl=%f cr=%f, csl=%f csr=%f                ",control_left,control_right,m_CalibratedScaler_Left,m_CalibratedScaler_Right);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,Encoder_LeftVelocity,RightVelocity,Encoder_RightVelocity);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,m_CalibratedScaler_Left,RightVelocity,m_CalibratedScaler_Right);
		//printf("\rp=%f e=%f d=%f cs=%f          ",RightVelocity,Encoder_RightVelocity,RightVelocity-Encoder_RightVelocity,m_CalibratedScaler_Right);
		
		#if 0
		if (RightVelocity!=0.0)
		{
			double PosY=GetPos_m()[1];
			
			//if (!m_VoltageOverride)
			//	printf("y=%f p=%f e=%f d=%f cs=%f\n",PosY,RightVelocity,Encoder_RightVelocity,fabs(RightVelocity)-fabs(Encoder_RightVelocity),m_CalibratedScaler_Right);
			//else
			//	printf("y=%f VO p=%f e=%f d=%f cs=%f\n",PosY,RightVelocity,Encoder_RightVelocity,fabs(RightVelocity)-fabs(Encoder_RightVelocity),m_CalibratedScaler_Right);
			
			if (!m_VoltageOverride)
				printf("y=%f el=%f er=%f dl=%f dr=%f cs=%f\n",PosY,Encoder_LeftVelocity,Encoder_RightVelocity,fabs(LeftVelocity)-fabs(Encoder_LeftVelocity),fabs(RightVelocity)-fabs(Encoder_RightVelocity),m_CalibratedScaler_Right);
			else
				printf("y=%f VO el=%f er=%f dl=%f dr=%f cs=%f\n",PosY,Encoder_LeftVelocity,Encoder_RightVelocity,fabs(LeftVelocity)-fabs(Encoder_LeftVelocity),fabs(RightVelocity)-fabs(Encoder_RightVelocity),m_CalibratedScaler_Right);

		}
		#endif

		//For most cases we do not need the dead zone skip
		m_UseDeadZoneSkip=false;
		
		//We only use deadzone when we are accelerating in either direction, so first check that both sides are going in the same direction
		//also only apply for lower speeds to avoid choppyness during the cruising phase
		if ((RightVelocity*LeftVelocity > 0.0) && (fabs(Encoder_RightVelocity)<0.5))
		{
			//both sides of velocities are going in the same direction we only need to test one side to determine if it is accelerating
			m_UseDeadZoneSkip=(RightVelocity<0) ? (RightVelocity<Encoder_RightVelocity) :  (RightVelocity>Encoder_RightVelocity); 
		}
		
		#if 1
		//Update the physics with the actual velocity
		Vec2d LocalVelocity;
		double AngularVelocity;
		InterpolateVelocities(Encoder_LeftVelocity,Encoder_RightVelocity,LocalVelocity,AngularVelocity,dTime_s);
		//TODO add gyro's yaw readings for Angular velocity here
		//Store the value here to be picked up in GetOldVelocity()
		m_EncoderGlobalVelocity=LocalToGlobal(GetAtt_r(),LocalVelocity);
		m_EncoderHeading=AngularVelocity;
		//printf("\rG[0]=%f G[1]=%f        ",m_EncoderGlobalVelocity[0],m_EncoderGlobalVelocity[1]);
		//printf("G[0]=%f G[1]=%f\n",m_EncoderGlobalVelocity[0],m_EncoderGlobalVelocity[1]);
		#endif
	}
	else
	{
		//Display encoders without applying calibration
		double Encoder_LeftVelocity,Encoder_RightVelocity;
		m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	}
	__super::TimeChange(dTime_s);
	Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	arm_entity.TimeChange(dTime_s);
	Entity1D &claw_entity=m_Claw;  //This gets around keeping time change protected in derived classes
	claw_entity.TimeChange(dTime_s);
}

bool FRC_2011_Robot::InjectDisplacement(double DeltaTime_s,Vec2d &PositionDisplacement,double &RotationDisplacement)
{
	bool ret=false;
	if (m_UsingEncoders)
	{
		Vec2d computedVelocity=m_Physics.GetLinearVelocity();
		//double computedAngularVelocity=m_Physics.GetAngularVelocity();
		m_Physics.SetLinearVelocity(m_EncoderGlobalVelocity);
		//m_Physics.SetAngularVelocity(m_EncoderHeading);
		m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
		//We must set this back so that the PID can compute the entire error
		m_Physics.SetLinearVelocity(computedVelocity);
		//m_Physics.SetAngularVelocity(computedAngularVelocity);
		ret=true;
	}
	return ret;
}

double FRC_2011_Robot::RPS_To_LinearVelocity(double RPS)
{
	return RPS * c_MotorToWheelGearRatio * M_PI * c_WheelDiameter; 
}

void FRC_2011_Robot::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
{
	m_VoltageOverride=false;
	if ((m_UsingEncoders)&&(VelocityToUse==0.0)&&(m_rotDisplacement_rad==0.0))
			m_VoltageOverride=true;
}

//TODO recalibrate the dead zones
//It prior to cow town these were 0.110, 0.04, 0.02, and 0.115  (keep these as a reference)
const double c_rMotorDriveForward_DeadZone=0.02;
const double c_rMotorDriveReverse_DeadZone=0.02;
const double c_lMotorDriveForward_DeadZone=0.02;
const double c_lMotorDriveReverse_DeadZone=0.02;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void FRC_2011_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	double LeftVelocity=GetLeftVelocity(),RightVelocity=GetRightVelocity();
	double LeftVoltage,RightVoltage;
	if (m_VoltageOverride)
		LeftVoltage=RightVoltage=0;
	else
	{
		{
			#if 0
			double Encoder_LeftVelocity,Encoder_RightVelocity;
			m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
			DOUT5("left=%f %f Right=%f %f",Encoder_LeftVelocity,LeftVelocity,Encoder_RightVelocity,RightVelocity);
			#endif
			//printf("\r%f %f           ",m_CalibratedScaler_Left,m_CalibratedScaler_Right);
			LeftVoltage=LeftVelocity/m_CalibratedScaler_Left,RightVoltage=RightVelocity/m_CalibratedScaler_Right;

			//In teleop always square as it feels right and gives more control to the user
			//for autonomous (i.e. using encoders) the natural distribution on acceleration will give the best results
			//we can use the m_UseDeadZoneSkip to determine if we are accelerating, more important we must square on
			//deceleration to improve our chance to not overshoot!
			if ((!m_UsingEncoders) || (!m_UseDeadZoneSkip))
			{
				LeftVoltage*=LeftVoltage,RightVoltage*=RightVoltage;  //square them for more give
				//Clip the voltage as it can become really high values when squaring
				if (LeftVoltage>1.0)
					LeftVoltage=1.0;
				if (RightVoltage>1.0)
					RightVoltage=1.0;
				//restore the sign
				if (LeftVelocity<0)
					LeftVoltage=-LeftVoltage;
				if (RightVelocity<0)
					RightVoltage=-RightVoltage;
			}
		}
		// m_UseDeadZoneSkip,  When true this is ideal for telop, and for acceleration in autonomous as it always starts movement
		// equally on both sides, and avoids stalls.  For deceleration in autonomous, set to false as using the correct 
		// linear distribution of voltage will help avoid over-compensation, especially as it gets closer to stopping
		if (m_UseDeadZoneSkip)
		{
			//Eliminate the deadzone
			if (LeftVoltage>0.0)
				LeftVoltage=(LeftVoltage * c_lMotorDriveForward_Range) + c_lMotorDriveForward_DeadZone;
			else if (LeftVoltage < 0.0)
				LeftVoltage=(LeftVoltage * c_lMotorDriveReverse_Range) - c_lMotorDriveReverse_DeadZone;
		
			if (RightVoltage>0.0)
				RightVoltage=(RightVoltage * c_rMotorDriveForward_Range) + c_rMotorDriveForward_DeadZone;
			else if (RightVoltage < 0.0)
				RightVoltage=(RightVoltage * c_rMotorDriveReverse_Range) - c_rMotorDriveReverse_DeadZone;
		}
	}
	//if (fabs(RightVoltage)>0.0) printf("RV %f dzk=%d ",RightVoltage,m_UseDeadZoneSkip);
	//Unfortunately the actual wheels are reversed (resolved here since this is this specific robot)
	m_RobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);
}

void FRC_2011_Robot::CloseDeploymentDoor(bool Close)
{
	m_RobotControl->CloseSolenoid(eDeployment,Close);
}

void FRC_2011_Robot::BindAdditionalEventControls(bool Bind)
{
	Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
		em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &FRC_2011_Robot::CloseDeploymentDoor);
	else
		em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &FRC_2011_Robot::CloseDeploymentDoor);

	Ship_1D &ArmShip_Access=m_Arm;
	ArmShip_Access.BindAdditionalEventControls(Bind);
	Ship_1D &ClawShip_Access=m_Claw;
	ClawShip_Access.BindAdditionalEventControls(Bind);
}


  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot_Properties														*/
/***********************************************************************************************************************************/

FRC_2011_Robot_Properties::FRC_2011_Robot_Properties() : m_ArmProps(
	"Arm",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	18.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse  find the balance between being quick enough without jarring the tube out of its grip
	Ship_1D_Properties::eRobotArm,
	c_UsingArmLimits,	//Using the range
	-c_OptimalAngleDn_r*c_ArmToGearRatio,c_OptimalAngleUp_r*c_ArmToGearRatio
	),
	m_ClawProps(
	"Claw",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//RS-550 motor with 64:1 BaneBots transmission, so this is spec at 19300 rpm free, and 17250 peak efficiency
	//17250 / 64 = 287.5 = rps of motor / 64 reduction = 4.492 rps * 2pi = 28.22524
	28,   //Max Speed (rounded as we need not have precision)
	112.0,112.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Properties::eRobotClaw,
	false	//No limit ever!
	)
{
}
  /***********************************************************************************************************************************/
 /*														FRC_2011_UI_Controller														*/
/***********************************************************************************************************************************/
#define __WindRiverJoysticks__
#undef __AirFlo__
#undef __UsingXTerminator__
#undef __UsingWPTH_UI__ //The WPLib Testing Harness UI (where the second joystick is on the UI itself)

FRC_2011_UI_Controller::FRC_2011_UI_Controller(Framework::UI::JoyStick_Binder &joy,AI_Base_Controller *base_controller) : UI_Controller(joy,base_controller)
{
	using namespace Framework::UI;
	#if 0
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Joystick_SetCurrentSpeed_2",true,1.0,0.1,false,"Joystick_1");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",false,1.0,0.1,true,"Joystick_1");
	//joy.AddJoy_Button_Default(6,"Slide",false);
	//joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Rot,"Analog_StrafeRight");
	#endif

	#ifdef __UsingXTerminator__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Rot,"Arm_SetCurrentVelocity",false,1.0,0.04,true,"Joystick_1");
	joy.AddJoy_Button_Default(6,"Arm_SetPos0feet",false);
	joy.AddJoy_Button_Default(5,"Arm_SetPos3feet",false);
	joy.AddJoy_Button_Default(4,"Arm_SetPos6feet",false);
	joy.AddJoy_Button_Default(8,"Arm_SetPos9feet",false);
	#endif
	#ifdef __UsingWPTH_UI__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Arm_SetCurrentVelocity",false,1.0,0.04,true,"Joystick_2");
	//joy.AddJoy_Button_Default(0,"Arm_SetPos0feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default(0,"Arm_Claw",true,false,"Joystick_2");
	//Not sure why the simulator skipped 1
	joy.AddJoy_Button_Default(2,"Robot_OpenDoor",true,false,"Joystick_2");
	joy.AddJoy_Button_Default(3,"Arm_SetPos0feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default(4,"Arm_SetPos9feet",false,false,"Joystick_2");
	#endif
	#ifdef __AirFlo__
	//For the Y Axis 3rd paramter false = down for up like flying a plane
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Arm_SetCurrentVelocity",false,1.0,0.1,true,"Joystick_1");
	//joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Arm_SetPotentiometerSafety",false,1.0,0.04,false,"Joystick_2");
	//This is no longer needed as the zero and the rest are the same
	//joy.AddJoy_Button_Default( 7,"Arm_SetPosRest",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 0,"Arm_SetPos0feet",false,false,"Joystick_1");
	joy.AddJoy_Button_Default( 2,"Arm_SetPos3feet",false,false,"Joystick_1");
	joy.AddJoy_Button_Default( 1,"Arm_SetPos6feet",false,false,"Joystick_1");
	joy.AddJoy_Button_Default( 3,"Arm_SetPos9feet",false,false,"Joystick_1");
	joy.AddJoy_Button_Default( 4,"Claw_Grip",true,false,"Joystick_1");
	joy.AddJoy_Button_Default( 5,"Claw_Squirt",true,false,"Joystick_1");
	joy.AddJoy_Button_Default( 7,"Claw_Close",true,false,"Joystick_1");
	joy.AddJoy_Button_Default( 6,"Arm_Rist",true,false,"Joystick_1");
	
	joy.AddJoy_Button_Default( 8,"Robot_CloseDoor",true,false,"Joystick_1");
	#endif
	#ifdef __WindRiverJoysticks__
	//For the Y Axis 3rd paramter false = down for up like flying a plane
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Arm_SetCurrentVelocity",false,1.0,0.1,true,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Arm_SetPotentiometerSafety",false,1.0,0.04,false,"Joystick_2");
	//This is no longer needed as the zero and the rest are the same
	//joy.AddJoy_Button_Default( 7,"Arm_SetPosRest",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 5,"Arm_SetPos0feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 6,"Arm_SetPos3feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default(10,"Arm_SetPos6feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 9,"Arm_SetPos9feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 0,"Claw_Grip",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 2,"Claw_Squirt",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 8,"Claw_Close",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 7,"Arm_Rist",true,false,"Joystick_2");
	
	joy.AddJoy_Button_Default( 7,"Robot_CloseDoor",true,false,"Joystick_1");
	#endif
}

  /***********************************************************************************************************************************/
 /*														Goal_OperateSolenoid														*/
/***********************************************************************************************************************************/

Goal_OperateSolenoid::Goal_OperateSolenoid(FRC_2011_Robot &robot,FRC_2011_Robot::SolenoidDevices SolenoidDevice,bool Close) : m_Robot(robot),
	m_SolenoidDevice(SolenoidDevice),m_Terminate(false),m_IsClosed(Close) 
{	
	m_Status=eInactive;
}

Goal_OperateSolenoid::Goal_Status Goal_OperateSolenoid::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	switch (m_SolenoidDevice)
	{
		case FRC_2011_Robot::eClaw:
			m_Robot.GetClaw().CloseClaw(m_IsClosed);
			break;
		case FRC_2011_Robot::eRist:
			m_Robot.GetArm().CloseRist(m_IsClosed);
			break;
		case FRC_2011_Robot::eDeployment:
			m_Robot.CloseDeploymentDoor(m_IsClosed);
			break;
	}
	m_Status=eCompleted;
	return m_Status;
}
