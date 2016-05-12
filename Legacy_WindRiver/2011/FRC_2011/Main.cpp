#include "WPILib.h"

#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2011_Robot.h"
#include "Common/InOut_Interface.h"
#include "TankDrive/Tank_Robot_Control.h"
#include "TankDrive/Servo_Robot_Control.h"
#include "Common/Debug.h"
#include "FRC2011_InOut_Interface.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;  
		Framework::UI::JoyStick_Binder m_JoyBinder;
		FRC_2011_Robot_Properties m_RobotProps;
		FRC_2011_Robot_Control m_Control; // robot drive system
		FRC_2011_Robot *m_pRobot; //This is a scoped pointer with late binding
		Framework::Base::EventMap m_EventMap;
		IEvent::HandlerList ehl;
		UI_Controller *m_pUI;
	public:
		
		void GoalComplete()
		{
			printf("Goals completed!\n");
		}
		
		SetUp_Manager(bool UseSafety,bool UseEncoders=false) : m_Joystick(2,0), //2 joysticks starting at port 0
			m_JoyBinder(m_Joystick),m_Control(UseSafety),m_pRobot(NULL),m_pUI(NULL)
		{
			m_Control.AsControlInterface().Initialize(&m_RobotProps);
			m_pRobot = new FRC_2011_Robot("FRC2011_Robot",&m_Control,UseEncoders);
			{
				Framework::Scripting::Script script;
				script.LoadScript("/FRC2011Robot.lua",true);
				script.NameMap["EXISTING_ENTITIES"] = "EXISTING_SHIPS";
				m_RobotProps.SetUpGlobalTable(script);
				m_RobotProps.LoadFromScript(script);
				m_pRobot->Initialize(m_EventMap,&m_RobotProps);
			}
			//Bind the ship's eventmap to the joystick
			m_JoyBinder.SetControlledEventMap(m_pRobot->GetEventMap());

			//To to bind the UI controller to the robot
			AI_Base_Controller *controller=m_pRobot->GetController();
			assert(controller);
			m_pUI=new UI_Controller(m_JoyBinder,controller); 
			if (controller->Try_SetUIController(m_pUI))
			{
				//Success... now to let the entity set things up
				m_pUI->HookUpUI(true);
			}
			else
			{
				m_pUI->Set_AI_Base_Controller(NULL);   //no luck... flush ship association
				assert(false);
			}
			//This is for testing purposes
			m_EventMap.Event_Map["Complete"].Subscribe(ehl,*this,&SetUp_Manager::GoalComplete);
		}
		void TimeChange(double dTime_s)
		{
			if (dTime_s==0.0) return; //avoid division by zero errors
			m_JoyBinder.UpdateJoyStick(dTime_s);
			//This is called implicitly within the robot (for ease of compatability with AI)
			//m_Control.TimeChange(dTime_s);
			m_pRobot->TimeChange(dTime_s);
		}

		~SetUp_Manager()
		{
			m_EventMap.Event_Map["Complete"].Remove(*this,&SetUp_Manager::GoalComplete);

			//Note: in visual studio the delete pointer implicitly checks for NULL, but I do not want to assume this for wind river.
			if (m_pUI)
			{
				delete m_pUI;
				m_pUI=NULL;
			}
			if (m_pRobot)
			{
				delete m_pRobot;
				m_pRobot=NULL;
			}
		}

		void SetAutoPilot(bool autoPilot) {m_pUI->SetAutoPilot(autoPilot);}
		FRC_2011_Robot *GetRobot() const {return m_pRobot;}
		void SetSafety(bool UseSafety) {m_Control.SetSafety(UseSafety);}
		void ResetPos() 
		{	
			//TODO scope this within __DebugLUA__
			#ifdef  __DebugLUA__
			{
				Framework::Scripting::Script script;
				script.LoadScript("/FRC2011Robot.lua",true);
				script.NameMap["EXISTING_ENTITIES"] = "EXISTING_SHIPS";

				m_RobotProps.SetUpGlobalTable(script);
				m_RobotProps.LoadFromScript(script);
				m_pRobot->Initialize(m_EventMap,&m_RobotProps);
			}
			#endif
			m_pRobot->ResetPos();
			m_Control.ResetPos();
		}
};

float GetDS_Distance()
{
	DriverStation *ds = DriverStation::GetInstance();
	float Multiplier=ds->GetDigitalIn(8)?2:1;
	float position=ds->GetAnalogIn(1);
	position*=Multiplier;
	return position;
}

Goal *Get_TestLengthGoal(FRC_2011_Robot *Robot)
{
	//float position=DriverStation::GetInstance()->GetAnalogIn(1);
	float position=GetDS_Distance();
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

Goal *Get_TestRotationGoal(Ship_Tester *ship)
{
	//Rotate 180 degrees.  (Note: I skipped adding 180 to current heading since we assume it starts at 0)
	Goal_Ship_RotateToPosition *goal=new Goal_Ship_RotateToPosition(ship->GetController(),DEG_2_RAD(180.0));
	return goal;
}

Goal *Get_UberTubeGoal(FRC_2011_Robot *Robot)
{
	using namespace FRC_2011_Goals;
	Ship_1D &Arm=Robot->GetArm();

	//Now to setup the goal

	//This must happen first to ensure the elbow starts to open at the correct angle
	Goal_Ship1D_MoveToPosition *goal_arm_initialrest=new Goal_Ship1D_MoveToPosition(Arm,FRC_2011_Robot::Robot_Arm::GetPosRest());
	Goal_OperateSolenoid *goal_CloseClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,true);

	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);  //9 feet
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.7018);   //67 inches
	//give ability to tweak the correct height
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r((double)DriverStation::GetInstance()->GetAnalogIn(2));
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	//Note: full length is 232 inches or 5.89 meters
	//const double starting_line=5.49656;  //18.03333
	const double starting_line=(double)GetDS_Distance();

	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);
	//We are going to score on 6 foot peg with elbow left close
	//Goal_OperateSolenoid *goal_OpenElbow=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eElbow,false);
	//Initial_Start_Goal->AddGoal(goal_OpenElbow);

	wp.Position[1]=starting_line+0.4572; //roughly 18 inches (Ideal would be half the distance of the pegs length)
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	
	Goal_OperateSolenoid *goal_OpenClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,false);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	
	//Note: Move the robot back enough to clear the peg without lowering the arm here (we will score by this point) ;)
	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	//Note: the last position could be back at start, but for now to be safe lets just go half the distance, unless its ability
	//to drive straight is doing better
	wp.Position[1]=starting_line/2.0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(-0.120);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *goal_arm_initialrest2=new Goal_Ship1D_MoveToPosition(Arm,0.06);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm_initialrest2);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_arm2);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_OpenClaw);
	MainGoal->AddSubgoal(goal_drive2);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	//Note: I need not bother multi-task these since the close claw is so quick
	MainGoal->AddSubgoal(goal_arm_initialrest);
	MainGoal->AddSubgoal(goal_CloseClaw);
	return MainGoal;
};

Goal *Get_UberTubeGoal_V2(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);  //9 feet
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.7018);   //67 inches
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.08712);   //42.8 inches
	//give ability to tweak the correct height
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(
			(double)DriverStation::GetInstance()->GetAnalogIn(2));
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	//Note: full length is 232 inches or 5.89 meters
	//const double starting_line=5.49656;  //18.03333
	//const double starting_line=2.3; //hack not calibrated
	//give ability to tweak on driver station
	//const double starting_line=(double)DriverStation::GetInstance()->GetAnalogIn(1);
	const double starting_line=(double)GetDS_Distance();
	
	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);

	//This is a hack only needed if we cannot use encoders.  This gives robot time to slow down.
	//Goal_Wait *goal_waitforstop=new Goal_Wait(1.0); 

	wp.Position[1]=starting_line+0.1;
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.83312);  //32.8 TODO find how much to lower
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second

	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	//wp.Position[1]=0;
	//instead of going all the way back to start we'll go half the distance (since the robot is having hard time going straight)
	wp.Position[1]=starting_line/2.0;
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
	//MainGoal->AddSubgoal(goal_waitforstop);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	return MainGoal;
};

Goal *Test_Arm(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal

	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.08712);   //42.8 inches
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(
			(double)DriverStation::GetInstance()->GetAnalogIn(2));

	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *Initial_Start_Goal=goal_arm;  //using the same variable name

	Goal_Wait *goal_waitfordrop=new Goal_Wait(5.0); //wait a half a second
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(-0.120);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);
	Goal_Wait *goal_waitfordrive=new Goal_Wait(2.0); //wait a half a second

	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(FRC_2011_Robot::Robot_Arm::GetPosRest());
	Goal_Ship1D_MoveToPosition *goal_arm3=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *End_Goal=goal_arm3;

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_waitfordrive);
	MainGoal->AddSubgoal(goal_arm2);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	return MainGoal;
};


Goal *Get_TestArmElbowClaw(FRC_2011_Robot *Robot)
{
	using namespace FRC_2011_Goals;
	Ship_1D &Arm=Robot->GetArm();

	//Now to setup the goal

	//This must happen first to ensure the elbow starts to open at the correct angle
	Goal_Ship1D_MoveToPosition *goal_arm_initialrest=new Goal_Ship1D_MoveToPosition(Arm,FRC_2011_Robot::Robot_Arm::GetPosRest());
	Goal_OperateSolenoid *goal_CloseClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,true);

	//give ability to tweak the correct height
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r((double)DriverStation::GetInstance()->GetAnalogIn(2));
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);


	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	//Goal_OperateSolenoid *goal_OpenElbow=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eElbow,false);
	//Initial_Start_Goal->AddGoal(goal_OpenElbow);

	Goal_OperateSolenoid *goal_OpenClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,false);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(-0.120);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *End_Goal=goal_arm2;

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_OpenClaw);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	//Note: I need not bother multi-task these since the close claw is so quick
	MainGoal->AddSubgoal(goal_arm_initialrest);
	MainGoal->AddSubgoal(goal_CloseClaw);
	return MainGoal;
};

//This is the main robot class used for FRC 2011 
//The SimpleRobot class is the base of a robot application that will automatically call your
 //Autonomous and OperatorControl methods at the right time as controlled by the switches on the driver station or the field controls.
class Robot_Main : public SimpleRobot
{
	SetUp_Manager m_Manager;

public:
	Robot_Main(void) : m_Manager(false) //disable safety by default
	{
	}
	
	void Autonomous(void)
	{
		m_Manager.ResetPos();  //We must reset the position to ensure the distance is measured properly
		//autonomous mode cannot have safety on
		m_Manager.SetSafety(false);
		m_Manager.SetAutoPilot(true);  //we are not driving the robot
		//Now to set up our goal
		Ship_Tester *ship=m_Manager.GetRobot();  //we can always cast down
		//m_Manager.GetRobot()->SetUseEncoders(true);
		m_Manager.GetRobot()->SetIsAutonomous(true);

		//assert(ship);
		size_t AutonomousValue=0;
		DriverStation *ds = DriverStation::GetInstance();
		AutonomousValue+=ds->GetDigitalIn(1)? 0x01 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(2)? 0x02 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(3)? 0x04 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(4)? 0x08 : 0x00;
		printf("Autonomous mode= %d \n",AutonomousValue);
		const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
		Goal *oldgoal=ship->ClearGoal();
		if (oldgoal)
			delete oldgoal;

		if (DoAutonomous)
		{
			Goal *goal=NULL;
			switch (AutonomousValue)
			{
				case 1:		goal=Test_Arm(m_Manager.GetRobot());				break;
				case 2:		goal=Get_TestLengthGoal(m_Manager.GetRobot());		break;
				//case 3:		goal=Get_TestRotationGoal(ship);				break;
				case 3:		goal=Get_TestArmElbowClaw(m_Manager.GetRobot());	break;
				case 4:		goal=Get_UberTubeGoal(m_Manager.GetRobot());		break;
			}
			if (goal)
				goal->Activate(); //now with the goal(s) loaded activate it
			ship->SetGoal(goal);
		}
		
		double tm = GetTime();
		
		while (IsAutonomous() && !IsDisabled())
		{
			double time=GetTime() - tm;
			tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			m_Manager.TimeChange(time);
			//using this from test runs from robo wranglers code
			Wait(0.010);				
		}
		printf("Autonomouse loop end IsA=%d IsD=%d \n",IsAutonomous(),IsDisabled());
		oldgoal=ship->ClearGoal();
		if (oldgoal)
			delete oldgoal;
		ship->SetGoal(NULL);

	}
	
	void OperatorControl(void)
	{
		if (c_UseDefaultControls)
		{
			RobotDrive myRobot(1,2,3,4); // robot drive system
			Joystick stick(1); // only 1 joystick
			myRobot.SetExpiration(0.1);
			
			// Runs the motors with arcade steering. 
			myRobot.SetSafetyEnabled(true);
			while (IsOperatorControl())
			{
				myRobot.ArcadeDrive(stick); // drive with arcade style (use right stick)
				Wait(0.005);				// wait for a motor update time
			}
		}
		else
		{
			printf("Starting TeleOp Session\n");
			m_Manager.ResetPos();  //This should avoid errors like the arm swinging backwards
			m_Manager.GetRobot()->SetIsAutonomous(false);
			m_Manager.SetAutoPilot(false);  //we are driving the robot
			double tm = GetTime();
			m_Manager.SetSafety(true);
			while (IsOperatorControl() && !IsDisabled())
			{
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.020;
				double time=GetTime() - tm;
				tm=GetTime();
				//Framework::Base::DebugOutput("%f\n",time),
				m_Manager.TimeChange(time);
				//using this from test runs from robo wranglers code
				Wait(0.010);
			}
		}
	}
};

START_ROBOT_CLASS(Robot_Main);

