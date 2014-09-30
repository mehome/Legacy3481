#include "stdafx.h"
#include "Robot_Tester.h"

#ifdef Robot_TesterCode
namespace Robot_Tester
{
	#include "Tank_Robot_UI.h"
	#include "CommonUI.h"
	#include "FRC2014_Robot.h"
}

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

#else

#include "FRC2014_Robot.h"
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
 /*														FRC_2014_Robot::Turret														*/
/***********************************************************************************************************************************/

FRC_2014_Robot::Turret::Turret(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 	m_pParent(parent),m_Velocity(0.0)
{
}


void FRC_2014_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Turret::Turret_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Turret::Turret_SetRequestedVelocity);
	}
}

void FRC_2014_Robot::Turret::TimeChange(double dTime_s)
{
	m_Velocity=0.0;

	//Note: GetRequestedVelocity tests for manual override, giving the tolerance a bit more grace for joystick dead-zone
	const bool IsTargeting=m_pParent->IsBallTargeting();
	//const bool IsTargeting=false;
	if (IsTargeting)
	{
			//if (m_AutoDriveState==eAutoDrive_YawOnly)		
			{
				//the POV turning call relative offsets adjustments here... the yaw is the opposite side so we apply the negative sign
				//if (fabs(m_YawAngle)>m_RobotProps.GetFRC2014RobotProps().YawTolerance)
				if (fabs(m_pParent->m_YawAngle)>DEG_2_RAD(1))
					m_pParent->m_controller->GetUIController_RW()->Turn_RelativeOffset(m_pParent->m_YawAngle,false);
				m_pParent->m_YawAngle=0;
			}
			//else if (m_AutoDriveState==eAutoDrive_FullAuto)
			//{
			//	bool HitWayPoint;
			//	{
			//		const double tolerance=GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			//		const Vec2d &currPos = GetPos_m();
			//		double position_delta=(m_TargetOffset-currPos).length();
			//		HitWayPoint=position_delta<tolerance;
			//	}
			//	if (!HitWayPoint)
			//	{
			//		Vec2d Temp(0,0);
			//		GetController()->DriveToLocation(m_TargetOffset, m_TargetOffset, 1.0, dTime_s,&Temp);
			//	}
			//	else
			//		GetController()->SetShipVelocity(0.0);
			//}

	}
}

void FRC_2014_Robot::Turret::ResetPos()
{
	m_Velocity=0.0;
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2014_Robot::PitchRamp::PitchRamp(FRC_2014_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),m_Velocity(0.0)
{
}


void FRC_2014_Robot::PitchRamp::TimeChange(double dTime_s)
{
	m_Velocity=0.0;
}

void FRC_2014_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::PitchRamp::Pitch_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::PitchRamp::Pitch_SetRequestedVelocity);
	}
}

void FRC_2014_Robot::PitchRamp::ResetPos()
{
	m_Velocity=0.0;
}

  /***********************************************************************************************************************************/
 /*														FRC_2014_Robot::Winch														*/
/***********************************************************************************************************************************/


class WinchFireManager : public AtomicGoal
{
private:
	FRC_2014_Robot &m_Robot;
	bool m_IsFireButtonDown;

	class SetUpProps
	{
	protected:
		WinchFireManager *m_Parent;
		FRC_2014_Robot &m_Robot;
		Entity2D_Kind::EventMap &m_EventMap;
	public:
		SetUpProps(WinchFireManager *Parent)	: m_Parent(Parent),m_Robot(Parent->m_Robot),m_EventMap(*m_Robot.GetEventMap())
		{	
		}
	};

	class Fire : public AtomicGoal, public SetUpProps
	{
	private:
		bool m_IsOn;
	public:
		Fire(WinchFireManager *Parent, bool On)	: SetUpProps(Parent),m_IsOn(On) {	m_Status=eInactive;	}
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s)
		{
			ActivateIfInactive();
			m_EventMap.EventOnOff_Map["Winch_Fire"].Fire(m_IsOn);
			m_Status=eCompleted;
			if (m_IsOn==false)
				m_Parent->m_Robot.SetWinchFireSequenceActive(false);  //the catapult should be disengaged long enough to auto-close intake
			return m_Status;
		}
	};

	class WaitWhileButtonDown : public AtomicGoal, public SetUpProps
	{
	public:
		WaitWhileButtonDown(WinchFireManager *Parent)	: SetUpProps(Parent) {	m_Status=eInactive;	}
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s)
		{
			m_Status=m_Parent->m_IsFireButtonDown?eActive:eCompleted;
			return m_Status;
		}
	};

	class EnsureArmDown : public AtomicGoal, public SetUpProps
	{
		public:
			EnsureArmDown(WinchFireManager *Parent)	: SetUpProps(Parent) {	m_Status=eInactive;	}
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				m_Status=m_Parent->m_Robot.GetIsArmDown()?eCompleted:eActive;
				return m_Status;
			}
	};

	class Fire_Sequence : public Generic_CompositeGoal, public SetUpProps
	{
	public:
		Fire_Sequence(WinchFireManager *Parent)	: Generic_CompositeGoal(true),SetUpProps(Parent) {	m_Status=eInactive;	}
		virtual void Activate()
		{
			AddSubgoal(new Fire(m_Parent,false));
			//AddSubgoal(new Goal_Wait(.500));
			MultitaskGoal *WaitingForRelease=new MultitaskGoal(true);  //wait for time and release of button
			WaitingForRelease->AddGoal(new Goal_Wait(.500));
			WaitingForRelease->AddGoal(new WaitWhileButtonDown(m_Parent));
			AddSubgoal(WaitingForRelease);
			AddSubgoal(new Fire(m_Parent,true));
			if (m_Parent->m_Robot.GetAutoDeployIntake())
				AddSubgoal(new EnsureArmDown(m_Parent));
			m_Status=eActive;
		}
	} *m_Fire_Sequence;
public:
	WinchFireManager(FRC_2014_Robot &robot) : m_Robot(robot),m_IsFireButtonDown(false)
	{
		m_Fire_Sequence=new Fire_Sequence(this);
		m_Status=eInactive;
	}
	~WinchFireManager()
	{
		if (m_Fire_Sequence)
		{
			delete m_Fire_Sequence;
			m_Fire_Sequence=NULL;
		}
	}
	void Activate()
	{
		//we'll do just simply discard for any state besides inactive
		if (m_Status!=eActive)
		{
			m_Status=eActive;
			m_Fire_Sequence->Activate();
			m_Robot.SetWinchFireSequenceActive(true);
		}
	}
	Goal_Status Process(double dTime_s)
	{
		if (m_IsFireButtonDown)
			Activate();
		if (m_Status==eActive)
			m_Status=m_Fire_Sequence->Process(dTime_s);
		return m_Status;
	}

	void Terminate() 
	{
		if (m_Fire_Sequence->GetStatus()!=eInactive)
			m_Fire_Sequence->Terminate();
		m_Status=eFailed;
	}
	void SetFireButton(bool ReleaseClutch)
	{
		m_IsFireButtonDown=ReleaseClutch;
	}
};


FRC_2014_Robot::Winch::Winch(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("Winch",robot_control,eWinch),m_pParent(parent),m_WinchFireManager(NULL),m_Advance(false)
{
	m_WinchFireManager=new WinchFireManager(*parent);
}

FRC_2014_Robot::Winch::~Winch()
{
	if (m_WinchFireManager)  //check for NULL on other platforms
	{
		delete m_WinchFireManager;
		m_WinchFireManager=NULL;
	}
}

void FRC_2014_Robot::Winch::Advance(bool on)
{
	m_Advance=on;
}

void FRC_2014_Robot::Winch::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	//const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button value
	if (m_Advance)
		SetCurrentLinearAcceleration(Accel);

	__super::TimeChange(dTime_s);
	//Trying to get away from debug outs... however keeping this around for reference to how the gear ratios are used
	//#ifdef Robot_TesterCode
	//const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	//const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
	//double Pos_m=GetPos_m();
	//DOUT4("Arm=%f Angle=%f",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio));
	//#endif

	const FRC_2014_Robot_Properties &RobotProps=m_pParent->GetRobotProps();
	if (RobotProps.GetWinchProps().GetRotaryProps().LoopState==Rotary_Props::eNone)
	{
		const FRC_2014_Robot_Props &props=RobotProps.GetFRC2014RobotProps();
		const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
		SmartDashboard::PutNumber("Catapult_Angle",90.0-(RAD_2_DEG(GetPos_m()*c_GearToArmRatio)));
	}
	m_WinchFireManager->Process(dTime_s);
}


double FRC_2014_Robot::Winch::PotentiometerRaw_To_Arm_r(double raw) const
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=props.Catapult_Robot_Props.PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}

void FRC_2014_Robot::Winch::SetChipShot()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Catapult_Robot_Props.ChipShotAngle * props.Catapult_Robot_Props.ArmToGearRatio);
}
void FRC_2014_Robot::Winch::SetGoalShot()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition( props.Catapult_Robot_Props.GoalShotAngle * props.Catapult_Robot_Props.ArmToGearRatio);
}
void FRC_2014_Robot::Winch::Fire_Catapult(bool ReleaseClutch)
{
	m_pParent->m_RobotControl->OpenSolenoid(eReleaseClutch,ReleaseClutch);
	//once released the encoder and position will be zero
	if (ReleaseClutch)
	{
		ResetPos();
		m_pParent->m_RobotControl->Reset_Rotary(eWinch);
	}
}
void FRC_2014_Robot::Winch::Winch_FireManager(bool ReleaseClutch)
{
	WinchFireManager *fm=dynamic_cast<WinchFireManager *>(m_WinchFireManager);
	fm->SetFireButton(ReleaseClutch);
}

bool FRC_2014_Robot::Winch::GetAutoDeployIntake() const
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	return props.Catapult_Robot_Props.AutoDeployArm;
}

bool FRC_2014_Robot::Winch::DidHitMaxLimit() const
{
	return m_pParent->m_RobotControl->GetBoolSensorState(eCatapultLimit);
}

void FRC_2014_Robot::Winch::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap();
	if (Bind)
	{
		em->EventValue_Map["Winch_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Winch::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Winch_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2014_Robot::Winch::SetPotentiometerSafety);
		
		em->Event_Map["Winch_SetChipShot"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::SetChipShot);
		em->Event_Map["Winch_SetGoalShot"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::SetGoalShot);

		em->EventOnOff_Map["Winch_Advance"].Subscribe(ehl,*this, &FRC_2014_Robot::Winch::Advance);

		em->EventOnOff_Map["Winch_Fire"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::Fire_Catapult);
		em->EventOnOff_Map["Winch_FireManager"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::Winch_FireManager);
	}
	else
	{
		em->EventValue_Map["Winch_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Winch::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Winch_SetPotentiometerSafety"].Remove(*this, &FRC_2014_Robot::Winch::SetPotentiometerSafety);

		em->Event_Map["Winch_SetChipShot"].Remove(*this, &FRC_2014_Robot::Winch::SetChipShot);
		em->Event_Map["Winch_SetGoalShot"].Remove(*this, &FRC_2014_Robot::Winch::SetGoalShot);

		em->EventOnOff_Map["Winch_Advance"].Remove(*this, &FRC_2014_Robot::Winch::Advance);

		em->EventOnOff_Map["Winch_Fire"]  .Remove(*this, &FRC_2014_Robot::Winch::Fire_Catapult);
		em->EventOnOff_Map["Winch_FireManager"]  .Remove(*this, &FRC_2014_Robot::Winch::Winch_FireManager);
	}
}
  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot::Intake_Arm														*/
/***********************************************************************************************************************************/

class IntakeArmManager : public AtomicGoal
{
private:
	FRC_2014_Robot &m_Robot;
	bool m_IsArmButtonDown;
	bool m_WinchFireSequenceActive;

	class SetUpProps
	{
	protected:
		IntakeArmManager *m_Parent;
		FRC_2014_Robot &m_Robot;
		Entity2D_Kind::EventMap &m_EventMap;
	public:
		SetUpProps(IntakeArmManager *Parent)	: m_Parent(Parent),m_Robot(Parent->m_Robot),m_EventMap(*m_Robot.GetEventMap())
		{	
		}
	};

	class WaitWhileButtonDown : public AtomicGoal, public SetUpProps
	{
	public:
		WaitWhileButtonDown(IntakeArmManager *Parent)	: SetUpProps(Parent) {	m_Status=eInactive;	}
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s)
		{
			m_Status=(m_Parent->m_IsArmButtonDown||m_Parent->m_WinchFireSequenceActive)?eActive:eCompleted;
			return m_Status;
		}
	};


	class Intake_Deploy : public AtomicGoal, public SetUpProps
	{
	private:
		bool m_IsOn;
	public:
		Intake_Deploy(IntakeArmManager *Parent, bool On)	: SetUpProps(Parent),m_IsOn(On) {	m_Status=eInactive;	}
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s)
		{
			ActivateIfInactive();
			m_EventMap.EventOnOff_Map["Robot_CatcherShooter"].Fire(m_IsOn);
			m_Status=eCompleted;
			return m_Status;
		}
	};

	class Intake_Sequence : public Generic_CompositeGoal, public SetUpProps
	{
	public:
		Intake_Sequence(IntakeArmManager *Parent)	: Generic_CompositeGoal(true),SetUpProps(Parent) {	m_Status=eInactive;	}
		virtual void Activate()
		{
			AddSubgoal(new Intake_Deploy(m_Parent,false));
			AddSubgoal(new WaitWhileButtonDown(m_Parent));
			AddSubgoal(new Intake_Deploy(m_Parent,true));
			m_Status=eActive;
		}
	} *m_Intake_Sequence;
public:
	IntakeArmManager(FRC_2014_Robot &robot) : m_Robot(robot),m_IsArmButtonDown(false),m_WinchFireSequenceActive(false)
	{
		m_Intake_Sequence=new Intake_Sequence(this);
		m_Status=eInactive;
	}
	~IntakeArmManager()
	{
		if (m_Intake_Sequence)
		{
			delete m_Intake_Sequence;
			m_Intake_Sequence=NULL;
		}
	}
	void Activate()
	{
		//we'll do just simply discard for any state besides inactive
		if (m_Status!=eActive)
		{
			m_Status=eActive;
			m_Intake_Sequence->Activate();
		}
	}
	Goal_Status Process(double dTime_s)
	{
		if (m_IsArmButtonDown || m_WinchFireSequenceActive)
			Activate();
		if (m_Status==eActive)
			m_Status=m_Intake_Sequence->Process(dTime_s);
		return m_Status;
	}

	void Terminate() 
	{
		if (m_Intake_Sequence->GetStatus()!=eInactive)
			m_Intake_Sequence->Terminate();
		m_Status=eFailed;
	}
	void SetIntakeButton(bool DeployArm)
	{
		m_IsArmButtonDown=DeployArm;
	}
	void SetWinchFireSequenceActive(bool WinchFireSequenceState)
	{
		m_WinchFireSequenceActive=WinchFireSequenceState;
	}
};

FRC_2014_Robot::Intake_Arm::Intake_Arm(FRC_2014_Robot *parent) : m_pParent(parent),m_ArmTimer(0.0)
{
	m_IntakeArmManager=new IntakeArmManager(*parent);
}

FRC_2014_Robot::Intake_Arm::~Intake_Arm()
{
	if (m_IntakeArmManager)  //check for NULL on other platforms
	{
		delete m_IntakeArmManager;
		m_IntakeArmManager=NULL;
	}
}

void FRC_2014_Robot::Intake_Arm::SetIntakeButton(bool DeployArm)
{
	IntakeArmManager *iam=dynamic_cast<IntakeArmManager *>(m_IntakeArmManager);
	iam->SetIntakeButton(DeployArm);
}

void FRC_2014_Robot::Intake_Arm::SetWinchFireSequenceActive(bool WinchFireSequenceState)
{
	if (m_pParent->GetAutoDeployIntake())
	{
		IntakeArmManager *iam=dynamic_cast<IntakeArmManager *>(m_IntakeArmManager);
		iam->SetWinchFireSequenceActive(WinchFireSequenceState);
	}
}

const double c_Intake_Arm_DownWait_Threshold=0.250;  //250 ms for arm to deploy (could script this if necessary)

bool FRC_2014_Robot::Intake_Arm::GetIsArmDown() const 
{
	return m_ArmTimer>=c_Intake_Arm_DownWait_Threshold;
}

void FRC_2014_Robot::Intake_Arm::TimeChange(double dTime_s)
{
	Goal::Goal_Status status=m_IntakeArmManager->Process(dTime_s);
	if (status==Goal::eActive)
	{
		if (m_ArmTimer<c_Intake_Arm_DownWait_Threshold)
			m_ArmTimer+=dTime_s;
	}
	else
		m_ArmTimer=0.0;
	//SmartDashboard::PutNumber("TestArmDownTime",m_ArmTimer);
}
void FRC_2014_Robot::Intake_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap();
	if (Bind)
	{
		em->EventOnOff_Map["IntakeArm_DeployManager"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetIntakeButton);
	}
	else
	{
		em->EventOnOff_Map["IntakeArm_DeployManager"]  .Remove(*this, &FRC_2014_Robot::Intake_Arm::SetIntakeButton);
	}

}


#if 0
FRC_2014_Robot::Intake_Arm::Intake_Arm(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("IntakeArm",robot_control,eIntakeArm1),m_pParent(parent),m_Advance(false),m_Retract(false)
{
}


void FRC_2014_Robot::Intake_Arm::Advance(bool on)
{
	m_Advance=on;
}
void FRC_2014_Robot::Intake_Arm::Retract(bool on)
{
	m_Retract=on;
}

void FRC_2014_Robot::Intake_Arm::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Advance ^ m_Retract)
		SetCurrentLinearAcceleration(m_Advance?Accel:-Brake);

	__super::TimeChange(dTime_s);
	//Since we have no potentiometer we can feedback where we think the arm angle is from the entity
	SmartDashboard::PutNumber("IntakeArm_Angle",RAD_2_DEG(GetPos_m()));
}


double FRC_2014_Robot::Intake_Arm::PotentiometerRaw_To_Arm_r(double raw) const
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=props.Intake_Robot_Props.PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}

void FRC_2014_Robot::Intake_Arm::SetStowed()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Intake_Robot_Props.Stowed_Angle);
}

void FRC_2014_Robot::Intake_Arm::SetDeployed()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Intake_Robot_Props.Deployed_Angle);
}

void FRC_2014_Robot::Intake_Arm::SetSquirt()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Intake_Robot_Props.Squirt_Angle);
}

bool FRC_2014_Robot::Intake_Arm::DidHitMinLimit() const
{
	return m_pParent->m_RobotControl->GetBoolSensorState(eIntakeMin1);
}

bool FRC_2014_Robot::Intake_Arm::DidHitMaxLimit() const
{
	return m_pParent->m_RobotControl->GetBoolSensorState(eIntakeMax1);
}

void FRC_2014_Robot::Intake_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["IntakeArm_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["IntakeArm_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::SetPotentiometerSafety);
		
		em->Event_Map["IntakeArm_SetStowed"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetStowed);
		em->Event_Map["IntakeArm_SetDeployed"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetDeployed);
		em->Event_Map["IntakeArm_SetSquirt"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetSquirt);

		em->EventOnOff_Map["IntakeArm_Advance"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::Advance);
		em->EventOnOff_Map["IntakeArm_Retract"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::Retract);
	}
	else
	{
		em->EventValue_Map["IntakeArm_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["IntakeArm_SetPotentiometerSafety"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetPotentiometerSafety);

		em->Event_Map["IntakeArm_SetStowed"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetStowed);
		em->Event_Map["IntakeArm_SetDeployed"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetDeployed);
		em->Event_Map["IntakeArm_SetSquirt"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetSquirt);

		em->EventOnOff_Map["IntakeArm_Advance"].Remove(*this, &FRC_2014_Robot::Intake_Arm::Advance);
		em->EventOnOff_Map["IntakeArm_Retract"].Remove(*this, &FRC_2014_Robot::Intake_Arm::Retract);
	}
}

#endif
  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot::Intake_Rollers													*/
/***********************************************************************************************************************************/

FRC_2014_Robot::Intake_Rollers::Intake_Rollers(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) :
	Rotary_Velocity_Control("IntakeRollers",robot_control,eRollers),m_pParent(parent),m_Grip(false),m_Squirt(false)
{
}

void FRC_2014_Robot::Intake_Rollers::TimeChange(double dTime_s)
{
	SetRequestedVelocity_FromNormalized(m_Velocity);
	m_Velocity=0.0;

	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Grip ^ m_Squirt)
		SetCurrentLinearAcceleration(m_Grip?Accel:-Brake);

	__super::TimeChange(dTime_s);
}

void FRC_2014_Robot::Intake_Rollers::Grip(bool on)
{
	m_Grip=on;
}

void FRC_2014_Robot::Intake_Rollers::Squirt(bool on)
{
	m_Squirt=on;
}

void FRC_2014_Robot::Intake_Rollers::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["IntakeRollers_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Rollers::Intake_Rollers_SetRequestedVelocity);
		em->EventOnOff_Map["IntakeRollers_Grip"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Rollers::Grip);
		em->EventOnOff_Map["IntakeRollers_Squirt"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Rollers::Squirt);
	}
	else
	{
		em->EventValue_Map["IntakeRollers_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Intake_Rollers::Intake_Rollers_SetRequestedVelocity);
		em->EventOnOff_Map["IntakeRollers_Grip"]  .Remove(*this, &FRC_2014_Robot::Intake_Rollers::Grip);
		em->EventOnOff_Map["IntakeRollers_Squirt"]  .Remove(*this, &FRC_2014_Robot::Intake_Rollers::Squirt);
	}
}

  /***********************************************************************************************************************************/
 /*															FRC_2014_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;

FRC_2014_Robot::FRC_2014_Robot(const char EntityName[],FRC_2014_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), 
		m_Turret(this,robot_control),m_PitchRamp(this,robot_control),m_Winch(this,robot_control),m_Intake_Arm(this),
		m_Intake_Rollers(this,robot_control),m_DefensiveKeyPosition(Vec2D(0.0,0.0)),m_LatencyCounter(0.0),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),
		m_AutonPresetIndex(0),m_YawAngle(0.0),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_SetLowGear(false),m_SetDriverOverride(false),
		m_IsBallTargeting(false)
{
	//ensure the variables are initialized before calling get
	SmartDashboard::PutNumber("X Position",0.0);
	SmartDashboard::PutNumber("Y Position",0.0);
	SmartDashboard::PutBoolean("Main_Is_Targeting",false);
	//Note: The processing vision is setup to use these same variables for both tracking processes (i.e. front and rear camera) we should only need to be tracking one of them at a time
	//We may want to add a prefix window to identify which window they are coming from, but this may not be necessary.
}

void FRC_2014_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2014_Robot_Properties *RobotProps=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)

	//set to the default key position
	//const FRC_2014_Robot_Props &robot2014props=RobotProps->GetFRC2014RobotProps();
	m_Winch.Initialize(em,RobotProps?&RobotProps->GetWinchProps():NULL);
	//m_Intake_Arm.Initialize(em,RobotProps?&RobotProps->GetIntake_ArmProps():NULL);
	m_Intake_Rollers.Initialize(em,RobotProps?&RobotProps->GetIntakeRollersProps():NULL);
}
void FRC_2014_Robot::ResetPos()
{
	__super::ResetPos();
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	//TODO this is tacky... will have better low gear method soon
	if (!GetBypassPosAtt_Update())
	{
		m_Winch.ResetPos();
		//m_Intake_Arm.ResetPos();
		SetLowGear(true);
	}
	m_Intake_Rollers.ResetPos();  //ha pedantic
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

void FRC_2014_Robot::TimeChange(double dTime_s)
{
	//const FRC_2014_Robot_Props &robot_props=m_RobotProps.GetFRC2014RobotProps();

	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.TimeChange(dTime_s);
	m_PitchRamp.TimeChange(dTime_s);
	m_Winch.AsEntity1D().TimeChange(dTime_s);
	m_Intake_Arm.TimeChange(dTime_s);
	m_Intake_Rollers.AsEntity1D().TimeChange(dTime_s);

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
	const double XOffset=SmartDashboard::GetNumber("X Position");
	
	using namespace VisionConversion;

	SmartDashboard::PutBoolean("IsBallTargeting",IsBallTargeting());
	if (IsBallTargeting())
	{
		const FRC_2014_Robot_Props::BallTargeting &ball_props=m_RobotProps.GetFRC2014RobotProps().BallTargeting_Props;
		const double CurrentYaw=GetAtt_r();
		//the POV turning call relative offsets adjustments here... the yaw is the opposite side so we apply the negative sign
		#ifndef __UseFileTargetTracking__
		const double SmoothingYaw=ball_props.CameraOffsetScalar;
		//const double NewYaw=CurrentYaw+atan(yaw/distance);
		const double NewYaw=CurrentYaw+(atan(XOffset * c_AspectRatio_recip * c_ez_x)*SmoothingYaw);
		#else
		//Enable this for playback of file since it cannot really cannot control the pitch
		//const double NewYaw=atan(yaw/distance)-GetAtt_r();
		const double NewYaw=atan(XOffset * c_AspectRatio_recip * c_ez_x)-GetAtt_r();
		#endif

		//Use precision tolerance asset to determine whether to make the change
		//m_YawAngle=(fabs(NewYaw-CurrentYaw)>m_RobotProps.GetTurretProps().GetRotaryProps().PrecisionTolerance)?NewYaw:CurrentYaw;
		const double PrecisionTolerance=DEG_2_RAD(0.5); //TODO put in properties try to keep as low as possible if we need to drive straight
		const double YawAngle=NormalizeRotation2((fabs(NewYaw-CurrentYaw)>PrecisionTolerance)?NewYaw:CurrentYaw);
		//Note: limits will be solved at ship level
		SmartDashboard::PutNumber("Ball Tracking Yaw Angle",RAD_2_DEG(YawAngle-CurrentYaw));
		#if 1
		//if (IsBallTargeting())
		{
			m_LatencyCounter+=dTime_s;
			if ((double)m_LatencyCounter>(ball_props.LatencyCounterThreshold))
			{
				m_YawAngle=YawAngle-CurrentYaw;
				m_LatencyCounter=0.0;
			}
			else
				m_YawAngle=0.0;
		}
		#else
		m_YawAngle=YawAngle;
		#endif
	}

	bool LED_OnState=SmartDashboard::GetBoolean("Main_Is_Targeting");
	m_RobotControl->UpdateVoltage(eCameraLED,LED_OnState?1.0:0.0);
}

const FRC_2014_Robot_Properties &FRC_2014_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

FRC_2014_Robot_Props::Autonomous_Properties &FRC_2014_Robot::GetAutonProps()
{
	return m_RobotProps.GetFRC2014RobotProps_rw().Autonomous_Props;
}

bool FRC_2014_Robot::GetCatapultLimit() const
{
	return m_RobotControl->GetBoolSensorState(eCatapultLimit);
}

void FRC_2014_Robot::SetLowGear(bool on) 
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

void FRC_2014_Robot::SetLowGearValue(double Value)
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

void FRC_2014_Robot::SetCatcherShooter(bool on)
{
	m_CatcherShooter=on;
	m_RobotControl->OpenSolenoid(eCatcherShooter,on);
}

void FRC_2014_Robot::SetCatcherIntake(bool on)
{
	m_CatcherIntake=on;
	m_RobotControl->OpenSolenoid(eCatcherIntake,on);
}


void FRC_2014_Robot::SetDriverOverride(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//I am not yet certain if this if statement is necessary... I'll have to check what all is involved in setting a variable that is already equal
	if (m_SetDriverOverride!=on)
		SmartDashboard::PutBoolean("DriverOverride",on);
	m_SetDriverOverride=on;
}

void FRC_2014_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Robot_SetLowGear"].Subscribe(ehl, *this, &FRC_2014_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"].Subscribe(ehl, *this, &FRC_2014_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"].Subscribe(ehl, *this, &FRC_2014_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Subscribe(ehl,*this, &FRC_2014_Robot::SetLowGearValue);
		em->EventOnOff_Map["Robot_SetDriverOverride"].Subscribe(ehl, *this, &FRC_2014_Robot::SetDriverOverride);
		em->EventOnOff_Map["Robot_BallTargeting"].Subscribe(ehl, *this, &FRC_2014_Robot::SetBallTargeting);
		em->Event_Map["Robot_BallTargeting_On"].Subscribe(ehl, *this, &FRC_2014_Robot::SetBallTargetingOn);
		em->Event_Map["Robot_BallTargeting_Off"].Subscribe(ehl, *this, &FRC_2014_Robot::SetBallTargetingOff);

		em->EventOnOff_Map["Robot_CatcherShooter"].Subscribe(ehl, *this, &FRC_2014_Robot::SetCatcherShooter);
		em->Event_Map["Robot_CatcherShooter_On"].Subscribe(ehl, *this, &FRC_2014_Robot::SetCatcherShooterOn);
		em->Event_Map["Robot_CatcherShooter_Off"].Subscribe(ehl, *this, &FRC_2014_Robot::SetCatcherShooterOff);

		em->EventOnOff_Map["Robot_CatcherIntake"].Subscribe(ehl, *this, &FRC_2014_Robot::SetCatcherIntake);
		em->Event_Map["Robot_CatcherIntake_On"].Subscribe(ehl, *this, &FRC_2014_Robot::SetCatcherIntakeOn);
		em->Event_Map["Robot_CatcherIntake_Off"].Subscribe(ehl, *this, &FRC_2014_Robot::SetCatcherIntakeOff);
		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"].Subscribe(ehl, *this, &FRC_2014_Robot::TestAutonomous);
		em->Event_Map["Complete"].Subscribe(ehl,*this,&FRC_2014_Robot::GoalComplete);
		#endif
	}
	else
	{
		em->EventOnOff_Map["Robot_SetLowGear"]  .Remove(*this, &FRC_2014_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"]  .Remove(*this, &FRC_2014_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"]  .Remove(*this, &FRC_2014_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Remove(*this, &FRC_2014_Robot::SetLowGearValue);
		em->EventOnOff_Map["Robot_SetDriverOverride"]  .Remove(*this, &FRC_2014_Robot::SetDriverOverride);
		em->EventOnOff_Map["Robot_BallTargeting"]  .Remove(*this, &FRC_2014_Robot::SetBallTargeting);
		em->Event_Map["Robot_BallTargeting_On"]  .Remove(*this, &FRC_2014_Robot::SetBallTargetingOn);
		em->Event_Map["Robot_BallTargeting_Off"]  .Remove(*this, &FRC_2014_Robot::SetBallTargetingOff);

		em->EventOnOff_Map["Robot_CatcherShooter"]  .Remove(*this, &FRC_2014_Robot::SetCatcherShooter);
		em->Event_Map["Robot_CatcherShooter_On"]  .Remove(*this, &FRC_2014_Robot::SetCatcherShooterOn);
		em->Event_Map["Robot_CatcherShooter_Off"]  .Remove(*this, &FRC_2014_Robot::SetCatcherShooterOff);

		em->EventOnOff_Map["Robot_CatcherIntake"]  .Remove(*this, &FRC_2014_Robot::SetCatcherIntake);
		em->Event_Map["Robot_CatcherIntake_On"]  .Remove(*this, &FRC_2014_Robot::SetCatcherIntakeOn);
		em->Event_Map["Robot_CatcherIntake_Off"]  .Remove(*this, &FRC_2014_Robot::SetCatcherIntakeOff);
		#ifdef Robot_TesterCode
		em->Event_Map["TestAuton"]  .Remove(*this, &FRC_2014_Robot::TestAutonomous);
		em->Event_Map["Complete"]  .Remove(*this, &FRC_2014_Robot::GoalComplete);
		#endif
	}

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_Winch.AsShip1D().BindAdditionalEventControls(Bind);
	m_Intake_Arm.BindAdditionalEventControls(Bind);
	m_Intake_Rollers.AsShip1D().BindAdditionalEventControls(Bind);
	#ifdef Robot_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
	__super::BindAdditionalEventControls(Bind);
}

void FRC_2014_Robot::BindAdditionalUIControls(bool Bind,void *joy, void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}

void FRC_2014_Robot::UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s)
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
void FRC_2014_Robot::TestAutonomous()
{
	Goal *oldgoal=ClearGoal();
	if (oldgoal)
		delete oldgoal;

	{
		Goal *goal=NULL;
		goal=FRC_2014_Goals::Get_FRC2014_Autonomous(this);
		if (goal)
			goal->Activate(); //now with the goal(s) loaded activate it
		SetGoal(goal);
		//enable autopilot (note windriver does this in main)
		m_controller->GetUIController_RW()->SetAutoPilot(true);
	}
}

void FRC_2014_Robot::GoalComplete()
{
	printf("Goals completed!\n");
	m_controller->GetUIController_RW()->SetAutoPilot(false);
}
#endif

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2014_Robot_Properties::FRC_2014_Robot_Properties()  : m_TurretProps(
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
	m_IntakeRollersProps(
	"Rollers",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//RS-550 motor with 64:1 BaneBots transmission, so this is spec at 19300 rpm free, and 17250 peak efficiency
	//17250 / 64 = 287.5 = rps of motor / 64 reduction = 4.492 rps * 2pi = 28.22524
	28,   //Max Speed (rounded as we need not have precision)
	112.0,112.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false	//No limit ever!
	),
	m_RobotControls(&s_ControlsEvents)
{
	{
		const double c_ArmToGearRatio=72.0/28.0;
		const double c_PotentiometerToArmRatio=36.0/54.0;

		FRC_2014_Robot_Props props;
		//const double KeyDistance=Inches2Meters(144);
		//const double KeyWidth=Inches2Meters(101);
		//const double KeyDepth=Inches2Meters(48);   //not used (yet)
		//const double DefaultY=c_HalfCourtLength-KeyDistance;
		//const double HalfKeyWidth=KeyWidth/2.0;

		props.Catapult_Robot_Props.ArmToGearRatio=c_ArmToGearRatio;
		props.Catapult_Robot_Props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
		//The winch is set up to force the numbers to go up from 0 - 90 where 0 is pointing up
		//This allows gain assist to apply max voltage to its descent
		props.Catapult_Robot_Props.ChipShotAngle=DEG_2_RAD(45.0);
		props.Catapult_Robot_Props.GoalShotAngle=DEG_2_RAD(90.0);
		props.Catapult_Robot_Props.AutoDeployArm=false;

		props.Intake_Robot_Props.ArmToGearRatio=c_ArmToGearRatio;
		props.Intake_Robot_Props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
		//The intake uses a starting point of 90 to force numbers down from 90 - 0 where zero is pointing straight out
		//This allows the gain assist to apply max force when it goes from deployed to stowed
		props.Intake_Robot_Props.Stowed_Angle=DEG_2_RAD(90.0);
		props.Intake_Robot_Props.Deployed_Angle=DEG_2_RAD(61.0);
		props.Intake_Robot_Props.Squirt_Angle=DEG_2_RAD(90.0);

		props.BallTargeting_Props.CameraOffsetScalar=0.20;
		props.BallTargeting_Props.LatencyCounterThreshold=0.200; //A bit slow but confirmed

		FRC_2014_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		auton.FirstMove_ft=2.0;
		auton.FirstMoveWait_s=0.500;
		auton.SecondMove_ft=4.0;
		auton.ScootBack_ft=0.5;
		auton.SecondBallRollerTime_s=0.500;  //wishful thinking
		auton.RollUpLoadSpeed=1.0;
		auton.LandOnBallRollerTime_s=0.500;
		auton.LandOnBallRollerSpeed=1.0;
		auton.RollerDriveScalar=0.5;  //WAG
		auton.LoadedBallWait_s=0.500;
		auton.ThreeBallRotation_deg=45;
		//In theory that would put it back to start shifted over
		auton.ThreeBallDistance_ft=-1 * auton.FirstMove_ft/cos(DEG_2_RAD(auton.ThreeBallRotation_deg));
		m_FRC2014RobotProps=props;
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

const char *ProcessVec2D(FRC_2014_Robot_Props &m_FRC2014RobotProps,Scripting::Script& script,Vec2d &Dest)
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
const char * const g_FRC_2014_Controls_Events[] = 
{
	"Turret_SetCurrentVelocity","Turret_SetIntendedPosition","Turret_SetPotentiometerSafety",
	"PitchRamp_SetCurrentVelocity","PitchRamp_SetIntendedPosition","PitchRamp_SetPotentiometerSafety",
	"Robot_SetLowGear","Robot_SetLowGearOn","Robot_SetLowGearOff","Robot_SetLowGearValue",
	"Robot_SetDriverOverride",
	"Winch_SetChipShot","Winch_SetGoalShot","Winch_SetCurrentVelocity","Winch_Fire","Winch_FireManager","Winch_Advance",
	//"IntakeArm_SetCurrentVelocity","IntakeArm_SetStowed","IntakeArm_SetDeployed","IntakeArm_SetSquirt","IntakeArm_Advance","IntakeArm_Retract",
	"IntakeArm_DeployManager",
	"Robot_BallTargeting","Robot_BallTargeting_On","Robot_BallTargeting_Off",
	"Robot_CatcherShooter","Robot_CatcherShooter_On","Robot_CatcherShooter_Off",
	"Robot_CatcherIntake","Robot_CatcherIntake_On","Robot_CatcherIntake_Off",
	"IntakeRollers_Grip","IntakeRollers_Squirt","IntakeRollers_SetCurrentVelocity",
	"TestAuton"
};

const char *FRC_2014_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_FRC_2014_Controls_Events))?g_FRC_2014_Controls_Events[index] : NULL;
}
FRC_2014_Robot_Properties::ControlEvents FRC_2014_Robot_Properties::s_ControlsEvents;

void FRC_2014_Robot_Props::Autonomous_Properties::ShowAutonParameters()
{
	if (ShowParameters)
	{
		const char * const SmartNames[]={"first_move_ft",	"first_move_wait",	"second_move_ft",	"land_on_ball_roller_time",
			"land_on_ball_roller_speed",	"load_ball_roller_speed",	"scoot_back_ft",	"second_ball_roller_time",		
			"roller_drive_speed",	"loaded_ball_wait",		"third_ball_angle_deg",		"third_ball_distance_ft"};
		double * const SmartVariables[]={&FirstMove_ft,&FirstMoveWait_s,&SecondMove_ft,&LandOnBallRollerTime_s,
			&LandOnBallRollerSpeed,&RollUpLoadSpeed,&ScootBack_ft,&SecondBallRollerTime_s,
			&RollerDriveScalar,&LoadedBallWait_s,&ThreeBallRotation_deg,&ThreeBallDistance_ft};
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

	}
}

void FRC_2014_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	FRC_2014_Robot_Props &props=m_FRC2014RobotProps;

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
		err = script.GetFieldTable("catapult");
		if (!err)
		{
			FRC_2014_Robot_Props::Catapult &cat_props=props.Catapult_Robot_Props;
			err=script.GetField("arm_to_motor", NULL, NULL, &fTest);
			if (!err)
				cat_props.ArmToGearRatio=fTest;
			err=script.GetField("pot_to_arm", NULL, NULL, &fTest);
			if (!err)
				cat_props.PotentiometerToArmRatio=fTest;
			err=script.GetField("chipshot_angle_deg", NULL, NULL, &fTest);
			if (!err)
				cat_props.ChipShotAngle=DEG_2_RAD(fTest);
			err=script.GetField("goalshot_angle_deg", NULL, NULL, &fTest);
			if (!err)
				cat_props.GoalShotAngle=DEG_2_RAD(fTest);
			SCRIPT_TEST_BOOL_YES(cat_props.AutoDeployArm,"auto_deploy_arm");
			script.Pop();
		}
		err = script.GetFieldTable("intake");
		if (!err)
		{
			FRC_2014_Robot_Props::Intake &intake_props=props.Intake_Robot_Props;
			err=script.GetField("arm_to_motor", NULL, NULL, &fTest);
			if (!err)
				intake_props.ArmToGearRatio=fTest;
			err=script.GetField("pot_to_arm", NULL, NULL, &fTest);
			if (!err)
				intake_props.PotentiometerToArmRatio=fTest;
			err=script.GetField("stowed_angle_deg", NULL, NULL, &fTest);
			if (!err)
				intake_props.Stowed_Angle=DEG_2_RAD(fTest);
			err=script.GetField("deployed_angle", NULL, NULL, &fTest);
			if (!err)
				intake_props.Deployed_Angle=DEG_2_RAD(fTest);
			err=script.GetField("squirt_angle", NULL, NULL, &fTest);
			if (!err)
				intake_props.Squirt_Angle=DEG_2_RAD(fTest);
			script.Pop();
		}

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
		err = script.GetFieldTable("winch");
		if (!err)
		{
			m_WinchProps.LoadFromScript(script);
			script.Pop();
		}
		//err = script.GetFieldTable("intake_arm");
		//if (!err)
		//{
		//	m_Intake_ArmProps.LoadFromScript(script);
		//	script.Pop();
		//}

		m_LowGearProps=*this;  //copy redundant data first
		err = script.GetFieldTable("low_gear");
		if (!err)
		{
			m_LowGearProps.LoadFromScript(script);
			script.Pop();
		}

		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct FRC_2014_Robot_Props::Autonomous_Properties &auton=m_FRC2014RobotProps.Autonomous_Props;
			{
				err = script.GetField("first_move_ft", NULL, NULL,&fTest);
				if (!err)
					auton.FirstMove_ft=fTest;
				err = script.GetField("first_move_wait", NULL, NULL,&fTest);
				if (!err)
					auton.FirstMoveWait_s=fTest;
				err = script.GetField("second_move_ft", NULL, NULL,&fTest);
				if (!err)
					auton.SecondMove_ft=fTest;
				err = script.GetField("scoot_back_ft", NULL, NULL,&fTest);
				if (!err)
					auton.ScootBack_ft=fTest;
				err = script.GetField("land_on_ball_roller_time", NULL, NULL,&fTest);
				if (!err)
					auton.LandOnBallRollerTime_s=fTest;
				err = script.GetField("land_on_ball_roller_speed", NULL, NULL,&fTest);
				if (!err)
					auton.LandOnBallRollerSpeed=fTest;
				err = script.GetField("load_ball_roller_speed", NULL, NULL,&fTest);
				if (!err)
					auton.RollUpLoadSpeed=fTest;
				err = script.GetField("second_ball_roller_time", NULL, NULL,&fTest);
				if (!err)
					auton.SecondBallRollerTime_s=fTest;
				err = script.GetField("roller_drive_speed", NULL, NULL,&fTest);
				if (!err)
					auton.RollerDriveScalar=fTest;
				err = script.GetField("loaded_ball_wait", NULL, NULL,&fTest);
				if (!err)
					auton.LoadedBallWait_s=fTest;
				err = script.GetField("third_ball_angle_deg", NULL, NULL,&fTest);
				if (!err)
					auton.ThreeBallRotation_deg=fTest;
				err = script.GetField("third_ball_distance_ft", NULL, NULL,&fTest);
				if (!err)
					auton.ThreeBallDistance_ft=fTest;

				SCRIPT_TEST_BOOL_YES(auton.IsSupportingHotSpot,"support_hotspot");
				SCRIPT_TEST_BOOL_YES(auton.ShowParameters,"show_auton_variables");
				auton.ShowAutonParameters();
			}
			script.Pop();
		}

		err=script.GetField("ball_camera_scalar", NULL, NULL, &fTest);
		if (!err)
			props.BallTargeting_Props.CameraOffsetScalar=fTest;
		err=script.GetField("ball_latency_count", NULL, NULL, &fTest);
		if (!err)
			props.BallTargeting_Props.LatencyCounterThreshold=fTest;

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
 /*															FRC_2014_Goals															*/
/***********************************************************************************************************************************/


class FRC_2014_Goals_Impl : public AtomicGoal
{
	private:
		FRC_2014_Robot &m_Robot;
		double m_Timer;

		class SetUpProps
		{
		protected:
			FRC_2014_Goals_Impl *m_Parent;
			FRC_2014_Robot &m_Robot;
			FRC_2014_Robot_Props::Autonomous_Properties m_AutonProps;
			Entity2D_Kind::EventMap &m_EventMap;
		public:
			SetUpProps(FRC_2014_Goals_Impl *Parent)	: m_Parent(Parent),m_Robot(Parent->m_Robot),m_EventMap(*m_Robot.GetEventMap())
			{	
				m_AutonProps=m_Robot.GetRobotProps().GetFRC2014RobotProps().Autonomous_Props;
			}
		};

		class goal_clock : public AtomicGoal
		{
		private:
			FRC_2014_Goals_Impl *m_Parent;
		public:
			goal_clock(FRC_2014_Goals_Impl *Parent)	: m_Parent(Parent) {	m_Status=eInactive;	}
			void Activate()  {	m_Status=eActive;	}
			Goal_Status Process(double dTime_s)
			{
				double &Timer=m_Parent->m_Timer;
				if (m_Status==eActive)
				{
					SmartDashboard::PutNumber("Timer",10.0-Timer);
					Timer+=dTime_s;
					if (Timer>=10.0)
						m_Status=eCompleted;
				}
				return m_Status;
			}
			void Terminate() {	m_Status=eFailed;	}
		};
		MultitaskGoal m_Primer;
		bool m_IsHot;
		bool m_HasSecondShotFired;

		class MoveStraight_WithRoller : public Goal_Ship_MoveToRelativePosition, public SetUpProps
		{
		private:
			#ifndef Robot_TesterCode
			typedef Goal_Ship_MoveToRelativePosition __super;
			#endif
			double m_RollerScalar;
		public:
			MoveStraight_WithRoller(FRC_2014_Goals_Impl *Parent,double RollerScalar,AI_Base_Controller *controller,const WayPoint &waypoint,bool UseSafeStop=true,
				bool LockOrientation=false,double safestop_tolerance=0.03) : 
				Goal_Ship_MoveToRelativePosition(controller,waypoint,UseSafeStop,LockOrientation,safestop_tolerance),
				SetUpProps(Parent),m_RollerScalar(RollerScalar)
			{}

			virtual Goal_Status Process(double dTime_s)
			{
				m_Status=__super::Process(dTime_s);
				if (m_Status==eActive)
				{
					//Now to grab the current velocity
					const double Velocity=Feet2Meters(SmartDashboard::GetNumber("Velocity"));
					const double ratio=Velocity / m_Robot.GetRobotProps().GetEngagedMaxSpeed();
					m_EventMap.EventValue_Map["IntakeRollers_SetCurrentVelocity"].Fire(ratio * m_AutonProps.RollerDriveScalar);
				}
				return m_Status;
			}
		};

		/// \param RollerScalar if this is 0.0 then the MoveStraight_WithRoller goal will not be used
		static Goal * Move_Straight(FRC_2014_Goals_Impl *Parent,double length_ft,double RollerScalar=0.0)
		{
			FRC_2014_Robot *Robot=&Parent->m_Robot;
			//Construct a way point
			WayPoint wp;
			const Vec2d Local_GoalTarget(0.0,Feet2Meters(length_ft));
			wp.Position=Local_GoalTarget;
			wp.Power=1.0;
			//Now to setup the goal
			const bool LockOrientation=true;
			const double PrecisionTolerance=Robot->GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			Goal_Ship_MoveToPosition *goal_drive=NULL;
			if (RollerScalar==0.0)
				goal_drive=new Goal_Ship_MoveToRelativePosition(Robot->GetController(),wp,true,LockOrientation,PrecisionTolerance);
			else
				goal_drive=new MoveStraight_WithRoller(Parent,RollerScalar,Robot->GetController(),wp,true,LockOrientation,PrecisionTolerance);
			return goal_drive;
		}


		class Fire : public AtomicGoal, public SetUpProps
		{
		private:
			bool m_IsOn;
		public:
			Fire(FRC_2014_Goals_Impl *Parent, bool On)	: SetUpProps(Parent),m_IsOn(On) {	m_Status=eInactive;	}
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				m_EventMap.EventOnOff_Map["Winch_Fire"].Fire(m_IsOn);
				m_Status=eCompleted;
				return m_Status;
			}
		};

		class Intake_Deploy : public AtomicGoal, public SetUpProps
		{
		private:
			bool m_IsOn;
		public:
			Intake_Deploy(FRC_2014_Goals_Impl *Parent, bool On)	: SetUpProps(Parent),m_IsOn(On) {	m_Status=eInactive;	}
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				m_EventMap.EventOnOff_Map["Robot_CatcherShooter"].Fire(m_IsOn);
				m_Status=eCompleted;
				return m_Status;
			}
		};

		class Fire_Sequence : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			Fire_Sequence(FRC_2014_Goals_Impl *Parent)	: Generic_CompositeGoal(true),SetUpProps(Parent) {	m_Status=eInactive;	}
			virtual void Activate()
			{
				AddSubgoal(new Fire(m_Parent,false));
				AddSubgoal(new Goal_Wait(.500));
				AddSubgoal(new Fire(m_Parent,true));
				m_Status=eActive;
			}
		};

		class Reset_Catapult : public AtomicGoal, public SetUpProps
		{
		private:
			bool m_WaitForComplete;
			Goal_Ship1D_MoveToPosition *m_GoalMoveToPosition; //This is used if wait for complete is true
		public:
			Reset_Catapult(FRC_2014_Goals_Impl *Parent, bool WaitForComplete=false)	: SetUpProps(Parent),m_WaitForComplete(WaitForComplete),m_GoalMoveToPosition(NULL) 
			{	m_Status=eInactive;	}
			virtual void Activate() 
			{
				m_Status=eActive;
				if (m_WaitForComplete)
				{
					const FRC_2014_Robot_Props &props=m_Robot.GetRobotProps().GetFRC2014RobotProps();
					const double IntendedPosition=( props.Catapult_Robot_Props.GoalShotAngle * props.Catapult_Robot_Props.ArmToGearRatio);

					m_GoalMoveToPosition=new Goal_Ship1D_MoveToPosition(m_Robot.GetWinch(),IntendedPosition,m_Robot.GetRobotProps().GetWinchProps().GetRotaryProps().PrecisionTolerance);
					m_GoalMoveToPosition->Activate(); //now would be good
				}
			}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				if (!m_WaitForComplete)
				{
					m_EventMap.Event_Map["Winch_SetGoalShot"].Fire();
					m_Status=eCompleted;
				}
				else
				{
					m_Status=m_GoalMoveToPosition->Process(dTime_s);  //just pass through let it determine when its done
					if (m_Robot.GetCatapultLimit())
						m_Status=eCompleted;
				}
				return m_Status;
			}
		};

		class Fire_Conditional : public Generic_CompositeGoal, public SetUpProps
		{
		private:
			bool m_FireIfNotFiredYet;
		public:
			Fire_Conditional(FRC_2014_Goals_Impl *Parent,bool FireIfNotFiredYet=false)	: Generic_CompositeGoal(true),SetUpProps(Parent),
				m_FireIfNotFiredYet(FireIfNotFiredYet)
			{	m_Status=eInactive;	}
			virtual void Activate()
			{
				if ((m_Parent->m_IsHot) || ((m_FireIfNotFiredYet)&&(!m_Parent->m_HasSecondShotFired)))
				{
					//always reset for this one
					AddSubgoal(new Reset_Catapult(m_Parent,true));
					AddSubgoal(new Fire(m_Parent,false));
					AddSubgoal(new Goal_Wait(.500));
					AddSubgoal(new Fire(m_Parent,true));
					m_Parent->m_HasSecondShotFired=true;
				}
				else
					AddSubgoal(new Goal_Wait(.100));
				m_Status=eActive;
			}
		};

		class WaitForHot : public AtomicGoal, public SetUpProps
		{
		public:
			WaitForHot(FRC_2014_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eInactive;	}
			virtual void Activate() 
			{
				m_Status=eActive;
				SmartDashboard::PutBoolean("Main_Is_Targeting",true);
			}
			virtual Goal_Status Process(double dTime_s)
			{
				double &Timer=m_Parent->m_Timer;
				ActivateIfInactive();
				double IsHot=0.0;
				try
				{
					IsHot=SmartDashboard::GetNumber("TargetHot");
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber("TargetHot",0.0);
				}
				if ((IsHot!=0.0)||(Timer>5.2))
					m_Status=eCompleted;
				return m_Status;
			}

			virtual void Terminate() 
			{
				SmartDashboard::PutBoolean("Main_Is_Targeting",false);
			}
		};

		//Like WaitForHot but only waits for a specified time for hot or if hot and will return value to parent
		class ProbeForHot : public AtomicGoal, public SetUpProps
		{
		private:
			double m_WaitTime;
			double m_TimeAccrued;
		public:
			ProbeForHot(FRC_2014_Goals_Impl *Parent,double WaitTime)	: SetUpProps(Parent),m_WaitTime(WaitTime),m_TimeAccrued(0.0)
			{	m_Status=eInactive;	}
			virtual void Activate() 
			{
				m_Status=eActive;
				SmartDashboard::PutBoolean("Main_Is_Targeting",true);
			}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				m_TimeAccrued+=dTime_s;
				double IsHot=0.0;
				try
				{
					IsHot=SmartDashboard::GetNumber("TargetHot");
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber("TargetHot",0.0);
				}
				if ((IsHot!=0.0)||(m_TimeAccrued>m_WaitTime))
				{
					m_Parent->m_IsHot=(IsHot!=0.0);
					m_Status=eCompleted;
				}

				return m_Status;
			}

			virtual void Terminate() 
			{
				SmartDashboard::PutBoolean("Main_Is_Targeting",false);
			}
		};

		class OneBallAuton : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			OneBallAuton(FRC_2014_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				//const bool SupporingHotSpot=m_AutonProps.IsSupportingHotSpot;
				//Note: these are reversed
				AddSubgoal(Move_Straight(m_Parent,m_AutonProps.SecondMove_ft));
				AddSubgoal(new Intake_Deploy(m_Parent,false));
				//TODO enable once ready
				//AddSubgoal(new Reset_Catapult(m_Parent));
				AddSubgoal(new Goal_Wait(0.500));  //ensure catapult has finished launching ball before moving
				AddSubgoal(new Fire_Sequence(m_Parent));
				if (m_AutonProps.IsSupportingHotSpot)
				{
					//We can wait for hot spot detection even if it is not supported
					AddSubgoal(new WaitForHot(m_Parent));
				}

				AddSubgoal(new Goal_Wait(m_AutonProps.FirstMoveWait_s));  //avoid motion shot

				AddSubgoal(Move_Straight(m_Parent,m_AutonProps.FirstMove_ft));
				AddSubgoal(new Intake_Deploy(m_Parent,true));
				m_Status=eActive;
			}
		};

		class SetRollerSpeed_WithTime : public AtomicGoal, public SetUpProps
		{
		private:
			double m_Speed;
			double m_WaitTime;
			double m_TimeAccrued;
		public:
			SetRollerSpeed_WithTime(FRC_2014_Goals_Impl *Parent, double Speed,double WaitTime)	: SetUpProps(Parent),m_Speed(Speed),m_WaitTime(WaitTime),m_TimeAccrued(0.0) 
			{	m_Status=eInactive;	}
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				m_TimeAccrued+=dTime_s;

				m_EventMap.EventValue_Map["IntakeRollers_SetCurrentVelocity"].Fire(m_Speed);
				//Note: I need not zero speed... it will do that automatically per time change
				if (m_TimeAccrued>m_WaitTime)
					m_Status=eCompleted;
				return m_Status;
			}
		};

		class TwoBallAuton : public Generic_CompositeGoal, public SetUpProps
		{
		protected:
			virtual bool IsThreeBall() {return false;}
			//inject third ball goals here
			virtual void Add_GetThirdBallGoals() {}
		public:
			TwoBallAuton(FRC_2014_Goals_Impl *Parent)	: SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				//const bool SupporingHotSpot=m_AutonProps.IsSupportingHotSpot;
				//Note: these are reversed
				AddSubgoal(Move_Straight(m_Parent,m_AutonProps.SecondMove_ft));
				AddSubgoal(new Intake_Deploy(m_Parent,false));

				//As it stands... it might be possible that time runs out before the catapult can complete... for safety reasons I am disabling
				//this goal, but we may be able to enable it if we test that we gained more time via limit switch... there is no harm in
				//keeping it disabled just to be safe.
				#if 0
				//multi goal these... we want to wait at least 500ms but still start resetting the catapult
				{
					MultitaskGoal *NewMultiTaskGoal=new MultitaskGoal(false);
					NewMultiTaskGoal->AddGoal(new Reset_Catapult(m_Parent));
					NewMultiTaskGoal->AddGoal(new Goal_Wait(0.500));  //ensure catapult has finished launching ball before moving
					AddSubgoal(NewMultiTaskGoal);
				}
				#else
				AddSubgoal(new Goal_Wait(0.500));  //ensure catapult has finished launching ball before moving
				#endif
				//This is redundant as the default is to do nothing, but makes it more readable
				if (IsThreeBall())
					Add_GetThirdBallGoals();

				AddSubgoal(new Fire_Sequence(m_Parent));
				if (!IsThreeBall())
				{
					//This one is here in case we decide to disable the supporting hot spot... in which case if the hot spot is on the
					//last 5 seconds the second ball would ensure and wait until afterwards to shoot.  Other than this case it shouldn't
					//have to wait at all
					AddSubgoal(new WaitForHot(m_Parent));
				}
				AddSubgoal(new Goal_Wait(m_AutonProps.LoadedBallWait_s)); //give time for ball to settle
				//roll up the ball second ball
				AddSubgoal(new SetRollerSpeed_WithTime(m_Parent,m_AutonProps.RollUpLoadSpeed,m_AutonProps.SecondBallRollerTime_s));
				AddSubgoal(new Reset_Catapult(m_Parent,true));
				AddSubgoal(new Fire_Sequence(m_Parent));
				//This may need to be disabled... it all depends on how long it takes to load and shoot
				#if 0
				if (!IsThreeBall())
				{
					if (m_AutonProps.IsSupportingHotSpot)
						AddSubgoal(new WaitForHot(m_Parent));
					else
						AddSubgoal(new Goal_Wait(0.400));  //avoid motion shot
				}
				#else
				AddSubgoal(new Goal_Wait(m_AutonProps.FirstMoveWait_s));  //avoid motion shot
				#endif
				//Note we add the scoot back distance to overall distance of first move... so it in theory is back where it started
				AddSubgoal(Move_Straight(m_Parent,m_AutonProps.FirstMove_ft+m_AutonProps.ScootBack_ft,m_AutonProps.RollerDriveScalar));
				AddSubgoal(new SetRollerSpeed_WithTime(m_Parent,m_AutonProps.LandOnBallRollerSpeed,m_AutonProps.LandOnBallRollerTime_s));
				AddSubgoal(new Intake_Deploy(m_Parent,true));
				AddSubgoal(Move_Straight(m_Parent,-m_AutonProps.ScootBack_ft,m_AutonProps.RollerDriveScalar));
				m_Status=eActive;
			}
		};

		class ThreeBallAuton : public TwoBallAuton
		{
		public:
			ThreeBallAuton(FRC_2014_Goals_Impl *Parent)	: TwoBallAuton(Parent) {	m_Status=eActive;	}
		protected:
			virtual bool IsThreeBall() {return true;}
			virtual void Add_GetThirdBallGoals()
			{
				//fire 3rd ball
				AddSubgoal(new Fire_Sequence(m_Parent));
				//Rotate back
				AddSubgoal(new Goal_Ship_RotateToRelativePosition(m_Robot.GetController(),DEG_2_RAD(-m_AutonProps.ThreeBallRotation_deg)));
				//Move back
				AddSubgoal(Move_Straight(m_Parent,-m_AutonProps.ThreeBallDistance_ft,m_AutonProps.RollerDriveScalar));
				AddSubgoal(new Goal_Wait(m_AutonProps.LoadedBallWait_s)); //give time for ball to settle
				//load up third ball (doing it before moving back avoids the need to turn with the ball on the floor)
				AddSubgoal(new SetRollerSpeed_WithTime(m_Parent,m_AutonProps.RollUpLoadSpeed,m_AutonProps.SecondBallRollerTime_s));
				//Note this first turning is while the intake goes down
				AddSubgoal(new SetRollerSpeed_WithTime(m_Parent,m_AutonProps.LandOnBallRollerSpeed,m_AutonProps.LandOnBallRollerTime_s));
				AddSubgoal(new Intake_Deploy(m_Parent,true));
				//Move to it
				//multi goal these... we want to wait at least 500ms but still start resetting the catapult
				{
					MultitaskGoal *NewMultiTaskGoal=new MultitaskGoal(true);
					NewMultiTaskGoal->AddGoal(new Reset_Catapult(m_Parent));
					AddSubgoal(Move_Straight(m_Parent,m_AutonProps.ThreeBallDistance_ft));
					AddSubgoal(NewMultiTaskGoal);
				}
				//Rotate to it
				AddSubgoal(new Goal_Ship_RotateToRelativePosition(m_Robot.GetController(),DEG_2_RAD(m_AutonProps.ThreeBallRotation_deg)));
				AddSubgoal(new Intake_Deploy(m_Parent,false));
				//Go back to get third ball-------
			}
		};

		enum AutonType
		{
			eDoNothing,
			eOneBall,
			eTwoBall,
			eThreeBall,
			eNoAutonTypes
		} m_AutonType;
		enum Robot_Position
		{
			ePosition_Center,
			ePosition_Left,
			ePosition_Right
		} m_RobotPosition;
	public:
		FRC_2014_Goals_Impl(FRC_2014_Robot &robot) : m_Robot(robot), m_Timer(0.0), 
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
				const double fBallCount=SmartDashboard::GetNumber("Auton BallCount");
				int BallCount=(size_t)fBallCount;
				if ((BallCount<0)||(BallCount>eNoAutonTypes))
					BallCount=eDoNothing;
				m_AutonType=(AutonType)BallCount;
			}
			catch (...)
			{
				m_AutonType=eDoNothing;
				SmartDashboard::PutNumber("Auton BallCount",0.0);
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

			FRC_2014_Robot_Props::Autonomous_Properties &auton=m_Robot.GetAutonProps();
			auton.ShowAutonParameters();  //Grab again now in case user has tweaked values

			printf("ball count=%d position=%d\n",m_AutonType,m_RobotPosition);
			switch(m_AutonType)
			{
			case eOneBall:
				m_Primer.AddGoal(new OneBallAuton(this));
				break;
			case eTwoBall:
				m_Primer.AddGoal(new TwoBallAuton(this));
				break;
			case eThreeBall:
				m_Primer.AddGoal(new ThreeBallAuton(this));
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

Goal *FRC_2014_Goals::Get_FRC2014_Autonomous(FRC_2014_Robot *Robot)
{
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),(char *)"Complete");
	SmartDashboard::PutNumber("Sequence",1.0);  //ensure we are on the right sequence
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(new FRC_2014_Goals_Impl(*Robot));
	//MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot_Control															*/
/***********************************************************************************************************************************/

#if defined Robot_TesterCode && !defined __TestControlAssignments__

void FRC_2014_Robot_Control::ResetPos()
{
	//Set the solenoids to their default positions
	OpenSolenoid(FRC_2014_Robot::eUseLowGear,true);
	CloseSolenoid(FRC_2014_Robot::eReleaseClutch,true);
	CloseSolenoid(FRC_2014_Robot::eCatcherShooter,true);
	CloseSolenoid(FRC_2014_Robot::eCatcherIntake,true);
}

void FRC_2014_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) Voltage=0.0;

	switch (index)
	{
		case FRC_2014_Robot::eWinch:
			{
				//	printf("Turret=%f\n",Voltage);
				//DOUT3("Turret Voltage=%f",Voltage);
				const double VoltageToUse=(Voltage>0.0)?Voltage:0.0;
				m_WinchVoltage=VoltageToUse * m_RobotProps.GetWinchProps().GetRotaryProps().VoltageScalar;
				//if (m_WinchVoltage>0.08)
				//	printf("%f\n",m_WinchVoltage);
				m_Winch_Pot.UpdatePotentiometerVoltage(VoltageToUse);
				m_Winch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2014_Robot::eIntakeArm1:
			{
				//	printf("Pitch=%f\n",Voltage);
				//DOUT3("Pitch Voltage=%f",Voltage);
				m_IntakeArmVoltage=Voltage * m_RobotProps.GetIntake_ArmProps().GetRotaryProps().VoltageScalar;
				m_IntakeArm_Pot.UpdatePotentiometerVoltage(Voltage);
				m_IntakeArm_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2014_Robot::eRollers:
			SmartDashboard::PutNumber("RollerVoltage",Voltage);
			break;
	}
}

FRC_2014_Robot_Control::FRC_2014_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_WinchVoltage(0.0),m_IntakeArmVoltage(0.0)
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

void FRC_2014_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case FRC_2014_Robot::eWinch:
			m_Winch_Pot.ResetPos();
			break;
		case FRC_2014_Robot::eIntakeArm1:
			m_IntakeArm_Pot.ResetPos();
			//We may want this for more accurate simulation
			//m_Pitch_Pot.SetPos_m((m_Pitch_Pot.GetMinRange()+m_Pitch_Pot.GetMaxRange()) / 2.0);
			break;
	}
}

//This is only for Robot Tester
void FRC_2014_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
}

void FRC_2014_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2014_Robot_Properties *robot_props=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		Rotary_Properties turret_props=robot_props->GetTurretProps();
		//turret_props.SetMinRange(0);
		//turret_props.SetMaxRange(Pi2);
		turret_props.SetUsingRange(false);
		m_Winch_Pot.Initialize(&robot_props->GetWinchProps());
		m_IntakeArm_Pot.Initialize(&robot_props->GetIntake_ArmProps());
	}
	ResetPos();
}

void FRC_2014_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Winch_Pot.SetTimeDelta(dTime_s);
	m_IntakeArm_Pot.SetTimeDelta(dTime_s);

	//Let's do away with this since we are using the smart dashboard
	////display voltages
	//DOUT(2,"l=%.2f r=%.2f t=%.2f pi=%.2f pw=%.2f lc=%.2f mc=%.2f fc=%.2f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),
	//	m_WinchVoltage,m_IntakeArmVoltage,m_PowerWheelVoltage,m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage);

	SmartDashboard::PutNumber("WinchVoltage",m_WinchVoltage);
	SmartDashboard::PutNumber("IntakeArmVoltage",m_IntakeArmVoltage);
}


double FRC_2014_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;
	const FRC_2014_Robot_Props &props=m_RobotProps.GetFRC2014RobotProps();

	switch (index)
	{
		case FRC_2014_Robot::eWinch:
			{
				const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
				//result=(m_Potentiometer.GetDistance() * m_RobotProps.GetArmProps().GetRotaryProps().EncoderToRS_Ratio) + 0.0;
				//no conversion needed in simulation
				result=(m_Winch_Pot.GetPotentiometerCurrentPosition()) + 0.0;

				//result = m_KalFilter_Arm(result);  //apply the Kalman filter
				SmartDashboard::PutNumber("Catapult_Angle",90-RAD_2_DEG(result*c_GearToArmRatio));
				//SmartDashboard::PutNumber("Catapult_Angle",RAD_2_DEG(result*c_GearToArmRatio));
			}
			break;
		case FRC_2014_Robot::eIntakeArm1:
			{
				assert(false);  //no potentiometer 
				const double c_GearToArmRatio=1.0/props.Intake_Robot_Props.ArmToGearRatio;
				result=m_IntakeArm_Pot.GetPotentiometerCurrentPosition();
				SmartDashboard::PutNumber("IntakeArm_Angle",90-RAD_2_DEG(result*c_GearToArmRatio));
			}
			break;
	}

	//Let's do away with this and use smart dashboard instead... enable if team doesn't want to use it
	//#ifdef __DebugLUA__
	//switch (index)
	//{
	//	case FRC_2014_Robot::eWinch:
	//		Dout(m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,14,"d=%.1f",RAD_2_DEG(result));
	//		break;
	//	case FRC_2014_Robot::eIntake_Arm:
	//		Dout(m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,14,"p=%.1f",RAD_2_DEG(result));
	//		break;
	//}
	//#endif

	return result;
}

void FRC_2014_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2014_Robot::eUseLowGear:
		printf("UseLowGear=%d\n",Open);
		SmartDashboard::PutBoolean("UseHighGear",!Open);
		break;
	case FRC_2014_Robot::eReleaseClutch:
		printf("ReleaseClutch=%d\n",Open);
		SmartDashboard::PutBoolean("ClutchEngaged",!Open);
		break;
	case FRC_2014_Robot::eCatcherShooter:
		printf("CatcherShooter=%d\n",Open);
		SmartDashboard::PutBoolean("CatcherShooter",Open);
		break;
	case  FRC_2014_Robot::eCatcherIntake:
		printf("CatcherIntake=%d\n",Open);
		SmartDashboard::PutBoolean("CatcherIntake",Open);
		break;
	}
}

#else


void FRC_2014_Robot_Control::ResetPos()
{
	//Enable this code if we have a compressor 
	m_Compressor->Stop();
	printf("RobotControl::ResetPos Compressor->Stop()\n");
	#ifndef Robot_TesterCode
	//Allow driver station to control if they want to run the compressor
	//if (DriverStation::GetInstance()->GetDigitalIn(8))
	//TODO use smart dashboard checkbox for compressor
	if (true)
	#endif
	{
		printf("RobotControl::ResetPos Compressor->Start()\n");
		m_Compressor->Start();
	}
	//Set the solenoids to their default positions
	OpenSolenoid(FRC_2014_Robot::eUseLowGear,true);
	CloseSolenoid(FRC_2014_Robot::eReleaseClutch,true);
	CloseSolenoid(FRC_2014_Robot::eCatcherShooter,true);
	CloseSolenoid(FRC_2014_Robot::eCatcherIntake,true);
}

void FRC_2014_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	switch (index)
	{
	case FRC_2014_Robot::eWinch:
		{
			double VoltageToUse=(Voltage>0.0)?Voltage:0.0;
			m_WinchVoltage=VoltageToUse=VoltageToUse * m_RobotProps.GetWinchProps().GetRotaryProps().VoltageScalar;
			Victor_UpdateVoltage(index,VoltageToUse);
			SmartDashboard::PutNumber("WinchVoltage",VoltageToUse);
		}
		break;
	case FRC_2014_Robot::eIntakeArm1:
		{
			Voltage=Voltage * m_RobotProps.GetIntake_ArmProps().GetRotaryProps().VoltageScalar;
			Victor_UpdateVoltage(FRC_2014_Robot::eIntakeArm1,Voltage);
			Victor_UpdateVoltage(FRC_2014_Robot::eIntakeArm2,Voltage);
			SmartDashboard::PutNumber("IntakeArmVoltage",Voltage);
		}
		break;
	case FRC_2014_Robot::eRollers:
		Victor_UpdateVoltage(index,Voltage);
		SmartDashboard::PutNumber("RollerVoltage",Voltage);
		break;
	case FRC_2014_Robot::eCameraLED:
		TranslateToRelay(index,Voltage);
		//I don't need this since we have another variable that represents it, but enable for diagnostics
		//SmartDashboard::PutBoolean("CameraLED",Voltage==0.0?false:true);
		break;
	}
}

bool FRC_2014_Robot_Control::GetBoolSensorState(size_t index) const
{
	bool ret;
	switch (index)
	{
	case FRC_2014_Robot::eIntakeMin1:
		ret=(m_Limit_IntakeMin1||m_Limit_IntakeMin2);
		break;
	case FRC_2014_Robot::eIntakeMax1:
		ret=(m_Limit_IntakeMax1||m_Limit_IntakeMax2);
		break;
	case FRC_2014_Robot::eCatapultLimit:
		ret=m_Limit_Catapult;
		break;
	default:
		assert (false);
	}
	return ret;
}

FRC_2014_Robot_Control::FRC_2014_Robot_Control(bool UseSafety) : m_TankRobotControl(UseSafety),m_pTankRobotControl(&m_TankRobotControl),
		m_Compressor(NULL),m_WinchVoltage(0.0)
{
}

FRC_2014_Robot_Control::~FRC_2014_Robot_Control()
{
	Encoder_Stop(FRC_2014_Robot::eWinch);
	DestroyCompressor(m_Compressor);
	m_Compressor=NULL;
}

void FRC_2014_Robot_Control::Reset_Rotary(size_t index)
{
	Encoder_Reset(index);  //This will check for encoder existence implicitly
}

#ifdef Robot_TesterCode
void FRC_2014_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
}
#endif

void FRC_2014_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2014_Robot_Properties *robot_props=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	//TODO this is to be changed to an assert once we handle low gear properly
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		Rotary_Properties turret_props=robot_props->GetTurretProps();
		turret_props.SetUsingRange(false); //TODO why is this here?		
	}
	
	//Note: Initialize may be called multiple times so we'll only set this stuff up on first run
	if (!m_Compressor)
	{
		//This one one must also be called for the lists that are specific to the robot
		RobotControlCommon_Initialize(robot_props->Get_ControlAssignmentProps());		
		m_Compressor=CreateCompressor();
		//Note: RobotControlCommon_Initialize() must occur before calling any encoder startup code
		const double EncoderPulseRate=(1.0/360.0);
		Encoder_SetDistancePerPulse(FRC_2014_Robot::eWinch,EncoderPulseRate);
		Encoder_Start(FRC_2014_Robot::eWinch);
		ResetPos(); //must be called after compressor is created
	}

}

void FRC_2014_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef Robot_TesterCode
	const Rotary_Props &rotary=m_RobotProps.GetWinchProps().GetRotaryProps();
	const double adjustment= m_WinchVoltage*m_RobotProps.GetWinchProps().GetMaxSpeed() * dTime_s * (1.0/rotary.EncoderToRS_Ratio);
	Encoder_TimeChange(FRC_2014_Robot::eWinch,dTime_s,adjustment);
	#else
		#ifdef __ShowLCD__
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			lcd->UpdateLCD();
		#endif
	#endif
	m_Limit_IntakeMin1=BoolSensor_GetState(FRC_2014_Robot::eIntakeMin1);
	m_Limit_IntakeMin2=BoolSensor_GetState(FRC_2014_Robot::eIntakeMin2);
	m_Limit_IntakeMax1=BoolSensor_GetState(FRC_2014_Robot::eIntakeMax1);
	m_Limit_IntakeMax2=BoolSensor_GetState(FRC_2014_Robot::eIntakeMax2);
	m_Limit_Catapult=BoolSensor_GetState(FRC_2014_Robot::eCatapultLimit);
	SmartDashboard::PutBoolean("LimitIntakeMin1",m_Limit_IntakeMin1);
	SmartDashboard::PutBoolean("LimitIntakeMax1",m_Limit_IntakeMax1);
	SmartDashboard::PutBoolean("LimitIntakeMin2",m_Limit_IntakeMin2);
	SmartDashboard::PutBoolean("LimitIntakeMax2",m_Limit_IntakeMax2);
	SmartDashboard::PutBoolean("LimitCatapult",m_Limit_Catapult);
}

void FRC_2014_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) 
{
	const Tank_Robot_Props &TankRobotProps=m_RobotProps.GetTankRobotProps();
	if (!TankRobotProps.ReverseSteering)
	{
		Victor_UpdateVoltage(FRC_2014_Robot::eLeftDrive3,(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
		Victor_UpdateVoltage(FRC_2014_Robot::eRightDrive3,-(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
	}
	else
	{
		Victor_UpdateVoltage(FRC_2014_Robot::eLeftDrive3,(float)RightVoltage * TankRobotProps.VoltageScalar_Right);
		Victor_UpdateVoltage(FRC_2014_Robot::eRightDrive3,-(float)LeftVoltage * TankRobotProps.VoltageScalar_Left);
	}
	m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}

double FRC_2014_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;
	const FRC_2014_Robot_Props &props=m_RobotProps.GetFRC2014RobotProps();

	switch (index)
	{
	case FRC_2014_Robot::eWinch:
		{
			const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
			const double distance=Encoder_GetDistance(index);
			result=(distance * m_RobotProps.GetWinchProps().GetRotaryProps().EncoderToRS_Ratio) + 0.0;

			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			SmartDashboard::PutNumber("Catapult_Angle",90-RAD_2_DEG(result*c_GearToArmRatio));
			//SmartDashboard::PutNumber("Catapult_Angle",RAD_2_DEG(result));
		}
		break;
	case FRC_2014_Robot::eIntakeArm1:
	case FRC_2014_Robot::eIntakeArm2:
		assert(false);  //no potentiometer 
		break;
	}

	return result;
}

void FRC_2014_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2014_Robot::eUseLowGear:
		SmartDashboard::PutBoolean("UseHighGear",!Open);
		Solenoid_Open(index,Open);
		break;
	case FRC_2014_Robot::eReleaseClutch:
		SmartDashboard::PutBoolean("ClutchEngaged",!Open);
		Solenoid_Open(index,Open);
		break;
	case FRC_2014_Robot::eCatcherShooter:
		SmartDashboard::PutBoolean("CatcherShooter",Open);
		Solenoid_Open(index,Open);
		break;
	case  FRC_2014_Robot::eCatcherIntake:
		SmartDashboard::PutBoolean("CatcherIntake",Open);
		Solenoid_Open(index,Open);
		break;
	}
}

#endif

#ifdef Robot_TesterCode
  /***************************************************************************************************************/
 /*												FRC_2014_Robot_UI												*/
/***************************************************************************************************************/

FRC_2014_Robot_UI::FRC_2014_Robot_UI(const char EntityName[]) : FRC_2014_Robot(EntityName,this),FRC_2014_Robot_Control(),
		m_TankUI(this)
{
}

void FRC_2014_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
}
void FRC_2014_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
}

void FRC_2014_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
}
void FRC_2014_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
}
void FRC_2014_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
}
void FRC_2014_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
}

#endif

