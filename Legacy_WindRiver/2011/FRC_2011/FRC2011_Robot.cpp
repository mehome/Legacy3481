#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2011_Robot.h"

#undef __UseTestKitArmRatios__
namespace Robot_Tester
{
	namespace FRC_2011_Goals
	{
	
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

Goal *Get_TestLengthGoal(FRC_2011_Robot *Robot)
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

static Goal *Get_UberTubeGoal_OLD(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=8.5;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);

	wp.Position[1]=9;
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	wp.Position[1]=8.5;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.0);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm2);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete"); //will fire Complete once it is done
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_waitfordrop);
	//TODO drop claw here
	MainGoal->AddSubgoal(goal_drive2);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	MainGoal->Activate(); //now with the goal(s) loaded activate it
	//Now to subscribe to this event... it will call Stop Loop when the goal is finished
	//Robot->GetEventMap()->Event_Map["Complete"].Subscribe(ehl,*this,&SetUp_Autonomous::StopLoop);
	return MainGoal;
}

Goal *Get_UberTubeGoal(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);  //9 feet
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.7018);   //67 inches
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.08712);   //42.8 inches
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

	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.83312);  //32.8 TODO find how much to lower
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second

	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.0);
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

	} // end namespace FRC 2011 goals
}

const bool c_UsingArmLimits=true;

using namespace Robot_Tester;
//using namespace GG_Framework::Base;
//using namespace osg;
using namespace std;

const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
const double c_ArmLength_m=1.8288;  //6 feet
const double c_ArmToGearRatio=72.0/28.0;
const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
//const double c_PotentiometerToGearRatio=60.0/32.0;
//const double c_PotentiometerToArmRatio=c_PotentiometerToGearRatio * c_GearToArmRatio;
const double c_PotentiometerToArmRatio=36.0/54.0;
const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;
//const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Claw														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Claw::Robot_Claw(const char EntityName[],Robot_Control_Interface *robot_control) :
	Ship_1D(EntityName),m_RobotControl(robot_control),m_Grip(false),m_Squirt(false)
{
}

void FRC_2011_Robot::Robot_Claw::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;
	const double MaxSpeed=m_Ship_1D_Props.MAX_SPEED;
	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Grip ^ m_Squirt)
		SetCurrentLinearAcceleration(m_Grip?Accel:-Brake);

	__super::TimeChange(dTime_s);
	//send out the voltage
	double CurrentVelocity=m_Physics.GetVelocity();
	double Voltage=CurrentVelocity/MaxSpeed;

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
	Entity2D_Kind::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
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

FRC_2011_Robot::Robot_Arm::Robot_Arm(const char EntityName[],Arm_Control_Interface *robot_control,size_t InstanceIndex) : 
	Ship_1D(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
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

void FRC_2011_Robot::Robot_Arm::Initialize(Entity2D_Kind::EventMap& em,const Entity1D_Properties *props)
{
	m_LastPosition=m_RobotControl->GetArmCurrentPosition(m_InstanceIndex)*c_ArmToGearRatio;
	__super::Initialize(em,props);
	const Ship_1D_Properties *ship=dynamic_cast<const Ship_1D_Properties *>(props);
	assert(ship);
	m_MaxSpeedReference=ship->GetMaxSpeed();
	m_PIDController.SetInputRange(-m_MaxSpeedReference,m_MaxSpeedReference);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDController.SetOutputRange(-m_MaxSpeedReference*tolerance,m_MaxSpeedReference*tolerance);
	m_PIDController.Enable();
	m_CalibratedScaler=m_Ship_1D_Props.MAX_SPEED;
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
			double NewPosition=m_RobotControl->GetArmCurrentPosition(m_InstanceIndex)*c_ArmToGearRatio;

			//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
			double Displacement=NewPosition-m_LastPosition;
			PotentiometerVelocity=Displacement/m_LastTime;
			double PotentiometerSpeed=fabs(PotentiometerVelocity);

			double control=0.0;
			control=-m_PIDController(LastSpeed,PotentiometerSpeed,dTime_s);
			m_CalibratedScaler=m_Ship_1D_Props.MAX_SPEED+control;

			//DOUT5("pSpeed=%f cal=%f Max=%f",PotentiometerSpeed,m_CalibratedScaler,m_MaxSpeed);
			//printf("\rpSp=%f cal=%f Max=%f                 ",PotentiometerSpeed,m_CalibratedScaler,m_MaxSpeed);

			SetPos_m(NewPosition);
			m_LastPosition=NewPosition;
		}
		m_LastTime=dTime_s;
	}
	else
	{
		//Test potentiometer readings without applying to current position (disabled by default)
		m_RobotControl->GetArmCurrentPosition(m_InstanceIndex);
		//This is only as a sanity fix for manual mode... it should be this already (I'd assert if I could)
		//m_MaxSpeed=m_CalibratedScaler=1.0;
	}
	__super::TimeChange(dTime_s);
	double CurrentVelocity=m_Physics.GetVelocity();
	//Unfortunately something happened when the wires got crossed during the texas round up, now needing to reverse the voltage
	//This was also reversed for the testing kit.  We apply reverse on current velocity for squaring operation to work properly, and
	//must not do this in the interface, since that will support next year's robot.
	CurrentVelocity*=-1.0; 
	double Voltage=CurrentVelocity/m_CalibratedScaler;

	//Keep voltage override disabled for simulation to test precision stability
	//if (!m_VoltageOverride)
	if (true)
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

	m_RobotControl->UpdateArmVoltage(m_InstanceIndex,Voltage);
	//Show current height (only in Robot Tester)
	#ifdef Robot_TesterCode
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
	if ((m_UsingPotentiometer)&&(!GetLockShipToPosition())&&(fabs(posDisplacement_m)<0.1))
		m_VoltageOverride=true;
}

void FRC_2011_Robot::Robot_Arm::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	if (m_UsingPotentiometer)
	{
		m_PIDController.Reset();
		m_RobotControl->Reset_Arm(m_InstanceIndex);
		double NewPosition=m_RobotControl->GetArmCurrentPosition(m_InstanceIndex)*c_ArmToGearRatio;
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
			//m_MaxSpeed=m_MaxSpeedReference;
			m_LastPosition=0.0;
			m_CalibratedScaler=m_Ship_1D_Props.MAX_SPEED;
			m_LastTime=0.0;
			m_Ship_1D_Props.UsingRange=false;
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
			m_Ship_1D_Props.UsingRange=true;
			m_CalibratedScaler=m_Ship_1D_Props.MAX_SPEED;
		}
	}
}

static double ArmHeightToBack(double value)
{
	const double Vertical=Pi/2.0*c_ArmToGearRatio;
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
	SetIntendedPosition( HeightToAngle_r(0.0) );
}
void FRC_2011_Robot::Robot_Arm::SetPos3feet()
{
	//Not used, but kept for reference
	//SetIntendedPosition(ArmHeightToBack( HeightToAngle_r(1.143)) );
	SetIntendedPosition(HeightToAngle_r(0.9144));
}
void FRC_2011_Robot::Robot_Arm::SetPos6feet()
{
	SetIntendedPosition( HeightToAngle_r(1.8288) );
}
void FRC_2011_Robot::Robot_Arm::SetPos9feet()
{
	SetIntendedPosition( HeightToAngle_r(2.7432) );
}
void FRC_2011_Robot::Robot_Arm::CloseRist(bool Close)
{
	m_RobotControl->CloseRist(Close);
}

void FRC_2011_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
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
FRC_2011_Robot::FRC_2011_Robot(const char EntityName[],FRC_2011_Control_Interface *robot_control,bool UseEncoders) : 
	Tank_Robot(EntityName,robot_control,UseEncoders), m_RobotControl(robot_control), m_Arm(EntityName,robot_control), m_Claw(EntityName,robot_control)
{
}

void FRC_2011_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const FRC_2011_Robot_Properties *RobotProps=dynamic_cast<const FRC_2011_Robot_Properties *>(props);
	m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
	m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);
}
void FRC_2011_Robot::ResetPos()
{
	__super::ResetPos();
	m_Arm.ResetPos();
	m_Claw.ResetPos();
}

void FRC_2011_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	arm_entity.TimeChange(dTime_s);
	Entity1D &claw_entity=m_Claw;  //This gets around keeping time change protected in derived classes
	claw_entity.TimeChange(dTime_s);
}

void FRC_2011_Robot::CloseDeploymentDoor(bool Close)
{
	m_RobotControl->CloseSolenoid(eDeployment,Close);
}

void FRC_2011_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
		em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &FRC_2011_Robot::CloseDeploymentDoor);
	else
		em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &FRC_2011_Robot::CloseDeploymentDoor);

	Ship_1D &ArmShip_Access=m_Arm;
	ArmShip_Access.BindAdditionalEventControls(Bind);
	Ship_1D &ClawShip_Access=m_Claw;
	ClawShip_Access.BindAdditionalEventControls(Bind);
}

#ifdef Robot_TesterCode
  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2011_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	switch (index)
	{
		case FRC_2011_Robot::eArm:
		{
			//	printf("Arm=%f\n",Voltage);
			//DOUT3("Arm Voltage=%f",Voltage);
			m_ArmVoltage=Voltage;
			//Note: I have to reverse the voltage again since the wires are currently crossed on the robot
			m_Potentiometer.UpdatePotentiometerVoltage(-Voltage);
			m_Potentiometer.TimeChange();  //have this velocity immediately take effect
		}
			break;
		case FRC_2011_Robot::eRollers:
			m_RollerVoltage=Voltage;
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
	}
}
void FRC_2011_Robot_Control::CloseSolenoid(size_t index,bool Close)
{
	switch (index)
	{
		case FRC_2011_Robot::eDeployment:
			DebugOutput("CloseDeploymentDoor=%d\n",Close);
			m_Deployment=Close;
			break;
		case FRC_2011_Robot::eClaw:
			DebugOutput("CloseClaw=%d\n",Close);
			m_Claw=Close;
			//This was used to test error with the potentiometer
			//m_Potentiometer.SetBypass(Close);
			break;
		case FRC_2011_Robot::eRist:
			DebugOutput("CloseRist=%d\n",Close);
			m_Rist=Close;
			break;
	}
}


FRC_2011_Robot_Control::FRC_2011_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_ArmVoltage(0.0),m_RollerVoltage(0.0),
	m_Deployment(false),m_Claw(false),m_Rist(false)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
}

void FRC_2011_Robot_Control::Reset_Arm(size_t index)
{
	m_KalFilter_Arm.Reset();
}

void FRC_2011_Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2011_Robot_Properties *robot_props=dynamic_cast<const FRC_2011_Robot_Properties *>(props);

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		assert(robot_props);
		m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
	}
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
}

void FRC_2011_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Potentiometer.SetTimeDelta(dTime_s);
	//display voltages
	DOUT2("l=%f r=%f a=%f r=%f D%dC%dR%d\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),m_ArmVoltage,m_RollerVoltage,
		m_Deployment,m_Claw,m_Rist
		);
}

//const double c_Arm_DeadZone=0.150;  //was 0.085 for out off
const double c_Arm_DeadZone=0.085;   //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

//void Robot_Control::UpdateVoltage(size_t index,double Voltage)
//{
//}

double FRC_2011_Robot_Control::GetArmCurrentPosition(size_t index)
{
	double result=m_Potentiometer.GetPotentiometerCurrentPosition()*c_PotentiometerToArmRatio;
	//result = m_KalFilter_Arm(result);  //apply the Kalman filter
	return result;
}

#endif

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
	Ship_1D_Props::eRobotArm,
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
	Ship_1D_Props::eSimpleMotor,
	false	//No limit ever!
	)
{
	Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

	//Late assign this to override the initial default
	props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
	props.WheelDiameter=c_WheelDiameter;
	props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
	props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
	m_TankRobotProps=props;
}

  /***********************************************************************************************************************************/
 /*														Goal_OperateSolenoid														*/
/***********************************************************************************************************************************/

namespace FRC_2011_Goals
{

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

}
