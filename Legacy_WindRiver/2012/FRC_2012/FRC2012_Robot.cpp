#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2012_Robot.h"
#include "Common/InOut_Interface.h"
#include "TankDrive/Tank_Robot_Control.h"
#include "TankDrive/Servo_Robot_Control.h"
#include "Common/Debug.h"
#include "FRC2012_InOut_Interface.h"

using namespace Framework::Base;
using namespace std;

#define __DisableTurretTargeting__
#define __DisableEncoderTracking__

//This will make the scale to half with a 0.1 dead zone
static double PositionToVelocity_Tweak(double Value)
{
	const double FilterRange=0.1;
	const double Multiplier=0.5;
	const bool isSquared=true;
	double Temp=fabs(Value); //take out the sign... put it back in the end
	Temp=(Temp>=FilterRange) ? Temp-FilterRange:0.0; 

	Temp=Multiplier*(Temp/(1.0-FilterRange)); //apply scale first then 
	if (isSquared) Temp*=Temp;  //square it if it is squared

	//Now to restore the sign
	Value=(Value<0.0)?-Temp:Temp;
	return Value;
}

  /***********************************************************************************************************************************/
 /*														FRC_2012_Robot::Turret														*/
/***********************************************************************************************************************************/

FRC_2012_Robot::Turret::Turret(FRC_2012_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("Turret",robot_control,eTurret),m_pParent(parent),m_Velocity(0.0),m_LastIntendedPosition(0.0)
{
}

void FRC_2012_Robot::Turret::SetIntendedPosition_Plus(double Position)
{
	if (GetPotUsage()!=Rotary_Position_Control::eNoPot)
	{
		if (((fabs(m_LastIntendedPosition-Position)<0.01)) || (!(IsZero(GetRequestedVelocity()))) )
			return;
		bool IsTargeting=(m_pParent->m_IsTargeting);
		if ((!IsTargeting) || m_pParent->m_DisableTurretTargetingValue)
		{
			m_LastIntendedPosition=Position; //grab it before all the conversions
			Position=-Position; 
			//By default this goes from -1 to 1.0 
			//first get the range from 0 - 1
			double positive_range = (Position * 0.5) + 0.5;
			//positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=GetMinRange();
			const double maxRange=GetMaxRange();
			const double Scale=(maxRange-minRange);
			Position=(positive_range * Scale) + minRange;
			//DOUT5("Test=%f",RAD_2_DEG(Position));
			SetIntendedPosition(Position);
		}
	}
	else
		Turret_SetRequestedVelocity(PositionToVelocity_Tweak(Position));   //allow manual use of same control
}

void FRC_2012_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::Turret_SetRequestedVelocity);
		em->EventValue_Map["Turret_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetIntendedPosition_Plus);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Turret::Turret_SetRequestedVelocity);
		em->EventValue_Map["Turret_SetIntendedPosition"].Remove(*this, &FRC_2012_Robot::Turret::SetIntendedPosition_Plus);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
}

void FRC_2012_Robot::Turret::TimeChange(double dTime_s)
{
	SetRequestedVelocity_FromNormalized(m_Velocity);
	m_Velocity=0.0;

	#ifndef __DisableTurretTargeting__
	if ((!m_pParent->m_DisableTurretTargetingValue) && (m_pParent->m_IsTargeting)&&(IsZero(GetRequestedVelocity())) && GetIsUsingPotentiometer())
	{
		Vec2D Target=m_pParent->m_TargetOffset;
		Target-=m_pParent->GetPos_m();
		const double Angle=atan2(Target[1],Target[0]);
		double AngleToUse=-(Angle-PI_2);
		AngleToUse-=m_pParent->GetAtt_r();
		SetIntendedPosition(NormalizeRotation2(AngleToUse) * m_pParent->m_YawErrorCorrection);
		//TODO factor in velocity once we have our ball velocity (to solve for time)
	}
	#endif

	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2012_Robot::Turret::ResetPos()
{
	__super::ResetPos();
	SetPos_m(-Pi);  //It starts out backwards
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::PitchRamp::PitchRamp(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("PitchRamp",robot_control,ePitchRamp),m_pParent(pParent)
{
}

void FRC_2012_Robot::PitchRamp::SetIntendedPosition_Plus(double Position)
{
	if (GetPotUsage()!=Rotary_Position_Control::eNoPot)
	{
		bool IsTargeting=(m_pParent->m_IsTargeting);
		if (!IsTargeting)
		{
			Position=-Position; //flip this around I want the pitch and power to work in the same direction where far away is lower pitch
			//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
			//first get the range from 0 - 1
			double positive_range = (Position * 0.5) + 0.5;
			//positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=GetMinRange();
			const double maxRange=GetMaxRange();
			const double Scale=(maxRange-minRange) / maxRange;
			Position=(positive_range * Scale) + minRange;
		}
		//DOUT5("Test=%f",RAD_2_DEG(Position));
		SetIntendedPosition(Position);
	}
	else
		SetRequestedVelocity_FromNormalized(PositionToVelocity_Tweak(Position));   //allow manual use of same control

}

void FRC_2012_Robot::PitchRamp::TimeChange(double dTime_s)
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && (IsZero(GetRequestedVelocity())) && (GetPotUsage()!=Rotary_Position_Control::eNoPot) );
	if (IsTargeting)
	{
		__super::SetIntendedPosition(m_pParent->m_PitchAngle);
	}
	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2012_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetPotentiometerSafety);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::PowerWheels														*/
/***********************************************************************************************************************************/

FRC_2012_Robot::PowerWheels::PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Velocity_Control("PowerWheels",robot_control,ePowerWheels,eActive),m_pParent(pParent),m_ManualVelocity(0.0),m_IsRunning(false)
{
}

void FRC_2012_Robot::PowerWheels::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetIsRunning);
	}
	else
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetIsRunning);
	}
}

void FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized(double Velocity) 
{
	//bool IsTargeting=((m_pParent->m_IsTargeting) && GetEncoderUsage()==eActive);
	//This variable is dedicated to non-targeting mode
	m_ManualVelocity=Velocity;
}

void FRC_2012_Robot::PowerWheels::TimeChange(double dTime_s)
{
	const double MaxSpeed=m_Ship_1D_Props.MAX_SPEED;
	bool IsTargeting=((m_pParent->m_IsTargeting) && GetEncoderUsage()==eActive);
	if (  IsTargeting )
	{
		if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested())) 
		{
			//convert linear velocity to angular velocity
			double RPS=m_pParent->m_LinearVelocity / (Pi * GetDimension());
			RPS*=(2.0 * m_pParent->m_PowerErrorCorrection);  //For hooded shoot we'll have to move twice as fast
			SetRequestedVelocity(RPS * Pi2);
			//DOUT5("rps=%f rad=%f",RPS,RPS*Pi2);
		}
		else
			SetRequestedVelocity(0);
	}
	else
	{
		if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested()))
		{
			//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
			//first get the range from 0 - 1
			double positive_range = (m_ManualVelocity * 0.5) + 0.5;
			positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=GetMinRange();
			const double maxRange=MaxSpeed;
			const double Scale=(maxRange-minRange) / MaxSpeed;
			const double Offset=minRange/MaxSpeed;
			const double Velocity=(positive_range * Scale) + Offset;
			//DOUT5("%f",Velocity);
			size_t DisplayRow=m_pParent->m_RobotProps.GetFRC2012RobotProps().PowerVelocity_DisplayRow;
			if (DisplayRow!=(size_t)-1)
			{
				const double rps=(Velocity * MaxSpeed) / Pi2;
				Dout(DisplayRow,"%f ,%f",rps,Meters2Feet(rps * Pi * GetDimension()));
			}

			Rotary_Velocity_Control::SetRequestedVelocity_FromNormalized(Velocity);
		}
		else
			Rotary_Velocity_Control::SetRequestedVelocity_FromNormalized(0.0);
	}
	__super::TimeChange(dTime_s);
}

void FRC_2012_Robot::PowerWheels::ResetPos()
{
	m_IsRunning=false;
	__super::ResetPos();
}

  /***********************************************************************************************************************************/
 /*												FRC_2012_Robot::BallConveyorSystem													*/
/***********************************************************************************************************************************/

FRC_2012_Robot::BallConveyorSystem::BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),
	m_LowerConveyor("LowerConveyor",robot_control,eLowerConveyor),m_MiddleConveyor("MiddleConveyor",robot_control,eMiddleConveyor),
	m_FireConveyor("FireConveyor",robot_control,eFireConveyor),
	m_FireDelayTrigger_Time(0.0),m_FireStayOn_Time(0.0),
	m_FireDelayTriggerOn(false),m_FireStayOn(false)
{
	m_ControlSignals.raw=0;
	//This are always open loop as there is no encoder and this is specified by default
}

void FRC_2012_Robot::BallConveyorSystem::Initialize(Entity2D_Kind::EventMap& em,const Entity1D_Properties *props)
{
	//These share the same props and fire is scaled from this level
	m_LowerConveyor.Initialize(em,props);
	m_MiddleConveyor.Initialize(em,props);
	m_FireConveyor.Initialize(em,props);
}
void FRC_2012_Robot::BallConveyorSystem::ResetPos() 
{
	m_LowerConveyor.ResetPos(),m_MiddleConveyor.ResetPos(),m_FireConveyor.ResetPos();
	m_ControlSignals.raw=0;
}

void FRC_2012_Robot::BallConveyorSystem::TimeChange(double dTime_s)
{
	const bool LowerSensor=m_pParent->m_RobotControl->GetBoolSensorState(eLowerConveyor_Sensor);
	const bool MiddleSensor=m_pParent->m_RobotControl->GetBoolSensorState(eMiddleConveyor_Sensor);
	const bool FireSensor=m_pParent->m_RobotControl->GetBoolSensorState(eFireConveyor_Sensor);
	const double PowerWheelSpeedDifference=m_pParent->m_PowerWheels.GetRequestedVelocity_Difference();
	const bool PowerWheelReachedTolerance=(m_pParent->m_PowerWheels.GetRequestedVelocity()!=0.0) &&
		(fabs(PowerWheelSpeedDifference)<m_pParent->m_PowerWheels.GetRotary_Properties().PrecisionTolerance);
	//Only fire when the wheel has reached its aiming speed
	bool Fire=(m_ControlSignals.bits.Fire==1) && PowerWheelReachedTolerance;
	bool Grip=m_ControlSignals.bits.Grip==1;
	bool GripL=m_ControlSignals.bits.GripL==1;
	bool GripM=m_ControlSignals.bits.GripM==1;
	bool GripH=m_ControlSignals.bits.GripH==1;
	bool Squirt=m_ControlSignals.bits.Squirt==1;

	if (Fire)
	{
		if (m_FireDelayTriggerOn)
		{
			m_FireDelayTrigger_Time+=dTime_s;
			//printf("Fire delaying =%f\n",m_FireDelayTrigger_Time);
			if (m_FireDelayTrigger_Time>m_pParent->m_RobotProps.GetFRC2012RobotProps().FireTriggerDelay)
				m_FireDelayTriggerOn=false;
		}
	}
	else
	{
		m_FireDelayTriggerOn=true;
		m_FireDelayTrigger_Time=0.0;
	}

	Fire = Fire && !m_FireDelayTriggerOn;

	if (Fire)
	{
		m_FireStayOn=true;
		m_FireStayOn_Time=0.0;
	}
	else
	{
		if (m_FireStayOn)
		{
			m_FireStayOn_Time+=dTime_s;
			//printf("Fire Staying on=%f\n",m_FireStayOn_Time);
			if (m_FireStayOn_Time>m_pParent->m_RobotProps.GetFRC2012RobotProps().FireButtonStayOn_Time)
				m_FireStayOn=false;
		}
	}

	//This assumes the motors are in the same orientation: 
	double LowerAcceleration=((Grip & (!LowerSensor)) || (LowerSensor & (!MiddleSensor))) | GripL | Squirt | Fire ?
		((Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_LowerConveyor.SetCurrentLinearAcceleration(LowerAcceleration);

	double MiddleAcceleration= ((LowerSensor & (!MiddleSensor)) || (MiddleSensor & (!FireSensor))) | GripM | Squirt | Fire  ?
		((Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_MiddleConveyor.SetCurrentLinearAcceleration(MiddleAcceleration);

	double FireAcceleration= (MiddleSensor & (!FireSensor)) | GripH | Squirt | Fire | m_FireStayOn ?
		((Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_FireConveyor.SetCurrentLinearAcceleration(FireAcceleration);

	m_LowerConveyor.AsEntity1D().TimeChange(dTime_s);
	m_MiddleConveyor.AsEntity1D().TimeChange(dTime_s);
	m_FireConveyor.AsEntity1D().TimeChange(dTime_s);
}

//This is the manual override, but probably not used if we use spike as it would be wasteful to have a analog control for this
void FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized(double Velocity)
{
	m_LowerConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_MiddleConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_FireConveyor.SetRequestedVelocity_FromNormalized(Velocity);
}

void FRC_2012_Robot::BallConveyorSystem::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_MiddleConveyor.GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//Ball_SetCurrentVelocity is the manual override
		em->EventValue_Map["Ball_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_GripL"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::GripL);
		em->EventOnOff_Map["Ball_GripM"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::GripM);
		em->EventOnOff_Map["Ball_GripH"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::GripH);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_GripL"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::GripL);
		em->EventOnOff_Map["Ball_GripM"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::GripM);
		em->EventOnOff_Map["Ball_GripH"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::GripH);
		em->EventOnOff_Map["Ball_Squirt"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::Flippers														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::Flippers::Flippers(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
Rotary_Position_Control("Flippers",robot_control,eFlippers),m_pParent(pParent),m_Advance(false),m_Retract(false)
{
}

void FRC_2012_Robot::Flippers::SetIntendedPosition(double Position)
{
	//DOUT5("Test=%f",RAD_2_DEG(Position));
	__super::SetIntendedPosition(Position);
}

void FRC_2012_Robot::Flippers::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Advance ^ m_Retract)
		SetCurrentLinearAcceleration(m_Advance?Accel:-Brake);

	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetFlipperProps().GetRotaryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2012_Robot::Flippers::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Flippers_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Flippers_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::SetIntendedPosition);
		em->EventOnOff_Map["Flippers_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::SetPotentiometerSafety);
		em->EventOnOff_Map["Flippers_Advance"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::Advance);
		em->EventOnOff_Map["Flippers_Retract"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::Retract);
	}
	else
	{
		em->EventValue_Map["Flippers_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Flippers::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Flippers_SetIntendedPosition"].Remove(*this, &FRC_2012_Robot::Flippers::SetIntendedPosition);
		em->EventOnOff_Map["Flippers_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Flippers::SetPotentiometerSafety);
		em->EventOnOff_Map["Flippers_Advance"].Remove(*this, &FRC_2012_Robot::Flippers::Advance);
		em->EventOnOff_Map["Flippers_Retract"].Remove(*this, &FRC_2012_Robot::Flippers::Retract);
	}
}

  /***********************************************************************************************************************************/
 /*															FRC_2012_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;
const Vec2D c_BridgeDimensions=Vec2D(Inches2Meters(48),Inches2Meters(88)); //width x length

const Vec2D c_TargetBasePosition=Vec2D(0.0,c_HalfCourtLength);
const double c_BallShootHeight_inches=55.0;
const double c_TargetBaseHeight= Inches2Meters(98.0 - c_BallShootHeight_inches);
const double c_Target_MidBase_Height= Inches2Meters(61.0 - c_BallShootHeight_inches);
const double c_Target_MiddleHoop_XOffset=Inches2Meters(27+3/8);

//http://www.sciencedaily.com/releases/2011/03/110310151224.htm
//The results show the optimal aim points make a "V" shape near the top center of the backboard's "square," which is actually a 
//24-inch by 18-inch rectangle which surrounds the rim
const Vec2D c_BankShot_Box=Vec2D(Inches2Meters(24),Inches2Meters(18)); //width x length
//http://esciencenews.com/articles/2011/03/10/the.physics.bank.shots
const double c_BankShot_BackboardY_Offset=Inches2Meters(3.327);
const double c_BankShot_V_Angle=0.64267086025537;
const double c_BankShot_V_Distance=Inches2Meters(166164/8299); //about 20.02 inches
const double c_BankShot_V_MiddlePointSaturationDistance=Inches2Meters(1066219/331960);  //about 3.211
const double c_BankShot_V_Hieght_plus_SatPoint=Inches2Meters(12.86);
const double c_BankShot_V_SatHieght=Inches2Meters(2.57111110379327);
const double c_BankShot_Initial_V_Hieght=Inches2Meters(12.86)-c_BankShot_V_SatHieght; //about 10.288 inches to the point of the V

FRC_2012_Robot::FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), m_Turret(this,robot_control),m_PitchRamp(this,robot_control),
		m_PowerWheels(this,robot_control),m_BallConveyorSystem(this,robot_control),m_Flippers(this,robot_control),
		m_Target(eCenterHighGoal),m_DefensiveKeyPosition(Vec2D(0.0,0.0)),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),m_AutonPresetIndex(0),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_IsTargeting(true),m_SetLowGear(false)
{
}

void FRC_2012_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2012_Robot_Properties *RobotProps=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)
	m_Turret.Initialize(em,RobotProps?&RobotProps->GetTurretProps():NULL);
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);
	m_PowerWheels.Initialize(em,RobotProps?&RobotProps->GetPowerWheelProps():NULL);
	m_BallConveyorSystem.Initialize(em,RobotProps?&RobotProps->GetConveyorProps():NULL);
	m_Flippers.Initialize(em,RobotProps?&RobotProps->GetFlipperProps():NULL);

	//set to the default key position
	const FRC_2012_Robot_Props &robot2012props=RobotProps->GetFRC2012RobotProps();
	SetDefaultPosition(robot2012props.PresetPositions[m_DefaultPresetIndex]);
}
void FRC_2012_Robot::ResetPos()
{
	//We cannot reset position between auton and telop
	SetBypassPosAtt_Update(true);
	__super::ResetPos();
	SetBypassPosAtt_Update(false);

	//This should be false to avoid any conflicts during a reset
	m_IsTargeting=false;
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	m_PowerWheels.ResetPos();
	m_BallConveyorSystem.ResetPos();
	m_Flippers.ResetPos();
}

FRC_2012_Robot::BallConveyorSystem &FRC_2012_Robot::GetBallConveyorSystem()
{
	return m_BallConveyorSystem;
}

FRC_2012_Robot::PowerWheels &FRC_2012_Robot::GetPowerWheels()
{
	return m_PowerWheels;
}

void FRC_2012_Robot::ApplyErrorCorrection()
{
	const FRC_2012_Robot_Props &robot_props=m_RobotProps.GetFRC2012RobotProps();
	#ifndef __DisableEncoderTracking__
	const Vec2d &Pos_m=GetPos_m();
	#else
	const Vec2d &Pos_m=	robot_props.PresetPositions[m_AutonPresetIndex];
	#endif
	//first determine which quadrant we are in
	//These offsets are offsets added to the array indexes 
	const size_t XOffset=(Pos_m[0]>(robot_props.KeyGrid[1][1])[0]) ? 1 : 0;
	const double YCenterKey=(robot_props.KeyGrid[1][1])[1];
	//The coordinate system is backwards for Y 
	const size_t YOffset=(Pos_m[1] < YCenterKey) ? 1 : 0;
	//Find our normalized targeted coordinates; saturate as needed
	const Vec2D &q00=robot_props.KeyGrid[0+YOffset][0+XOffset];
	const Vec2D &q01=robot_props.KeyGrid[0+YOffset][1+XOffset];
	const double XWidth=q01[0]-q00[0];
	const double xStart=max(Pos_m[0]-q00[0],0.0);
	const double x=min(xStart/XWidth,1.0);
	const Vec2D &q10=robot_props.KeyGrid[1+YOffset][0+XOffset];
	const double YToUse=(c_HalfCourtLength-Pos_m[1]) + (2.0*YCenterKey - c_HalfCourtLength);
	const double YWidth=q10[1]-q00[1];
	const double yStart=max(YToUse-q00[1],0.0);
	const double y=min(yStart/YWidth,1.0);
	//Now to blend.  Top half, bottom half then the halves
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c00=robot_props.KeyCorrections[0+YOffset][0+XOffset];
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c01=robot_props.KeyCorrections[0+YOffset][1+XOffset];
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c10=robot_props.KeyCorrections[1+YOffset][0+XOffset];
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c11=robot_props.KeyCorrections[1+YOffset][1+XOffset];

	const double pc_TopHalf=    (x * c01.PowerCorrection) + ((1.0-x)*c00.PowerCorrection);
	const double pc_BottomHalf= (x * c11.PowerCorrection) + ((1.0-x)*c10.PowerCorrection);
	const double pc = (y * pc_BottomHalf) + ((1.0-y) * pc_TopHalf);

	const double yc_TopHalf=    (x * c01.YawCorrection) + ((1.0-x)*c00.YawCorrection);
	const double yc_BottomHalf= (x * c11.YawCorrection) + ((1.0-x)*c10.YawCorrection);
	const double yc = (y * yc_TopHalf) + ((1.0-y) * yc_BottomHalf);

	//Now to apply correction... for now we'll apply to the easiest pieces possible and change if needed
	m_YawErrorCorrection=yc;
	m_PowerErrorCorrection=pc;
	//DOUT(5,"pc=%f yc=%f x=%f y=%f",pc,yc,x,y);
	//We can use the error grid cells directly by simply positioning the robot at the right place
	size_t HackedIndex;
	switch (m_Target)
	{
	case eLeftGoal:
	case eRightGoal:
		m_PowerErrorCorrection=robot_props.Autonomous_Props.TwoShotScaler;
		break;
	default:
		HackedIndex=0;
		break;
	}

}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	const FRC_2012_Robot_Props &robot_props=m_RobotProps.GetFRC2012RobotProps();
	#ifndef __DisableEncoderTracking__
	const Vec2d &Pos_m=GetPos_m();
	//Got to make this fit within 20 chars :(
	Dout(robot_props.Coordinates_DiplayRow,"%.2f %.2f %.1f",Meters2Feet(Pos_m[0]),
		Meters2Feet(Pos_m[1]),RAD_2_DEG(GetAtt_r()));
	#else
	const Vec2d &Pos_m=	robot_props.PresetPositions[m_AutonPresetIndex];
	{	//Even though this is disabled... still want it to read correctly for encoder reading and calibration
		const Vec2d &Pos_temp=GetPos_m();
		Dout(robot_props.Coordinates_DiplayRow,"%.2f %.2f %.1f",Meters2Feet(Pos_temp[0]),
			Meters2Feet(Pos_temp[1]),RAD_2_DEG(GetAtt_r()));
	}
	#endif

	switch (m_Target)
	{
		case eCenterHighGoal:
			m_TargetOffset=c_TargetBasePosition;  //2d top view x,y of the target
			m_TargetHeight=c_TargetBaseHeight;    //1d z height (front view) of the target
			break;
		case eLeftGoal:
			m_TargetOffset=Vec2D(-c_Target_MiddleHoop_XOffset,c_HalfCourtLength);
			m_TargetHeight=c_Target_MidBase_Height;
			break;
		case eRightGoal:
			m_TargetOffset=Vec2D(c_Target_MiddleHoop_XOffset,c_HalfCourtLength);
			m_TargetHeight=c_Target_MidBase_Height;
			break;
		case eDefensiveKey:
			m_TargetOffset=m_DefensiveKeyPosition;
			m_TargetHeight=0;
			break;
	}

	const double x=Vec2D(Pos_m-m_TargetOffset).length();

	if (m_Target != eDefensiveKey)
	{
		const bool DoBankShot= (x < Feet2Meters(16));  //if we are less than 16 feet away
		if (DoBankShot)
		{
			m_TargetOffset[1]+=c_BankShot_BackboardY_Offset;  //The point extends beyond the backboard
			const double XOffsetRatio=min (fabs(Pos_m[0]/c_HalfCourtWidth),1.0);  //restore sign in the end
			//Use this ratio to travel along the V distance... linear distribution should be adequate
			const double VOffset=XOffsetRatio*c_BankShot_V_Distance;
			//generate x / y from our VOffset
			double YawOffset=sin(c_BankShot_V_Angle) * VOffset;
			double PitchOffset=cos(c_BankShot_V_Angle) * VOffset;
			if (PitchOffset<c_BankShot_V_SatHieght)
				PitchOffset=c_BankShot_V_SatHieght;
			if (Pos_m[0]<0.0)
				YawOffset=-YawOffset;
			//DOUT(5,"v=%f x=%f y=%f",Meters2Inches(VOffset),Meters2Inches(YawOffset),Meters2Inches(PitchOffset) );
			m_TargetOffset[0]+=YawOffset;
			m_TargetHeight+=(c_BankShot_Initial_V_Hieght + PitchOffset);
			//DOUT(5,"x=%f y=%f",Meters2Inches(m_TargetOffset[0]),Meters2Inches(m_TargetHeight) );
		}
		else
			m_TargetOffset[1]-=Inches2Meters(9+6);  //hoop diameter 18... half that is 9 plus the 6 inch extension out

	}

	//TODO tweak adjustments based off my position in the field here
	//
	//Now to compute my pitch, power, and hang time
	{
		//TODO factor in rotation if it is significant
		const double y=m_TargetHeight;
		const double y2=y*y;
		const double x2=x*x;
		const double g=9.80665;
		//These equations come from here http://www.lightingsciences.ca/pdf/BNEWSEM2.PDF

		//Where y = height displacement (or goal - player)
		//	[theta=atan(sqrt(y^2+x^2)/x+y/x)]
		//This is equation 8 solving theta
		m_PitchAngle=atan(sqrt(y2+x2)/x+y/x);

		//Be sure G is in the same units as x and y!  (all in meters in code)
		//	V=sqrt(G(sqrt(y^2+x^2)+y))
		//	This is equation 7 solving v
		m_LinearVelocity=sqrt(g*(sqrt(y2+x2)+y));

		ApplyErrorCorrection();

		//ta=(sin(theta)*v)/g   //This is equation 2 solving for t1
		//tb=(x-ta*cos(theta)*v)/(cos(theta)*v)   //this is equation 3 solving for t2
		//	hang time= ta+tb 
		double ta,tb;
		ta=(sin(m_PitchAngle)*m_LinearVelocity)/g;
		tb=(x-ta*cos(m_PitchAngle)*m_LinearVelocity)/(cos(m_PitchAngle)*m_LinearVelocity);
		m_HangTime = ta+tb;
		{
			DOUT(5,"d=%f p=%f v=%f ht=%f",Meters2Feet(x) ,RAD_2_DEG(m_PitchAngle),Meters2Feet(m_LinearVelocity),m_HangTime);
			Dout(robot_props.TargetVars_DisplayRow,"%.2f %.2f %.1f",RAD_2_DEG(m_Turret.GetPos_m()) ,RAD_2_DEG(m_PitchAngle),Meters2Feet(m_LinearVelocity));
		}
	}
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.AsEntity1D().TimeChange(dTime_s);
	m_PitchRamp.AsEntity1D().TimeChange(dTime_s);
	m_PowerWheels.AsEntity1D().TimeChange(dTime_s);
	m_BallConveyorSystem.TimeChange(dTime_s);
	m_Flippers.AsEntity1D().TimeChange(dTime_s);
}

const FRC_2012_Robot_Properties &FRC_2012_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

void FRC_2012_Robot::SetTargetingValue(double Value)
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//TODO determine final scaler factor for the pitch (may want to make this a property)
	//printf("\r%f       ",Value);
	if (Value > -0.98)
	{
		if (m_IsTargeting)
		{
			m_IsTargeting=false;
			printf("Disabling Targeting\n");
		}
	}
	else
	{
		if (!m_IsTargeting)
		{
			m_IsTargeting=true;
			printf("Enabling Targeting\n");
		}
	}
}

void FRC_2012_Robot::SetLowGear(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	m_SetLowGear=on;
	SetBypassPosAtt_Update(true);
	m_Turret.SetBypassPos_Update(true);
	m_PitchRamp.SetBypassPos_Update(true);

	//Now for some real magic with the properties!
	__super::Initialize(*GetEventMap(),m_SetLowGear?&m_RobotProps.GetLowGearProps():&m_RobotProps);
	SetBypassPosAtt_Update(false);
	m_Turret.SetBypassPos_Update(false);
	m_PitchRamp.SetBypassPos_Update(false);

	m_RobotControl->OpenSolenoid(eUseLowGear,on);
}

void FRC_2012_Robot::SetLowGearValue(double Value)
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//printf("\r%f       ",Value);
	if (Value > 0.0)
	{
		if (m_SetLowGear)
		{
			SetLowGear(false);
			printf("Now in HighGear\n");
		}
	}
	else
	{
		if (!m_SetLowGear)
		{
			SetLowGear(true);
			printf("Now in LowGear\n");
		}
	}
}

void FRC_2012_Robot::SetPresetPosition(size_t index,bool IgnoreOrientation)
{
	Vec2D position=m_RobotProps.GetFRC2012RobotProps().PresetPositions[index];
	SetPosition(position[0],position[1]);

	#ifndef __DisableTurretTargeting__
	if (!IgnoreOrientation)
	{	
		Vec2D Target=m_TargetOffset;
		Target-=GetPos_m();
		const double Angle=atan2(Target[1],Target[0]);
		double AngleToUse=-(Angle-PI_2);

		double TurretPos=NormalizeRotation2(AngleToUse)-m_Turret.GetPos_m();
		SetAttitude(TurretPos);
	}
	#else
	if (!IgnoreOrientation)
	{	
		//with turret not working assume its always in the zero position
		SetAttitude(0.0);
	}
	#endif
}

void FRC_2012_Robot::Set_Auton_PresetPosition(size_t index)
{
	m_AutonPresetIndex=index;
	m_IsTargeting=true;  //This is just in case the pitch is in wrong position or if it is missing
	SetPresetPosition(index,true);
	SetAttitude(Pi);
	m_Turret.SetPos_m(-Pi);
}

void FRC_2012_Robot::SetTarget(Targets target)
{
	m_Target=target;
}

void FRC_2012_Robot::SetDefensiveKeyOn()
{
	//We'll simply plot the coordinates of the key based on position and orientation of the turret
	//This is really a scale of 0 - 1 multiplied against 40 feet, but simplified to 0 - 2 * 20
	const double Distance=Feet2Meters((m_DefensiveKeyNormalizedDistance + 1.0) * 20.0);
	//determine our turret direction 
	#ifndef __DisableTurretTargeting__
	double Direction=NormalizeRotation2(GetAtt_r() + m_Turret.GetPos_m());
	#else
	double Direction=0.0;
	#endif
	double Y=(sin(Direction+PI_2) * Distance) + GetPos_m()[1];
	double X=(cos(-Direction+PI_2) * Distance) + GetPos_m()[0];
	printf("Direction=%f Distance=%f x=%f y=%f\n",RAD_2_DEG(Direction),Meters2Feet(Distance),Meters2Feet(X),Meters2Feet(Y));
	m_DefensiveKeyPosition=Vec2D(X,Y);
	m_Target=eDefensiveKey;
}


void FRC_2012_Robot::SetPresetPOV (double value)
{
	//We put the typical case first (save the amount of branching)
	if (value!=-1)
	{
		if (!m_POVSetValve)
		{
			m_POVSetValve=true;
			//so breaking down the index
			//0 = up
			//1 = up right
			//2 = right
			//3 = down right
			//4 = down
			//5 = down left
			//6 = left
			//7 = left up
			size_t index=(size_t)(value/45.0);
			switch (index)
			{
				case 0:	SetPresetPosition(0);	break;
				case 2: SetPresetPosition(2);	break;
				case 6: SetPresetPosition(1);	break;
			}
		}
	}
	else 
		m_POVSetValve=false;
}

void FRC_2012_Robot::Robot_SetCreepMode(bool on) 
{
	SetUseEncoders(on,false);  //High gear can use them
	if (m_SetLowGear)
	{
		m_RobotControl->OpenSolenoid(eUseBreakDrive,on);
	}
}

void FRC_2012_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Robot_IsTargeting"].Subscribe(ehl, *this, &FRC_2012_Robot::IsTargeting);
		em->Event_Map["Robot_SetTargetingOn"].Subscribe(ehl, *this, &FRC_2012_Robot::SetTargetingOn);
		em->Event_Map["Robot_SetTargetingOff"].Subscribe(ehl, *this, &FRC_2012_Robot::SetTargetingOff);
		em->EventOnOff_Map["Robot_TurretSetTargetingOff"].Subscribe(ehl,*this, &FRC_2012_Robot::SetTurretTargetingOff);
		em->EventValue_Map["Robot_SetTargetingValue"].Subscribe(ehl,*this, &FRC_2012_Robot::SetTargetingValue);
		em->EventValue_Map["Robot_SetDefensiveKeyValue"].Subscribe(ehl,*this, &FRC_2012_Robot::SetDefensiveKeyPosition);
		em->Event_Map["Robot_SetDefensiveKeyOn"].Subscribe(ehl, *this, &FRC_2012_Robot::SetDefensiveKeyOn);
		em->Event_Map["Robot_SetDefensiveKeyOff"].Subscribe(ehl, *this, &FRC_2012_Robot::SetDefensiveKeyOff);
		em->EventOnOff_Map["Robot_Flippers_Solenoid"].Subscribe(ehl,*this, &FRC_2012_Robot::SetFlipperPneumatic);

		em->EventOnOff_Map["Robot_SetLowGear"].Subscribe(ehl, *this, &FRC_2012_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"].Subscribe(ehl, *this, &FRC_2012_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"].Subscribe(ehl, *this, &FRC_2012_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Subscribe(ehl,*this, &FRC_2012_Robot::SetLowGearValue);

		em->Event_Map["Robot_SetPreset1"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPreset1);
		em->Event_Map["Robot_SetPreset2"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPreset2);
		em->Event_Map["Robot_SetPreset3"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPreset3);
		em->EventValue_Map["Robot_SetPresetPOV"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPresetPOV);
		em->EventOnOff_Map["Robot_SetCreepMode"].Subscribe(ehl, *this, &FRC_2012_Robot::Robot_SetCreepMode);
	}
	else
	{
		em->EventOnOff_Map["Robot_IsTargeting"]  .Remove(*this, &FRC_2012_Robot::IsTargeting);
		em->Event_Map["Robot_SetTargetingOn"]  .Remove(*this, &FRC_2012_Robot::SetTargetingOn);
		em->Event_Map["Robot_SetTargetingOff"]  .Remove(*this, &FRC_2012_Robot::SetTargetingOff);
		em->EventOnOff_Map["Robot_TurretSetTargetingOff"].Remove(*this, &FRC_2012_Robot::SetTurretTargetingOff);
		em->EventValue_Map["Robot_SetTargetingValue"].Remove(*this, &FRC_2012_Robot::SetTargetingValue);
		em->EventValue_Map["Robot_SetDefensiveKeyValue"].Remove(*this, &FRC_2012_Robot::SetDefensiveKeyPosition);
		em->Event_Map["Robot_SetDefensiveKeyOn"]  .Remove(*this, &FRC_2012_Robot::SetDefensiveKeyOn);
		em->Event_Map["Robot_SetDefensiveKeyOff"]  .Remove(*this, &FRC_2012_Robot::SetDefensiveKeyOff);
		em->EventOnOff_Map["Robot_Flippers_Solenoid"]  .Remove(*this, &FRC_2012_Robot::SetFlipperPneumatic);

		em->EventOnOff_Map["Robot_SetLowGear"]  .Remove(*this, &FRC_2012_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"]  .Remove(*this, &FRC_2012_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"]  .Remove(*this, &FRC_2012_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Remove(*this, &FRC_2012_Robot::SetLowGearValue);

		em->Event_Map["Robot_SetPreset1"]  .Remove(*this, &FRC_2012_Robot::SetPreset1);
		em->Event_Map["Robot_SetPreset2"]  .Remove(*this, &FRC_2012_Robot::SetPreset2);
		em->Event_Map["Robot_SetPreset3"]  .Remove(*this, &FRC_2012_Robot::SetPreset3);
		em->EventValue_Map["Robot_SetPresetPOV"]  .Remove(*this, &FRC_2012_Robot::SetPresetPOV);
		em->EventOnOff_Map["Robot_SetCreepMode"]  .Remove(*this, &FRC_2012_Robot::Robot_SetCreepMode);
	}

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_PowerWheels.BindAdditionalEventControls(Bind);
	m_BallConveyorSystem.BindAdditionalEventControls(Bind);
	m_Flippers.BindAdditionalEventControls(Bind);
	#ifdef Robot_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
}

void FRC_2012_Robot::BindAdditionalUIControls(bool Bind,void *joy, void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2012_Robot_Properties::FRC_2012_Robot_Properties()  : m_TurretProps(
	"Turret",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eSwivel,
	true,	//Using the range
	-Pi,Pi
	),
	m_PitchRampProps(
	"Pitch",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eRobotArm,
	true,	//Using the range
	DEG_2_RAD(45-3),DEG_2_RAD(70+3) //add padding for quick response time (as close to limits will slow it down)
	),
	m_PowerWheelProps(
	"PowerWheels",
	2.0,    //Mass
	Inches2Meters(6),   //Dimension  (needed to convert linear to angular velocity)
	(5000.0/60.0) * Pi2,   //Max Speed (This is clocked at 5000 rpm) 
	60.0,60.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	60.0,60.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false,28.0 * Pi2,0.0,	//No limit ever!  (but we are using the min range as a way to set minimum speed)
	true //This is angular
	),
	m_ConveyorProps(
	"Conveyor",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//RS-550 motor with 64:1 BaneBots transmission, so this is spec at 19300 rpm free, and 17250 peak efficiency
	//17250 / 64 = 287.5 = rps of motor / 64 reduction = 4.492 rps * 2pi = 28.22524
	28,   //Max Speed (rounded as we need not have precision)
	112.0,112.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	),
	m_FlipperProps(
	"Flippers",
	2.0,    //Mass
	Inches2Meters(12),   //Dimension  (this should be correct)
	1.4 * Pi2,   //Max Speed  (Parker gave this one, should be good)
	10.0,10.0, //ACCEL, BRAKE  (should be relatively quick)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eRobotArm,
	true,	//Using the range
	-PI_2,PI_2 //TODO
	),
	m_RobotControls(&s_ControlsEvents)
{
	{
		FRC_2012_Robot_Props props;
		const double KeyDistance=Inches2Meters(144);
		const double KeyWidth=Inches2Meters(101);
		//const double KeyDepth=Inches2Meters(48);   //not used (yet)
		const double DefaultY=c_HalfCourtLength-KeyDistance;
		const double HalfKeyWidth=KeyWidth/2.0;
		props.PresetPositions[0]=Vec2D(0.0,DefaultY);
		props.PresetPositions[1]=Vec2D(-HalfKeyWidth,DefaultY);
		props.PresetPositions[2]=Vec2D(HalfKeyWidth,DefaultY);
		props.FireTriggerDelay=0.100;  //e.g. 10 iterations of good tolerance
		props.FireButtonStayOn_Time=0.100; //100 ms
		props.Coordinates_DiplayRow=(size_t)-1;
		props.TargetVars_DisplayRow=(size_t)-1;
		props.PowerVelocity_DisplayRow=(size_t)-1;

		for (size_t row=0;row<3;row++)
		{
			for (size_t column=0;column<3;column++)
			{
				Vec2D &cell=props.KeyGrid[row][column];
				const double spread=Feet2Meters(7.0);
				const double x=spread * ((double)column-1.0);
				const double y=(spread * ((double)row-1.0)) + DefaultY;
				cell=Vec2D(x,y);
				props.KeyCorrections[row][column].PowerCorrection=1.0;
				props.KeyCorrections[row][column].YawCorrection=1.0;
			}
		}

		FRC_2012_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		auton.MoveForward=0.0;
		auton.TwoShotScaler=1.0;
		auton.RampLeft_ErrorCorrection_Offset=
		auton.RampRight_ErrorCorrection_Offset=
		auton.RampCenter_ErrorCorrection_Offset=Vec2D(0.0,0.0);
		auton.XLeftArc=auton.XRightArc=1.9;
		FRC_2012_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_1=auton.FirstBall_Wait;
		ball_1.InitialWait=4.0;
		ball_1.TimeOutWait=-1.0;
		ball_1.ToleranceThreshold=0.0;
		FRC_2012_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_2=auton.SecondBall_Wait;
		ball_2.InitialWait=4.0;
		ball_2.TimeOutWait=-1.0;
		ball_2.ToleranceThreshold=0.0;
		m_FRC2012RobotProps=props;
	}
	{
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

		//Late assign this to override the initial default
		//Was originally 0.4953 19.5 width for 2011
		//Now is 0.517652 20.38 according to Parker  (not too worried about the length)
		props.WheelDimensions=Vec2D(0.517652,0.6985); //27.5 x 20.38
		props.WheelDiameter=c_WheelDiameter;
		props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
		props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
		m_TankRobotProps=props;
	}
	{
		Rotary_Props props=m_TurretProps.RotaryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_TurretProps.RotaryProps()=props;
	}
	{
		Rotary_Props props=m_PitchRampProps.RotaryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_PitchRampProps.RotaryProps()=props;
	}
	{
		Rotary_Props props=m_PowerWheelProps.RotaryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.1; //we need decent precision (this will depend on ramp up time too)
		m_PowerWheelProps.RotaryProps()=props;
	}
}

const char *ProcessVec2D(FRC_2012_Robot_Props &m_FRC2012RobotProps,Scripting::Script& script,Vec2d &Dest)
{
	const char *err;
	double length, width;	
	//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
	err = script.GetField("y", NULL, NULL,&length);
	if (err)
	{
		err = script.GetField("y_ft", NULL, NULL,&length);
		if (!err)
			length=Feet2Meters(length);
		else
		{
			err = script.GetField("y_in", NULL, NULL,&length);
			if (!err)
				length=Inches2Meters(length);
		}

	}
	ASSERT_MSG(!err, err);
	err = script.GetField("x", NULL, NULL,&width);
	if (err)
	{
		err = script.GetField("x_ft", NULL, NULL,&width);
		if (!err)
			width=Feet2Meters(width);
		else
		{
			err = script.GetField("x_in", NULL, NULL,&width);
			if (!err)
				width=Inches2Meters(width);
		}
	}
	ASSERT_MSG(!err, err);
	Dest=Vec2D(width,length);  //x,y  where x=width
	script.Pop();
	return err;
}

const char *ProcessKey(FRC_2012_Robot_Props &m_FRC2012RobotProps,Scripting::Script& script,size_t index)
{
	const char *err;
	Vec2D PresetPosition;
	err=ProcessVec2D(m_FRC2012RobotProps,script,PresetPosition);
	ASSERT_MSG(!err, err);
	PresetPosition[1]=c_HalfCourtLength-PresetPosition[1];
	m_FRC2012RobotProps.PresetPositions[index]=PresetPosition;  //x,y  where x=width
	return err;
}

const char *ProcessKeyCorrection(FRC_2012_Robot_Props &m_FRC2012RobotProps,Scripting::Script& script,size_t row,size_t column)
{
	const char* err=NULL;
	char CellName[4];
	CellName[0]='c';
	CellName[1]='1'+row;
	CellName[2]='1'+column;
	CellName[3]=0;
	err = script.GetFieldTable(CellName);

	err = script.GetField("p", NULL, NULL,&m_FRC2012RobotProps.KeyCorrections[row][column].PowerCorrection);
	err = script.GetField("x", NULL, NULL,&m_FRC2012RobotProps.KeyCorrections[row][column].YawCorrection);

	script.Pop();
	return err;
}

//declared as global to avoid allocation on stack each iteration
const char * const g_FRC_2012_Controls_Events[] = 
{
	"Turret_SetCurrentVelocity","Turret_SetIntendedPosition","Turret_SetPotentiometerSafety",
	"PitchRamp_SetCurrentVelocity","PitchRamp_SetIntendedPosition","PitchRamp_SetPotentiometerSafety",
	"PowerWheels_SetCurrentVelocity","PowerWheels_SetEncoderSafety","PowerWheels_IsRunning",
	"Ball_SetCurrentVelocity","Ball_Fire","Ball_Squirt","Ball_Grip","Ball_GripL","Ball_GripM","Ball_GripH",
	"Flippers_SetCurrentVelocity","Flippers_SetIntendedPosition","Flippers_SetPotentiometerSafety",
	"Flippers_Advance","Flippers_Retract",
	"Robot_IsTargeting","Robot_SetTargetingOn","Robot_SetTargetingOff","Robot_TurretSetTargetingOff","Robot_SetTargetingValue",
	"Robot_SetLowGear","Robot_SetLowGearOn","Robot_SetLowGearOff","Robot_SetLowGearValue",
	"Robot_SetPreset1","Robot_SetPreset2","Robot_SetPreset3","Robot_SetPresetPOV",
	"Robot_SetDefensiveKeyValue","Robot_SetDefensiveKeyOn","Robot_SetDefensiveKeyOff",
	"Robot_SetCreepMode","Robot_Flippers_Solenoid"
	//Robot Tester events only
	#ifdef Robot_TesterCode
	,"Ball_SlowWheel"
	#endif
};

const char *FRC_2012_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_FRC_2012_Controls_Events))?g_FRC_2012_Controls_Events[index] : NULL;
}
FRC_2012_Robot_Properties::ControlEvents FRC_2012_Robot_Properties::s_ControlsEvents;

void FRC_2012_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	{
		double version;
		err=script.GetField("version", NULL, NULL, &version);
		if (!err)
			printf ("Version=%.2f\n",version);
	}

	__super::LoadFromScript(script);
	err = script.GetFieldTable("robot_settings");
	if (!err) 
	{
		err = script.GetFieldTable("turret");
		if (!err)
		{
			m_TurretProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("pitch");
		if (!err)
		{
			m_PitchRampProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("power");
		if (!err)
		{
			m_PowerWheelProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("conveyor");
		if (!err)
		{
			m_ConveyorProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("flippers");
		if (!err)
		{
			m_FlipperProps.LoadFromScript(script);
			script.Pop();
		}

		m_LowGearProps=*this;  //copy redundant data first
		err = script.GetFieldTable("low_gear");
		if (!err)
		{
			m_LowGearProps.LoadFromScript(script);
			script.Pop();
		}

		
		err = script.GetFieldTable("key_1");
		if (!err) ProcessKey(m_FRC2012RobotProps,script,0);

		err = script.GetFieldTable("key_2");
		if (!err) ProcessKey(m_FRC2012RobotProps,script,1);

		err = script.GetFieldTable("key_3");
		if (!err) ProcessKey(m_FRC2012RobotProps,script,2);

		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2012RobotProps.Coordinates_DiplayRow=(size_t)fDisplayRow;
		err=script.GetField("ds_target_vars_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2012RobotProps.TargetVars_DisplayRow=(size_t)fDisplayRow;

		err=script.GetField("ds_power_velocity_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2012RobotProps.PowerVelocity_DisplayRow=(size_t)fDisplayRow;

		script.GetField("fire_trigger_delay", NULL, NULL, &m_FRC2012RobotProps.FireTriggerDelay);
		script.GetField("fire_stay_on_time", NULL, NULL, &m_FRC2012RobotProps.FireButtonStayOn_Time);

		err = script.GetFieldTable("grid_corrections");
		if (!err)
		{
			for (size_t row=0;row<3;row++)
			{
				for (size_t column=0;column<3;column++)
				{
					err=ProcessKeyCorrection(m_FRC2012RobotProps,script,row,column);
					assert(!err);
				}
			}
			script.Pop();
		}
		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct FRC_2012_Robot_Props::Autonomous_Properties &auton=m_FRC2012RobotProps.Autonomous_Props;
			{
				double length;
				err = script.GetField("move_forward_ft", NULL, NULL,&length);
				if (!err)
					auton.MoveForward=Feet2Meters(length);
			}
			err = script.GetField("two_shot_scaler", NULL, NULL,&auton.TwoShotScaler);

			err = script.GetFieldTable("ramp_left");
			if (!err)
			{
				Vec2D OffsetPosition;
				err=ProcessVec2D(m_FRC2012RobotProps,script,OffsetPosition);
				ASSERT_MSG(!err, err);
				auton.RampLeft_ErrorCorrection_Offset=OffsetPosition;
			}
			err = script.GetFieldTable("ramp_right");
			if (!err)
			{
				Vec2D OffsetPosition;
				err=ProcessVec2D(m_FRC2012RobotProps,script,OffsetPosition);
				ASSERT_MSG(!err, err);
				auton.RampRight_ErrorCorrection_Offset=OffsetPosition;
			}
			err = script.GetFieldTable("ramp_center");
			if (!err)
			{
				Vec2D OffsetPosition;
				err=ProcessVec2D(m_FRC2012RobotProps,script,OffsetPosition);
				ASSERT_MSG(!err, err);
				auton.RampCenter_ErrorCorrection_Offset=OffsetPosition;
			}
			{
				const char * const fieldTable[]=
				{
					"ball_1","ball_2"
				};
				//You just gotta love pointers to do this!  ;)
				FRC_2012_Robot_Props::Autonomous_Properties::WaitForBall_Info *ballTable[]=
				{
					&auton.FirstBall_Wait,&auton.SecondBall_Wait
				};
				for (size_t i=0;i<2;i++)
				{
					err = script.GetFieldTable(fieldTable[i]);
					if (!err)
					{
						FRC_2012_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball=*ballTable[i];
						err=script.GetField("initial_wait", NULL, NULL, &ball.InitialWait);
						ASSERT_MSG(!err, err);
						err=script.GetField("timeout_wait", NULL, NULL, &ball.TimeOutWait);
						ASSERT_MSG(!err, err);
						err=script.GetField("tolerance", NULL, NULL, &ball.ToleranceThreshold);
						ASSERT_MSG(!err, err);
						script.Pop();
					}
				}
			}

			script.GetField("x_left_arc", NULL, NULL, &auton.XLeftArc);
			script.GetField("x_right_arc", NULL, NULL, &auton.XRightArc);
			script.Pop();
		}
		//This is the main robot settings pop
		script.Pop();
	}
	err = script.GetFieldTable("controls");
	if (!err)
	{
		m_RobotControls.LoadFromScript(script);
		script.Pop();
	}
}


  /***********************************************************************************************************************************/
 /*														FRC_2012_Goals::Fire														*/
/***********************************************************************************************************************************/

FRC_2012_Goals::Fire::Fire(FRC_2012_Robot &robot,bool On, bool DoSquirt) : m_Robot(robot),m_Terminate(false),m_IsOn(On),m_DoSquirt(DoSquirt)
{
	m_Status=eInactive;
}
FRC_2012_Goals::Fire::Goal_Status FRC_2012_Goals::Fire::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	if (!m_DoSquirt)
		m_Robot.GetBallConveyorSystem().Fire(m_IsOn);
	else
		m_Robot.GetBallConveyorSystem().Squirt(m_IsOn);
		
	m_Status=eCompleted;
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Goals::WaitForBall														*/
/***********************************************************************************************************************************/

FRC_2012_Goals::WaitForBall::WaitForBall(FRC_2012_Robot &robot,double Tolerance) :  m_Robot(robot),m_Tolerance(Tolerance),m_Terminate(false)
{
	m_Status=eInactive;
}
Goal::Goal_Status FRC_2012_Goals::WaitForBall::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	//Keep simple... if we have noisy artifacts that yield false positives then we may need to consider blend and / or Kalman
	const double PowerWheelSpeedDifference=m_Robot.GetPowerWheels().GetRequestedVelocity_Difference();
	//when we encounter the tolerance dip in speed we are done
	if (fabs(PowerWheelSpeedDifference)>m_Tolerance)
	{
		printf("Ball Deployed\n");
		m_Status=eCompleted;
	}
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Goals::OperateSolenoid													*/
/***********************************************************************************************************************************/

FRC_2012_Goals::OperateSolenoid::OperateSolenoid(FRC_2012_Robot &robot,FRC_2012_Robot::SolenoidDevices SolenoidDevice,bool Open) : m_Robot(robot),
m_SolenoidDevice(SolenoidDevice),m_Terminate(false),m_IsOpen(Open) 
{	
	m_Status=eInactive;
}

FRC_2012_Goals::OperateSolenoid::Goal_Status FRC_2012_Goals::OperateSolenoid::Process(double dTime_s)
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
		case FRC_2012_Robot::eFlipperDown:
			m_Robot.SetFlipperPneumatic(m_IsOpen);
			break;
		case FRC_2012_Robot::eUseLowGear:
		case FRC_2012_Robot::eUseBreakDrive:
		case FRC_2012_Robot::eRampDeployment:
			assert(false);
			break;
	}
	m_Status=eCompleted;
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*															FRC_2012_Goals															*/
/***********************************************************************************************************************************/

Goal *FRC_2012_Goals::Get_ShootBalls(FRC_2012_Robot *Robot,bool DoSquirt)
{
	//Goal_Wait *goal_waitforturret=new Goal_Wait(1.0); //wait for turret
	Goal_Wait *goal_waitforballs1=new Goal_Wait(8.0); //wait for balls
	Fire *FireOn=new Fire(*Robot,true,DoSquirt);
	Goal_Wait *goal_waitforballs2=new Goal_Wait(7.0); //wait for balls
	Fire *FireOff=new Fire(*Robot,false,DoSquirt);
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(FireOff);
	MainGoal->AddSubgoal(goal_waitforballs2);
	MainGoal->AddSubgoal(FireOn);
	MainGoal->AddSubgoal(goal_waitforballs1);
	//MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

Goal *FRC_2012_Goals::Get_ShootBalls_WithPreset(FRC_2012_Robot *Robot,size_t KeyIndex)
{
	Robot->Set_Auton_PresetPosition(KeyIndex);
	return Get_ShootBalls(Robot);
}

Goal *FRC_2012_Goals::Get_FRC2012_Autonomous(FRC_2012_Robot *Robot,size_t KeyIndex,size_t TargetIndex,size_t RampIndex)
{
	const FRC_2012_Robot_Props::Autonomous_Properties &auton=Robot->GetRobotProps().GetFRC2012RobotProps().Autonomous_Props;
	Robot->Set_Auton_PresetPosition(KeyIndex);
	Robot->SetTarget((FRC_2012_Robot::Targets)TargetIndex);
	Fire *FireOn=new Fire(*Robot,true);
	#if 0
	Goal_Wait *goal_waitforballs=new Goal_Wait(auton.FirstBall_Wait.InitialWait); //wait for balls
	#else
	Goal_Ship_MoveToPosition *goal_drive_foward=NULL;
	if (auton.MoveForward!=0.0)
	{
		const Vec2d start_pos=Robot->GetRobotProps().GetFRC2012RobotProps().PresetPositions[KeyIndex];
		WayPoint wp;
		wp.Position[0]=start_pos[0];
		wp.Position[1]=start_pos[1]+auton.MoveForward;
		wp.Power=1.0;
		goal_drive_foward=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	}

	Generic_CompositeGoal *goal_waitforballs= new Generic_CompositeGoal;
	{
		const FRC_2012_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_1=auton.FirstBall_Wait;
		const FRC_2012_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_2=auton.SecondBall_Wait;

		Generic_CompositeGoal *Ball_1_Composite= new Generic_CompositeGoal;
		if (ball_1.ToleranceThreshold!=0.0)
		{
			//Create the wait for ball goal
			WaitForBall *Ball_1_Wait=new WaitForBall(*Robot,ball_1.ToleranceThreshold);
			//determine if we have a timeout
			if (ball_1.TimeOutWait==-1.0)
				Ball_1_Composite->AddSubgoal(Ball_1_Wait);	
			else
			{
				MultitaskGoal *WaitWithTimeout1=new MultitaskGoal(false);
				WaitWithTimeout1->AddGoal(Ball_1_Wait);
				WaitWithTimeout1->AddGoal(new Goal_Wait(ball_1.TimeOutWait));
				Ball_1_Composite->AddSubgoal(WaitWithTimeout1);
			}
		}
		Ball_1_Composite->AddSubgoal(new Goal_Wait(ball_1.InitialWait));
		Ball_1_Composite->Activate(); //ready to go


		Generic_CompositeGoal *Ball_2_Composite= new Generic_CompositeGoal;
		if (ball_2.ToleranceThreshold!=0.0)
		{
			//Create the wait for ball goal
			WaitForBall *Ball_2_Wait=new WaitForBall(*Robot,ball_2.ToleranceThreshold);
			//determine if we have a timeout
			if (ball_2.TimeOutWait==-1.0)
				Ball_2_Composite->AddSubgoal(Ball_2_Wait);	
			else
			{
				MultitaskGoal *WaitWithTimeout2=new MultitaskGoal(false);
				WaitWithTimeout2->AddGoal(Ball_2_Wait);
				WaitWithTimeout2->AddGoal(new Goal_Wait(ball_2.TimeOutWait));
				Ball_2_Composite->AddSubgoal(WaitWithTimeout2);
			}
		}
		Ball_2_Composite->AddSubgoal(new Goal_Wait(ball_2.InitialWait));
		Ball_2_Composite->Activate(); //ready to go

		//put in backwards
		goal_waitforballs->AddSubgoal(Ball_2_Composite);
		goal_waitforballs->AddSubgoal(Ball_1_Composite);
		goal_waitforballs->Activate(); //ready to go
	}
	#endif
	Fire *FireOff=new Fire(*Robot,false);

	Goal_Ship_MoveToPosition *goal_drive_1=NULL;
	Goal_Ship_MoveToPosition *goal_drive_2=NULL;
	OperateSolenoid *DeployFlipper=NULL;
	Fire *EndSomeFire_On=NULL;
	Goal_Wait *goal_waitEndFire=NULL;
	Fire *EndSomeFire_Off=NULL;
	if (RampIndex != (size_t)-1)
	{
		DeployFlipper=new OperateSolenoid(*Robot,FRC_2012_Robot::eFlipperDown,true);
		const double YPad=Inches2Meters(5); //establish our Y being 5 inches from the ramp
		double Y = (c_BridgeDimensions[1] / 2.0) + YPad;
		double X;
		double X_Tweak;
		switch (RampIndex)
		{
			case 0: 
				X=0,X_Tweak=0; 
				X+=auton.RampCenter_ErrorCorrection_Offset[0];
				Y+=auton.RampCenter_ErrorCorrection_Offset[1];
				break;
			case 1: 
				X=-(c_HalfCourtWidth-(c_BridgeDimensions[0]/2.0)),X_Tweak=-(c_HalfCourtWidth+auton.XLeftArc); 
				X+=auton.RampLeft_ErrorCorrection_Offset[0];
				Y+=auton.RampLeft_ErrorCorrection_Offset[1];
				break;
			case 2: 
				X= (c_HalfCourtWidth-(c_BridgeDimensions[0]/2.0)),X_Tweak= (c_HalfCourtWidth+auton.XRightArc); 
				X+=auton.RampRight_ErrorCorrection_Offset[0];
				Y+=auton.RampRight_ErrorCorrection_Offset[1];
				break;
		}
		WayPoint wp;
		wp.Position[0]=X;
		wp.Position[1]=Y;
		wp.Power=1.0;
		goal_drive_2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp);
		wp.Position[1]= (Robot->GetPos_m()[1] + Y) / 2.0;  //mid point on the Y so it can straighten out
		wp.Position[0]=  X_Tweak;
		goal_drive_1=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,false,false,0.01); //don't stop on this one
		//Since turret is disabled for targeting only fire if we are in the middle key
		if (RampIndex==0)
		{
			EndSomeFire_On=new Fire(*Robot,true);
			goal_waitEndFire=new Goal_Wait(8.0); //wait for balls
			EndSomeFire_Off=new Fire(*Robot,false);
		}
	}
	//Inserted in reverse since this is LIFO stack list
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	if (goal_drive_1)
	{
		if (RampIndex==0)
		{
			MainGoal->AddSubgoal(EndSomeFire_Off);
			MainGoal->AddSubgoal(goal_waitEndFire);
		}
		MainGoal->AddSubgoal(goal_drive_2);
		if (RampIndex==0)
			MainGoal->AddSubgoal(EndSomeFire_On);
		MainGoal->AddSubgoal(goal_drive_1);
		MainGoal->AddSubgoal(DeployFlipper);
	}
	MainGoal->AddSubgoal(FireOff);
	MainGoal->AddSubgoal(goal_waitforballs);
	if (goal_drive_foward)
		MainGoal->AddSubgoal(goal_drive_foward);
	MainGoal->AddSubgoal(FireOn);
	MainGoal->Activate();
	return MainGoal;
}

#ifdef Robot_TesterCode
  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2012_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) Voltage=0.0;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			{
				//	printf("Turret=%f\n",Voltage);
				//DOUT3("Turret Voltage=%f",Voltage);
				m_TurretVoltage=Voltage;
				m_Turret_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Turret_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::ePitchRamp:
			{
				//	printf("Pitch=%f\n",Voltage);
				//DOUT3("Pitch Voltage=%f",Voltage);
				m_PitchRampVoltage=Voltage;
				m_Pitch_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Pitch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::eFlippers:
			{
				//	printf("Flippers=%f\n",Voltage);
				//DOUT3("Flippers Voltage=%f",Voltage);
				m_FlipperVoltage=Voltage;
				m_Flippers_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Flippers_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::ePowerWheels:
			if (m_SlowWheel) Voltage=0.0;
			m_PowerWheelVoltage=Voltage;
			m_PowerWheel_Enc.UpdateEncoderVoltage(Voltage);
			m_PowerWheel_Enc.TimeChange();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2012_Robot::eLowerConveyor:
			m_LowerConveyorVoltage=Voltage;
			m_LowerConveyor_Enc.UpdateEncoderVoltage(Voltage);
			m_LowerConveyor_Enc.TimeChange();
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			m_MiddleConveyorVoltage=Voltage;
			m_MiddleConveyor_Enc.UpdateEncoderVoltage(Voltage);
			m_MiddleConveyor_Enc.TimeChange();
			break;
		case FRC_2012_Robot::eFireConveyor:
			m_FireConveyorVoltage=Voltage;
			m_FireConveyor_Enc.UpdateEncoderVoltage(Voltage);
			m_FireConveyor_Enc.TimeChange();
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
	case FRC_2012_Robot::eTurret:
		Dout(m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,1,"t=%.2f",Voltage);
		break;
	case FRC_2012_Robot::ePitchRamp:
		Dout(m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,1,"p=%.2f",Voltage);
		break;
	case FRC_2012_Robot::ePowerWheels:
		Dout(m_RobotProps.GetPowerWheelProps().GetRotaryProps().Feedback_DiplayRow,1,"po_v=%.2f",Voltage);
		break;
	case FRC_2012_Robot::eFlippers:
		Dout(m_RobotProps.GetFlipperProps().GetRotaryProps().Feedback_DiplayRow,1,"f=%.2f",Voltage);
		break;
	}
	#endif
}

bool FRC_2012_Robot_Control::GetBoolSensorState(size_t index)
{
	bool ret;
	switch (index)
	{
	case FRC_2012_Robot::eLowerConveyor_Sensor:
		ret=m_LowerSensor;
		break;
	case FRC_2012_Robot::eMiddleConveyor_Sensor:
		ret=m_MiddleSensor;
		break;
	case FRC_2012_Robot::eFireConveyor_Sensor:
		ret=m_FireSensor;
		break;
	default:
		assert (false);
	}
	return ret;
}

FRC_2012_Robot_Control::FRC_2012_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_TurretVoltage(0.0),m_PowerWheelVoltage(0.0),
	m_LowerSensor(false),m_MiddleSensor(false),m_FireSensor(false),m_SlowWheel(false)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
	#if 0
	Dout(1,"");
	Dout(2,"");
	Dout(3,"");
	Dout(4,"");
	Dout(5,"");
	#endif
}

void FRC_2012_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			m_Turret_Pot.ResetPos();
			break;
		case FRC_2012_Robot::ePitchRamp:
			m_Pitch_Pot.ResetPos();
			//We may want this for more accurate simulation
			//m_Pitch_Pot.SetPos_m((m_Pitch_Pot.GetMinRange()+m_Pitch_Pot.GetMaxRange()) / 2.0);
			break;
		case FRC_2012_Robot::eFlippers:
			m_Flippers_Pot.ResetPos();
			break;
		case FRC_2012_Robot::ePowerWheels:
			m_PowerWheel_Enc.ResetPos();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2012_Robot::eLowerConveyor:
			m_LowerConveyor_Enc.ResetPos();
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			m_MiddleConveyor_Enc.ResetPos();
			break;
		case FRC_2012_Robot::eFireConveyor:
			m_FireConveyor_Enc.ResetPos();
			break;
	}
}

//This is only for Robot Tester
void FRC_2012_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
	if (Bind)
	{
		em->EventOnOff_Map["Ball_LowerSensor"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::TriggerLower);
		em->EventOnOff_Map["Ball_MiddleSensor"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::TriggerMiddle);
		em->EventOnOff_Map["Ball_FireSensor"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::TriggerFire);
		em->EventOnOff_Map["Ball_SlowWheel"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::SlowWheel);
	}
	else
	{
		em->EventOnOff_Map["Ball_LowerSensor"]  .Remove(*this, &FRC_2012_Robot_Control::TriggerLower);
		em->EventOnOff_Map["Ball_MiddleSensor"]  .Remove(*this, &FRC_2012_Robot_Control::TriggerMiddle);
		em->EventOnOff_Map["Ball_FireSensor"]  .Remove(*this, &FRC_2012_Robot_Control::TriggerFire);
		em->EventOnOff_Map["Ball_SlowWheel"]  .Remove(*this, &FRC_2012_Robot_Control::SlowWheel);
	}
}

void FRC_2012_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2012_Robot_Properties *robot_props=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		Rotary_Properties turret_props=robot_props->GetTurretProps();
		//turret_props.SetMinRange(0);
		//turret_props.SetMaxRange(Pi2);
		turret_props.SetUsingRange(false);
		m_Turret_Pot.Initialize(&turret_props);
		m_Pitch_Pot.Initialize(&robot_props->GetPitchRampProps());
		m_Flippers_Pot.Initialize(&robot_props->GetFlipperProps());
		m_PowerWheel_Enc.Initialize(&robot_props->GetPowerWheelProps());
		m_LowerConveyor_Enc.Initialize(&robot_props->GetConveyorProps());
		m_MiddleConveyor_Enc.Initialize(&robot_props->GetConveyorProps());
		m_FireConveyor_Enc.Initialize(&robot_props->GetConveyorProps());
	}
}

void FRC_2012_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Turret_Pot.SetTimeDelta(dTime_s);
	m_Pitch_Pot.SetTimeDelta(dTime_s);
	m_Flippers_Pot.SetTimeDelta(dTime_s);
	m_PowerWheel_Enc.SetTimeDelta(dTime_s);
	m_LowerConveyor_Enc.SetTimeDelta(dTime_s);
	m_MiddleConveyor_Enc.SetTimeDelta(dTime_s);
	m_FireConveyor_Enc.SetTimeDelta(dTime_s);
	//display voltages
	DOUT(2,"l=%.2f r=%.2f t=%.2f pi=%.2f pw=%.2f lc=%.2f mc=%.2f fc=%.2f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),
		m_TurretVoltage,m_PitchRampVoltage,m_PowerWheelVoltage,m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage);
}


double FRC_2012_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
		
			result=NormalizeRotation2(m_Turret_Pot.GetPotentiometerCurrentPosition() - Pi);
			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			break;
		case FRC_2012_Robot::ePitchRamp:

			result=m_Pitch_Pot.GetPotentiometerCurrentPosition();
			DOUT (4,"pitch=%f flippers=%f",RAD_2_DEG(result),RAD_2_DEG(m_Flippers_Pot.GetPotentiometerCurrentPosition()));
			break;
		case FRC_2012_Robot::eFlippers:
			result=m_Flippers_Pot.GetPotentiometerCurrentPosition();
			break;
		case FRC_2012_Robot::ePowerWheels:
			result=m_PowerWheel_Enc.GetEncoderVelocity();
			break;
		case FRC_2012_Robot::eLowerConveyor:
			result=m_LowerConveyor_Enc.GetEncoderVelocity();
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			result=m_MiddleConveyor_Enc.GetEncoderVelocity();
			//DOUT4 ("vel=%f",result);
			break;
		case FRC_2012_Robot::eFireConveyor:
			result=m_FireConveyor_Enc.GetEncoderVelocity();
			//DOUT5 ("vel=%f",result);
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			Dout(m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,14,"d=%.1f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,14,"p=%.1f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePowerWheels:
			Dout(m_RobotProps.GetPowerWheelProps().GetRotaryProps().Feedback_DiplayRow,11,"rs=%.2f",result / Pi2);
			break;
		case FRC_2012_Robot::eFlippers:
			Dout(m_RobotProps.GetFlipperProps().GetRotaryProps().Feedback_DiplayRow,14,"f=%.1f",RAD_2_DEG(result));
			break;
	}
	#endif

	return result;
}

void FRC_2012_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2012_Robot::eUseLowGear:
		printf("UseLowGear=%d\n",Open);
		//m_UseLowGear=Open;
		break;
	case FRC_2012_Robot::eFlipperDown:
		printf("FlipperDown=%d\n",Open);
		break;
	case FRC_2012_Robot::eUseBreakDrive:
		printf("UseBreakDrive=%d\n",Open);
		break;
	case FRC_2012_Robot::eRampDeployment:
		printf("RampDeployment=%d\n",Open);
		//m_RampDeployment=Open;
		break;
	}
}
  /***********************************************************************************************************************************/
 /*														FRC_2012_Turret_UI															*/
/***********************************************************************************************************************************/

void FRC_2012_Turret_UI::Initialize(Entity2D::EventMap& em, const Turret_Properties *props)
{
	if (props)
		m_props=*props;
	else
		m_props.YOffset=3.0;
}
void FRC_2012_Turret_UI::UI_Init(Actor_Text *parent)
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Turret= new osgText::Text;
	m_Turret->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
	m_Turret->setCharacterSize(m_UIParent->GetFontSize());
	m_Turret->setFontResolution(10,10);
	m_Turret->setPosition(position);
	m_Turret->setAlignment(osgText::Text::CENTER_CENTER);
	m_Turret->setText(L"\\/\n|\n( )");
	m_Turret->setUpdateCallback(m_UIParent);
}
void FRC_2012_Turret_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	FRC_2012_Control_Interface *turret_access=m_RobotControl;
	double Swivel=(-turret_access->GetRotaryCurrentPorV(FRC_2012_Robot::eTurret));
	double HeadingToUse=Heading+Swivel;
	const double FS=m_UIParent->GetFontSize();

	const double TurretLength=1.0;
	Vec2d TurretOffset(0,TurretLength);
	TurretOffset=GlobalToLocal(Swivel ,TurretOffset);
	TurretOffset[1]+=(m_props.YOffset-TurretLength);

	const Vec2d TurretLocalOffset=GlobalToLocal(Heading ,TurretOffset);
	const osg::Vec3 TurretPos (parent_pos[0]+( TurretLocalOffset[0]*FS),parent_pos[1]+( TurretLocalOffset[1]*FS),parent_pos[2]);

	//const char *TeamName=m_UIParent->GetEntityProperties_Interface()->GetTeamName();
	//if (strcmp(TeamName,"red")==0)
	//	m_Turret->setColor(osg::Vec4(1.0f,0.0f,0.5f,1.0f));  //This is almost magenta (easier to see)
	//else if (strcmp(TeamName,"blue")==0)
	//	m_Turret->setColor(osg::Vec4(0.0f,0.5f,1.0f,1.0f));  //This is almost cyan (easier to see too)

	if (m_Turret.valid())
	{
		m_Turret->setPosition(TurretPos);
		m_Turret->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}

}
void FRC_2012_Turret_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Turret.valid()) m_Turret->setCharacterSize(SizeToUse);
}
void FRC_2012_Turret_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Turret.valid()) geode->addDrawable(m_Turret);
	else
		if (m_Turret.valid()) geode->removeDrawable(m_Turret);
}

  /***************************************************************************************************************/
 /*											FRC_2012_Power_Wheel_UI												*/
/***************************************************************************************************************/

void FRC_2012_Power_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.0,2.0);
	Myprops.m_Color=osg::Vec4(1.0,0.0,0.5,1.0);
	Myprops.m_TextDisplay=L"|";

	__super::Initialize(em,&Myprops);
	m_PowerWheelMaxSpeed=m_RobotControl->GetRobotProps().GetPowerWheelProps().GetMaxSpeed();
}

void FRC_2012_Power_Wheel_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double NormalizedVelocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::ePowerWheels) / m_PowerWheelMaxSpeed;
	//NormalizedVelocity-=0.2;
	//if (NormalizedVelocity<0.0)
	//	NormalizedVelocity=0.0;

	//Scale down the rotation to something easy to gauge in UI
	AddRotation((NormalizedVelocity * 18) * dTime_s);
}
  /***************************************************************************************************************/
 /*											FRC_2012_Lower_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Lower_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.0,-1.5);
	Myprops.m_Color=osg::Vec4(0.5,1.0,1.0,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Lower_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eLowerConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*											FRC_2012_Middle_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Middle_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.30,-0.5);
	Myprops.m_Color=osg::Vec4(1.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Middle_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eMiddleConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*											FRC_2012_Fire_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Fire_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.60,0.5);
	Myprops.m_Color=osg::Vec4(0.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Fire_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eFireConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*												FRC_2012_Robot_UI												*/
/***************************************************************************************************************/

FRC_2012_Robot_UI::FRC_2012_Robot_UI(const char EntityName[]) : FRC_2012_Robot(EntityName,this),FRC_2012_Robot_Control(),
		m_TankUI(this),m_TurretUI(this),m_PowerWheelUI(this),m_LowerConveyor(this),m_MiddleConveyor(this),m_FireConveyor(this)
{
}

void FRC_2012_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
	m_PowerWheelUI.TimeChange(dTime_s);
	m_LowerConveyor.TimeChange(dTime_s);
	m_MiddleConveyor.TimeChange(dTime_s);
	m_FireConveyor.TimeChange(dTime_s);
}
void FRC_2012_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
	m_TurretUI.Initialize(em);
	m_PowerWheelUI.Initialize(em);
	m_LowerConveyor.Initialize(em);
	m_MiddleConveyor.Initialize(em);
	m_FireConveyor.Initialize(em);
}

void FRC_2012_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
	m_TurretUI.UI_Init(parent);
	m_PowerWheelUI.UI_Init(parent);
	m_LowerConveyor.UI_Init(parent);
	m_MiddleConveyor.UI_Init(parent);
	m_FireConveyor.UI_Init(parent);
}
void FRC_2012_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
	m_TurretUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_PowerWheelUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_LowerConveyor.update(nv,draw,parent_pos,-GetAtt_r());
	m_MiddleConveyor.update(nv,draw,parent_pos,-GetAtt_r());
	m_FireConveyor.update(nv,draw,parent_pos,-GetAtt_r());
}
void FRC_2012_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
	m_TurretUI.Text_SizeToUse(SizeToUse);
	m_PowerWheelUI.Text_SizeToUse(SizeToUse);
	m_LowerConveyor.Text_SizeToUse(SizeToUse);
	m_MiddleConveyor.Text_SizeToUse(SizeToUse);
	m_FireConveyor.Text_SizeToUse(SizeToUse);
}
void FRC_2012_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
	m_TurretUI.UpdateScene(geode,AddOrRemove);
	m_PowerWheelUI.UpdateScene(geode,AddOrRemove);
	m_LowerConveyor.UpdateScene(geode,AddOrRemove);
	m_MiddleConveyor.UpdateScene(geode,AddOrRemove);
	m_FireConveyor.UpdateScene(geode,AddOrRemove);
}

#endif
