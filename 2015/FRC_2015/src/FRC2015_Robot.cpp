#include "stdafx.h"
#include "Robot_Tester.h"

#ifdef Robot_TesterCode
namespace Robot_Tester
{
	#include "Tank_Robot_UI.h"
	#include "CommonUI.h"
	#include "FRC2015_Robot.h"
}

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

#else

#include "FRC2015_Robot.h"
#include "SmartDashboard/SmartDashboard.h"
using namespace Framework::Base;
using namespace std;
#endif

#define __DisableEncoderTracking__
//Enable this to send remote coordinate to network variables to manipulate a shape for tracking
#undef __EnableShapeTrackingSimulation__
#if 0
#define UnitMeasure2UI Meters2Feet
#define UI2UnitMeasure Feet2Meters
#else
#define UnitMeasure2UI Meters2Inches
#define UI2UnitMeasure Inches2Meters
#endif

#if 0
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
#endif

  /***********************************************************************************************************************************/
 /*														FRC_2015_Robot::Turret														*/
/***********************************************************************************************************************************/

FRC_2015_Robot::Turret::Turret(FRC_2015_Robot *parent,Rotary_Control_Interface *robot_control) : 	m_pParent(parent),m_Velocity(0.0)
{
}


void FRC_2015_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2015_Robot::Turret::Turret_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2015_Robot::Turret::Turret_SetRequestedVelocity);
	}
}

void FRC_2015_Robot::Turret::TimeChange(double dTime_s)
{
	m_Velocity=0.0;
}

void FRC_2015_Robot::Turret::ResetPos()
{
	m_Velocity=0.0;
}

  /***********************************************************************************************************************************/
 /*													FRC_2015_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2015_Robot::PitchRamp::PitchRamp(FRC_2015_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),m_Velocity(0.0)
{
}


void FRC_2015_Robot::PitchRamp::TimeChange(double dTime_s)
{
	m_Velocity=0.0;
}

void FRC_2015_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2015_Robot::PitchRamp::Pitch_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2015_Robot::PitchRamp::Pitch_SetRequestedVelocity);
	}
}

void FRC_2015_Robot::PitchRamp::ResetPos()
{
	m_Velocity=0.0;
}

  /***********************************************************************************************************************************/
 /*													FRC_2015_Robot::Kicker_Wheel													*/
/***********************************************************************************************************************************/

FRC_2015_Robot::Kicker_Wheel::Kicker_Wheel(FRC_2015_Robot *parent,Rotary_Control_Interface *robot_control) :
	Rotary_Velocity_Control("KickerWheel",robot_control,eKickerWheel),m_pParent(parent)
{
}

void FRC_2015_Robot::Kicker_Wheel::TimeChange(double dTime_s)
{
	SetRequestedVelocity_FromNormalized(m_Velocity);
	m_Velocity=0.0;
	__super::TimeChange(dTime_s);
}

void FRC_2015_Robot::Kicker_Wheel::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["KickerWheel_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2015_Robot::Kicker_Wheel::Kicker_Wheel_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["KickerWheel_SetCurrentVelocity"].Remove(*this, &FRC_2015_Robot::Kicker_Wheel::Kicker_Wheel_SetRequestedVelocity);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2015_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

FRC_2015_Robot::Robot_Arm::Robot_Arm(FRC_2015_Robot *parent,Rotary_Control_Interface *robot_control) : 
Rotary_Position_Control("Arm",robot_control,eArm),m_pParent(parent),m_Advance(false),m_Retract(false)
{
}


void FRC_2015_Robot::Robot_Arm::Advance(bool on)
{
	m_Advance=on;
}
void FRC_2015_Robot::Robot_Arm::Retract(bool on)
{
	m_Retract=on;
}

bool FRC_2015_Robot::Robot_Arm::DidHitMinLimit() const
{
	return m_pParent->m_RobotControl->GetBoolSensorState(eDartLower);
}

bool FRC_2015_Robot::Robot_Arm::DidHitMaxLimit() const
{
	return m_pParent->m_RobotControl->GetBoolSensorState(eDartUpper);
}

void FRC_2015_Robot::Robot_Arm::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Advance ^ m_Retract)
		SetCurrentLinearAcceleration(m_Advance?Accel:-Brake);

	__super::TimeChange(dTime_s);
#if 0
#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetIntakeDeploymentProps().GetRotaryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
#endif
#endif
#ifdef Robot_TesterCode
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	const double c_GearToArmRatio=1.0/props.ArmToGearRatio;
	double Pos_m=GetPos_m();
	double height=AngleToHeight_m(Pos_m);
	DOUT4("Arm=%f Angle=%f %fft %fin",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
#endif
}


double FRC_2015_Robot::Robot_Arm::AngleToHeight_m(double Angle_r) const
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	const double c_GearToArmRatio=1.0/props.ArmToGearRatio;

	return (sin(Angle_r*c_GearToArmRatio)*props.ArmLength)+props.GearHeightOffset;
}
double FRC_2015_Robot::Robot_Arm::Arm_AngleToHeight_m(double Angle_r) const
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	return (sin(Angle_r)*props.ArmLength)+props.GearHeightOffset;
}

double FRC_2015_Robot::Robot_Arm::HeightToAngle_r(double Height_m) const
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	return asin((Height_m-props.GearHeightOffset)/props.ArmLength) * props.ArmToGearRatio;
}

double FRC_2015_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(double raw) const
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=props.PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}

void FRC_2015_Robot::Robot_Arm::SetPosRest()
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	SetIntendedPosition(HeightToAngle_r(Inches2Meters(props.ToteRestHeight))  );
}
void FRC_2015_Robot::Robot_Arm::SetTote2Height()
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	SetIntendedPosition( HeightToAngle_r(Inches2Meters(props.Tote2Height)) );
}
void FRC_2015_Robot::Robot_Arm::SetTote3Height()
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	SetIntendedPosition(HeightToAngle_r(Inches2Meters(props.Tote3Height)));
}
void FRC_2015_Robot::Robot_Arm::SetTote4Height()
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	SetIntendedPosition( HeightToAngle_r(Inches2Meters(props.Tote4Height)) );
}
void FRC_2015_Robot::Robot_Arm::SetTote5Height()
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	SetIntendedPosition( HeightToAngle_r(Inches2Meters(props.Tote5Height)) );
}
void FRC_2015_Robot::Robot_Arm::SetTote6Height()
{
	const FRC_2015_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2015RobotProps();
	SetIntendedPosition( HeightToAngle_r(Inches2Meters(props.Tote6Height)) );
}

void FRC_2015_Robot::Robot_Arm::CloseForkRight(bool Close)
{
	m_pParent->m_RobotControl->CloseSolenoid(eForkRight,Close);
}
void FRC_2015_Robot::Robot_Arm::CloseForkLeft(bool Close)
{
	m_pParent->m_RobotControl->CloseSolenoid(eForkLeft,Close);
}

void FRC_2015_Robot::Robot_Arm::CloseForkBoth(bool Close)
{
	CloseForkLeft(Close);
	CloseForkRight(Close);
}

void FRC_2015_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2015_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Arm_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2015_Robot::Robot_Arm::SetPotentiometerSafety);

		em->Event_Map["Arm_SetPosRest"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::SetPosRest);
		em->Event_Map["Arm_SetTote2Height"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::SetTote2Height);
		em->Event_Map["Arm_SetTote3Height"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::SetTote3Height);
		em->Event_Map["Arm_SetTote4Height"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::SetTote4Height);
		em->Event_Map["Arm_SetTote5Height"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::SetTote5Height);
		em->Event_Map["Arm_SetTote6Height"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::SetTote6Height);

		em->EventOnOff_Map["Arm_Advance"].Subscribe(ehl,*this, &FRC_2015_Robot::Robot_Arm::Advance);
		em->EventOnOff_Map["Arm_Retract"].Subscribe(ehl,*this, &FRC_2015_Robot::Robot_Arm::Retract);

		em->EventOnOff_Map["Arm_ForkRight"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::CloseForkRight);
		em->EventOnOff_Map["Arm_ForkLeft"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::CloseForkLeft);
		em->EventOnOff_Map["Arm_ForkBoth"].Subscribe(ehl, *this, &FRC_2015_Robot::Robot_Arm::CloseForkBoth);
	}
	else
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Arm_SetPotentiometerSafety"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetPotentiometerSafety);

		em->Event_Map["Arm_SetPosRest"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetPosRest);
		em->Event_Map["Arm_SetTote2Height"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetTote2Height);
		em->Event_Map["Arm_SetTote3Height"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetTote3Height);
		em->Event_Map["Arm_SetTote4Height"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetTote4Height);
		em->Event_Map["Arm_SetTote5Height"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetTote5Height);
		em->Event_Map["Arm_SetTote6Height"].Remove(*this, &FRC_2015_Robot::Robot_Arm::SetTote6Height);

		em->EventOnOff_Map["Arm_Advance"].Remove(*this, &FRC_2015_Robot::Robot_Arm::Advance);
		em->EventOnOff_Map["Arm_Retract"].Remove(*this, &FRC_2015_Robot::Robot_Arm::Retract);

		em->EventOnOff_Map["Arm_ForkRight"]  .Remove(*this, &FRC_2015_Robot::Robot_Arm::CloseForkRight);
		em->EventOnOff_Map["Arm_ForkLeft"]  .Remove(*this, &FRC_2015_Robot::Robot_Arm::CloseForkLeft);
		em->EventOnOff_Map["Arm_ForkBoth"]  .Remove(*this, &FRC_2015_Robot::Robot_Arm::CloseForkBoth);
	}
}



  /***********************************************************************************************************************************/
 /*															FRC_2015_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;

FRC_2015_Robot::FRC_2015_Robot(const char EntityName[],FRC_2015_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), 
		m_Turret(this,robot_control),m_PitchRamp(this,robot_control),
		m_Kicker_Wheel(this,robot_control),m_Arm(this,robot_control),m_LatencyCounter(0.0),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),
		m_AutonPresetIndex(0),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_SetLowGear(false),m_SetDriverOverride(false)
{
	//ensure the variables are initialized before calling get
	SmartDashboard::PutNumber("X Position",0.0);
	SmartDashboard::PutNumber("Y Position",0.0);
	SmartDashboard::PutBoolean("Main_Is_Targeting",false);
	//Note: The processing vision is setup to use these same variables for both tracking processes (i.e. front and rear camera) we should only need to be tracking one of them at a time
	//We may want to add a prefix window to identify which window they are coming from, but this may not be necessary.
}

void FRC_2015_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2015_Robot_Properties *RobotProps=dynamic_cast<const FRC_2015_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)

	//set to the default key position
	//const FRC_2015_Robot_Props &robot2015props=RobotProps->GetFRC2015RobotProps();
	m_Kicker_Wheel.Initialize(em,RobotProps?&RobotProps->GetKickerWheelProps():NULL);
	m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
}
void FRC_2015_Robot::ResetPos()
{
	__super::ResetPos();
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	m_Arm.ResetPos();
	//TODO this is tacky... will have better low gear method soon
	if (!GetBypassPosAtt_Update())
	{
		//m_Intake_Arm.ResetPos();
		SetLowGear(true);
	}
	m_Kicker_Wheel.ResetPos();  //ha pedantic
}

namespace VisionConversion
{
	const double c_TargetBaseHeight=Feet2Meters(2.0);
	//Note: for this camera we use square pixels, so we need not worry about pixel aspect ratio
	const double c_X_Image_Res=640.0;		//X Image resolution in pixels, should be 160, 320 or 640
	const double c_Y_Image_Res=480.0;
	const double c_AspectRatio=c_X_Image_Res/c_Y_Image_Res;
	const double c_AspectRatio_recip=c_Y_Image_Res/c_X_Image_Res;
	//const double c_ViewAngle=43.5;  //Axis M1011 camera (in code sample)
	const double c_ViewAngle_x=DEG_2_RAD(45);		//These are the angles I've measured
	const double c_ViewAngle_y=DEG_2_RAD(45);
	const double c_HalfViewAngle_y=c_ViewAngle_y/2.0;

	//doing it this way is faster since it never changes
	const double c_ez_y=(tan(c_ViewAngle_y/2.0));
	const double c_ez_y_recip=1.0/c_ez_y;
	const double c_ez_x=(tan(c_ViewAngle_x/2.0));
	const double c_ez_x_recip=1.0/c_ez_x;

	//For example if the target height is 22.16 feet the distance would be 50, or 10 foot height would be around 22 feet for distance
	//this constant is used to check my pitch math below (typically will be disabled)
	const double c_DistanceCheck=c_TargetBaseHeight*c_ez_y_recip;

	__inline void GetYawAndDistance(double bx,double by,double &dx,double dy,double &dz)
	{
		//Note: the camera angle for x is different than for y... thus for example we have 4:3 aspect ratio
		dz = (dy * c_ez_y_recip) / by;
		dx = (bx * dz) * c_ez_x;
	}

	//This transform is simplified to only works with pitch
	__inline void CameraTransform(double ThetaY,double dx, double dy, double dz, double &ax, double &ay, double &az)
	{
		//assert(ThetaY<PI);
		//ax=(dz*sin(ThetaY) + dx) / cos(ThetaY);
		ax=dx;  //I suspect that I may be interpreting the equation wrong... it seems to go in the wrong direction, but may need to retest 
		ay=dy;
		az=cos(ThetaY)*dz - sin(ThetaY)*dx;
	}

	__inline bool computeDistanceAndYaw (double Ax1,double Ay1,double currentPitch,double &ax1,double &az1) 
	{
		if (IsZero(Ay1)) return false;  //avoid division by zero
		//Now to input the aiming system for the d (x,y,z) equations prior to camera transformation
		const double dy = c_TargetBaseHeight;
		double dx,dz;
		GetYawAndDistance(Ax1,Ay1,dx,dy,dz);
		double ay1;
		CameraTransform(currentPitch,dx,dy,dz,ax1,ay1,az1);
		//TODO see if we want kalman
		//printf("\r x=%.2f y=%.2f dx=%.2f dz=%.2f       ",m_Dx(Ax1),m_Dz(Ay1),m_Ax(dx),m_Az(dz));
		//printf("\r dx=%.2f dz=%.2f ax=%.2f az=%.2f       ",m_Dx(dx),m_Dz(dz),m_Ax(ax1),m_Az(az1));

		//printf("x=%.2f y=%.2f dx=%.2f dz=%.2f ax=%.2f az=%.2f\n",Ax1,Ay1,dx,dz,ax1,az1);

		return true;
		//return c_X_Image_Res * c_TargetBaseHeight / (height * 12 * 2 * tan(DEG_2_RAD(c_ViewAngle)));
	}

}

void FRC_2015_Robot::TimeChange(double dTime_s)
{
	//const FRC_2015_Robot_Props &robot_props=m_RobotProps.GetFRC2015RobotProps();

	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.TimeChange(dTime_s);
	m_PitchRamp.TimeChange(dTime_s);
	m_Kicker_Wheel.AsEntity1D().TimeChange(dTime_s);
	m_Arm.AsEntity1D().TimeChange(dTime_s);

	#ifdef __EnableShapeTrackingSimulation__
	{
		const char * const csz_remote_name="land_reticle";
		Vec2D TargetPos(0.0,0.0);
		Vec2D GlobalPos=TargetPos-GetPos_m();
		Vec2D LocalPos=GlobalToLocal(GetAtt_r(),GlobalPos);
		std::string sBuild=csz_remote_name;
		sBuild+="_x";
		SmartDashboard::PutNumber(sBuild,UnitMeasure2UI(LocalPos[0]));
		sBuild=csz_remote_name;
		sBuild+="_z";
		SmartDashboard::PutNumber(sBuild,UnitMeasure2UI(LocalPos[1]));
		sBuild=csz_remote_name;
		sBuild+="_y";
		SmartDashboard::PutNumber(sBuild,UnitMeasure2UI(0.3048));  //always 1 foot high from center point
	}
	#endif

	//const double  YOffset=-SmartDashboard::GetNumber("Y Position");
	//const double XOffset=SmartDashboard::GetNumber("X Position");
	
	using namespace VisionConversion;

	bool LED_OnState=SmartDashboard::GetBoolean("Main_Is_Targeting");
	m_RobotControl->UpdateVoltage(eCameraLED,LED_OnState?1.0:0.0);
}

const FRC_2015_Robot_Properties &FRC_2015_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

FRC_2015_Robot_Props::Autonomous_Properties &FRC_2015_Robot::GetAutonProps()
{
	return m_RobotProps.GetFRC2015RobotProps_rw().Autonomous_Props;
}

void FRC_2015_Robot::SetLowGear(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	m_SetLowGear=on;
	SetBypassPosAtt_Update(true);
	//m_Turret.SetBypassPos_Update(true);
	//m_PitchRamp.SetBypassPos_Update(true);

	//Now for some real magic with the properties!
	__super::Initialize(*GetEventMap(),m_SetLowGear?&m_RobotProps.GetLowGearProps():&m_RobotProps);
	SetBypassPosAtt_Update(false);
	//m_Turret.SetBypassPos_Update(false);
	//m_PitchRamp.SetBypassPos_Update(false);

	m_RobotControl->OpenSolenoid(eUseLowGear,on);
}

void FRC_2015_Robot::SetLowGearValue(double Value)
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

void FRC_2015_Robot::SetDriverOverride(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//I am not yet certain if this if statement is necessary... I'll have to check what all is involved in setting a variable that is already equal
	if (m_SetDriverOverride!=on)
		SmartDashboard::PutBoolean("DriverOverride",on);
	m_SetDriverOverride=on;
}

void FRC_2015_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Robot_SetLowGear"].Subscribe(ehl, *this, &FRC_2015_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"].Subscribe(ehl, *this, &FRC_2015_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"].Subscribe(ehl, *this, &FRC_2015_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Subscribe(ehl,*this, &FRC_2015_Robot::SetLowGearValue);
		em->EventOnOff_Map["Robot_SetDriverOverride"].Subscribe(ehl, *this, &FRC_2015_Robot::SetDriverOverride);

		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"].Subscribe(ehl, *this, &FRC_2015_Robot::TestAutonomous);
		em->Event_Map["Complete"].Subscribe(ehl,*this,&FRC_2015_Robot::GoalComplete);
		#endif
	}
	else
	{
		em->EventOnOff_Map["Robot_SetLowGear"]  .Remove(*this, &FRC_2015_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"]  .Remove(*this, &FRC_2015_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"]  .Remove(*this, &FRC_2015_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Remove(*this, &FRC_2015_Robot::SetLowGearValue);
		em->EventOnOff_Map["Robot_SetDriverOverride"]  .Remove(*this, &FRC_2015_Robot::SetDriverOverride);

		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"]  .Remove(*this, &FRC_2015_Robot::TestAutonomous);
		em->Event_Map["Complete"]  .Remove(*this, &FRC_2015_Robot::GoalComplete);
		#endif
	}

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_Kicker_Wheel.AsShip1D().BindAdditionalEventControls(Bind);
	m_Arm.AsShip1D().BindAdditionalEventControls(Bind);

	#ifdef Robot_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
	__super::BindAdditionalEventControls(Bind);
}

void FRC_2015_Robot::BindAdditionalUIControls(bool Bind,void *joy, void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}

void FRC_2015_Robot::UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s)
{
	//Call predecessor (e.g. tank steering) to get some preliminary values
	__super::UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,LockShipHeadingToOrientation,dTime_s);
	if (!m_SetDriverOverride)
	{
		//Note: for now we'll just add the values in... we may wish to consider analyzing the existing direction and use the max, but this would require the joystick
		//values from UI, for now I don't wish to add that complexity as I feel a simple add will suffice
		//Now to add turret and pitch settings
		const double TurretAcceleration=m_Turret.GetCurrentVelocity()*GetHeadingSpeed();
		AngularAcceleration+=TurretAcceleration;
		const double PitchVelocity=m_PitchRamp.GetCurrentVelocity()*GetEngaged_Max_Speed();
		AuxVelocity+=PitchVelocity;
	}
}

#ifdef Robot_TesterCode
void FRC_2015_Robot::TestAutonomous()
{
	Goal *oldgoal=ClearGoal();
	if (oldgoal)
		delete oldgoal;

	{
		Goal *goal=NULL;
		goal=FRC_2015_Goals::Get_FRC2015_Autonomous(this);
		if (goal)
			goal->Activate(); //now with the goal(s) loaded activate it
		SetGoal(goal);
		//enable autopilot (note windriver does this in main)
		m_controller->GetUIController_RW()->SetAutoPilot(true);
	}
}

void FRC_2015_Robot::GoalComplete()
{
	printf("Goals completed!\n");
	m_controller->GetUIController_RW()->SetAutoPilot(false);
}
#endif

  /***********************************************************************************************************************************/
 /*													FRC_2015_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2015_Robot_Properties::FRC_2015_Robot_Properties()  : m_TurretProps(
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
	m_KickerWheelProps(
	"Kicker",
	50,    //Mass (about 110 pounds)
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	42,   //with 13.2 gear reduction in radians
	10.0,10.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false	//No limit ever!
	),
	m_RobotControls(&s_ControlsEvents)
{
	{
		//const double c_ArmToGearRatio=72.0/28.0;
		//const double c_PotentiometerToArmRatio=36.0/54.0;

		FRC_2015_Robot_Props props;

		{	//arm potentiometer and arm ratios
			const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
			const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
			const double c_ArmLength_m=Inches2Meters(48);
			const double c_ArmToGearRatio=72.0/28.0;
			//const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
			//const double c_PotentiometerToGearRatio=60.0/32.0;
			//const double c_PotentiometerToArmRatio=c_PotentiometerToGearRatio * c_GearToArmRatio;
			const double c_PotentiometerToArmRatio=36.0/54.0;
			//const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
			const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);
			const double c_GearHeightOffset=Inches2Meters(38.43);
			const double c_WheelDiameter=0.1524;  //6 inches
			const double c_MotorToWheelGearRatio=12.0/36.0;

			props.OptimalAngleUp=c_OptimalAngleUp_r;
			props.OptimalAngleDn=c_OptimalAngleDn_r;
			props.ArmLength=c_ArmLength_m;
			props.ArmToGearRatio=c_ArmToGearRatio;
			props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
			props.PotentiometerMaxRotation=c_PotentiometerMaxRotation;
			props.GearHeightOffset=c_GearHeightOffset;
			props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;

			//Get some good defaults
			const double Tote_stack_Height=11.75;
			const double Clearance=2;
			props.ToteRestHeight=0.0;
			props.Tote2Height=Tote_stack_Height*1+Clearance;
			props.Tote3Height=Tote_stack_Height*2+Clearance;
			props.Tote4Height=Tote_stack_Height*3+Clearance;
			props.Tote5Height=Tote_stack_Height*4+Clearance;
			props.Tote6Height=Tote_stack_Height*5+Clearance;
			m_FRC2015RobotProps=props;
		}

		FRC_2015_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		auton.FirstMove_ft=5.0;
		auton.SideMove_rad=20.0;
		auton.ArmMove_in=0.0;  //safe default
		m_FRC2015RobotProps=props;
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
}

const char *ProcessVec2D(FRC_2015_Robot_Props &m_FRC2015RobotProps,Scripting::Script& script,Vec2d &Dest)
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


//declared as global to avoid allocation on stack each iteration
const char * const g_FRC_2015_Controls_Events[] = 
{
	"Turret_SetCurrentVelocity","Turret_SetIntendedPosition","Turret_SetPotentiometerSafety",
	"PitchRamp_SetCurrentVelocity","PitchRamp_SetIntendedPosition","PitchRamp_SetPotentiometerSafety",
	"Robot_SetLowGear","Robot_SetLowGearOn","Robot_SetLowGearOff","Robot_SetLowGearValue",
	"Robot_SetDriverOverride",
	"IntakeArm_DeployManager",
	"KickerWheel_SetCurrentVelocity",
	"Arm_SetCurrentVelocity","Arm_SetPotentiometerSafety","Arm_SetPosRest",
	"Arm_SetTote2Height","Arm_SetTote3Height","Arm_SetTote4Height","Arm_SetTote5Height","Arm_SetTote6Height",
	"Arm_ForkRight","Arm_ForkLeft","Arm_ForkBoth","Arm_Advance","Arm_Retract",
	"TestAuton"
};

const char *FRC_2015_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_FRC_2015_Controls_Events))?g_FRC_2015_Controls_Events[index] : NULL;
}
FRC_2015_Robot_Properties::ControlEvents FRC_2015_Robot_Properties::s_ControlsEvents;

void FRC_2015_Robot_Props::Autonomous_Properties::ShowAutonParameters()
{
	if (ShowParameters)
	{
		const char * const SmartNames[]={"first_move_ft","side_move_rad","arm_height_in"};
		double * const SmartVariables[]={&FirstMove_ft,&SideMove_rad,&ArmMove_in};

		#if defined Robot_TesterCode || !defined __USE_LEGACY_WPI_LIBRARIES__
		for (size_t i=0;i<_countof(SmartNames);i++)
		try
		{
			*(SmartVariables[i])=SmartDashboard::GetNumber(SmartNames[i]);
		}
		catch (...)
		{
			//I may need to prime the pump here
			SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
		}
		try
		{
			IsSupportingHotSpot=SmartDashboard::GetBoolean("support_hotspot");
		}
		catch (...)
		{
			//I may need to prime the pump here
			SmartDashboard::PutBoolean("support_hotspot",IsSupportingHotSpot);
		}
		#else
		//for whatever reason the Thunder RIO solution is having an issue with using catch(...) with a recurcive termination
		//TODO find more specific catch type to see if it resolves issue
		for (size_t i=0;i<_countof(SmartNames);i++)
		{
			//I may need to prime the pump here
			SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
		}
		//I may need to prime the pump here
		SmartDashboard::PutBoolean("support_hotspot",IsSupportingHotSpot);
		#endif
	}
}

void FRC_2015_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	FRC_2015_Robot_Props &props=m_FRC2015RobotProps;

	const char* err=NULL;
	{
		double version;
		err=script.GetField("version", NULL, NULL, &version);
		if (!err)
			printf ("Version=%.2f\n",version);
	}

	m_ControlAssignmentProps.LoadFromScript(script);
	__super::LoadFromScript(script);
	err = script.GetFieldTable("robot_settings");
	double fTest;
	std::string sTest;
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
		err = script.GetFieldTable("arm");
		if (!err)
		{
			m_ArmProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("kicker");
		if (!err)
		{
			m_KickerWheelProps.LoadFromScript(script);
			script.Pop();
		}

		m_LowGearProps=*this;  //copy redundant data first
		err = script.GetFieldTable("low_gear");
		if (!err)
		{
			m_LowGearProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("height_presets");
		if (!err)
		{
			double fTest;
			err=script.GetField("rest", NULL, NULL, &fTest);
			if (!err)
				props.ToteRestHeight=fTest;
			err=script.GetField("tote_2", NULL, NULL, &fTest);
			if (!err)
				props.Tote2Height=fTest;
			err=script.GetField("tote_3", NULL, NULL, &fTest);
			if (!err)
				props.Tote3Height=fTest;
			err=script.GetField("tote_4", NULL, NULL, &fTest);
			if (!err)
				props.Tote4Height=fTest;
			err=script.GetField("tote_5", NULL, NULL, &fTest);
			if (!err)
				props.Tote5Height=fTest;
			err=script.GetField("tote_6", NULL, NULL, &fTest);
			if (!err)
				props.Tote6Height=fTest;
			script.Pop();
		}

		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct FRC_2015_Robot_Props::Autonomous_Properties &auton=m_FRC2015RobotProps.Autonomous_Props;
			{
				err = script.GetField("first_move_ft", NULL, NULL,&fTest);
				if (!err)
					auton.FirstMove_ft=fTest;

				err = script.GetField("side_move_rad", NULL, NULL,&fTest);
				if (!err)
					auton.SideMove_rad=fTest;

				err = script.GetField("arm_height_in", NULL, NULL,&fTest);
				if (!err)
					auton.ArmMove_in=fTest;

				SCRIPT_TEST_BOOL_YES(auton.IsSupportingHotSpot,"support_hotspot");
				SCRIPT_TEST_BOOL_YES(auton.ShowParameters,"show_auton_variables");
				auton.ShowAutonParameters();
			}
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
 /*															FRC_2015_Goals															*/
/***********************************************************************************************************************************/


class FRC_2015_Goals_Impl : public AtomicGoal
{
	private:
		FRC_2015_Robot &m_Robot;
		double m_Timer;

		class SetUpProps
		{
		protected:
			FRC_2015_Goals_Impl *m_Parent;
			FRC_2015_Robot &m_Robot;
			FRC_2015_Robot_Props::Autonomous_Properties m_AutonProps;
			Entity2D_Kind::EventMap &m_EventMap;
		public:
			SetUpProps(FRC_2015_Goals_Impl *Parent)	: m_Parent(Parent),m_Robot(Parent->m_Robot),m_EventMap(*m_Robot.GetEventMap())
			{	
				m_AutonProps=m_Robot.GetRobotProps().GetFRC2015RobotProps().Autonomous_Props;
			}
		};
		class goal_clock : public AtomicGoal
		{
		private:
			FRC_2015_Goals_Impl *m_Parent;
		public:
			goal_clock(FRC_2015_Goals_Impl *Parent)	: m_Parent(Parent) {	m_Status=eInactive;	}
			void Activate()  {	m_Status=eActive;	}
			Goal_Status Process(double dTime_s)
			{
				const double AutonomousTimeLimit=15.0;
				double &Timer=m_Parent->m_Timer;
				if (m_Status==eActive)
				{
					SmartDashboard::PutNumber("Timer",AutonomousTimeLimit-Timer);
					Timer+=dTime_s;
					if (Timer>=AutonomousTimeLimit)
						m_Status=eCompleted;
				}
				return m_Status;
			}
			void Terminate() {	m_Status=eFailed;	}
		};
		MultitaskGoal m_Primer;
		bool m_IsHot;
		bool m_HasSecondShotFired;

		static Goal * Move_Straight(FRC_2015_Goals_Impl *Parent,double length_ft)
		{
			FRC_2015_Robot *Robot=&Parent->m_Robot;
			//Construct a way point
			WayPoint wp;
			const Vec2d Local_GoalTarget(0.0,Feet2Meters(length_ft));
			wp.Position=Local_GoalTarget;
			wp.Power=1.0;
			//Now to setup the goal
			const bool LockOrientation=true;
			const double PrecisionTolerance=Robot->GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			Goal_Ship_MoveToPosition *goal_drive=NULL;
			goal_drive=new Goal_Ship_MoveToRelativePosition(Robot->GetController(),wp,true,LockOrientation,PrecisionTolerance);
			return goal_drive;
		}

		static Goal * Move_Sideways(FRC_2015_Goals_Impl *Parent,double length_ft)
		{
			FRC_2015_Robot *Robot=&Parent->m_Robot;
			FRC_2015_Robot::Kicker_Wheel &KickerWheel=Robot->GetKickerWheel();
			//const double PrecisionTolerance=Robot->GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_kicker=NULL;
			goal_kicker=new Goal_Ship1D_MoveToRelativePosition(KickerWheel,length_ft);
			return goal_kicker;
		}

		static Goal * Move_ArmPosition(FRC_2015_Goals_Impl *Parent,double height_in)
		{
			FRC_2015_Robot *Robot=&Parent->m_Robot;
			FRC_2015_Robot::Robot_Arm &Arm=Robot->GetArm();
			//const double PrecisionTolerance=Robot->GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			const double position=FRC_2015_Robot::Robot_Arm::HeightToAngle_r(&Arm,Inches2Meters(height_in));
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);
			return goal_arm;
		}

		class MoveForward : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			MoveForward(FRC_2015_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				AddSubgoal(new Goal_Wait(0.500));  //Testing
				AddSubgoal(Move_Straight(m_Parent,m_AutonProps.FirstMove_ft));
				m_Status=eActive;
			}
		};

		class MoveSideways : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			MoveSideways(FRC_2015_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				AddSubgoal(new Goal_Wait(0.500));  //Testing
				AddSubgoal(Move_Sideways(m_Parent,m_AutonProps.SideMove_rad));
				m_Status=eActive;
			}
		};

		class CanSteal : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			CanSteal(FRC_2015_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				const FRC_2015_Robot_Props &props=m_Robot.GetRobotProps().GetFRC2015RobotProps();
				AddSubgoal(Move_Straight(m_Parent,-m_AutonProps.FirstMove_ft));
				AddSubgoal(new Goal_Wait(0.500));  //may not be needed
				AddSubgoal(Move_ArmPosition(m_Parent,m_AutonProps.ArmMove_in));
				m_Status=eActive;
			}
		};

		class SimpleOneTote : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			SimpleOneTote(FRC_2015_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				const FRC_2015_Robot_Props &props=m_Robot.GetRobotProps().GetFRC2015RobotProps();
				AddSubgoal(Move_Straight(m_Parent,-m_AutonProps.FirstMove_ft));
				AddSubgoal(Move_ArmPosition(m_Parent,props.Tote2Height));
				AddSubgoal(new Goal_Wait(0.500));  //may not be needed
				AddSubgoal(Move_Straight(m_Parent,2.0)); //TODO make property
				AddSubgoal(Move_ArmPosition(m_Parent,props.ToteRestHeight));
				m_Status=eActive;
			}
		};

		enum AutonType
		{
			eDoNothing,
			eJustMoveSideways,
			eJustMoveForward,
			eCanSteal,
			eSimpleOneTote,
			eNoAutonTypes
		} m_AutonType;
		enum Robot_Position
		{
			ePosition_Center,
			ePosition_Left,
			ePosition_Right
		} m_RobotPosition;
	public:
		FRC_2015_Goals_Impl(FRC_2015_Robot &robot) : m_Robot(robot), m_Timer(0.0), 
			m_Primer(false),  //who ever is done first on this will complete the goals (i.e. if time runs out)
			m_IsHot(false),m_HasSecondShotFired(false)
		{
			m_Status=eInactive;
		}
		void Activate() 
		{
			m_Primer.AsGoal().Terminate();  //sanity check clear previous session

			//pull parameters from SmartDashboard
			try
			{
				const double fGoalSelection=SmartDashboard::GetNumber("Auton GoalSelection");
				int GoalSelection=(size_t)fGoalSelection;
				if ((GoalSelection<0)||(GoalSelection>eNoAutonTypes))
					GoalSelection=eDoNothing;
				m_AutonType=(AutonType)GoalSelection;
			}
			catch (...)
			{
				m_AutonType=eDoNothing;
				SmartDashboard::PutNumber("Auton GoalSelection",0.0);
			}

			try
			{
				const double fPosition=SmartDashboard::GetNumber("Auton Position");
				int Position=(size_t)fPosition;
				if ((Position<0)||(Position>eNoAutonTypes))
					Position=eDoNothing;
				m_RobotPosition=(Robot_Position)Position;
			}
			catch (...)
			{
				m_RobotPosition=ePosition_Center;
				SmartDashboard::PutNumber("Auton Position",0.0);
			}

			FRC_2015_Robot_Props::Autonomous_Properties &auton=m_Robot.GetAutonProps();
			auton.ShowAutonParameters();  //Grab again now in case user has tweaked values

			printf("ball count=%d position=%d\n",m_AutonType,m_RobotPosition);
			switch(m_AutonType)
			{
			case eJustMoveSideways:
				m_Primer.AddGoal(new MoveSideways(this));
				break;
			case eJustMoveForward:
				m_Primer.AddGoal(new MoveForward(this));
				break;
			case eCanSteal:
				m_Primer.AddGoal(new CanSteal(this));
				break;
			case  eSimpleOneTote:
				m_Primer.AddGoal(new SimpleOneTote(this));
				break;
			case eDoNothing:
			case eNoAutonTypes: //grrr windriver and warning 1250
				break;
			}
			m_Primer.AddGoal(new goal_clock(this));
			m_Status=eActive;
		}

		Goal_Status Process(double dTime_s)
		{
			ActivateIfInactive();
			if (m_Status==eActive)
				m_Status=m_Primer.AsGoal().Process(dTime_s);
			return m_Status;
		}
		void Terminate() 
		{
			m_Primer.AsGoal().Terminate();
			m_Status=eFailed;
		}
};

Goal *FRC_2015_Goals::Get_FRC2015_Autonomous(FRC_2015_Robot *Robot)
{
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),(char *)"Complete");
	SmartDashboard::PutNumber("Sequence",1.0);  //ensure we are on the right sequence
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(new FRC_2015_Goals_Impl(*Robot));
	//MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

  /***********************************************************************************************************************************/
 /*													FRC_2015_Robot_Control															*/
/***********************************************************************************************************************************/



void FRC_2015_Robot_Control::ResetPos()
{
	//Enable this code if we have a compressor 
	m_Compressor->Stop();
	printf("RobotControl::ResetPos Compressor->Stop()\n");
	#ifndef Robot_TesterCode
	//Allow driver station to control if they want to run the compressor
	//if (DriverStation::GetInstance()->GetDigitalIn(8))
	#endif
	{
		printf("RobotControl::ResetPos Compressor->Start()\n");
		m_Compressor->Start();
	}
	//Set the solenoids to their default positions
	OpenSolenoid(FRC_2015_Robot::eUseLowGear,true);
}

void FRC_2015_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	switch (index)
	{
	case FRC_2015_Robot::eKickerWheel:
		Victor_UpdateVoltage(index,Voltage);
		//TODO change to KickerWheel once I have a 2015 layout in SmartDashboard
		//  [1/24/2015 JamesK]
		SmartDashboard::PutNumber("RollerVoltage",Voltage);
		break;
	case FRC_2015_Robot::eArm:
		Voltage=Voltage * m_RobotProps.GetArmProps().GetRotaryProps().VoltageScalar;
		Victor_UpdateVoltage(index,Voltage);
		SmartDashboard::PutNumber("ArmVoltage",Voltage);
		#ifdef Robot_TesterCode
		m_Potentiometer.UpdatePotentiometerVoltage(Voltage);
		m_Potentiometer.TimeChange();  //have this velocity immediately take effect
		#endif
		break;
	case FRC_2015_Robot::eCameraLED:
		TranslateToRelay(index,Voltage);
		//I don't need this since we have another variable that represents it, but enable for diagnostics
		//SmartDashboard::PutBoolean("CameraLED",Voltage==0.0?false:true);
		break;
	}
}

bool FRC_2015_Robot_Control::GetBoolSensorState(size_t index) const
{
	bool ret;
	switch (index)
	{
	case FRC_2015_Robot::eDartUpper:
		ret=m_Limit_DartUpper;
		break;
	case FRC_2015_Robot::eDartLower:
		ret=m_Limit_DartLower;
		break;
	default:
		assert (false);
	}
	return ret;
}

FRC_2015_Robot_Control::FRC_2015_Robot_Control(bool UseSafety) : m_TankRobotControl(UseSafety),m_pTankRobotControl(&m_TankRobotControl),
		m_Compressor(NULL),m_RoboRIO_Accelerometer(NULL)
{
}

FRC_2015_Robot_Control::~FRC_2015_Robot_Control()
{
	//Encoder_Stop(FRC_2015_Robot::eWinch);
	DestroyCompressor(m_Compressor);
	m_Compressor=NULL;
	DestroyBuiltInAccelerometer(m_RoboRIO_Accelerometer);
	m_RoboRIO_Accelerometer=NULL;
}

void FRC_2015_Robot_Control::Reset_Rotary(size_t index)
{
	Encoder_Reset(index);  //This will check for encoder existence implicitly

	switch (index)
	{
	case FRC_2015_Robot::eArm:
		m_KalFilter_Arm.Reset();
		#ifdef Robot_TesterCode
		m_Potentiometer.ResetPos();
		#endif
		break;
	}
}

#ifdef Robot_TesterCode
void FRC_2015_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
}
#endif

void FRC_2015_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2015_Robot_Properties *robot_props=dynamic_cast<const FRC_2015_Robot_Properties *>(props);
	//TODO this is to be changed to an assert once we handle low gear properly
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		Rotary_Properties turret_props=robot_props->GetTurretProps();
		turret_props.SetUsingRange(false); //TODO why is this here?	

		#ifdef Robot_TesterCode
		Rotary_Properties writeable_arm_props=robot_props->GetArmProps();
		//m_ArmMaxSpeed=writeable_arm_props.GetMaxSpeed();
		//This is not perfect but will work for our simulation purposes
		writeable_arm_props.RotaryProps().EncoderToRS_Ratio=robot_props->GetFRC2015RobotProps().ArmToGearRatio;
		m_Potentiometer.Initialize(&writeable_arm_props);
		#endif
	}
	
	//Note: Initialize may be called multiple times so we'll only set this stuff up on first run
	//For now... we'll use m_Compressor as the variable to determine first run, but perhaps this should be its own boolean here
	if (!m_Compressor)
	{
		//This one one must also be called for the lists that are specific to the robot
		RobotControlCommon_Initialize(robot_props->Get_ControlAssignmentProps());
		//This may return NULL for systems that do not support it
		m_RoboRIO_Accelerometer=CreateBuiltInAccelerometer();
		m_Compressor=CreateCompressor();
		//Note: RobotControlCommon_Initialize() must occur before calling any encoder startup code
		//const double EncoderPulseRate=(1.0/360.0);
		//Encoder_SetDistancePerPulse(FRC_2015_Robot::eWinch,EncoderPulseRate);
		//Encoder_Start(FRC_2015_Robot::eWinch);
		ResetPos(); //must be called after compressor is created
		SmartDashboard::PutNumber("Arm_Raw_high",4013.0);
		SmartDashboard::PutNumber("Arm_Raw_Range",24.0);
	}

}

void FRC_2015_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Limit_DartUpper=BoolSensor_GetState(FRC_2015_Robot::eDartUpper);
	SmartDashboard::PutBoolean("LimitDartUpper",m_Limit_DartUpper);
	m_Limit_DartLower=BoolSensor_GetState(FRC_2015_Robot::eDartLower);
	SmartDashboard::PutBoolean("LimitDartLower",m_Limit_DartLower);

	#ifdef Robot_TesterCode
	m_Potentiometer.SetTimeDelta(dTime_s);
	#endif

	//Testing the accelerometer
	#if 1
	if (m_RoboRIO_Accelerometer)
	{
		SmartDashboard::PutNumber("RoboAccelX", m_RoboRIO_Accelerometer->GetX());
		SmartDashboard::PutNumber("RoboAccelY", m_RoboRIO_Accelerometer->GetY());
		SmartDashboard::PutNumber("RoboAccelZ", m_RoboRIO_Accelerometer->GetZ());
	}
	#endif
}

void FRC_2015_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) 
{
	#ifdef __USING_6CIMS__
	const Tank_Robot_Props &TankRobotProps=m_RobotProps.GetTankRobotProps();
	if (!TankRobotProps.ReverseSteering)
	{
		Victor_UpdateVoltage(FRC_2015_Robot::eLeftDrive3,(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
		Victor_UpdateVoltage(FRC_2015_Robot::eRightDrive3,-(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
	}
	else
	{
		Victor_UpdateVoltage(FRC_2015_Robot::eLeftDrive3,(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
		Victor_UpdateVoltage(FRC_2015_Robot::eRightDrive3,-(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
	}
	#endif
	m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}

double FRC_2015_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;
	const FRC_2015_Robot_Props &props=m_RobotProps.GetFRC2015RobotProps();

	switch (index)
	{
		case FRC_2015_Robot::eArmPotentiometer:
		{
			#ifndef Robot_TesterCode
			//double raw_value = (double)m_Potentiometer.GetAverageValue();
			double raw_value=(double)Analog_GetAverageValue(FRC_2015_Robot::eArmPotentiometer);
			raw_value = m_KalFilter_Arm(raw_value);  //apply the Kalman filter
			raw_value=m_ArmAverager.GetAverage(raw_value); //and Ricks x element averager
			//Note: we keep the raw value in its native form... just averaging at most for less noise

			double PotentiometerRaw_To_Arm;

			//TODO may wish to script this
			#if 0
			const double lowRange=155;
			const double HiRange=1125;
			#else
			const double HiRange=SmartDashboard::GetNumber("Arm_Raw_high");
			const double LowRange=HiRange-SmartDashboard::GetNumber("Arm_Raw_Range",24.0);
			//If this is true, the value is inverted with the negative operator
			const bool FlipRange=true;
			#endif

			PotentiometerRaw_To_Arm = raw_value-LowRange;//zeros the potentiometer
			PotentiometerRaw_To_Arm = PotentiometerRaw_To_Arm/(HiRange-LowRange);//scales values from 0 to 1 with +- .001

			//Clip Range
			//I imagine .001 corrections will not be harmful for when in use.
			if (PotentiometerRaw_To_Arm < 0) PotentiometerRaw_To_Arm = 0;//corrects .001 or less causing a negative value
			if (PotentiometerRaw_To_Arm > 1 || PotentiometerRaw_To_Arm > .999) PotentiometerRaw_To_Arm = 1;//corrects .001 or lass causing value greater than 1

			//TODO fix in lua
			//PotentiometerRaw_To_Arm*=props.PotentiometerToArmRatio;  //convert to arm's gear ratio
			if (FlipRange)
				PotentiometerRaw_To_Arm=1.0-PotentiometerRaw_To_Arm;

			SmartDashboard::PutNumber("Arm_Raw",raw_value);
			SmartDashboard::PutNumber("Arm_PotRaw",PotentiometerRaw_To_Arm);

			//Now to compute the result... we start with the normalized value and give it the apprioriate offset and scale
			//the offset is delegated in script in the final scale units, and the scale is the total range in radians
			result=PotentiometerRaw_To_Arm;
			//get scale
			const Ship_1D_Props &shipprops=m_RobotProps.GetArmProps().GetShip_1D_Props();
			//SmartDashboard::PutNumber("Arm_ScaleTest",shipprops.MaxRange-shipprops.MinRange);
			result*=shipprops.MaxRange-shipprops.MinRange;  //compute the total distance in radians
			//get offset... Note: scale comes first since the offset is of that scale
			result+=m_RobotProps.GetArmProps().GetRotaryProps().PotentiometerOffset;
			#else
			result=(m_Potentiometer.GetPotentiometerCurrentPosition()) + 0.0;
			#endif

			SmartDashboard::PutNumber("ArmAngle",RAD_2_DEG(result));
			const double height= (sin(result)*props.ArmLength)+props.GearHeightOffset;
			SmartDashboard::PutNumber("Height",height*3.2808399);
			//I'm keeping this disabled unless we decide to use vision with the target reticle
			#if 0
			{
				//Now to have some camera hud aligning updates in vision
				//Note: I usually work with radians and meters... but in this small example I've
				//chosen to work in degrees and inches for easier readability

				const double StandAdjustedAngle=-7.2;  //in degrees
				const double pivot_radius_in=7.3;  //distance from pivot point to camera lens
				const double pivot_offset=-18.80; //This takes into account the offset of the camera mounted position
				const double Camera_Z_offset=-32.45; //distance from lens to front of robot
				const double Camera_Y_offset=49.77 - pivot_radius_in;

				const double Arm_Length_in=17.0;
				//pitch is the sensed angle use this to determine other geometry (it's the ArmAngle)
				const double pitch=RAD_2_DEG(result);
				const double Tote_stack_Height=11.75;

				const double stand_angle=pitch+StandAdjustedAngle;  //determine angle of stand  
				const double height_in=Meters2Inches(height);
				SmartDashboard::PutNumber("Camera_rot_y",stand_angle);
				SmartDashboard::PutNumber("Camera_y",cos(DEG_2_RAD(pitch+pivot_offset))*pivot_radius_in+Camera_Y_offset);
				SmartDashboard::PutNumber("Camera_z",sin(DEG_2_RAD(-(pitch+pivot_offset)))*pivot_radius_in+Camera_Z_offset);
				SmartDashboard::PutNumber("height_indicator_y",height_in);
				SmartDashboard::PutNumber("fork_right_y",height_in);
				SmartDashboard::PutNumber("fork_left_y",height_in);
				SmartDashboard::PutNumber("inner_fork_right_y",height_in);
				SmartDashboard::PutNumber("inner_fork_left_y",height_in);
				SmartDashboard::PutBoolean("tote_2_enabled",height_in>Tote_stack_Height);
				SmartDashboard::PutBoolean("tote_3_enabled",height_in>Tote_stack_Height*2);
				SmartDashboard::PutBoolean("tote_4_enabled",height_in>Tote_stack_Height*3);
				SmartDashboard::PutBoolean("tote_5_enabled",height_in>Tote_stack_Height*4);
				SmartDashboard::PutBoolean("tote_6_enabled",height_in>Tote_stack_Height*5);
			}
			#endif
			//Now to convert to the motor gear ratio as this is what we work in
			result*=props.ArmToGearRatio;
		}
	}
	return result;
}

void FRC_2015_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2015_Robot::eUseLowGear:
		SmartDashboard::PutBoolean("UseHighGear",!Open);
		Solenoid_Open(index,Open);
		break;
	case FRC_2015_Robot::eForkLeft:
		SmartDashboard::PutBoolean("ForkLeft",!Open);
		Solenoid_Open(index,Open);
		break;
	case FRC_2015_Robot::eForkRight:
		SmartDashboard::PutBoolean("ForkRight",!Open);
		Solenoid_Open(index,Open);
		break;
	}
}


#ifdef Robot_TesterCode
  /***************************************************************************************************************/
 /*												FRC_2015_Robot_UI												*/
/***************************************************************************************************************/

FRC_2015_Robot_UI::FRC_2015_Robot_UI(const char EntityName[]) : FRC_2015_Robot(EntityName,this),FRC_2015_Robot_Control(),
		m_TankUI(this)
{
}

void FRC_2015_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
}
void FRC_2015_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
}

void FRC_2015_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
}
void FRC_2015_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
}
void FRC_2015_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
}
void FRC_2015_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
}

#endif
