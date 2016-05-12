#include "stdafx.h"
#include "Robot_Tester.h"

#ifdef Robot_TesterCode
namespace Robot_Tester
{
	#include "Tank_Robot_UI.h"
	#include "HikingViking_Robot.h"
}

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi2=M_PI*2.0;
#else
#include "HikingViking_Robot.h"
using namespace Framework::Base;
using namespace std;
#endif

#ifdef Robot_TesterCode
namespace Robot_Tester
{
#endif
	namespace HikingViking_Goals
	{

#if 0
static Goal *Get_TestLengthGoal_OLD(Ship_Tester *ship)
{
	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0.0;
	wp.Position[1]=1.0;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal=new Goal_Ship_MoveToPosition(ship->GetController(),wp,true,true);
	return goal;
}
#endif

Goal *Get_TestLengthGoal(HikingViking_Robot *Robot)
{
	//float position=DriverStation::GetInstance()->GetAnalogIn(1);
	float position=1.0;
	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0.0;
	wp.Position[1]=position;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_move1=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	Goal_Wait *goal_wait=new Goal_Wait(2.0); //wait
	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_move2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");

	MainGoal->AddSubgoal(goal_move2);
	MainGoal->AddSubgoal(goal_wait);
	MainGoal->AddSubgoal(goal_move1);
	return MainGoal;
}


Goal *Get_UberTubeGoal(HikingViking_Robot *Robot)
{
	HikingViking_Robot::Robot_Arm &Arm=Robot->GetArm();
	//Now to setup the goal
	//double position=HikingViking_Robot::Robot_Arm::HeightToAngle_r(2.7432);  //9 feet
	//double position=HikingViking_Robot::Robot_Arm::HeightToAngle_r(1.7018);   //67 inches
	double position=Arm.HeightToAngle_r(1.08712);   //42.8 inches
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	//Note: full length is 232 inches or 5.89 meters
	const double starting_line=5.49656;  //18.03333
	//const double starting_line=2.3; //hack not calibrated
	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);

	wp.Position[1]=starting_line+0.1;
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	position=Arm.HeightToAngle_r(0.83312);  //32.8 TODO find how much to lower
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second

	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	position=Arm.HeightToAngle_r(0.0);
	Goal_Ship1D_MoveToPosition *goal_arm3=new Goal_Ship1D_MoveToPosition(Arm,position);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm3);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_arm2);
	MainGoal->AddSubgoal(goal_drive2);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	return MainGoal;
};
	} // end namespace hiking viking goals
	
#ifdef Robot_TesterCode
}
#endif

const bool c_UsingArmLimits=true;

  /***********************************************************************************************************************************/
 /*													HikingViking_Robot::Robot_Claw													*/
/***********************************************************************************************************************************/

HikingViking_Robot::Robot_Claw::Robot_Claw(HikingViking_Robot *parent,Rotary_Control_Interface *robot_control) :
	Rotary_Velocity_Control("Claw",robot_control,eRollers),m_pParent(parent),m_Grip(false),m_Squirt(false)
{
}

void HikingViking_Robot::Robot_Claw::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Grip ^ m_Squirt)
		SetCurrentLinearAcceleration(m_Grip?Accel:-Brake);

	__super::TimeChange(dTime_s);
}

void HikingViking_Robot::Robot_Claw::CloseClaw(bool Close)
{
	m_pParent->m_RobotControl->CloseSolenoid(eClaw,Close);
}

void HikingViking_Robot::Robot_Claw::Grip(bool on)
{
	m_Grip=on;
}

void HikingViking_Robot::Robot_Claw::Squirt(bool on)
{
	m_Squirt=on;
}

void HikingViking_Robot::Robot_Claw::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Claw_SetCurrentVelocity"].Subscribe(ehl,*this, &HikingViking_Robot::Robot_Claw::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Claw_Close"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Claw::CloseClaw);
		em->EventOnOff_Map["Claw_Grip"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Claw::Grip);
		em->EventOnOff_Map["Claw_Squirt"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Claw::Squirt);
	}
	else
	{
		em->EventValue_Map["Claw_SetCurrentVelocity"].Remove(*this, &HikingViking_Robot::Robot_Claw::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Claw_Close"]  .Remove(*this, &HikingViking_Robot::Robot_Claw::CloseClaw);
		em->EventOnOff_Map["Claw_Grip"]  .Remove(*this, &HikingViking_Robot::Robot_Claw::Grip);
		em->EventOnOff_Map["Claw_Squirt"]  .Remove(*this, &HikingViking_Robot::Robot_Claw::Squirt);
	}
}

  /***********************************************************************************************************************************/
 /*													HikingViking_Robot::Robot_Arm													*/
/***********************************************************************************************************************************/

HikingViking_Robot::Robot_Arm::Robot_Arm(HikingViking_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("Arm",robot_control,eArm),m_pParent(parent),m_Advance(false),m_Retract(false)
{
}


void HikingViking_Robot::Robot_Arm::Advance(bool on)
{
	m_Advance=on;
}
void HikingViking_Robot::Robot_Arm::Retract(bool on)
{
	m_Retract=on;
}

void HikingViking_Robot::Robot_Arm::TimeChange(double dTime_s)
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
	const HikingViking_Robot_Props &props=m_pParent->GetRobotProps().GetHikingVikingRobotProps();
	const double c_GearToArmRatio=1.0/props.ArmToGearRatio;
	double Pos_m=GetPos_m();
	double height=AngleToHeight_m(Pos_m);
	DOUT4("Arm=%f Angle=%f %fft %fin",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
	#endif
	}


double HikingViking_Robot::Robot_Arm::AngleToHeight_m(double Angle_r) const
{
	const HikingViking_Robot_Props &props=m_pParent->GetRobotProps().GetHikingVikingRobotProps();
	const double c_GearToArmRatio=1.0/props.ArmToGearRatio;

	return (sin(Angle_r*c_GearToArmRatio)*props.ArmLength)+props.GearHeightOffset;
}
double HikingViking_Robot::Robot_Arm::Arm_AngleToHeight_m(double Angle_r) const
{
	const HikingViking_Robot_Props &props=m_pParent->GetRobotProps().GetHikingVikingRobotProps();
	return (sin(Angle_r)*props.ArmLength)+props.GearHeightOffset;
}

double HikingViking_Robot::Robot_Arm::HeightToAngle_r(double Height_m) const
{
	const HikingViking_Robot_Props &props=m_pParent->GetRobotProps().GetHikingVikingRobotProps();
	return asin((Height_m-props.GearHeightOffset)/props.ArmLength) * props.ArmToGearRatio;
}

double HikingViking_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(double raw) const
{
	const HikingViking_Robot_Props &props=m_pParent->GetRobotProps().GetHikingVikingRobotProps();
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=props.PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}

double HikingViking_Robot::Robot_Arm::GetPosRest()
{
	return HeightToAngle_r(-0.02);
}
void HikingViking_Robot::Robot_Arm::SetPosRest()
{
	SetIntendedPosition(GetPosRest()  );
}
void HikingViking_Robot::Robot_Arm::SetPos0feet()
{
	SetIntendedPosition( HeightToAngle_r(0.0) );
}
void HikingViking_Robot::Robot_Arm::SetPos3feet()
{
	//Not used, but kept for reference
	SetIntendedPosition(HeightToAngle_r(0.9144));
}
void HikingViking_Robot::Robot_Arm::SetPos6feet()
{
	SetIntendedPosition( HeightToAngle_r(1.8288) );
}
void HikingViking_Robot::Robot_Arm::SetPos9feet()
{
	SetIntendedPosition( HeightToAngle_r(2.7432) );
}
void HikingViking_Robot::Robot_Arm::CloseRist(bool Close)
{
	m_pParent->m_RobotControl->CloseSolenoid(eRist,Close);
}

void HikingViking_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Subscribe(ehl,*this, &HikingViking_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Arm_SetPotentiometerSafety"].Subscribe(ehl,*this, &HikingViking_Robot::Robot_Arm::SetPotentiometerSafety);
		
		em->Event_Map["Arm_SetPosRest"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Arm::SetPosRest);
		em->Event_Map["Arm_SetPos0feet"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Arm::SetPos9feet);

		em->EventOnOff_Map["Arm_Advance"].Subscribe(ehl,*this, &HikingViking_Robot::Robot_Arm::Advance);
		em->EventOnOff_Map["Arm_Retract"].Subscribe(ehl,*this, &HikingViking_Robot::Robot_Arm::Retract);

		em->EventOnOff_Map["Arm_Rist"].Subscribe(ehl, *this, &HikingViking_Robot::Robot_Arm::CloseRist);
	}
	else
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Arm_SetPotentiometerSafety"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetPotentiometerSafety);

		em->Event_Map["Arm_SetPosRest"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetPosRest);
		em->Event_Map["Arm_SetPos0feet"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Remove(*this, &HikingViking_Robot::Robot_Arm::SetPos9feet);

		em->EventOnOff_Map["Arm_Advance"].Remove(*this, &HikingViking_Robot::Robot_Arm::Advance);
		em->EventOnOff_Map["Arm_Retract"].Remove(*this, &HikingViking_Robot::Robot_Arm::Retract);

		em->EventOnOff_Map["Arm_Rist"]  .Remove(*this, &HikingViking_Robot::Robot_Arm::CloseRist);
	}
}

  /***********************************************************************************************************************************/
 /*															HikingViking_Robot														*/
/***********************************************************************************************************************************/
HikingViking_Robot::HikingViking_Robot(const char EntityName[],HikingViking_Control_Interface *robot_control,bool UseEncoders) : 
	Tank_Robot(EntityName,robot_control,UseEncoders), m_RobotControl(robot_control), m_Arm(this,robot_control), m_Claw(this,robot_control)
{
}

void HikingViking_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const HikingViking_Robot_Properties *RobotProps=dynamic_cast<const HikingViking_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)

	m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
	m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);
}
void HikingViking_Robot::ResetPos()
{
	__super::ResetPos();
	m_Arm.ResetPos();
	m_Claw.ResetPos();
}

const HikingViking_Robot_Properties &HikingViking_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

void HikingViking_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	arm_entity.TimeChange(dTime_s);
	Entity1D &claw_entity=m_Claw;  //This gets around keeping time change protected in derived classes
	claw_entity.TimeChange(dTime_s);
}

void HikingViking_Robot::CloseDeploymentDoor(bool Close)
{
	m_RobotControl->CloseSolenoid(eDeployment,Close);
}

void HikingViking_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
		em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &HikingViking_Robot::CloseDeploymentDoor);
	else
		em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &HikingViking_Robot::CloseDeploymentDoor);

	Ship_1D &ArmShip_Access=m_Arm;
	ArmShip_Access.BindAdditionalEventControls(Bind);
	Ship_1D &ClawShip_Access=m_Claw;
	ClawShip_Access.BindAdditionalEventControls(Bind);
	__super::BindAdditionalEventControls(Bind);
}

void HikingViking_Robot::BindAdditionalUIControls(bool Bind,void *joy,void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}


  /***********************************************************************************************************************************/
 /*													HikingViking_Robot_Properties													*/
/***********************************************************************************************************************************/

HikingViking_Robot_Properties::HikingViking_Robot_Properties() : m_RobotControls(&s_ControlsEvents)
{
	const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
	const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
	const double c_ArmLength_m=1.8288;  //6 feet
	const double c_ArmToGearRatio=72.0/28.0;
	//const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
	//const double c_PotentiometerToGearRatio=60.0/32.0;
	//const double c_PotentiometerToArmRatio=c_PotentiometerToGearRatio * c_GearToArmRatio;
	const double c_PotentiometerToArmRatio=36.0/54.0;
	//const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
	const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);
	const double c_GearHeightOffset=1.397;  //55 inches
	const double c_WheelDiameter=0.1524;  //6 inches
	const double c_MotorToWheelGearRatio=12.0/36.0;

	{
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings
		//Late assign this to override the initial default
		props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
		props.WheelDiameter=c_WheelDiameter;
		props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
		props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
		m_TankRobotProps=props;
	}

	HikingViking_Robot_Props props;
	props.OptimalAngleUp=c_OptimalAngleUp_r;
	props.OptimalAngleDn=c_OptimalAngleDn_r;
	props.ArmLength=c_ArmLength_m;
	props.ArmToGearRatio=c_ArmToGearRatio;
	props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
	props.PotentiometerMaxRotation=c_PotentiometerMaxRotation;
	props.GearHeightOffset=c_GearHeightOffset;
	props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
	m_HikingVikingRobotProps=props;
}

//declared as global to avoid allocation on stack each iteration
const char * const g_HikingViking_Controls_Events[] = 
{
	"Claw_SetCurrentVelocity","Claw_Close",
	"Claw_Grip","Claw_Squirt",
	"Arm_SetCurrentVelocity","Arm_SetPotentiometerSafety","Arm_SetPosRest",
	"Arm_SetPos0feet","Arm_SetPos3feet","Arm_SetPos6feet","Arm_SetPos9feet",
	"Arm_Rist","Arm_Advance","Arm_Retract",
	"Robot_CloseDoor"
};

const char *HikingViking_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_HikingViking_Controls_Events))?g_HikingViking_Controls_Events[index] : NULL;
}
HikingViking_Robot_Properties::ControlEvents HikingViking_Robot_Properties::s_ControlsEvents;

void HikingViking_Robot_Properties::LoadFromScript(Scripting::Script& script)
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
		err = script.GetFieldTable("arm");
		if (!err)
		{
			m_ArmProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("claw");
		if (!err)
		{
			m_ClawProps.LoadFromScript(script);
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
 /*														Goal_OperateSolenoid														*/
/***********************************************************************************************************************************/

using namespace HikingViking_Goals;

Goal_OperateSolenoid::Goal_OperateSolenoid(HikingViking_Robot &robot,HikingViking_Robot::SolenoidDevices SolenoidDevice,bool Close) : m_Robot(robot),
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
		case HikingViking_Robot::eClaw:
			m_Robot.GetClaw().CloseClaw(m_IsClosed);
			break;
		case HikingViking_Robot::eRist:
			m_Robot.GetArm().CloseRist(m_IsClosed);
			break;
		case HikingViking_Robot::eDeployment:
			m_Robot.CloseDeploymentDoor(m_IsClosed);
			break;
	}
	m_Status=eCompleted;
	return m_Status;
}


#ifdef Robot_TesterCode

  /***********************************************************************************************************************************/
 /*													HikingViking_Robot_Control														*/
/***********************************************************************************************************************************/

void HikingViking_Robot_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	switch (index)
	{
		case HikingViking_Robot::eArm:
			{
				m_ArmVoltage=Voltage * m_RobotProps.GetArmProps().GetRotaryProps().VoltageScalar;
				m_Potentiometer.UpdatePotentiometerVoltage(m_ArmVoltage);
				m_Potentiometer.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case HikingViking_Robot::eRollers:
			m_RollerVoltage=Voltage;
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
	}
}
void HikingViking_Robot_Control::CloseSolenoid(size_t index,bool Close)
{
	switch (index)
	{
		case HikingViking_Robot::eDeployment:
			//DebugOutput("CloseDeploymentDoor=%d\n",Close);
			m_Deployment=Close;
			SmartDashboard::PutBoolean("Deployment",m_Deployment);
			break;
		case HikingViking_Robot::eClaw:
			//DebugOutput("CloseClaw=%d\n",Close);
			m_Claw=Close;
			SmartDashboard::PutBoolean("Claw",m_Claw);
			//This was used to test error with the potentiometer
			//m_Potentiometer.SetBypass(Close);
			break;
		case HikingViking_Robot::eRist:
			//DebugOutput("CloseRist=%d\n",Close);
			m_Rist=Close;
			SmartDashboard::PutBoolean("Wrist",m_Rist);
			break;
	}
}


HikingViking_Robot_Control::HikingViking_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_ArmVoltage(0.0),m_RollerVoltage(0.0),
	m_Deployment(false),m_Claw(false),m_Rist(false)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
}

void HikingViking_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case HikingViking_Robot::eArm:
			m_KalFilter_Arm.Reset();
			m_Potentiometer.ResetPos();
			break;
	}
}

void HikingViking_Robot_Control::Initialize(const Entity_Properties *props)
{
	const HikingViking_Robot_Properties *robot_props=dynamic_cast<const HikingViking_Robot_Properties *>(props);

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy
		assert(robot_props);
		Rotary_Properties writeable_arm_props=robot_props->GetArmProps();
		m_ArmMaxSpeed=writeable_arm_props.GetMaxSpeed();
		//This is not perfect but will work for our simulation purposes
		writeable_arm_props.RotaryProps().EncoderToRS_Ratio=robot_props->GetHikingVikingRobotProps().ArmToGearRatio;
		m_Potentiometer.Initialize(&writeable_arm_props);
	}
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
}

void HikingViking_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Potentiometer.SetTimeDelta(dTime_s);
	//display voltages
	DOUT2("l=%f r=%f a=%f r=%f D%dC%dR%d\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),m_ArmVoltage,m_RollerVoltage,
		m_Deployment,m_Claw,m_Rist
		);
	SmartDashboard::PutNumber("ArmVoltage",m_ArmVoltage);
	SmartDashboard::PutNumber("RollerVoltage",m_RollerVoltage);
}

//const double c_Arm_DeadZone=0.150;  //was 0.085 for out off
const double c_Arm_DeadZone=0.085;   //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

//void Robot_Control::UpdateVoltage(size_t index,double Voltage)
//{
//}

double HikingViking_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case HikingViking_Robot::eArm:
		{
			const HikingViking_Robot_Props &props=m_RobotProps.GetHikingVikingRobotProps();
			const double c_GearToArmRatio=1.0/props.ArmToGearRatio;
			//result=(m_Potentiometer.GetDistance() * m_RobotProps.GetArmProps().GetRotaryProps().EncoderToRS_Ratio) + 0.0;
			//no conversion needed in simulation
			result=(m_Potentiometer.GetPotentiometerCurrentPosition()) + 0.0;

			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			SmartDashboard::PutNumber("ArmAngle",RAD_2_DEG(result*c_GearToArmRatio));
			const double height= (sin(result*c_GearToArmRatio)*props.ArmLength)+props.GearHeightOffset;
			SmartDashboard::PutNumber("Height",height*3.2808399);
		}
		break;
	}
	return result;
}

#endif
