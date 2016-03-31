#include "stdafx.h"
#include "Robot_Tester.h"

#ifdef Robot_TesterCode
namespace Robot_Tester
{
	#include "Tank_Robot_UI.h"
	#include "CommonUI.h"
	#include "Curivator_Robot.h"
}

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

#else

#include "Curivator_Robot.h"
#include "SmartDashboard/SmartDashboard.h"
using namespace Framework::Base;
using namespace std;
#endif

#define __DisableEncoderTracking__
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
 /*													Curivator_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

Curivator_Robot::Robot_Arm::Robot_Arm(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control) : 
Rotary_Position_Control(csz_Curivator_Robot_SpeedControllerDevices_Enum[index],robot_control,index),m_Index(index),m_pParent(parent),m_Advance(false),m_Retract(false)
{
}


void Curivator_Robot::Robot_Arm::Advance(bool on)
{
	m_Advance=on;
}
void Curivator_Robot::Robot_Arm::Retract(bool on)
{
	m_Retract=on;
}

bool Curivator_Robot::Robot_Arm::DidHitMinLimit() const
{
	return false;  //no limit switches used
}

bool Curivator_Robot::Robot_Arm::DidHitMaxLimit() const
{
	return false;    //no limit switches used
}

void Curivator_Robot::Robot_Arm::TimeChange(double dTime_s)
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
//we'll just use smart dashboard since we have multiple instances here
//#ifdef Robot_TesterCode
//	const Curivator_Robot_Props &props=m_pParent->GetRobotProps().GetCurivatorRobotProps();
//	const double c_GearToArmRatio=1.0/props.ArmToGearRatio;
//	double Pos_m=GetPos_m();
//	double height=AngleToHeight_m(Pos_m);
//	DOUT4("Arm=%f Angle=%f %fft %fin",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
//#endif
}


//double Curivator_Robot::Robot_Arm::AngleToHeight_m(double Angle_r) const
//{
//	const Curivator_Robot_Props &props=m_pParent->GetRobotProps().GetCurivatorRobotProps();
//	const double c_GearToArmRatio=1.0/props.ArmToGearRatio;
//
//	return (sin(Angle_r*c_GearToArmRatio)*props.ArmLength)+props.GearHeightOffset;
//}
//double Curivator_Robot::Robot_Arm::Arm_AngleToHeight_m(double Angle_r) const
//{
//	const Curivator_Robot_Props &props=m_pParent->GetRobotProps().GetCurivatorRobotProps();
//	return (sin(Angle_r)*props.ArmLength)+props.GearHeightOffset;
//}
//
//double Curivator_Robot::Robot_Arm::HeightToAngle_r(double Height_m) const
//{
//	const Curivator_Robot_Props &props=m_pParent->GetRobotProps().GetCurivatorRobotProps();
//	return asin((Height_m-props.GearHeightOffset)/props.ArmLength) * props.ArmToGearRatio;
//}
//
//double Curivator_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(double raw) const
//{
//	const Curivator_Robot_Props &props=m_pParent->GetRobotProps().GetCurivatorRobotProps();
//	const int RawRangeHalf=512;
//	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
//	ret*=props.PotentiometerToArmRatio;  //convert to arm's gear ratio
//	return ret;
//}

void Curivator_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[m_Index];
	string ContructedName;
	if (Bind)
	{
		ContructedName=Prefix,ContructedName+="_SetCurrentVelocity";
		em->EventValue_Map[ContructedName.c_str()].Subscribe(ehl,*this, &Curivator_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		ContructedName=Prefix,ContructedName+="_SetPotentiometerSafety";
		em->EventOnOff_Map[ContructedName.c_str()].Subscribe(ehl,*this, &Curivator_Robot::Robot_Arm::SetPotentiometerSafety);

		ContructedName=Prefix,ContructedName+="_Advance";
		em->EventOnOff_Map[ContructedName.c_str()].Subscribe(ehl,*this, &Curivator_Robot::Robot_Arm::Advance);
		ContructedName=Prefix,ContructedName+="_Retract";
		em->EventOnOff_Map[ContructedName.c_str()].Subscribe(ehl,*this, &Curivator_Robot::Robot_Arm::Retract);
	}
	else
	{
		ContructedName=Prefix,ContructedName+="_SetCurrentVelocity";
		em->EventValue_Map[ContructedName.c_str()].Remove(*this, &Curivator_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		ContructedName=Prefix,ContructedName+="_SetPotentiometerSafety";
		em->EventOnOff_Map[ContructedName.c_str()].Remove(*this, &Curivator_Robot::Robot_Arm::SetPotentiometerSafety);

		ContructedName=Prefix,ContructedName+="_Advance";
		em->EventOnOff_Map[ContructedName.c_str()].Remove(*this, &Curivator_Robot::Robot_Arm::Advance);
		ContructedName=Prefix,ContructedName+="_Retract";
		em->EventOnOff_Map[ContructedName.c_str()].Remove(*this, &Curivator_Robot::Robot_Arm::Retract);
	}
}

  /***********************************************************************************************************************************/
 /*														Curivator_Robot::BigArm														*/
/***********************************************************************************************************************************/
//Note: all of these constants are in inches (as they are in the CAD)
const double BigArm_BigArmRadius=39.77287247;
const double BigArm_DartLengthAt90=29.72864858;
const double BigArm_DartToArmDistance=9.0;
const double BigArm_DistanceFromTipDartToClevis=2.0915;
const double BigArm_DistanceDartPivotToTip=17.225;
const double BigArm_ConnectionOffset=5.39557923;
const double BigArm_DistanceBigArmPivottoDartPivot=26.01076402;
//or use this
const double BigArm_AngleToDartPivotInterface=DEG_2_RAD(11.7190273);
const double BigArm_AngleToDartPivotInterface_Length=26.56448983;
const double BigArm_DartPerpendicularAngle= 61.03779676;


Curivator_Robot::BigArm::BigArm(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control) : Robot_Arm(index,parent,robot_control)
{
}
void Curivator_Robot::BigArm::TimeChange(double dTime_s)
{
	__super::TimeChange(dTime_s);
	//Now to compute where we are based from our length of extension
	//first start with the extension:
	const double ShaftExtension_in=GetPos_m();  //expecting a value from 0-12 in inches
	const double FullActuatorLength=ShaftExtension_in+BigArm_DistanceDartPivotToTip+BigArm_DistanceFromTipDartToClevis;  //from center point to center point
	//Now that we know all three lengths to the triangle use law of cosines to solve the angle of the linear actuator
	//http://mathcentral.uregina.ca/QQ/database/QQ.09.07/h/lucy1.html
	//c2 = a2 + b2 - 2ab cos(C)
	//c is FullActuatorLength
	//b is dart distance to arm
	//a is the AngleToDartPivotInterface_Length
	//rearranged to solve for cos(C)
	//x = -1 * ( (c*c - b*b - a*a) / (2*a*b)    )
	const double cos_FullActuatorLength=-1.0 *
		(((FullActuatorLength*FullActuatorLength)-
		(BigArm_DartToArmDistance*BigArm_DartToArmDistance)-
		(BigArm_AngleToDartPivotInterface_Length*BigArm_AngleToDartPivotInterface_Length))  / 
		(2 * BigArm_AngleToDartPivotInterface_Length * BigArm_DartToArmDistance));
	const double BigAngleDartInterface=acos(cos_FullActuatorLength);
	//SmartDashboard::PutNumber("BigAngleDartInterface",RAD_2_DEG(BigAngleDartInterface));
	m_BigArmAngle=BigAngleDartInterface-BigArm_AngleToDartPivotInterface;
	//SmartDashboard::PutNumber("BigAngleAngle",RAD_2_DEG(m_BigArmAngle));
	//With this angle we can pull sin and cos for height and outward length using the big arm's radius constant
}

double Curivator_Robot::BigArm::GetBigArmLength() const
{
	const double BigArmLength=cos(m_BigArmAngle) * BigArm_BigArmRadius;
	//SmartDashboard::PutNumber("BigArmLength",BigArmLength);
	return BigArmLength;
}
double Curivator_Robot::BigArm::GetBigArmHeight() const
{
	const double BigArmHeight=sin(m_BigArmAngle) * BigArm_BigArmRadius;
	//SmartDashboard::PutNumber("BigArmHeight",BigArmHeight);
	return BigArmHeight;
}


  /***********************************************************************************************************************************/
 /*														Curivator_Robot::Boom														*/
/***********************************************************************************************************************************/
//Note: all of these constants are in inches (as they are in the CAD)
const double Boom_BoomRadius=26.03003069;
const double Boom_DartToArmDistance=18.51956156;
const double Boom_DistanceFromTipDartToClevis=2.0915;  //Note: these may be different depending on how many turns it took to orient properly
const double Boom_DistanceDartPivotToTip=11.5;
//const double Boom_ConnectionOffset=5.39557923;
//const double Boom_DistanceBoomPivottoDartPivot=26.01076402;
//or use this
const double Boom_AngleToDartPivotInterface=DEG_2_RAD(4.83505068);
const double Boom_AngleToDartPivotInterface_Length=6.87954395;
const double Boom_AngleBigArmToDartPivot= DEG_2_RAD(36.18122057);


Curivator_Robot::Boom::Boom(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control, BigArm &bigarm) : Robot_Arm(index,parent,robot_control),
	m_BigArm(bigarm)
{
}
void Curivator_Robot::Boom::TimeChange(double dTime_s)
{
	__super::TimeChange(dTime_s);
	//Now to compute where we are based from our length of extension
	//first start with the extension:
	const double ShaftExtension_in=GetPos_m();  //expecting a value from 0-12 in inches
	const double FullActuatorLength=ShaftExtension_in+Boom_DistanceDartPivotToTip+Boom_DistanceFromTipDartToClevis;  //from center point to center point
	//Now that we know all three lengths to the triangle use law of cosines to solve the angle of the linear actuator
	//http://mathcentral.uregina.ca/QQ/database/QQ.09.07/h/lucy1.html
	//c2 = a2 + b2 - 2ab cos(C)
	//c is FullActuatorLength
	//b is dart distance to arm
	//a is the AngleToDartPivotInterface_Length
	//rearranged to solve for cos(C)
	//x = -1 * ( (c*c - b*b - a*a) / (2*a*b)    )
	const double cos_FullActuatorLength=-1.0 *
		(((FullActuatorLength*FullActuatorLength)-
		(Boom_DartToArmDistance*Boom_DartToArmDistance)-
		(Boom_AngleToDartPivotInterface_Length*Boom_AngleToDartPivotInterface_Length))  / 
		(2 * Boom_AngleToDartPivotInterface_Length * Boom_DartToArmDistance));
	const double BigAngleDartInterface=acos(cos_FullActuatorLength);
	//SmartDashboard::PutNumber("BoomDartInterface",RAD_2_DEG(BigAngleDartInterface));
	const double local_BoomAngle=M_PI-BigAngleDartInterface+Boom_AngleToDartPivotInterface;
	//To convert to global we subtract the sum of both the boom dart to bigarm constant angle and the angle of the big arm... this angle is global from 
	//a vertical line that aligns with the big arm's pivot point for the boom
	m_BoomAngle=local_BoomAngle-((PI_2-m_BigArm.GetBigArmAngle())+Boom_AngleBigArmToDartPivot);
	SmartDashboard::PutNumber("BoomAngle",RAD_2_DEG(m_BoomAngle));
	//With this angle we can pull sin and cos for height and outward length using the big arm's radius constant
	GetBoomLength();
	GetBoomHeight();
}

double Curivator_Robot::Boom::GetBoomLength() const
{
	const double LocalBoomLength=sin(m_BoomAngle) * Boom_BoomRadius;
	const double BoomLength=LocalBoomLength+m_BigArm.GetBigArmLength();
	SmartDashboard::PutNumber("BoomLength",BoomLength);
	return BoomLength;
}
double Curivator_Robot::Boom::GetBoomHeight() const
{
	const double LocalBoomHeight=cos(m_BoomAngle) * Boom_BoomRadius;
	const double BoomHeight=m_BigArm.GetBigArmHeight()-LocalBoomHeight;
	SmartDashboard::PutNumber("BoomHeight",BoomHeight);
	return BoomHeight;
}


  /***********************************************************************************************************************************/
 /*															Curivator_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;

Curivator_Robot::Curivator_Robot(const char EntityName[],Curivator_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), 
		m_Turret(eTurret,this,robot_control),m_Arm(eArm,this,robot_control),m_LatencyCounter(0.0),
		m_Boom(eBoom,this,robot_control,m_Arm),m_Bucket(eBucket,this,robot_control),m_Clasp(eClasp,this,robot_control),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_AutonPresetIndex(0)
{
	mp_Arm[eTurret]=&m_Turret;
	mp_Arm[eArm]=&m_Arm;
	mp_Arm[eBoom]=&m_Boom;
	mp_Arm[eBucket]=&m_Bucket;
	mp_Arm[eClasp]=&m_Clasp;
	//ensure the variables are initialized before calling get
	SmartDashboard::PutNumber("X Position",0.0);
	SmartDashboard::PutNumber("Y Position",0.0);
	SmartDashboard::PutBoolean("Main_Is_Targeting",false);
	//Note: The processing vision is setup to use these same variables for both tracking processes (i.e. front and rear camera) we should only need to be tracking one of them at a time
	//We may want to add a prefix window to identify which window they are coming from, but this may not be necessary.
}

void Curivator_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const Curivator_Robot_Properties *RobotProps=dynamic_cast<const Curivator_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)

	for (size_t i=0;i<5;i++)
		mp_Arm[i]->Initialize(em,RobotProps?&RobotProps->GetRotaryProps(i):NULL);
}
void Curivator_Robot::ResetPos()
{
	__super::ResetPos();
	for (size_t i=0;i<5;i++)
		mp_Arm[i]->ResetPos();
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

void Curivator_Robot::TimeChange(double dTime_s)
{
	//const Curivator_Robot_Props &robot_props=m_RobotProps.GetCurivatorRobotProps();

	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);

	for (size_t i=0;i<5;i++)
		mp_Arm[i]->AsEntity1D().TimeChange(dTime_s);

	//const double  YOffset=-SmartDashboard::GetNumber("Y Position");
	//const double XOffset=SmartDashboard::GetNumber("X Position");
}

const Curivator_Robot_Properties &Curivator_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

Curivator_Robot_Props::Autonomous_Properties &Curivator_Robot::GetAutonProps()
{
	return m_RobotProps.GetCurivatorRobotProps_rw().Autonomous_Props;
}


void Curivator_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"].Subscribe(ehl, *this, &Curivator_Robot::TestAutonomous);
		em->Event_Map["Complete"].Subscribe(ehl,*this,&Curivator_Robot::GoalComplete);
		#endif
	}
	else
	{
		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"]  .Remove(*this, &Curivator_Robot::TestAutonomous);
		em->Event_Map["Complete"]  .Remove(*this, &Curivator_Robot::GoalComplete);
		#endif
	}

	for (size_t i=0;i<5;i++)
		mp_Arm[i]->AsShip1D().BindAdditionalEventControls(Bind);

	#ifdef Robot_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
	__super::BindAdditionalEventControls(Bind);
}

void Curivator_Robot::BindAdditionalUIControls(bool Bind,void *joy, void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}

void Curivator_Robot::UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s)
{
	//Call predecessor (e.g. tank steering) to get some preliminary values
	__super::UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,LockShipHeadingToOrientation,dTime_s);
}

#ifdef Robot_TesterCode
void Curivator_Robot::TestAutonomous()
{
	Goal *oldgoal=ClearGoal();
	if (oldgoal)
		delete oldgoal;

	{
		Goal *goal=NULL;
		goal=Curivator_Goals::Get_Curivator_Autonomous(this);
		if (goal)
			goal->Activate(); //now with the goal(s) loaded activate it
		SetGoal(goal);
		//enable autopilot (note wind river does this in main)
		m_controller->GetUIController_RW()->SetAutoPilot(true);
	}
}

void Curivator_Robot::GoalComplete()
{
	printf("Goals completed!\n");
	m_controller->GetUIController_RW()->SetAutoPilot(false);
}
#endif

  /***********************************************************************************************************************************/
 /*													Curivator_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

Curivator_Robot_Properties::Curivator_Robot_Properties()  : m_RobotControls(&s_ControlsEvents)
{
	{
		//const double c_ArmToGearRatio=72.0/28.0;
		//const double c_PotentiometerToArmRatio=36.0/54.0;

		Curivator_Robot_Props props;

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

			m_CurivatorRobotProps=props;
		}

		Curivator_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		m_CurivatorRobotProps=props;
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
		Rotary_Props props=m_RotaryProps[Curivator_Robot::eTurret].RotaryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_RotaryProps[Curivator_Robot::eTurret].RotaryProps()=props;
	}
}

const char *ProcessVec2D(Curivator_Robot_Props &m_CurivatorRobotProps,Scripting::Script& script,Vec2d &Dest)
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
const char * const g_Curivator_Controls_Events[] = 
{
	"turret_SetCurrentVelocity","turret_SetIntendedPosition","turret_SetPotentiometerSafety","turret_Advance","turret_Retract",
	"IntakeArm_DeployManager",
	"arm_SetCurrentVelocity","arm_SetPotentiometerSafety","arm_Advance","arm_Retract",
	"boom_SetCurrentVelocity","boom_SetPotentiometerSafety","boom_Advance","boom_Retract",
	"bucket_SetCurrentVelocity","bucket_SetPotentiometerSafety","bucket_Advance","bucket_Retract",
	"clasp_SetCurrentVelocity","clasp_SetPotentiometerSafety","clasp_Advance","clasp_Retract",
	"TestAuton"
};

const char *Curivator_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_Curivator_Controls_Events))?g_Curivator_Controls_Events[index] : NULL;
}
Curivator_Robot_Properties::ControlEvents Curivator_Robot_Properties::s_ControlsEvents;

//enable when we are ready to use auton parameters
#if 0
void Curivator_Robot_Props::Autonomous_Properties::ShowAutonParameters()
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
#endif

void Curivator_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	Curivator_Robot_Props &props=m_CurivatorRobotProps;

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
	//double fTest;
	std::string sTest;
	if (!err) 
	{
		err = script.GetFieldTable("turret");
		if (!err)
		{
			m_RotaryProps[Curivator_Robot::eTurret].LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("arm");
		if (!err)
		{
			m_RotaryProps[Curivator_Robot::eArm].LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("boom");
		if (!err)
		{
			m_RotaryProps[Curivator_Robot::eBoom].LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("bucket");
		if (!err)
		{
			m_RotaryProps[Curivator_Robot::eBucket].LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("clasp");
		if (!err)
		{
			m_RotaryProps[Curivator_Robot::eClasp].LoadFromScript(script);
			script.Pop();
		}

		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct Curivator_Robot_Props::Autonomous_Properties &auton=m_CurivatorRobotProps.Autonomous_Props;
			{
				//err = script.GetField("first_move_ft", NULL, NULL,&fTest);
				//if (!err)
				//	auton.FirstMove_ft=fTest;

				//err = script.GetField("side_move_rad", NULL, NULL,&fTest);
				//if (!err)
				//	auton.SideMove_rad=fTest;

				//err = script.GetField("arm_height_in", NULL, NULL,&fTest);
				//if (!err)
				//	auton.ArmMove_in=fTest;

				//SCRIPT_TEST_BOOL_YES(auton.IsSupportingHotSpot,"support_hotspot");
				//SCRIPT_TEST_BOOL_YES(auton.ShowParameters,"show_auton_variables");
				//auton.ShowAutonParameters();
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
 /*															Curivator_Goals															*/
/***********************************************************************************************************************************/


class Curivator_Goals_Impl : public AtomicGoal
{
	private:
		Curivator_Robot &m_Robot;
		double m_Timer;

		class SetUpProps
		{
		protected:
			Curivator_Goals_Impl *m_Parent;
			Curivator_Robot &m_Robot;
			Curivator_Robot_Props::Autonomous_Properties m_AutonProps;
			Entity2D_Kind::EventMap &m_EventMap;
		public:
			SetUpProps(Curivator_Goals_Impl *Parent)	: m_Parent(Parent),m_Robot(Parent->m_Robot),m_EventMap(*m_Robot.GetEventMap())
			{	
				m_AutonProps=m_Robot.GetRobotProps().GetCurivatorRobotProps().Autonomous_Props;
			}
		};
		class goal_clock : public AtomicGoal
		{
		private:
			Curivator_Goals_Impl *m_Parent;
		public:
			goal_clock(Curivator_Goals_Impl *Parent)	: m_Parent(Parent) {	m_Status=eInactive;	}
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

		static Goal * Move_Straight(Curivator_Goals_Impl *Parent,double length_ft)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
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

		static Goal * Move_ArmPosition(Curivator_Goals_Impl *Parent,double height_in)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			Curivator_Robot::Robot_Arm &Arm=Robot->GetArm();
			//const double PrecisionTolerance=Robot->GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			//const double position=Curivator_Robot::Robot_Arm::HeightToAngle_r(&Arm,Inches2Meters(height_in));
			const double position=0;
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);
			return goal_arm;
		}

		class MoveForward : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			MoveForward(Curivator_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				AddSubgoal(new Goal_Wait(0.500));  //Testing
				AddSubgoal(Move_Straight(m_Parent,1.0));
				m_Status=eActive;
			}
		};

		enum AutonType
		{
			eDoNothing,
			eJustMoveForward,
			eNoAutonTypes
		} m_AutonType;
	public:
		Curivator_Goals_Impl(Curivator_Robot &robot) : m_Robot(robot), m_Timer(0.0), 
			m_Primer(false)  //who ever is done first on this will complete the goals (i.e. if time runs out)
		{
			m_Status=eInactive;
		}
		void Activate() 
		{
			m_Primer.AsGoal().Terminate();  //sanity check clear previous session

			//pull parameters from SmartDashboard
			Curivator_Robot_Props::Autonomous_Properties &auton=m_Robot.GetAutonProps();
			//auton.ShowAutonParameters();  //Grab again now in case user has tweaked values

			m_AutonType = eDoNothing;  //TODO ... do something.  :)
			printf("ball count=%d \n",m_AutonType);
			switch(m_AutonType)
			{
			case eJustMoveForward:
				m_Primer.AddGoal(new MoveForward(this));
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

Goal *Curivator_Goals::Get_Curivator_Autonomous(Curivator_Robot *Robot)
{
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),(char *)"Complete");
	SmartDashboard::PutNumber("Sequence",1.0);  //ensure we are on the right sequence
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(new Curivator_Goals_Impl(*Robot));
	//MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

  /***********************************************************************************************************************************/
 /*													Curivator_Robot_Control															*/
/***********************************************************************************************************************************/



void Curivator_Robot_Control::ResetPos()
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
}

void Curivator_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	double VoltageScalar=1.0;

	switch (index)
	{
	case Curivator_Robot::eArm:
	case Curivator_Robot::eTurret:
	case Curivator_Robot::eBoom:
	case Curivator_Robot::eBucket:
	case Curivator_Robot::eClasp:
		#ifdef Robot_TesterCode
		m_Potentiometer[index].UpdatePotentiometerVoltage(Voltage);
		m_Potentiometer[index].TimeChange();  //have this velocity immediately take effect
		#endif
		break;
	}
	VoltageScalar=m_RobotProps.GetRotaryProps(index).GetRotaryProps().VoltageScalar;
	Voltage*=VoltageScalar;
	std::string SmartLabel=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
	SmartLabel[0]-=32; //Make first letter uppercase
	SmartLabel+="Voltage";
	SmartDashboard::PutNumber(SmartLabel.c_str(),Voltage);
	Victor_UpdateVoltage(index,Voltage);
}

//bool Curivator_Robot_Control::GetBoolSensorState(size_t index) const
//{
//	bool ret;
//	switch (index)
//	{
//	case Curivator_Robot::eDartUpper:
//		break;
//	case Curivator_Robot::eDartLower:
//		break;
//	default:
//		assert (false);
//	}
//	return ret;
//}

Curivator_Robot_Control::Curivator_Robot_Control(bool UseSafety) : m_TankRobotControl(UseSafety),m_pTankRobotControl(&m_TankRobotControl),
		m_Compressor(NULL),m_RoboRIO_Accelerometer(NULL)
{
}

Curivator_Robot_Control::~Curivator_Robot_Control()
{
	//Encoder_Stop(Curivator_Robot::eWinch);
	DestroyCompressor(m_Compressor);
	m_Compressor=NULL;
	DestroyBuiltInAccelerometer(m_RoboRIO_Accelerometer);
	m_RoboRIO_Accelerometer=NULL;
}

void Curivator_Robot_Control::Reset_Rotary(size_t index)
{
	Encoder_Reset(index);  //This will check for encoder existence implicitly

	switch (index)
	{
	case Curivator_Robot::eTurret:
	case Curivator_Robot::eArm:
	case Curivator_Robot::eBoom:
	case Curivator_Robot::eBucket:
	case Curivator_Robot::eClasp:
		m_KalFilter[index].Reset();
		break;
	}

	#ifdef Robot_TesterCode
	switch (index)
	{
	case Curivator_Robot::eTurret:
	case Curivator_Robot::eArm:
	case Curivator_Robot::eBoom:
	case Curivator_Robot::eBucket:
	case Curivator_Robot::eClasp:
		m_Potentiometer[index].ResetPos();
	}
	#endif
}

#ifdef Robot_TesterCode
void Curivator_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
}
#endif

void Curivator_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const Curivator_Robot_Properties *robot_props=dynamic_cast<const Curivator_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		#ifdef Robot_TesterCode
		for (size_t index=0;index<5;index++)
		{
			Rotary_Properties writeable_arm_props=robot_props->GetRotaryProps(index);
			m_Potentiometer[index].Initialize(&writeable_arm_props);
		}
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
		//Encoder_SetDistancePerPulse(Curivator_Robot::eWinch,EncoderPulseRate);
		//Encoder_Start(Curivator_Robot::eWinch);
		ResetPos(); //must be called after compressor is created
		//Typically disabled, but may wish to enable initially
		#if 0
		for (size_t i=0;i<2;i++)
		{
			const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[i];
			string ContructedName;
			ContructedName=Prefix,ContructedName+="_Raw_high";
			SmartDashboard::PutNumber(ContructedName.c_str(),m_RobotProps.GetRotaryProps(i).GetRotary_Pot_Properties().PotMaxValue);
			ContructedName=Prefix,ContructedName+="_Raw_low";
			SmartDashboard::PutNumber(ContructedName.c_str(),m_RobotProps.GetRotaryProps(i).GetRotary_Pot_Properties().PotMinValue);
			ContructedName=Prefix,ContructedName+="_Pot_Range_Flipped";
			SmartDashboard::PutBoolean(ContructedName.c_str(),m_RobotProps.GetRotaryProps(i).GetRotary_Pot_Properties().IsFlipped);
		}
		#endif
	}

}

void Curivator_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef Robot_TesterCode
	for (size_t index=0;index<5;index++)
		m_Potentiometer[index].SetTimeDelta(dTime_s);
	#endif

	//Testing the accelerometer
	#if 0
	if (m_RoboRIO_Accelerometer)
	{
		SmartDashboard::PutNumber("RoboAccelX", m_RoboRIO_Accelerometer->GetX());
		SmartDashboard::PutNumber("RoboAccelY", m_RoboRIO_Accelerometer->GetY());
		SmartDashboard::PutNumber("RoboAccelZ", m_RoboRIO_Accelerometer->GetZ());
	}
	#endif
}

void Curivator_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) 
{
	#ifdef __USING_6CIMS__
	const Tank_Robot_Props &TankRobotProps=m_RobotProps.GetTankRobotProps();
	if (!TankRobotProps.ReverseSteering)
	{
		Victor_UpdateVoltage(Curivator_Robot::eLeftDrive3,(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
		Victor_UpdateVoltage(Curivator_Robot::eRightDrive3,-(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
	}
	else
	{
		Victor_UpdateVoltage(Curivator_Robot::eLeftDrive3,(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
		Victor_UpdateVoltage(Curivator_Robot::eRightDrive3,-(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
	}
	#endif
	m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}

__inline double Curivator_Robot_Control::Pot_GetRawValue(size_t index)
{
	//double raw_value = (double)m_Potentiometer.GetAverageValue();
	double raw_value=(double)Analog_GetAverageValue(index);
	raw_value = m_KalFilter[index](raw_value);  //apply the Kalman filter
	raw_value=m_Averager[index].GetAverage(raw_value); //and Ricks x element averager
	//Note: we keep the raw value in its native form... just averaging at most for less noise
	return raw_value;
}

double Curivator_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;
	const Curivator_Robot_Props &props=m_RobotProps.GetCurivatorRobotProps();

	switch (index)
	{
		case Curivator_Robot::eTurret:
		case Curivator_Robot::eArmPot:
		case Curivator_Robot::eBoom:
		case Curivator_Robot::eBucket:
		case Curivator_Robot::eClasp:
		{
			#ifndef Robot_TesterCode
			//double raw_value = (double)m_Potentiometer.GetAverageValue();
			double raw_value=Pot_GetRawValue(index);

			double PotentiometerRaw_To_Arm;

			const double HiRange=m_RobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotMaxValue;
			const double LowRange=m_RobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotMinValue;
			//If this is true, the value is inverted with the negative operator
			const bool FlipRange=m_RobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().IsFlipped;

			PotentiometerRaw_To_Arm = raw_value-LowRange;//zeros the potentiometer
			PotentiometerRaw_To_Arm = PotentiometerRaw_To_Arm/(HiRange-LowRange);//scales values from 0 to 1 with +- .001

			//Clip Range
			//I imagine .001 corrections will not be harmful for when in use.
			if (PotentiometerRaw_To_Arm < 0) PotentiometerRaw_To_Arm = 0;//corrects .001 or less causing a negative value
			if (PotentiometerRaw_To_Arm > 1 || PotentiometerRaw_To_Arm > .999) PotentiometerRaw_To_Arm = 1;//corrects .001 or lass causing value greater than 1

			//TODO see if we need a ratio multiply here... otherwise range is from 0-1 for full motion

			if (FlipRange)
				PotentiometerRaw_To_Arm=1.0-PotentiometerRaw_To_Arm;

			const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
			string ContructedName;
			ContructedName=Prefix,ContructedName+="_Raw";
			SmartDashboard::PutNumber(ContructedName.c_str(),raw_value);
			ContructedName=Prefix,ContructedName+="Pot_Raw";
			SmartDashboard::PutNumber(ContructedName.c_str(),PotentiometerRaw_To_Arm);

			//Now to compute the result... we start with the normalized value and give it the appropriate offset and scale
			//the offset is delegated in script in the final scale units, and the scale is the total range in radians
			result=PotentiometerRaw_To_Arm;
			//get scale
			const Ship_1D_Props &shipprops=m_RobotProps.GetRotaryProps(index).GetShip_1D_Props();
			//SmartDashboard::PutNumber("Arm_ScaleTest",shipprops.MaxRange-shipprops.MinRange);
			result*=shipprops.MaxRange-shipprops.MinRange;  //compute the total distance in radians
			//get offset... Note: scale comes first since the offset is of that scale
			result+=m_RobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotentiometerOffset;
			#else
			result=(m_Potentiometer[index].GetPotentiometerCurrentPosition()) + 0.0;
			//Now to normalize it
			const Ship_1D_Props &shipprops=m_RobotProps.GetRotaryProps(index).GetShip_1D_Props();
			const double NormalizedResult= (result - shipprops.MinRange)  / (shipprops.MaxRange - shipprops.MinRange);
			const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
			string ContructedName;
			ContructedName=Prefix,ContructedName+="_Raw";
			SmartDashboard::PutNumber(ContructedName.c_str(),result);  //this one is a bit different as it is the selected units we use
			ContructedName=Prefix,ContructedName+="Pot_Raw";
			SmartDashboard::PutNumber(ContructedName.c_str(),NormalizedResult);
			#endif
		}
		break;
	}
	return result;
}

void Curivator_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	//no solenoids
}


#ifdef Robot_TesterCode
  /***************************************************************************************************************/
 /*												Curivator_Robot_UI												*/
/***************************************************************************************************************/

Curivator_Robot_UI::Curivator_Robot_UI(const char EntityName[]) : Curivator_Robot(EntityName,this),Curivator_Robot_Control(),
		m_TankUI(this)
{
}

void Curivator_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
}
void Curivator_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
}

void Curivator_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
}
void Curivator_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
}
void Curivator_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
}
void Curivator_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
}

#endif
