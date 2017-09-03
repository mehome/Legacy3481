#include "stdafx.h"
#include "Robot_Tester.h"
//#define __UsingTankDrive__
#define __EnableRobotArmDisable__
#ifdef Robot_TesterCode
namespace Robot_Tester
{
	#include "CommonUI.h"
	#ifdef __UsingTankDrive__
	#include "Tank_Robot_UI.h"
	#else
	#include "Swerve_Robot_UI.h"
	#endif
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



__inline double LawOfCosines(double a,double b,double c)
{
	//Given all three lengths to the triangle use law of cosines to solve the angle c
	//http://mathcentral.uregina.ca/QQ/database/QQ.09.07/h/lucy1.html
	//c2 = a2 + b2 - 2ab cos(C)
	//rearranged to solve for cos(C)
	//x = -1 * ( (c*c - b*b - a*a) / (2*a*b) )
	//
	//         C  -------------a-----------   B
	//            ""--b----+         +==="" 
	//                      A------c+
	const double cos_C=-1.0 *( ((c*c)-(b*b)-(a*a)) / (2 * a * b));
	const double x=acos(cos_C);
	return x;
}

  /***********************************************************************************************************************************/
 /*													Curivator_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

Curivator_Robot::Robot_Arm::Robot_Arm(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control) : 
Rotary_Position_Control(csz_Curivator_Robot_SpeedControllerDevices_Enum[index],robot_control,index),m_Index(index),m_pParent(parent),m_LastIntendedPosition(0.0),
	m_Advance(false),m_Retract(false)
{
	SmartDashboard::PutBoolean("Disable_Setpoints",false);
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



void Curivator_Robot::Robot_Arm::SetIntendedPosition_Plus(double Position)
{
	const bool Disable_Setpoints=SmartDashboard::GetBoolean("Disable_Setpoints");
	if (Disable_Setpoints)
		return;
	//if (GetPotUsage()!=Rotary_Position_Control::eNoPot)
	{
		//if (((fabs(m_LastIntendedPosition-Position)<0.01)) || (!(IsZero(GetRequestedVelocity()))) )
		//	return;
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
	//else
	//	SetRequestedVelocity_FromNormalized(Position);   //allow manual use of same control
}

void Curivator_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[m_Index];
	string ContructedName;
	if (Bind)
	{
		ContructedName=Prefix,ContructedName+="_SetIntendedPosition";
		em->EventValue_Map[ContructedName.c_str()].Subscribe(ehl,*this, &Curivator_Robot::Robot_Arm::SetIntendedPosition_Plus);
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
		ContructedName=Prefix,ContructedName+="_SetIntendedPosition";
		em->EventValue_Map[ContructedName.c_str()].Remove(*this, &Curivator_Robot::Robot_Arm::SetIntendedPosition_Plus);
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
	//Note: the position is inverted due to the nature of the darts... we subtract the range from position to acquire inverted value
	const double ShaftExtension_in=m_Ship_1D_Props.MaxRange-GetPos_m()+m_Ship_1D_Props.MinRange;  //expecting a value from 0-12 in inches
	const double FullActuatorLength=ShaftExtension_in+BigArm_DistanceDartPivotToTip+BigArm_DistanceFromTipDartToClevis;  //from center point to center point
	//Now that we know all three lengths to the triangle use law of cosines to solve the angle of the linear actuator
	//c is FullActuatorLength
	//b is dart distance to arm
	//a is the AngleToDartPivotInterface_Length
	const double BigAngleDartInterface=LawOfCosines(BigArm_AngleToDartPivotInterface_Length,BigArm_DartToArmDistance,FullActuatorLength);
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
const double Boom_BoomRadius=23.03394231;  //Note to the boom rocker hole (not the bucket pivot hole as there was some design conflict)
const double Boom_BoomRadius_BP=26.03003642;  //The length to the bucket pivot point, almost 3 inches more... but a slight angle change to make it less
const double Boom_BP_To_RBP_RadiusAngle=DEG_2_RAD(0.35809296);  //The slight angle change... where the BP segment is the more acute angle to the big arm
const double Boom_BP_To_Lever_angle=DEG_2_RAD(175.16494932);
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
	//Note: the position is inverted due to the nature of the darts... we subtract the range from position to acquire inverted value
	const double ShaftExtension_in=m_Ship_1D_Props.MaxRange-GetPos_m()+m_Ship_1D_Props.MinRange;  //expecting a value from 0-12 in inches
	const double FullActuatorLength=ShaftExtension_in+Boom_DistanceDartPivotToTip+Boom_DistanceFromTipDartToClevis;  //from center point to center point
	//Now that we know all three lengths to the triangle use law of cosines to solve the angle of the linear actuator
	//c is FullActuatorLength
	//b is dart distance to arm
	//a is the AngleToDartPivotInterface_Length
	const double BigAngleDartInterface=LawOfCosines(Boom_AngleToDartPivotInterface_Length,Boom_DartToArmDistance,FullActuatorLength);
	//SmartDashboard::PutNumber("BoomDartInterface",RAD_2_DEG(BigAngleDartInterface));
	const double local_BoomAngle=M_PI-BigAngleDartInterface+Boom_AngleToDartPivotInterface;
	//To convert to global we subtract the sum of both the boom dart to bigarm constant angle and the angle of the big arm... this angle is global from 
	//a vertical line that aligns with the big arm's pivot point for the boom
	m_BoomAngle=local_BoomAngle-((PI_2-m_BigArm.GetBigArmAngle())+Boom_AngleBigArmToDartPivot);
	//SmartDashboard::PutNumber("BoomAngle",RAD_2_DEG(m_BoomAngle));
	//With this angle we can pull sin and cos for height and outward length using the big arm's radius constant
}

double Curivator_Robot::Boom::GetBoomLength() const
{
	const double LocalBoomLength=sin(m_BoomAngle) * Boom_BoomRadius;
	const double BoomLength=LocalBoomLength+m_BigArm.GetBigArmLength();
	//SmartDashboard::PutNumber("BoomLength",BoomLength);
	return BoomLength;
}
double Curivator_Robot::Boom::GetBoomHeight() const
{
	const double LocalBoomHeight=cos(m_BoomAngle) * Boom_BoomRadius;
	const double BoomHeight=m_BigArm.GetBigArmHeight()-LocalBoomHeight;
	//SmartDashboard::PutNumber("BoomHeight",BoomHeight);
	return BoomHeight;
}

  /***********************************************************************************************************************************/
 /*														Curivator_Robot::Bucket														*/
/***********************************************************************************************************************************/
// BRP=boom rocker pivot
// LAB=linear actuator for bucket
// BP=Bucket Pivot
// BucketRP=Bucket Rocker Pivot
const double Bucket_BRP_To_LAB=17.2528303;
const double Bucket_LAB_houseingLength=12.4;
const double Bucket_RockerBoomLength=6.49874999;
const double Bucket_BRP_LABtoBRP_BP_Angle=DEG_2_RAD(175.01);
const double Bucket_BRP_To_BP=3.0;
const double Bucket_BP_To_BucketRP=7.79685753;
const double Bucket_RockerBucketLength=7.3799994;
const double Bucket_BP_To_BucketCoM=7.70049652;
const double Bucket_BoomAngleToLAB_Angle=DEG_2_RAD(8.09856217);
const double Bucket_HorizontaltoBRP_BP_Angle=Bucket_BRP_LABtoBRP_BP_Angle+Bucket_BoomAngleToLAB_Angle-DEG_2_RAD(90);
const double Bucket_BucketRPtoBucketCoM_Angle=DEG_2_RAD(46.01897815);
const double Bucket_localConstantBRP_BP_height=sin(Bucket_HorizontaltoBRP_BP_Angle) * Bucket_BRP_To_BP;
const double Bucket_localConstantBRP_BP_distance=cos(Bucket_HorizontaltoBRP_BP_Angle) * Bucket_BRP_To_BP;
const double Bucket_CoMtoTip_Angle=DEG_2_RAD(32.1449117);
const double Bucket_BPBT_ToBucketRP_Angle=Bucket_CoMtoTip_Angle+Bucket_BucketRPtoBucketCoM_Angle;  //save an add
const double Bucket_BP_to_BucketTip=13.12746417;
const double Bucket_BPTip_to_BucketInterface_Angle=DEG_2_RAD(12.4082803);
const double Bucket_CoM_Radius=5.0;
Curivator_Robot::Bucket::Bucket(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control, Boom &boom) : 
	Robot_Arm(index,parent,robot_control),m_Boom(boom)
{
}

void Curivator_Robot::Bucket::TimeChange(double dTime_s)
{
	__super::TimeChange(dTime_s);
	//Now to compute where we are based from our length of extension
	//first start with the extension:
	const double ShaftExtension_in=GetPos_m();  //expecting a value from 0-12 in inches
	//Note: unlike the dart... we just include the clevis as part of the shaft extension
	const double FullActuatorLength=ShaftExtension_in+Bucket_LAB_houseingLength;  //from center point to center point
	//Now that we know all three lengths to the triangle use law of cosines to solve the angle of the linear actuator
	//c is FullActuatorLength
	//b is rocker boom length
	//a is boom rocker pivot to linear actuator mount length
	const double RockerBoomAngle=LawOfCosines(Bucket_BRP_To_LAB,Bucket_RockerBoomLength,FullActuatorLength);
	const double QuadRockerBoomAngle=Bucket_BRP_LABtoBRP_BP_Angle-RockerBoomAngle;  //start to grab the interior angle of the quadrilateral
	//Given this angle... compute the x and y coordinates of the rocker's interface of the actuator as this will be the length of the bisected quadrilateral
	const double RockerInterfaceY=(cos(QuadRockerBoomAngle) * Bucket_RockerBoomLength) - Bucket_BRP_To_BP;  //factor in the difference for the distance formula
	const double RockerInterfaceX=sin(QuadRockerBoomAngle) * Bucket_RockerBoomLength;
	// use distance formula to solve the length
	const double QuadBisectLength=sqrt((RockerInterfaceX*RockerInterfaceX)+(RockerInterfaceY*RockerInterfaceY));
	//Now we find the Buckets pivot angle in two parts... first the upper triangle (now from the bisecting of the quadrilateral)
	const double BucketPivotUpperAngle=LawOfCosines(Bucket_BRP_To_BP,QuadBisectLength,Bucket_RockerBoomLength);
	const double BucketPivotLowerAngle=LawOfCosines(Bucket_BP_To_BucketRP,QuadBisectLength,Bucket_RockerBucketLength);
	const double BucketPivotAngle=BucketPivotUpperAngle+BucketPivotLowerAngle;
	const double BucketCoMPivotAngleHorz=BucketPivotAngle+Bucket_BucketRPtoBucketCoM_Angle-DEG_2_RAD(90) - (DEG_2_RAD(90) - Bucket_HorizontaltoBRP_BP_Angle);
	const double BoomAngle=m_Boom.GetBoomAngle();
	//Now to compute the local height... distance from boom origin downward 
	m_Bucket_globalBRP_BP_height=(sin(Bucket_HorizontaltoBRP_BP_Angle-BoomAngle) * Bucket_BRP_To_BP);
	//Note this first equation omits the boom angle as a reference in a local setting
	//const double LocalCoMHeight=Bucket_localConstantBRP_BP_height+(sin(BucketCoMPivotAngleHorz)*Bucket_BP_To_BucketCoM);
	m_GlobalCoMHeight=m_Bucket_globalBRP_BP_height+(sin(BucketCoMPivotAngleHorz-BoomAngle)*Bucket_BP_To_BucketCoM);
	//This equation is optional... used for simulation geometry
	m_GlobalCoMDistance=m_Bucket_globalBRP_BP_distance+(cos(BucketCoMPivotAngleHorz-BoomAngle)*Bucket_BP_To_BucketCoM);
	//Note this first equation omits the boom angle as a reference in a local setting
	//const double LocalTipHeight=Bucket_localConstantBRP_BP_height+(sin(BucketCoMPivotAngleHorz + Bucket_CoMtoTip_Angle)*Bucket_BP_to_BucketTip);
	const double LocalTipHeight=m_Bucket_globalBRP_BP_height+(sin(BucketCoMPivotAngleHorz + Bucket_CoMtoTip_Angle-BoomAngle)*Bucket_BP_to_BucketTip);
	m_GlobalTipHeight=m_Boom.GetBoomHeight()-LocalTipHeight;
	m_LocalBucketAngle=DEG_2_RAD(180)- (BucketCoMPivotAngleHorz + Bucket_CoMtoTip_Angle) - Bucket_BPTip_to_BucketInterface_Angle;
	//const double LocalHeight=max(LocalTipHeight,LocalCoMHeight+Bucket_CoM_Radius);
	//LocalDistance=Bucket_localConstantBRP_BP_distance+(cos(BucketCoMPivotAngleHorz + Bucket_CoMtoTip_Angle)*Bucket_BP_to_BucketTip);
	m_Bucket_globalBRP_BP_distance=cos(Bucket_HorizontaltoBRP_BP_Angle-BoomAngle) * Bucket_BRP_To_BP;
	m_GlobalDistance=m_Bucket_globalBRP_BP_distance+(cos(BucketCoMPivotAngleHorz + Bucket_CoMtoTip_Angle-BoomAngle)*Bucket_BP_to_BucketTip);
	const double globalBucketDistance=GetBucketLength();
	const double globalTipHeight=GetBucketTipHeight();
	const double globalRoundEndHeight=GetBucketRoundEndHeight();
	const double globalBucketAngle_deg=RAD_2_DEG(GetBucketAngle());
	SmartDashboard::PutNumber("BucketDistance",globalBucketDistance);
	//SmartDashboard::PutNumber("BucketTipHeight",globalTipHeight);
	//SmartDashboard::PutNumber("BucketRoundEndHeight",globalRoundEndHeight);
	SmartDashboard::PutNumber("BucketHeight",min(globalTipHeight,globalRoundEndHeight));
	SmartDashboard::PutNumber("BucketAngle",globalBucketAngle_deg);
}

double Curivator_Robot::Bucket::GetBucketLength() const
{
 const double globalBucketDistance=m_GlobalDistance + m_Boom.GetBoomLength();
 return globalBucketDistance;
}
double Curivator_Robot::Bucket::GetBucketRoundEndHeight() const
{
	const double globalRoundEndHeight=m_Boom.GetBoomHeight()-(m_GlobalCoMHeight+Bucket_CoM_Radius);
	return globalRoundEndHeight;
}
double Curivator_Robot::Bucket::GetCoMHeight() const 
{
	const double globalCoMHeight=m_Boom.GetBoomHeight()-m_GlobalCoMHeight;
	return globalCoMHeight;
}
double Curivator_Robot::Bucket::GetCoMDistance() const 
{
	const double globalCoMDistance=m_GlobalCoMDistance + m_Boom.GetBoomLength();
	return globalCoMDistance;
}

double Curivator_Robot::Bucket::GetBucketAngle() const
{
	const double globalBucketAngle=m_LocalBucketAngle+m_Boom.GetBoomAngle();
	return globalBucketAngle;
}

  /***********************************************************************************************************************************/
 /*														Curivator_Robot::Clasp														*/
/***********************************************************************************************************************************/
// BRP=boom rocker pivot  (this is our point of origin)
// LAC=linear actuator for clasp
// CP=Clasp Pivot  (same hole used for bucket pivot)
// CPMT= segment from clasp pivot to midline tip
const double Clasp_BRP_To_LAC=10.97993151;
const double Clasp_LAC_houseingLength=7.4;
const double Clasp_CP_To_LAC=3.11799058;
const double Clasp_BoomAngleToLAC_Angle=DEG_2_RAD(9.58288198);
const double Clasp_MidlineSegment=10.31720455;
const double Clasp_LA_Interface_to_Midline_Angle=DEG_2_RAD(125.84975925);
const double Clasp_MidlineToEdge_Angle=DEG_2_RAD(101.61480361);
const double Clasp_BottomToSideAngle=DEG_2_RAD(98.23909595);
const double Clasp_BottomEdgeLength=1.507182;  //used to find lowest point
const double Clasp_BottomEdgeLength_Half=Clasp_BottomEdgeLength/2.0;  //spare this computation
const double Clasp_CP_To_MidlineTip=12.40350045; //used in ComputeArmPosition for length
const double Clasp_CPMT_ToSide_Angle=DEG_2_RAD(8.09713426);
const double Clasp_MidLineToEdge_Angle=Clasp_MidlineToEdge_Angle+Clasp_BottomToSideAngle-DEG_2_RAD(180);  //about 19.85
Curivator_Robot::Clasp::Clasp(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control, Bucket &bucket) : 
Robot_Arm(index,parent,robot_control),m_Bucket(bucket)
{
}

void Curivator_Robot::Clasp::TimeChange(double dTime_s)
{
	__super::TimeChange(dTime_s);
	//Now to compute where we are based from our length of extension
	//first start with the extension:
	const double ShaftExtension_in=GetPos_m();  //expecting a value from 0-12 in inches
	//Note: unlike the dart... we just include the clevis as part of the shaft extension
	const double FullActuatorLength=ShaftExtension_in+Clasp_LAC_houseingLength;  //from center point to center point
	//Now that we know all three lengths to the triangle use law of cosines to solve the angle of the linear actuator
	//c is FullActuatorLength
	//b is rocker boom length
	//a is boom rocker pivot to linear actuator mount length
	const double ClaspLA_Interface_Angle=LawOfCosines(Clasp_BRP_To_LAC,Clasp_CP_To_LAC,FullActuatorLength);
	const double ClaspLA_Interface_Angle_Horizontal=(DEG_2_RAD(90)-(ClaspLA_Interface_Angle+Clasp_BoomAngleToLAC_Angle));
	const double Clasp_MidlineSegment_Angle=Clasp_LA_Interface_to_Midline_Angle+ClaspLA_Interface_Angle_Horizontal; //angle from horizontal
	//----------------
	const Boom &boom=m_Bucket.GetBoom();
	const double BoomAngle=boom.GetBoomAngle();
	//const double BoomAngle=0.0; local testing
	//Next to find the height and length... start with Clasp pivot... then add the clasp interface with midline segment
	//use for local testing
	//const double ClaspPivotHeight=Bucket_localConstantBRP_BP_height;
	const double ClaspPivotHeight=m_Bucket.GetBucket_globalBRP_BP_height();
	const double Clasp_CP_To_LAC_Height=sin(ClaspLA_Interface_Angle_Horizontal-BoomAngle)*Clasp_CP_To_LAC;
	const double Clasp_MidlineSegment_Height=sin(Clasp_MidlineSegment_Angle-BoomAngle)*Clasp_MidlineSegment;
	const double localClasp_MidlineHeight=ClaspPivotHeight-Clasp_CP_To_LAC_Height+Clasp_MidlineSegment_Height;
	m_GlobalMidlineHeight=boom.GetBoomHeight()-localClasp_MidlineHeight;
	//for length (aka horizontal distance) use the similar technique as with height
	//use for local testing
	//const double ClaspPivotDistance=Bucket_localConstantBRP_BP_distance;
	const double ClaspPivotDistance=m_Bucket.GetBucket_globalBRP_BP_distance();
	const double Clasp_CP_To_LAC_Distance=cos(ClaspLA_Interface_Angle_Horizontal-BoomAngle)*Clasp_CP_To_LAC;
	const double Clasp_MidlineSegment_Distance=cos(Clasp_MidlineSegment_Angle-BoomAngle)*Clasp_MidlineSegment;
	const double localClasp_MidlineDistance=ClaspPivotDistance-Clasp_CP_To_LAC_Distance+Clasp_MidlineSegment_Distance;
	m_GlobalMidlineDistance=boom.GetBoomLength()+localClasp_MidlineDistance;
	//Now to solve the angle of the side
	m_Clasp_MidlineToEdge_Angle_Horizontal=Clasp_MidlineSegment_Angle-(DEG_2_RAD(180)-Clasp_MidlineToEdge_Angle);
	const double localSideFromHorizontal_Angle=DEG_2_RAD(180)-(m_Clasp_MidlineToEdge_Angle_Horizontal+Clasp_BottomToSideAngle);
	m_GlobalClaspAngle=localSideFromHorizontal_Angle+boom.GetBoomAngle();
	GetMinHeight();
}

double Curivator_Robot::Clasp::GetInnerTipHieght() const
{
	const Boom &boom=m_Bucket.GetBoom();
	const double BoomAngle=boom.GetBoomAngle();
	const double InnerTipHieght=(m_GlobalMidlineHeight-(sin(m_Clasp_MidlineToEdge_Angle_Horizontal-BoomAngle)*Clasp_BottomEdgeLength_Half));
	return InnerTipHieght;
}
double Curivator_Robot::Clasp::GetOuterTipHieght() const
{
	const Boom &boom=m_Bucket.GetBoom();
	const double BoomAngle=boom.GetBoomAngle();
	const double OuterTipHieght=sin(m_Clasp_MidlineToEdge_Angle_Horizontal-BoomAngle)*Clasp_BottomEdgeLength_Half+m_GlobalMidlineHeight;
	return OuterTipHieght;
}

double Curivator_Robot::Clasp::GetMinHeight() const
{
	const double minHeight=std::min(GetInnerTipHieght(),GetOuterTipHieght());
	return minHeight;
}
  /***********************************************************************************************************************************/
 /*															Curivator_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;

Curivator_Robot::Curivator_Robot(const char EntityName[],Curivator_Control_Interface *robot_control,bool IsAutonomous) : 
#ifdef __UsingTankDrive__
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), 
#else
	Swerve_Robot(EntityName,robot_control,eDriveOffset,IsAutonomous), m_RobotControl(robot_control), 
#endif
		m_Turret(eTurret,this,robot_control),m_Arm(eArm,this,robot_control),m_LatencyCounter(0.0),
		m_Boom(eBoom,this,robot_control,m_Arm),m_Bucket(eBucket,this,robot_control,m_Boom),m_Clasp(eClasp,this,robot_control,m_Bucket),
		m_ArmXpos(eArm_Xpos,this,robot_control),m_ArmYpos(eArm_Ypos,this,robot_control),m_BucketAngle(eBucket_Angle,this,robot_control),
		m_ClaspAngle(eClasp_Angle,this,robot_control),
		m_CenterLeftWheel(csz_Curivator_Robot_SpeedControllerDevices_Enum[eWheel_CL],robot_control,eWheel_CL),
		m_CenterRightWheel(csz_Curivator_Robot_SpeedControllerDevices_Enum[eWheel_CR],robot_control,eWheel_CR),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_AutonPresetIndex(0),m_FreezeArm(false),m_LockPosition(false)
{
	mp_Arm[eTurret]=&m_Turret;
	mp_Arm[eArm]=&m_Arm;
	mp_Arm[eBoom]=&m_Boom;
	mp_Arm[eBucket]=&m_Bucket;
	mp_Arm[eClasp]=&m_Clasp;
	mp_Arm[eArm_Xpos]=&m_ArmXpos;
	mp_Arm[eArm_Ypos]=&m_ArmYpos;
	mp_Arm[eBucket_Angle]=&m_BucketAngle;
	mp_Arm[eClasp_Angle]=&m_ClaspAngle;
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

	for (size_t i=0;i<Curivator_Robot_NoRobotArm;i++)
		mp_Arm[i]->Initialize(em,RobotProps?&RobotProps->GetRotaryProps(i):NULL);
	#ifdef Robot_TesterCode
	if (RobotProps)
	{
		for (size_t i=0;i<2;i++)
		{
			Rotary_Properties drive=RobotProps->GetRotaryProps(i+eWheel_CL);
			drive.EncoderSimulationProps()=RobotProps->GetEncoderSimulationProps();
		}
	}
	#endif
	m_CenterLeftWheel.Initialize(em,RobotProps?&RobotProps->GetRotaryProps(eWheel_CL):NULL);
	m_CenterRightWheel.Initialize(em,RobotProps?&RobotProps->GetRotaryProps(eWheel_CR):NULL);
}
void Curivator_Robot::ResetPos()
{
	__super::ResetPos();
	for (size_t i=0;i<Curivator_Robot_NoRobotArm;i++)
		mp_Arm[i]->ResetPos();
	m_CenterLeftWheel.ResetPos();
	m_CenterRightWheel.ResetPos();
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

	//Inject the velocities from the swerve drive wheels to the center wheels.
	{
		//The velocity between the front and back wheels are typically the same but can be different during time when wheel angles are not 
		//tangent to their set point (should be minimal), so we'll simply average between the velocities for best rate.
		const double IntendedVelocityLeft=(GetIntendedDriveVelocity(Swerve_Robot::eWheel_FL)+GetIntendedDriveVelocity(Swerve_Robot::eWheel_RL))/2.0;
		const double IntendedVelocityRight=(GetIntendedDriveVelocity(Swerve_Robot::eWheel_FR)+GetIntendedDriveVelocity(Swerve_Robot::eWheel_RR))/2.0;
		m_CenterLeftWheel.SetRequestedVelocity(IntendedVelocityLeft);
		m_CenterRightWheel.SetRequestedVelocity(IntendedVelocityRight);
		m_CenterLeftWheel.AsEntity1D().TimeChange(dTime_s);
		m_CenterRightWheel.AsEntity1D().TimeChange(dTime_s);
	}

	for (size_t i=0;i<Curivator_Robot_NoRobotArm;i++)
		mp_Arm[i]->AsEntity1D().TimeChange(dTime_s);

	//const double  YOffset=-SmartDashboard::GetNumber("Y Position");
	//const double XOffset=SmartDashboard::GetNumber("X Position");

	//Apply the position and rotation of bucket to their children
	if (m_RobotProps.GetCurivatorRobotProps().EnableArmAutoPosition)
	{
		const double xpos=!m_LockPosition?m_ArmXpos.GetPos_m():m_Last_xpos;
		const double ypos=!m_LockPosition?m_ArmYpos.GetPos_m():m_Last_ypos;
		const double bucket_angle=!m_LockPosition?m_BucketAngle.GetPos_m():m_Last_bucket_angle;
		const double clasp_angle=!m_LockPosition?m_ClaspAngle.GetPos_m():m_Last_clasp_angle;
		//no harm in always assigning these... for sake of avoiding a branch
		m_Last_xpos=xpos;
		m_Last_ypos=ypos;
		m_Last_bucket_angle=bucket_angle;
		m_Last_clasp_angle=clasp_angle;

		SmartDashboard::PutNumber("arm_xpos",xpos);
		SmartDashboard::PutNumber("arm_ypos",ypos);
		SmartDashboard::PutNumber("bucket_angle",bucket_angle);
		SmartDashboard::PutNumber("clasp_angle",clasp_angle);
		double BigArm_ShaftLength,Boom_ShaftLength,BucketShaftLength,ClaspShaftLength;
		ComputeArmPosition(ypos,xpos,bucket_angle,clasp_angle,BigArm_ShaftLength,Boom_ShaftLength,BucketShaftLength,ClaspShaftLength);
		//Output them before they are inverted to be more readable
		SmartDashboard::PutNumber("BigArm_ShaftLength",BigArm_ShaftLength);
		SmartDashboard::PutNumber("Boom_ShaftLength",Boom_ShaftLength);
		//invert the boom and big arm lengths due to how the darts are wired
		Boom_ShaftLength=m_Boom.GetMaxRange()-Boom_ShaftLength+m_Boom.GetMinRange();
		BigArm_ShaftLength=m_Arm.GetMaxRange()-BigArm_ShaftLength+m_Arm.GetMinRange();
		SmartDashboard::PutNumber("BucketShaftLength",BucketShaftLength);
		SmartDashboard::PutNumber("ClaspShaftLength",ClaspShaftLength);
		//apply these values to their children
		if (!m_FreezeArm)
		{
			m_Arm.SetIntendedPosition(BigArm_ShaftLength);
			m_Boom.SetIntendedPosition(Boom_ShaftLength);
			m_Bucket.SetIntendedPosition(BucketShaftLength);
			m_Clasp.SetIntendedPosition(ClaspShaftLength);
		}
		else
		{
			m_Arm.SetRequestedVelocity(0.0);
			m_Boom.SetRequestedVelocity(0.0);
			m_Bucket.SetRequestedVelocity(0.0);
			m_Clasp.SetRequestedVelocity(0.0);
			m_ArmXpos.SetRequestedVelocity(0.0);
			m_ArmYpos.SetRequestedVelocity(0.0);
			m_BucketAngle.SetRequestedVelocity(0.0);
			m_ClaspAngle.SetRequestedVelocity(0.0);
		}
	}
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
		em->EventOnOff_Map["StopAuton"].Subscribe(ehl,*this, &Curivator_Robot::StopAuton);
		em->EventOnOff_Map["Robot_FreezeArm"].Subscribe(ehl,*this, &Curivator_Robot::FreezeArm);
		em->EventOnOff_Map["Robot_LockPosition"].Subscribe(ehl,*this, &Curivator_Robot::LockPosition);
	}
	else
	{
		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"]  .Remove(*this, &Curivator_Robot::TestAutonomous);
		em->Event_Map["Complete"]  .Remove(*this, &Curivator_Robot::GoalComplete);
		#endif
		em->EventOnOff_Map["StopAuton"].Remove(*this, &Curivator_Robot::StopAuton);
		em->EventOnOff_Map["Robot_FreezeArm"].Remove(*this, &Curivator_Robot::FreezeArm);
		em->EventOnOff_Map["Robot_LockPosition"].Remove(*this, &Curivator_Robot::LockPosition);
	}

	for (size_t i=0;i<Curivator_Robot_NoRobotArm;i++)
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

__inline double GetDistance(double x1,double y1, double x2, double y2)
{
	const double x=fabs(x2-x1);
	const double y=fabs(y2-y1);
	const double hypotenuse=sqrt((x*x)+(y*y));
	return hypotenuse;
}

__inline double EnforceShaftLimits(double InputValue,double minRange=0.75,double maxRange=11.0)
{
	double ret=std::max(std::min(InputValue,maxRange),minRange);
	//check for nan
	if (!(InputValue<0.0||InputValue>0.0))
		ret=((maxRange-minRange)/2.0)+minRange;   //pick center as a fallback
	return ret;
}

void Curivator_Robot::ComputeArmPosition(double GlobalHeight,double GlobalDistance,double BucketAngle_deg,double ClaspOpeningAngle_deg,
										 double &BigArm_ShaftLength,double &Boom_ShaftLength,double &BucketShaftLength,double &ClaspShaftLength)
{
	const double BucketAngle=DEG_2_RAD(BucketAngle_deg);
	//Working in reverse starting with a global environment
	//First find the bucket pivot point (global coordinates)
	const double BucketPivotUsingTip_y=sin(BucketAngle+Bucket_BPTip_to_BucketInterface_Angle)*Bucket_BP_to_BucketTip+GlobalHeight;

	const double BucketCOMtoVerticle_Angle=(BucketAngle+Bucket_BPTip_to_BucketInterface_Angle-DEG_2_RAD(90)) + Bucket_CoMtoTip_Angle;
	const double BucketPivotUsingCOM_y=cos(BucketCOMtoVerticle_Angle)*Bucket_BP_To_BucketCoM+GlobalHeight+Bucket_CoM_Radius;
	//Which ever is higher will be the one to use to ensure lowest point meets height requirements
	const double BucketPivotPoint_y=max(BucketPivotUsingTip_y,BucketPivotUsingCOM_y);
	const double BucketPivotPoint_x=cos(BucketAngle+Bucket_BPTip_to_BucketInterface_Angle)*Bucket_BP_to_BucketTip+GlobalDistance;
	//with the bucket pivot point we must solve the boom and bigarm where together they are able to provide the pivot point to this location
	//We can first solve the boom angle... like before this angle is based off of vertical (i.e. 0 is vertical positive outward extended)
	//since the big arm pivot is the point of origin between this and the bucket pivot point we can compute length of a triangle, where:
	//point a is origin, point b is bucket pivot point, and c is unknown---
	//We know the lengths of the bigarm and boom, and with this we can use law of cosines to angle in point c... once this angle is known it is
	//possible to know the global point of the boom pivot as well as the angle of the big arm. which sets up for solving their linear actuator lengths.
	//--------------------------------
	//Compute ab segment
	const double OriginToBP=sqrt((BucketPivotPoint_x*BucketPivotPoint_x)+(BucketPivotPoint_y*BucketPivotPoint_y));
	//Now to define where the boom bigarm point exists... by simply first finding the bigarm angle first start with another law of cosines
	const double BigArmUpper_Angle=LawOfCosines(BigArm_BigArmRadius,OriginToBP,Boom_BoomRadius_BP);
	const double BigArmLower_Angle=atan2(BucketPivotPoint_y,BucketPivotPoint_x);
	const double BigArmAngle=BigArmUpper_Angle+BigArmLower_Angle;
	//const double BigArmBoomPivot_height=sin(BigArmAngle)*BigArm_BigArmRadius;
	//const double BigArmBoomPivot_length=cos(BigArmAngle)*BigArm_BigArmRadius;
	//Now that we know this point... we can find the boom angle from vertical using law of cosines from the big arm angle
	const double BigArmBoomBP_Angle=LawOfCosines(Boom_BoomRadius_BP,BigArm_BigArmRadius,OriginToBP);
	const double BoomAngle=BigArmBoomBP_Angle-(DEG_2_RAD(90)-BigArmAngle)+Boom_BP_To_RBP_RadiusAngle;
	//At this point... I'll work my way from bigarm to bucket
	//For the bigarm we have an angle... first determine where the linear actuators interface point is located
	const double BigArmLAInteface_height=sin(BigArmAngle+BigArm_AngleToDartPivotInterface)*BigArm_AngleToDartPivotInterface_Length;
	const double BigArmLAInteface_length=cos(BigArmAngle+BigArm_AngleToDartPivotInterface)*BigArm_AngleToDartPivotInterface_Length;
	const double BigArm_LA_Length_xLeg=fabs(BigArmLAInteface_length-BigArm_DartToArmDistance);
	const double BigArm_LA_Length=sqrt((BigArm_LA_Length_xLeg*BigArm_LA_Length_xLeg)+(BigArmLAInteface_height*BigArmLAInteface_height));
	//Yay got the big arm actuators length...
	BigArm_ShaftLength=EnforceShaftLimits(BigArm_LA_Length-BigArm_DistanceDartPivotToTip-Boom_DistanceFromTipDartToClevis);
	//next time to get the boom actuator length
	//First find point where boom interface is located
	const double BoomLeverAngle=BoomAngle+Boom_BP_To_Lever_angle-DEG_2_RAD(180);
	const double BoomLAInteface_height=cos(BoomLeverAngle)*Boom_AngleToDartPivotInterface_Length;
	const double BoomLAInteface_length=sin(BoomLeverAngle)*Boom_AngleToDartPivotInterface_Length;
	//unlike with the big arm... we'll have to find the lower point and use a full-blown distance formula
	//next find the big arm's mounting point for the end of the boom dart's actuator
	const double BoomLA_Mount_Angle=(DEG_2_RAD(90)-BigArmAngle)+Boom_AngleBigArmToDartPivot;
	const double BoomLA_Mount_height=cos(BoomLA_Mount_Angle)*Boom_DartToArmDistance;
	const double BoomLA_Mount_length=sin(BoomLA_Mount_Angle)*Boom_DartToArmDistance;
	const double Boom_LA_Length_xLeg=fabs(BoomLAInteface_length-BoomLA_Mount_length);
	const double Boom_LA_Length_yLeg=fabs(BoomLAInteface_height+BoomLA_Mount_height);  //added because they are going in different directions
	const double Boom_LA_Length=sqrt((Boom_LA_Length_xLeg*Boom_LA_Length_xLeg)+(Boom_LA_Length_yLeg*Boom_LA_Length_yLeg));
	//Yay got the boom actuators length...
	Boom_ShaftLength=EnforceShaftLimits(Boom_LA_Length-Boom_DistanceDartPivotToTip-Boom_DistanceFromTipDartToClevis);
	//now onto the bucket... first locate rocker boom's point
	const double BucketRBP_Angle=(DEG_2_RAD(180)-Bucket_BRP_LABtoBRP_BP_Angle)+(BoomAngle-Bucket_BoomAngleToLAB_Angle);
	const double RockerBoomPivotPoint_y=cos(BucketRBP_Angle)*Bucket_BRP_To_BP+BucketPivotPoint_y;
	const double RockerBoomPivotPoint_x=BucketPivotPoint_x-sin(BucketRBP_Angle)*Bucket_BRP_To_BP;
	//Next we pursue the difficult boom rocker pivot end that interfaces with the linear actuator... to find we must bisect the quadrilateral using
	//a new segment from the boom rocker pivot to the bucket rocker pivot.  Once this is created we can determine that angle and subtract it from
	//vertical via BucketRBP_Angle.
	//------------
	//First find the bucket rocker pivot point.
	//We'll just keep it all global to keep things easier to read and verify
	const double Veritcal_ToBucketRP_Angle=Bucket_BPBT_ToBucketRP_Angle - (DEG_2_RAD(90)-(BucketAngle+Bucket_BPTip_to_BucketInterface_Angle));
	const double RockerBucketPivotPoint_y=BucketPivotPoint_y-cos(Veritcal_ToBucketRP_Angle)*Bucket_BP_To_BucketRP;
	const double RockerBucketPivotPoint_x=sin(Veritcal_ToBucketRP_Angle)*Bucket_BP_To_BucketRP+BucketPivotPoint_x;
	const double brp_bucketrp_segment_length=GetDistance(RockerBoomPivotPoint_x,RockerBoomPivotPoint_y,RockerBucketPivotPoint_x,RockerBucketPivotPoint_y);
	//With this new segment... there are 2 angles to extract from the quadrelateral... the upper and lower:
	const double RockerBoomUpperAngle=LawOfCosines(Bucket_RockerBoomLength,brp_bucketrp_segment_length,Bucket_RockerBucketLength);
	double RockerBoomLowerAngle=LawOfCosines(Bucket_BRP_To_BP,brp_bucketrp_segment_length,Bucket_BP_To_BucketRP);
	//The lower angle is a bit tricky, because we can have a quaterlaterial where the bucket pivot point is inside the bisected line at which case
	//the rocker boom lower angle is negative... to determine this 
	//and use atan2 (with origin at RockerBoomPivotPoint) to find the angle and compare against BucketRBP_Angle
	const double SegmentAngleFromVerticle=DEG_2_RAD(90)+atan2(RockerBucketPivotPoint_y-RockerBoomPivotPoint_y,RockerBucketPivotPoint_x-RockerBoomPivotPoint_x);
	if (SegmentAngleFromVerticle<BucketRBP_Angle)
		RockerBoomLowerAngle=RockerBoomLowerAngle*-1.0;  //Note: the proper thing here may be to transform and use atan2 instead
	//With this we can now find the angle from vertical
	const double RockerBoomFromVertical_Angle=BucketRBP_Angle+RockerBoomUpperAngle+RockerBoomLowerAngle;
	//This angle allows use to fine the rocker pivot LA interface as the next point in the triangle
	const double RockerPivotLAInterface_y=RockerBoomPivotPoint_y-cos(RockerBoomFromVertical_Angle)*Bucket_RockerBoomLength;
	const double RockerPivotLAInterface_x=sin(RockerBoomFromVertical_Angle)*Bucket_RockerBoomLength+RockerBoomPivotPoint_x;
	//Now onto the last point the boom LA mount
	const double VerticleToLAB_Angle=BoomAngle-Bucket_BoomAngleToLAB_Angle;
	const double BoomLAMount_y=cos(VerticleToLAB_Angle)*Bucket_BRP_To_LAB+RockerBoomPivotPoint_y;
	const double BoomLAMount_x=RockerBoomPivotPoint_x-(sin(VerticleToLAB_Angle)*Bucket_BRP_To_LAB);
	//use distance formula between the LA mount and the Rocker pivot
	const double Bucket_LA_Length=GetDistance(BoomLAMount_x,BoomLAMount_y,RockerPivotLAInterface_x,RockerPivotLAInterface_y);
	//Yay got the bucket actuators length...
	BucketShaftLength=EnforceShaftLimits(Bucket_LA_Length-Bucket_LAB_houseingLength);
	//Now onto the clasp
	//Find the midline tip point global location by use of the CPMT segment
	double ClaspOpeningAngle=DEG_2_RAD(ClaspOpeningAngle_deg);
	const double CPMT_ToHorizontal=BucketAngle-(ClaspOpeningAngle-Clasp_CPMT_ToSide_Angle);
	const double MidlineTip_y=BucketPivotPoint_y-sin(CPMT_ToHorizontal)*Clasp_CP_To_MidlineTip;
	const double MidlineTip_x=BucketPivotPoint_x-cos(CPMT_ToHorizontal)*Clasp_CP_To_MidlineTip;
	//Compute the midline angle to horizontal
	const double MidlineToHorizontal=Clasp_MidLineToEdge_Angle+BucketAngle-ClaspOpeningAngle;
	//with the mid line tip offset and the midline angle we can find LA interface point
	const double ClaspLAInterface_y=sin(MidlineToHorizontal)*Clasp_MidlineSegment+MidlineTip_y;
	const double ClaspLAInterface_x=cos(MidlineToHorizontal)*Clasp_MidlineSegment+MidlineTip_x;
	//Now to find the LA clasp mount point
	const double LAMountVertical_Angle=BoomAngle+Clasp_BoomAngleToLAC_Angle;
	const double LAClaspMount_y=cos(LAMountVertical_Angle)*Clasp_BRP_To_LAC+BucketPivotPoint_y;
	const double LAClaspMount_x=BucketPivotPoint_x-sin(LAMountVertical_Angle)*Clasp_BRP_To_LAC;
	//with out 2 point find the distance
	const double Clasp_LA_Length=GetDistance(LAClaspMount_x,LAClaspMount_y,ClaspLAInterface_x,ClaspLAInterface_y);
	//Yay got the clasp actuators length...
	ClaspShaftLength=EnforceShaftLimits(Clasp_LA_Length-Clasp_LAC_houseingLength,0.75,6.0);
}

#ifdef Robot_TesterCode
void Curivator_Robot::TestAutonomous()
{
	//keep around to test geometry
	#if 0
	double BigArm_ShaftLength;
	double Boom_ShaftLength;
	double BucketShaftLength;
	double ClaspShaftLength;
	//In this test... these constants should return half lengths of each actuator
	ComputeArmPosition(-0.97606122071131374,32.801521314123598,78.070524788111342,13.19097419,
		BigArm_ShaftLength,Boom_ShaftLength,BucketShaftLength,ClaspShaftLength);
	return;
	#endif

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

double Curivator_Robot::GetBucketAngleContinuity()
{
	double testLimits_deg=fabs(m_BucketAngle.AsEntity1D().GetPos_m()-RAD_2_DEG( m_Bucket.GetBucketAngle()));
	return testLimits_deg;
}

void Curivator_Robot::StopAuton(bool isOn)
{
	FreezeArm(isOn);
	m_controller->GetUIController_RW()->SetAutoPilot(false);
	LockPosition(false);
}

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
			//const double c_WheelDiameter=0.1524;  //6 inches
			const double c_MotorToWheelGearRatio=12.0/36.0;

			props.OptimalAngleUp=c_OptimalAngleUp_r;
			props.OptimalAngleDn=c_OptimalAngleDn_r;
			props.ArmLength=c_ArmLength_m;
			props.ArmToGearRatio=c_ArmToGearRatio;
			props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
			props.PotentiometerMaxRotation=c_PotentiometerMaxRotation;
			props.GearHeightOffset=c_GearHeightOffset;
			props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
			props.EnableArmAutoPosition=false;

			m_CurivatorRobotProps=props;
		}

		//Curivator_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		m_CurivatorRobotProps=props;
	}
	{
		#ifdef __UsingTankDrive__
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

		//Late assign this to override the initial default
		//Was originally 0.4953 19.5 width for 2011
		//Now is 0.517652 20.38 according to Parker  (not too worried about the length)
		props.WheelDimensions=Vec2D(0.517652,0.6985); //27.5 x 20.38
		props.WheelDiameter=c_WheelDiameter;
		props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
		props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
		m_TankRobotProps=props;
		#endif 
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
	"arm_SetCurrentVelocity","arm_SetIntendedPosition","arm_SetPotentiometerSafety","arm_Advance","arm_Retract",
	"boom_SetCurrentVelocity","boom_SetIntendedPosition","boom_SetPotentiometerSafety","boom_Advance","boom_Retract",
	"bucket_SetCurrentVelocity","bucket_SetIntendedPosition","bucket_SetPotentiometerSafety","bucket_Advance","bucket_Retract",
	"clasp_SetCurrentVelocity","clasp_SetIntendedPosition","clasp_SetPotentiometerSafety","clasp_Advance","clasp_Retract",
	"arm_xpos_SetCurrentVelocity","arm_xpos_SetIntendedPosition","arm_xpos_SetPotentiometerSafety","arm_xpos_Advance","arm_xpos_Retract",
	"arm_ypos_SetCurrentVelocity","arm_ypos_SetIntendedPosition","arm_ypos_SetPotentiometerSafety","arm_ypos_Advance","arm_ypos_Retract",
	"bucket_angle_SetCurrentVelocity","bucket_angle_SetIntendedPosition","bucket_angle_SetPotentiometerSafety","bucket_angle_Advance","bucket_angle_Retract",
	"clasp_angle_SetCurrentVelocity","clasp_angle_SetIntendedPosition","clasp_angle_SetPotentiometerSafety","clasp_angle_Advance","clasp_angle_Retract",
	"TestAuton","Robot_FreezeArm","Robot_LockPosition","StopAuton"
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

		bool UsingCommon=false;
		{
			size_t tally=0;
			err = script.GetFieldTable("arm_common");
			if (!err)
			{
				tally++;
				m_CommonRotary.LoadFromScript(script);
				script.Pop();

				//delegate this value to each drive wheel for each that doesn't have its own override value
				for (size_t i=0;i<5;i++)
					m_RotaryProps[i]=m_CommonRotary;
			}

			err = script.GetFieldTable("arm_pos_common");
			if (!err)
			{
				m_CommonRotary.Init(); //reset the props for proper defaults
				tally++;
				m_CommonRotary.LoadFromScript(script);
				script.Pop();

				//delegate this value to each drive wheel for each that doesn't have its own override value
				for (size_t i=5;i<9;i++)
					m_RotaryProps[i]=m_CommonRotary;
			}
			UsingCommon=(tally==2);  //both commons must be loaded
			assert(tally==2 || tally==0);
		}

		for (size_t i=0;i<9;i++)
		{
			err = script.GetFieldTable(csz_Curivator_Robot_SpeedControllerDevices_Enum[i]);
			if (!err)
			{
				m_RotaryProps[i].LoadFromScript(script,UsingCommon);
				script.Pop();
			}
		}
		for (size_t i=9;i<11;i++)
		{
			err = script.GetFieldTable(csz_Curivator_Robot_SpeedControllerDevices_Enum[i]);
			if (!err)
			{
				m_RotaryProps[i].LoadFromScript(script,UsingCommon);
				script.Pop();
			}
		}

		SCRIPT_TEST_BOOL_YES(props.EnableArmAutoPosition,"enable_arm_auto_position");

		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct Curivator_Robot_Props::Autonomous_Properties &auton=m_CurivatorRobotProps.Autonomous_Props;
			{
				double fTest;
				err = script.GetField("auton_test", NULL, NULL,&fTest);
				if (!err)
					auton.AutonTest=(Curivator_Robot_Props::Autonomous_Properties::AutonType)((int)fTest);

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


__inline void CheckDisableSafety_Curivator(size_t index,bool &SafetyLock)
{
	//return;
	std::string SmartLabel=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
	SmartLabel[0]-=32; //Make first letter uppercase
	//This section is extra control of each system while 3D positioning is operational... enable for diagnostics
	std::string VoltageArmSafety=SmartLabel+"Disable";
	const bool bVoltageArmDisable=SmartDashboard::GetBoolean(VoltageArmSafety.c_str());
	if (bVoltageArmDisable)
		SafetyLock=true;
}

void Curivator_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	bool SafetyLock=SmartDashboard::GetBoolean("SafetyLock_Arm");
	bool SafetyLock_Drive=SmartDashboard::GetBoolean("SafetyLock_Drive");  //for center wheels
	double VoltageScalar=1.0;

	switch (index)
	{
	case Curivator_Robot::eArm:
	case Curivator_Robot::eTurret:
	case Curivator_Robot::eBoom:
	case Curivator_Robot::eBucket:
	case Curivator_Robot::eClasp:
		{
			#ifdef __EnableRobotArmDisable__
			CheckDisableSafety_Curivator(index,SafetyLock);
			#endif
			#ifdef Robot_TesterCode
			m_Potentiometer[index].UpdatePotentiometerVoltage(SafetyLock?0.0:Voltage);
			m_Potentiometer[index].TimeChange();  //have this velocity immediately take effect
			#endif
		}
		break;
	case Curivator_Robot::eWheel_CL:
	case Curivator_Robot::eWheel_CR:
		SafetyLock=SafetyLock_Drive;  //using the drive's check box
		#ifdef __EnableSafetyOnDrive__
		CheckDisableSafety_Curivator(index,SafetyLock);
		#endif
		#ifdef Robot_TesterCode
		if (SafetyLock)
			Voltage=0.0;
		m_Encoders[index-Curivator_Robot::eWheel_CL].UpdateEncoderVoltage(Voltage);
		m_Encoders[index-Curivator_Robot::eWheel_CL].TimeChange();
		#endif
		break;
	}
	if (index<Curivator_Robot::eDriveOffset)
	{
		VoltageScalar=m_RobotProps.GetRotaryProps(index).GetRotaryProps().VoltageScalar;
		Voltage*=VoltageScalar;
		std::string SmartLabel=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
		SmartLabel[0]-=32; //Make first letter uppercase
		if ((index == Curivator_Robot::eWheel_CL)||(index == Curivator_Robot::eWheel_CR))
			SmartLabel+="_";
		SmartLabel+="Voltage";
		SmartDashboard::PutNumber(SmartLabel.c_str(),Voltage);
		if (SafetyLock)
			Voltage=0.0;
		Victor_UpdateVoltage(index,Voltage);
	}
	#ifndef __UsingTankDrive__
	else
	{
		assert(index>=Curivator_Robot::eDriveOffset);
		m_pDriveRobotControl->UpdateRotaryVoltage(index-Curivator_Robot::eDriveOffset,Voltage);
	}
	#endif
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

Curivator_Robot_Control::Curivator_Robot_Control(bool UseSafety) : m_DriveRobotControl(UseSafety),m_pDriveRobotControl(&m_DriveRobotControl),
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
	case Curivator_Robot::eWheel_CL:
	case Curivator_Robot::eWheel_CR:
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
		break;
	case Curivator_Robot::eWheel_CL:
	case Curivator_Robot::eWheel_CR:
		Encoder_SetReverseDirection(index,m_RobotProps.GetRotaryProps(index).GetRotaryProps().EncoderReversed_Wheel);
		m_Encoders[index-Curivator_Robot::eWheel_CL].ResetPos();
		break;
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
	#ifdef __UsingTankDrive__
	Tank_Drive_Control_Interface *tank_interface=m_pDriveRobotControl;
	tank_interface->Initialize(props);
	#else
	Swerve_Drive_Control_Interface *drive_interface=m_pDriveRobotControl;
	drive_interface->Initialize(props);
	#endif

	const Curivator_Robot_Properties *robot_props=dynamic_cast<const Curivator_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		#ifdef Robot_TesterCode
		for (size_t index=0;index<Curivator_Robot_NoRobotArm;index++)
		{
			Rotary_Properties writeable_arm_props=robot_props->GetRotaryProps(index);
			m_Potentiometer[index].Initialize(&writeable_arm_props);
		}
		for (size_t index=0;index<2;index++)
		{
			Rotary_Properties drive=robot_props->GetRotaryProps(index+Curivator_Robot::eWheel_CL);
			drive.EncoderSimulationProps()=robot_props->GetEncoderSimulationProps();
			m_Encoders[index].Initialize(&drive);
		}
		#endif
	}
	
	//Note: Initialize may be called multiple times so we'll only set this stuff up on first run
	//For now... we'll use m_Compressor as the variable to determine first run, but perhaps this should be its own boolean here
	if (!m_Compressor)
	{
		#ifdef Robot_TesterCode
		SmartDashboard::PutBoolean("SafetyLock_Arm",false);
		#else
		SmartDashboard::PutBoolean("SafetyLock_Arm",true);
		#endif
		//This one one must also be called for the lists that are specific to the robot
		RobotControlCommon_Initialize(robot_props->Get_ControlAssignmentProps());
		//This may return NULL for systems that do not support it
		m_RoboRIO_Accelerometer=CreateBuiltInAccelerometer();
		m_Compressor=CreateCompressor();
		//Note: RobotControlCommon_Initialize() must occur before calling any encoder startup code
		//const double EncoderPulseRate=(1.0/360.0);
		//Encoder_SetDistancePerPulse(Curivator_Robot::eWinch,EncoderPulseRate);
		//Encoder_Start(Curivator_Robot::eWinch);

		for (size_t i=Curivator_Robot::eWheel_CL;i<=Curivator_Robot::eWheel_CR;i++)
		{
			double PulsesPerRevolution=robot_props->GetRotaryProps(i).GetRotaryProps().EncoderPulsesPerRevolution;
			if (PulsesPerRevolution==0.0) 
				PulsesPerRevolution=360.0;
			const double EncoderPulseRate=(1.0/PulsesPerRevolution);
			Encoder_SetReverseDirection(i,robot_props->GetRotaryProps(i).GetRotaryProps().EncoderReversed_Wheel);
			Encoder_SetDistancePerPulse(i,EncoderPulseRate);
			Encoder_Start(i);
		}

		ResetPos(); //must be called after compressor is created
		//Typically disabled, but may wish to enable initially
		#if 0
		for (size_t i=0;i<Curivator_Robot_NoArmRotarySystems;i++)
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
		#ifdef __EnableRobotArmDisable__
		for (size_t i=0;i<Curivator_Robot_NoArmRotarySystems;i++)
		{
			const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[i];
			string ContructedName;
			ContructedName=Prefix;
			ContructedName[0]-=32; //Make first letter uppercase
			ContructedName+="Disable";
			#ifdef Robot_TesterCode
			const bool DisableDefault=false;
			#else
			const bool DisableDefault=true;
			#endif
			SmartDashboard::PutBoolean(ContructedName.c_str(),DisableDefault);
		}
		#endif
		#ifdef __EnableSafetyOnDrive__
		for (size_t i=Curivator_Robot::eWheel_CL;i<=Curivator_Robot::eWheel_CR;i++)
		{
			const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[i];
			string ContructedName;
			ContructedName=Prefix;
			ContructedName[0]-=32; //Make first letter uppercase
			ContructedName+="Disable";
			#ifdef Robot_TesterCode
			const bool DisableDefault=false;
			#else
			const bool DisableDefault=true;
			#endif
			SmartDashboard::PutBoolean(ContructedName.c_str(),DisableDefault);
		}
		#endif
	}

}

void Curivator_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef Robot_TesterCode
	for (size_t index=0;index<5;index++)
		m_Potentiometer[index].SetTimeDelta(dTime_s);
	for (size_t index=0;index<2;index++)
		m_Encoders[index].SetTimeDelta(dTime_s);
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

//Note: Swerve drive voltage does not need to update victors through this
#ifdef __UsingTankDrive__
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
	m_pDriveRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}
#endif

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
	//const Curivator_Robot_Props &props=m_RobotProps.GetCurivatorRobotProps();

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
			//Potentiometer safety, if we lose wire connection it will be out of range in which case we turn on the safety (we'll see it turned on)
			if (raw_value>HiRange || raw_value<LowRange)
			{
				std::string SmartLabel=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
				SmartLabel[0]-=32; //Make first letter uppercase
				ContructedName=SmartLabel+"Disable";
				SmartDashboard::PutBoolean(ContructedName.c_str(),true);
			}

			//Now to compute the result... we start with the normalized value and give it the appropriate offset and scale
			//the offset is delegated in script in the final scale units, and the scale is the total range in radians
			result=PotentiometerRaw_To_Arm;
			//get scale
			const Ship_1D_Props &shipprops=m_RobotProps.GetRotaryProps(index).GetShip_1D_Props();
			//SmartDashboard::PutNumber("Arm_ScaleTest",shipprops.MaxRange-shipprops.MinRange);
			result*=shipprops.MaxRange-shipprops.MinRange;  //compute the total distance in radians
			result*=m_RobotProps.GetRotaryProps(index).GetRotaryProps().EncoderToRS_Ratio;
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
		case Curivator_Robot::eWheel_CL:
		case Curivator_Robot::eWheel_CR:
			{
				//double EncRate=Encoder_GetRate2(m_dTime_s);
				double EncRate=Encoder_GetRate(index);
				EncRate=m_KalFilter[index](EncRate);
				EncRate=m_Averager[index].GetAverage(EncRate);
				EncRate=IsZero(EncRate)?0.0:EncRate;

				const double EncVelocity=m_DriveRobotControl.RPS_To_LinearVelocity(EncRate);
				//Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", EncVelocity,RightVelocity);
				#ifdef Robot_TesterCode
				result=m_Encoders[index-Curivator_Robot::eWheel_CL].GetEncoderVelocity();
				#else
				result= EncVelocity;
				#endif
				const char * const Prefix=csz_Curivator_Robot_SpeedControllerDevices_Enum[index];
				string ContructedName;
				ContructedName=Prefix,ContructedName+="_Encoder";
				SmartDashboard::PutNumber(ContructedName.c_str(),result);
			}

			break;
		default:
			assert (index > Curivator_Robot::eClasp);
			//Note: the arm position indexes remain open so no work to be done here for them
			#ifndef __UsingTankDrive__
			if (index>=Curivator_Robot::eDriveOffset)
				result=m_pDriveRobotControl->GetRotaryCurrentPorV(index-Curivator_Robot::eDriveOffset);
			#endif
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

//#define __ShowUIGoal__
const double Curivator_Robot_UI_LinesVerticalOffset=200.0;
const double Curivator_Robot_UI_LinesHorizontalOffset=148.0;
Curivator_Robot_UI::Curivator_Robot_UI(const char EntityName[]) : Curivator_Robot(EntityName,this),Curivator_Robot_Control(),
		m_DriveUI(this)
{
	m_VertexData = new osg::Vec3Array;  //this will auto terminate
	m_VertexData->push_back(osg::Vec3(0,0,0)); 
	m_VertexData->push_back(osg::Vec3(0,0,0)); 
	m_VertexData->push_back(osg::Vec3(0,0,0)); 
	m_VertexData->push_back(osg::Vec3(0,0,0));
	m_VertexData->push_back(osg::Vec3(0,0,0));
	m_VertexData->push_back(osg::Vec3(0,0,0));
	m_VertexData->push_back(osg::Vec3(0,0,0));
	//m_VertexData->push_back(osg::Vec3(0,0,0));
	m_VertexData->push_back(osg::Vec3(0,0,0));
	m_VertexData->push_back(osg::Vec3(0,0,0));

	m_ColorData = new osg::Vec4Array;
	//Note... colors blend from point to point
	m_ColorData->push_back(osg::Vec4(0.49f, 0.62f, 0.75f, 1.0f) );
	m_ColorData->push_back(osg::Vec4(0.49f, 0.62f, 0.75f, 1.0f) );  //big arm end... boom start
	m_ColorData->push_back(osg::Vec4(0.49f, 0.62f, 0.75f, 1.0f) );  //rocker boom pivot
	m_ColorData->push_back(osg::Vec4(0.98f, 0.78f, 0.64f, 1.0f) ); //clasp start (bucket pivot)
	m_ColorData->push_back(osg::Vec4(0.98f, 0.78f, 0.64f, 1.0f) ); //clasp end
	m_ColorData->push_back(osg::Vec4(0.98f, 0.78f, 0.64f, 1.0f) ); //back to bucket pivot
	m_ColorData->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) ); // CoM
	//m_ColorData->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) ); // CoM to Bottom
	m_ColorData->push_back(osg::Vec4(0.49f, 0.62f, 0.75f, 1.0f) ); // bucket tip
	m_ColorData->push_back(osg::Vec4(0.98f, 0.78f, 0.64f, 1.0f) ); //bucket angle
}

void Curivator_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_DriveUI.TimeChange(dTime_s);
}

void Curivator_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_DriveUI.Initialize(em,props);
}

void Curivator_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_UI_Parent=parent;
	m_DriveUI.UI_Init(parent);
}
void Curivator_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_DriveUI.custom_update(nv,draw,parent_pos);
}
void Curivator_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_DriveUI.Text_SizeToUse(SizeToUse);
}

#include <osg/Geometry>
#include <osg/PositionAttitudeTransform>

//defined in RobotTester for quick variable manipulation
//double SineInfluence(double &rho,double freq_hz=0.1,double SampleRate=30.0,double amplitude=1.0);

void Curivator_Robot_UI::LinesUpdate::update(osg::NodeVisitor *nv, osg::Drawable *draw)
{

	//static double rho=0.0;
	//double testSample=((SineInfluence(rho)/2.0)+0.5) * 500.0;
	//(*m_pParent->m_VertexData)[1].set(testSample, testSample, 0.0);
	Curivator_Robot::BigArm &bigArm=m_pParent->m_Arm;
	Curivator_Robot::Boom &boom=m_pParent->m_Boom;
	Curivator_Robot::Bucket &bucket=m_pParent->m_Bucket;
	Curivator_Robot::Clasp &clasp=m_pParent->m_Clasp;
	(*m_pParent->m_VertexData)[1].set(bigArm.GetBigArmLength() * 10.0,bigArm.GetBigArmHeight() * 10.0,  0.0);
	(*m_pParent->m_VertexData)[2].set( boom.GetBoomLength() * 10.0,boom.GetBoomHeight() * 10.0, 0.0);
	//note: the boom length is really the rocker pivot point... cache the actual bucket pivot point here
	const double BucketPivotPoint_y=boom.GetBoomHeight()-bucket.GetBucket_globalBRP_BP_height();
	const double BucketPivotPoint_x=boom.GetBoomLength()+bucket.GetBucket_globalBRP_BP_distance();
	(*m_pParent->m_VertexData)[3].set( BucketPivotPoint_x * 10.0,BucketPivotPoint_y * 10.0, 0.0);
	(*m_pParent->m_VertexData)[4].set( clasp.GetClaspLength() * 10.0,clasp.GetClaspMidlineHeight() * 10.0, 0.0);
	//retrace to boom point for clasp
	(*m_pParent->m_VertexData)[5].set( BucketPivotPoint_x * 10.0,BucketPivotPoint_y * 10.0, 0.0);
	(*m_pParent->m_VertexData)[6].set( bucket.GetCoMDistance() * 10.0,bucket.GetCoMHeight() * 10.0, 0.0);
	//(*m_pParent->m_VertexData)[7].set( bucket.GetCoMDistance() * 10.0,bucket.GetBucketRoundEndHeight() * 10.0, 0.0);
	(*m_pParent->m_VertexData)[7].set( bucket.GetBucketLength() * 10.0,bucket.GetBucketTipHeight() * 10.0, 0.0);
	//finally we'll just compute the bucket angle here
	const double GlobalBucketAngle=bucket.GetBucketAngle();
	const double OpeningLength=10.0;  //it really is 10 inches in the original sketch of the bucket
	const double OpeningUpperPoint_y=bucket.GetBucketTipHeight()+(sin(GlobalBucketAngle)*OpeningLength);
	const double OpeningUpperPoint_x=bucket.GetBucketLength()+(cos(GlobalBucketAngle)*OpeningLength);
	(*m_pParent->m_VertexData)[8].set( OpeningUpperPoint_x * 10.0,OpeningUpperPoint_y * 10.0, 0.0);

	//perform blend as this will give us an intuitive idea of how far off the angle is
	{
		const double toleranceScale=3.0;  //we'll have to guage this on the actual bot
		const double errorRatio=std::min(m_pParent->GetBucketAngleContinuity()/toleranceScale,1.0);  //set to clip at 1.0 so its normalized
		const Vec3 errorColor(1,0,0);
		const Vec3 bucketTipColor(0.49,0.62,0.75);
		const Vec3 bucketAngleColor(0.98,0.78,0.64);
		const Vec3 BlendTipColor=errorColor*errorRatio + bucketTipColor*(1.0-errorRatio);
		const Vec3 BlendAngleColor=errorColor*errorRatio + bucketAngleColor*(1.0-errorRatio);
		(*m_pParent->m_ColorData)[7].set(BlendTipColor[0],BlendTipColor[1],BlendTipColor[2], 1.0f ); // bucket tip
		(*m_pParent->m_ColorData)[8].set(BlendAngleColor[0],BlendAngleColor[1],BlendAngleColor[2], 1.0f ); //bucket angle
	}

	
	

	draw->dirtyDisplayList();
	draw->dirtyBound();

	m_pParent->m_ArmTransform->setPosition( osg::Vec3(Curivator_Robot_UI_LinesHorizontalOffset,Curivator_Robot_UI_LinesVerticalOffset, 0.0) ); 

	//update circle too (we'll just borrow this callback)
	m_pParent->m_CircleTransform->setPosition( osg::Vec3( ((bucket.GetCoMDistance() * 10.0)+Curivator_Robot_UI_LinesHorizontalOffset),
		(bucket.GetCoMHeight() * 10.0)+Curivator_Robot_UI_LinesVerticalOffset, 0.0) ); 
	m_pParent->m_CircleTransform->setAttitude(osg::Quat(
		0.0	, osg::Vec3d(1,0,0),
		0.0	, osg::Vec3d(0,1,0),
		GlobalBucketAngle + DEG_2_RAD(30.58112256) - PI_2, osg::Vec3d(0,0,1)));

	#ifdef __ShowUIGoal__
	//and goal
	//Note: I kept the  + 0.0... for testing purposes to easily add an additional offset
	m_pParent->m_GoalTransform->setPosition( osg::Vec3( (((m_pParent->m_ArmXpos.GetPos_m() + 0.0) * 10.0)+Curivator_Robot_UI_LinesHorizontalOffset),
		(m_pParent->m_ArmYpos.GetPos_m() * 10.0)+Curivator_Robot_UI_LinesVerticalOffset, 0.0) ); 
	m_pParent->m_GoalTransform->setAttitude(osg::Quat(
		0.0	, osg::Vec3d(1,0,0),
		0.0	, osg::Vec3d(0,1,0),
		DEG_2_RAD(m_pParent->m_BucketAngle.GetPos_m()) , osg::Vec3d(0,0,1)));
	#endif
}

/* Create circle in XY plane. */
#define POLYGON_SIZE 256
//Keep for future reference
osg::ref_ptr<osg::Geometry> create_circle(float centerx, float centery, float rad)
{
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
	double theta, px, py;

	for(int i = 1; i <= POLYGON_SIZE; i++) {

		theta = 2.0 * M_PI/POLYGON_SIZE * i;
		px = centerx + rad * cos(theta);
		py = centery + rad * sin(theta);
		v->push_back(osg::Vec3(px, py, 0));
	}

	geom->setVertexArray( v.get() );
	geom->addPrimitiveSet(new osg::DrawArrays( osg::PrimitiveSet::LINE_LOOP, 0, POLYGON_SIZE) );

	return geom.get();
}

osg::ref_ptr<osg::Geometry> bucket_round_end()
{
	const float centerx=0;
	const float centery=0;
	const float rad=5*10.0;
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
	double theta, px, py;
	const double TopOfCircle_deg=30.18846043;  //to vertical
	//convert to ratio
	const double TopOfCircle_ratio=TopOfCircle_deg/360.0;
	const int TopOfCircleVertexCount= (int)(TopOfCircle_ratio*POLYGON_SIZE);
	for(int i = 1; i <= TopOfCircleVertexCount; i++) {

		theta = 2.0 * M_PI/POLYGON_SIZE * i;
		px = centerx + rad * cos(theta);
		py = centery + rad * sin(theta);
		v->push_back(osg::Vec3(px, py, 0));
	}
	//Make a straight line up as the next point
	const double InterfaceTipfromHorizontal=DEG_2_RAD(14.18776649+90.0);
	const double InterfaceTipLength=3.72263603 * 10.0;
	v->push_back(osg::Vec3(cos(InterfaceTipfromHorizontal)*InterfaceTipLength,sin(InterfaceTipfromHorizontal)*InterfaceTipLength, 0));
	const double BucketTipfromHorizontal=DEG_2_RAD(219.80557109);
	const double BucketTipLength=7.81024968 * 10.0;
	v->push_back(osg::Vec3(cos(BucketTipfromHorizontal)*BucketTipLength,sin(BucketTipfromHorizontal)*BucketTipLength, 0));

	const int StartCount=((270.0/360.0)*POLYGON_SIZE);  //e.g. 192
	for(int i = StartCount; i <= POLYGON_SIZE; i++) {

		theta = 2.0 * M_PI/POLYGON_SIZE * i;
		px = centerx + rad * cos(theta);
		py = centery + rad * sin(theta);
		v->push_back(osg::Vec3(px, py, 0));
	}

	geom->setVertexArray( v.get() );
	geom->addPrimitiveSet(new osg::DrawArrays( osg::PrimitiveSet::LINE_STRIP, 0, v->size()) );

	return geom.get();
}


void Curivator_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_DriveUI.UpdateScene(geode,AddOrRemove);
	if (AddOrRemove)
	{
		m_DriveUI.UpdateScene(geode,AddOrRemove);
		osg::Geometry* linesGeom = new osg::Geometry();// is my geometry 
		osg::DrawArrays* drawArrayLines = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP); 
		linesGeom->addPrimitiveSet(drawArrayLines); 
		//osg::Vec3Array* vertexData = new osg::Vec3Array; 
		linesGeom->setVertexArray(m_VertexData); 
		linesGeom->setColorArray(m_ColorData);
		linesGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::Geode* ArmGeode = new osg::Geode;
		ArmGeode->addDrawable(linesGeom);
		//geode->addDrawable(linesGeom);

		//vertexData->push_back(osg::Vec3(0,0,0)); 
		//vertexData->push_back(osg::Vec3(500,200,0)); 
		m_LinesUpdate=new LinesUpdate(this);
		linesGeom->setUpdateCallback(m_LinesUpdate);
		drawArrayLines->setFirst(0); 
		drawArrayLines->setCount(m_VertexData->size());
		//now for the transform
		osg::ref_ptr<osg::Group> rootnode=m_UI_Parent->GetParent()->GetRootNode();
		m_ArmTransform=new osg::PositionAttitudeTransform();
		rootnode->addChild(m_ArmTransform);
		m_ArmTransform->addChild(ArmGeode);

		//add a circle
		//m_Circle=create_circle(0,0,5*10.0);
		m_Circle=bucket_round_end();
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) );
		m_Circle->setColorArray(colors);
		m_Circle->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);

		osg::Geode* CircleGeode = new osg::Geode;
		CircleGeode->addDrawable(m_Circle);
		// Declare and initialize a transform node.
		m_CircleTransform = new osg::PositionAttitudeTransform();

		//Node *Test=CircleTransform;
		// Use the 'addChild' method of the osg::Group class to
		// add the transform as a child of the root node and the
		// pyramid node as a child of the transform.
		rootnode->addChild(m_CircleTransform);
		m_CircleTransform->addChild(CircleGeode);

		// Declare and initialize a Vec3 instance to change the
		// position of the tank model in the scene
		m_CircleTransform->setPosition( osg::Vec3(50,50,0) ); 

		#ifdef __ShowUIGoal__
		//setup the goal
		osg::Geode *GoalGeode = new osg::Geode;
		osg::Geometry *goalGeom = new osg::Geometry();
		osg::DrawArrays *goal_line = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP); 
		goalGeom->addPrimitiveSet(goal_line);
		osg::Vec3Array *goalVertexData = new osg::Vec3Array; 
		goalVertexData->push_back(osg::Vec3(0,0,0)); 
		const double goalLineLength=9.95569848;
		goalVertexData->push_back(osg::Vec3(goalLineLength * 10.0,0,0)); 
		goalGeom->setVertexArray(goalVertexData);
		goal_line->setFirst(0); 
		goal_line->setCount(goalVertexData->size());
		osg::Vec4Array* Goalcolor = new osg::Vec4Array;
		Goalcolor->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) );
		goalGeom->setColorArray(Goalcolor);
		goalGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		GoalGeode->addDrawable(goalGeom);
		m_GoalTransform=new osg::PositionAttitudeTransform();
		rootnode->addChild(m_GoalTransform);
		m_GoalTransform->addChild(GoalGeode);
		m_GoalTransform->setPosition( osg::Vec3(50,50,0) );
		#endif
	}
}

#endif
