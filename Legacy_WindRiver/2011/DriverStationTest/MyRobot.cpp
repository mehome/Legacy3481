#include <WPILib.h>
#include "DriverStationLCD.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"
#include "Robot_Tank.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"
#include "InOut_Interface.h"
#include "PIDController.h"
#include "FRC2011_Robot.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;  
		Framework::UI::JoyStick_Binder m_JoyBinder;
		FRC_2011_Robot_Properties m_RobotProps;
		Robot_Control m_Control; // robot drive system
		FRC_2011_Robot *m_pRobot; //This is a scoped pointer with late binding
		Framework::Base::EventMap m_EventMap;
		UI_Controller *m_pUI;
	public:
		SetUp_Manager(bool UseSafety,bool UseEncoders=false) : m_Joystick(2,0), //2 joysticks starting at port 0
			m_JoyBinder(m_Joystick),m_Control(UseSafety),m_pRobot(NULL),m_pUI(NULL)
		{
			m_Control.Initialize(&m_RobotProps);
			m_pRobot = new FRC_2011_Robot("FRC2011_Robot",&m_Control,UseEncoders);
			m_pRobot->Initialize(m_EventMap,&m_RobotProps);
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
		}
		void TimeChange(double dTime_s)
		{
			if (dTime_s==0.0) return; //avoid division by zero errors
			m_JoyBinder.UpdateJoyStick(dTime_s);
			m_pRobot->TimeChange(dTime_s);
		}

		~SetUp_Manager()
		{
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
		void ResetPos() {m_pRobot->ResetPos();}
};

Goal *Get_TestLengthGoal(Ship_Tester *ship)
{
	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0.0;
	wp.Position[1]=2.0;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal=new Goal_Ship_MoveToPosition(ship->GetController(),wp);
	return goal;
}

Goal *Get_TestRotationGoal(Ship_Tester *ship)
{
	//Rotate 180 degrees.  (Note: I skipped adding 180 to current heading since we assume it starts at 0)
	Goal_Ship_RotateToPosition *goal=new Goal_Ship_RotateToPosition(ship->GetController(),DEG_2_RAD(180.0));
	return goal;
}

Goal *Get_UberTubeGoal(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal
	Goal_OperateClaw *goal_CloseClaw=new Goal_OperateClaw(*Robot,true);

	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	//Note: full length is 232 inches or 5.89 meters
	const double starting_line=5.49656;  //18.03333
	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);

	wp.Position[1]=starting_line+0.5;
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	Goal_OperateClaw *goal_OpenClaw=new Goal_OperateClaw(*Robot,false);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.0);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm2);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_OpenClaw);
	MainGoal->AddSubgoal(goal_drive2);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	MainGoal->AddSubgoal(goal_CloseClaw);
	return MainGoal;
};

class RobotDemo : public SimpleRobot
{
	SetUp_Manager m_Manager;
public:
	RobotDemo(void) : m_Manager(false) //disable safety by default
	{
		GetWatchdog().SetExpiration(0.1);
	}

	void UpdateDS_LCD()
	{
		DriverStationLCD * lcd = DriverStationLCD::GetInstance();

		lcd->PrintfLine(DriverStationLCD::kMain_Line6, "Press select button");

		lcd->PrintfLine(DriverStationLCD::kUser_Line2, 
			"%d %d %d %d %d %d %d %d",
			(int)m_ds->GetDigitalIn(1),
			(int)m_ds->GetDigitalIn(2),
			(int)m_ds->GetDigitalIn(3),
			(int)m_ds->GetDigitalIn(4),
			(int)m_ds->GetDigitalIn(5),
			(int)m_ds->GetDigitalIn(6),
			(int)m_ds->GetDigitalIn(7),
			(int)m_ds->GetDigitalIn(8)
			);		

		lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: %.1f", 
			m_ds->GetAnalogIn(1));
		lcd->PrintfLine(DriverStationLCD::kUser_Line4, "2: %.1f", 
			m_ds->GetAnalogIn(2));
		lcd->PrintfLine(DriverStationLCD::kUser_Line5, "3: %.1f", 
			m_ds->GetAnalogIn(3));
		lcd->PrintfLine(DriverStationLCD::kUser_Line6, "4: %.1f", 
			m_ds->GetAnalogIn(4));

		lcd->UpdateLCD();
	}

	//Drive left & right motors for 2 seconds then stop
	void Autonomous(void)
	{
		m_Manager.ResetPos();  //We must reset the position to ensure the distance is measured properly
		//autonomous mode cannot have safety on
		m_Manager.SetSafety(false);
		m_Manager.SetAutoPilot(true);  //we are not driving the robot
		//Now to set up our goal
		Ship_Tester *ship=m_Manager.GetRobot();  //we can always cast down
		assert(ship);
		{
			Goal *oldgoal=ship->ClearGoal();
			if (oldgoal)
				delete oldgoal;

			//Goal *goal=Get_TestLengthGoal(ship);
			//Goal *goal=Get_TestRotationGoal(ship);
			Goal *goal=Get_UberTubeGoal(m_Manager.GetRobot());
			goal->Activate(); //now with the goal(s) loaded activate it
			ship->SetGoal(goal);
		}

		double tm = GetTime();
		GetWatchdog().SetEnabled(true);
		while (IsAutonomous() && !IsDisabled())
		{
			GetWatchdog().Feed();
			double time=GetTime() - tm;
			tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			m_Manager.TimeChange(time);
			//This is to simulate a 20ms interval... we'll need to measure the amount of overhead in the real robot
			Wait(0.002);				
			Wait(0.002);				
		}
	}

	void OperatorControl(void)
	{
#if 0
		double tm = GetTime();
		
		GetWatchdog().SetEnabled(true);
		while (IsOperatorControl())
		{
			GetWatchdog().Feed();
			
			if (GetTime() - tm > 0.1)
			{
				UpdateDS_LCD();								
				tm = GetTime();
			}
		}
#endif

		if (c_UseDefaultControls)
		{
			RobotDrive myRobot(1,2,3,4); // robot drive system
			Joystick stick(1); // only 1 joystick
			//myRobot.SetExpiration(0.1);

			// Runs the motors with arcade steering. 
			GetWatchdog().SetEnabled(true);
			while (IsOperatorControl())
			{
				GetWatchdog().Feed();
				myRobot.ArcadeDrive(stick); // drive with arcade style (use right stick)
				Wait(0.005);				// wait for a motor update time
			}
		}
		else
		{
			m_Manager.SetAutoPilot(false);  //we are driving the robot

			double tm = GetTime();
			GetWatchdog().SetEnabled(true);
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			m_Manager.SetSafety(true);
			while (IsOperatorControl())
			{
				GetWatchdog().Feed();
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.020;
				double time=GetTime() - tm;
				tm=GetTime();
				//Framework::Base::DebugOutput("%f\n",time),
				m_Manager.TimeChange(time);
				//60 FPS is well tested with the code.  Since there is more overhead to implement the physics, the idea is to
				//run at a pace that doesn't spike the CPU
				lcd->UpdateLCD();
				Wait(0.004);				
			}
		}
	}
};

START_ROBOT_CLASS(RobotDemo);

