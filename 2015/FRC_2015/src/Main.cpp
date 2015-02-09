#include "WPILib.h"

#include "stdafx.h"
#include "Robot_Tester.h"

#include "FRC2015_Robot.h"
#include "Common/src/InOut_Interface.h"
#include "TankDrive/src/Tank_Robot_Control.h"
#include "TankDrive/src/Servo_Robot_Control.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;
		Framework::UI::JoyStick_Binder m_JoyBinder;
		FRC_2015_Robot_Properties m_RobotProps;
		FRC_2015_Robot_Control m_Control; // robot drive system
		FRC_2015_Robot *m_pRobot; //This is a scoped pointer with late binding
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
			SmartDashboard::init();
			m_pRobot = new FRC_2015_Robot("FRC2015_Robot",&m_Control,UseEncoders);
			{
				Framework::Scripting::Script script;
				script.LoadScript("/FRC2015Robot.lua",true);
				script.NameMap["EXISTING_ENTITIES"] = "EXISTING_SHIPS";
				m_RobotProps.SetUpGlobalTable(script);
				m_RobotProps.LoadFromScript(script);
				m_Joystick.SetSlotList(m_RobotProps.Get_RobotControls().GetDriverStation_SlotList());
				m_pRobot->Initialize(m_EventMap,&m_RobotProps);
			}
			//The script must be loaded before initializing control since the wiring assignments are set there
			m_Control.AsControlInterface().Initialize(&m_RobotProps);

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
		FRC_2015_Robot *GetRobot() const {return m_pRobot;}
		void SetSafety(bool UseSafety) {m_Control.SetSafety(UseSafety);}
		void ResetPos()
		{
			//TODO scope this within __DebugLUA__
			#ifdef  __DebugLUA__
			{
				Framework::Scripting::Script script;
				script.LoadScript("/FRC2015Robot.lua",true);
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


//This is the main robot class used for FRC
//The SimpleRobot class is the base of a robot application that will automatically call your
 //Autonomous and OperatorControl methods at the right time as controlled by the switches on the driver station or the field controls.
#ifdef __USE_LEGACY_WPI_LIBRARIES__
class Robot_Main : public SimpleRobot
#else
class Robot_Main : public SampleRobot
#endif
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
		FRC_2015_Robot *Robot=m_Manager.GetRobot();  //we can always cast down
		Robot->SetIsAutonomous(true);
		//This was only needed because we couldn't script case where both auton and teleop didn't have encoders
		//Keep here if for some reason it doesn't work properly
		//m_Manager.GetRobot()->SetUseEncoders(false);

		//assert(ship);
		Goal *oldgoal=Robot->ClearGoal();
		if (oldgoal)
			delete oldgoal;

		const bool DoAutonomous=true;
		if (DoAutonomous)
		{
			Goal *goal=NULL;
			goal=FRC_2015_Goals::Get_FRC2015_Autonomous(Robot);
			if (goal)
				goal->Activate(); //now with the goal(s) loaded activate it
			Robot->SetGoal(goal);
		}

		double LastTime = GetTime();

		while (IsAutonomous() && !IsDisabled())
		{
			const double CurrentTime=GetTime();
			const double DeltaTime=CurrentTime - LastTime;
			LastTime=CurrentTime;
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			m_Manager.TimeChange(DeltaTime);
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
			FILE *test=fopen("/FRC2015Robot.lua","r");
			if (test)
			{
				char buffer[80];
				size_t count=fread(buffer,sizeof(char),80,test);
				printf("Bytes Read=%d\nContents:\n%s\n",count,buffer);
				fclose(test);
			}
			else
				printf("failed to open\n");
			#endif
			//Set Autonomous must happen before reset pos... as that will implicitly reload script and there are some checks for auton (like setting gears)
			m_Manager.GetRobot()->SetIsAutonomous(false);
			m_Manager.ResetPos();  //This should avoid errors like the arm swinging backwards
			//m_Manager.GetRobot()->SetUseEncoders(false);
			m_Manager.SetAutoPilot(false);  //we are driving the robot
			double LastTime = GetTime();
			m_Manager.SetSafety(true);
			while (IsOperatorControl() && !IsDisabled())
			{
				const double CurrentTime=GetTime();
				//I'll keep this around as a synthetic time option for debug purposes
				//const double DeltaTime=0.020;
				const double DeltaTime=CurrentTime - LastTime;
				LastTime=CurrentTime;
				//Framework::Base::DebugOutput("%f\n",time),
				m_Manager.TimeChange(DeltaTime);
				//using this from test runs from robo wranglers code
				Wait(0.010);
			}
		}
	}
};

START_ROBOT_CLASS(Robot_Main);

