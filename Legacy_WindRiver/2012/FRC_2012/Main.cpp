#include "WPILib.h"

#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2012_Robot.h"
#include "Common/InOut_Interface.h"
#include "TankDrive/Tank_Robot_Control.h"
#include "TankDrive/Servo_Robot_Control.h"
#include "Common/Debug.h"
#include "FRC2012_InOut_Interface.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;  
		Framework::UI::JoyStick_Binder m_JoyBinder;
		FRC_2012_Robot_Properties m_RobotProps;
		FRC_2012_Robot_Control m_Control; // robot drive system
		FRC_2012_Robot *m_pRobot; //This is a scoped pointer with late binding
		UI_Controller *m_pUI;
		Framework::Base::EventMap m_EventMap;
		IEvent::HandlerList ehl;
	public:
		
		void GoalComplete()
		{
			printf("Goals completed!\n");
		}
		
		SetUp_Manager(bool UseSafety,bool UseEncoders=false) : m_Joystick(3,0), //3 joysticks starting at port 0
			m_JoyBinder(m_Joystick),m_Control(UseSafety),m_pRobot(NULL),m_pUI(NULL)
		{
			m_Control.AsControlInterface().Initialize(&m_RobotProps);
			m_pRobot = new FRC_2012_Robot("FRC2012_Robot",&m_Control,UseEncoders);
			{
				Framework::Scripting::Script script;
				script.LoadScript("/FRC2012Robot.lua",true);
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
			
			//I put this if check to ensure joystick readings were not made but it appears I already have this logic in place
			//when I start in auto pilot, so I need not do this here.  I do want to keep this for future reference... I believe it
			//caused some implicit problems of variables that were set do to non-driving controls.
			//if (!m_pUI->GetAutoPilot())
			
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
		FRC_2012_Robot *GetRobot() const {return m_pRobot;}
		void SetSafety(bool UseSafety) {m_Control.SetSafety(UseSafety);}
		void ResetPos() 
		{	
			//TODO scope this within __DebugLUA__
			#ifdef  __DebugLUA__
			{
				Framework::Scripting::Script script;
				script.LoadScript("/FRC2012Robot.lua",true);
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
		FRC_2012_Robot *Robot=m_Manager.GetRobot();  //we can always cast down
		Robot->SetIsAutonomous(true);
		//m_Manager.GetRobot()->SetUseEncoders(false);

		//assert(ship);
		size_t AutonomousValue=0;
		DriverStation *ds = DriverStation::GetInstance();
		AutonomousValue+=ds->GetDigitalIn(1)? 0x01 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(2)? 0x02 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(3)? 0x04 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(4)? 0x08 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(5)? 0x10 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(6)? 0x20 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(7)? 0x40 : 0x00;
		printf("Autonomous mode= %d \n",AutonomousValue);
		const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
		Goal *oldgoal=Robot->ClearGoal();
		if (oldgoal)
			delete oldgoal;

		if (DoAutonomous)
		{
			if (ds->GetDigitalIn(7)==0)
			{
				//For this year we'll break up into 3 set pair of buttons (at least until vision is working)
				//First set is the key, second the target, and last the ramps.  Once vision is working we can
				//optionally remove key
				const size_t Key_Selection=   (AutonomousValue >> 0) & 3;
				const size_t Target_Selection=(AutonomousValue >> 2) & 3;
				const size_t Ramp_Selection=  (AutonomousValue >> 4) & 3;
				//Translate... the index is center left right, but we want right, left, and center
				const size_t KeyTable[4] = {(size_t)-1,2,1,0};
				const size_t Key=KeyTable[Key_Selection];
				//We'll want to have no buttons also represent the top target to compensate for user error (should always have a target!)
				const size_t TargetTable[4] = {0,2,1,0};
				const size_t Target=TargetTable[Target_Selection];
				const size_t Ramp=KeyTable[Ramp_Selection];
				//Just to be safe check (if they had the other buttons selected)
				if (Key!=(size_t)-1)
				{
					Goal *goal=NULL;
					goal=FRC_2012_Goals::Get_FRC2012_Autonomous(Robot,Key,Target,Ramp);
					if (goal)
						goal->Activate(); //now with the goal(s) loaded activate it
					Robot->SetGoal(goal);
				}
			}
			else
			{
				Goal *goal=NULL;
				goal=FRC_2012_Goals::Get_ShootBalls(Robot,true);
				if (goal)
					goal->Activate(); //now with the goal(s) loaded activate it
				Robot->SetGoal(goal);				
			}
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
		oldgoal=Robot->ClearGoal();
		if (oldgoal)
			delete oldgoal;
		Robot->SetGoal(NULL);
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
			#if 0
			printf("Starting TeleOp Session\n");
			FILE *test=fopen("/FRC2012Robot.lua","r");
			if (test)
			{
				char buffer[80];
				size_t count=fread(buffer,sizeof(char),80,test);
				printf("Bytes Read=%d\nContents:\n%s\n",count,buffer);
				fclose(test);
			}
			else
				printf("failes to open\n");
			#endif
			m_Manager.ResetPos();  //This should avoid errors like the arm swinging backwards
			//m_Manager.GetRobot()->SetUseEncoders(false);
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

