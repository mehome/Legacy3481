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


enum AutonType
{
	eDoNothing,
	eJustMoveForward,
	eJustRotate,
	eSimpleMoveRotateSequence,
	eTestBoxWayPoints,
	eTestArm,
	eArmGrabSequence,
	eTestTurret,
	eArmAndTurretTest,
	eNoAutonTypes
};


  /***********************************************************************************************************************************/
 /*															Curivator_Goals															*/
/***********************************************************************************************************************************/
const double CurivatorGoal_StartingPosition[4]={13.0,4.0,60.0,5.0};
const double CurivatorGoal_HoverPosition[4]={39.0,0.0,90.0,45.0};
const double CurivatorGoal_PickupPosition[4]={39.0,-20.0,90.0,45.0};

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
				const double AutonomousTimeLimit=30.0*60.0; //level 1 30 minutes
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
			#ifdef __UsingTankDrive__
			const double PrecisionTolerance=Robot->GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			#else
			const double PrecisionTolerance=Robot->GetRobotProps().GetSwerveRobotProps().PrecisionTolerance;
			#endif
			Goal_Ship_MoveToPosition *goal_drive=NULL;
			goal_drive=new Goal_Ship_MoveToRelativePosition(Robot->GetController(),wp,true,LockOrientation,PrecisionTolerance);
			return goal_drive;
		}

		static Goal * Rotate(Curivator_Goals_Impl *Parent,double Degrees)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			return new Goal_Ship_RotateToRelativePosition(Robot->GetController(),DEG_2_RAD(Degrees));
		}

		static Goal * Move_TurretPosition(Curivator_Goals_Impl *Parent,double Angle_Deg)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			Curivator_Robot::Robot_Arm &Arm=Robot->GetTurret();
			const double PrecisionTolerance=Robot->GetRobotProps().GetRotaryProps(Curivator_Robot::eTurret).GetRotaryProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			const double position=Angle_Deg;
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,DEG_2_RAD(position),PrecisionTolerance);
			return goal_arm;
		}
		static Goal * Move_ArmXPosition(Curivator_Goals_Impl *Parent,double length_in)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			Curivator_Robot::Robot_Arm &Arm=Robot->GetArmXpos();
			const double PrecisionTolerance=Robot->GetRobotProps().GetRotaryProps(Curivator_Robot::eArm_Ypos).GetRotaryProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			const double position=length_in;
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position,PrecisionTolerance);
			return goal_arm;
		}
		static Goal * Move_ArmYPosition(Curivator_Goals_Impl *Parent,double height_in)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			Curivator_Robot::Robot_Arm &Arm=Robot->GetArmYpos();
			const double PrecisionTolerance=Robot->GetRobotProps().GetRotaryProps(Curivator_Robot::eArm_Xpos).GetRotaryProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			const double position=height_in;
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position,PrecisionTolerance);
			return goal_arm;
		}

		static Goal * Move_BucketAngle(Curivator_Goals_Impl *Parent,double Angle_Deg)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			Curivator_Robot::Robot_Arm &Arm=Robot->GetBucketAngle();
			const double PrecisionTolerance=Robot->GetRobotProps().GetRotaryProps(Curivator_Robot::eBucket_Angle).GetRotaryProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			const double position=Angle_Deg;
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position,PrecisionTolerance);
			return goal_arm;
		}

		static Goal * Move_ClaspAngle(Curivator_Goals_Impl *Parent,double Angle_Deg)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			Curivator_Robot::Robot_Arm &Arm=Robot->GetClaspAngle();
			const double PrecisionTolerance=Robot->GetRobotProps().GetRotaryProps(Curivator_Robot::eClasp_Angle).GetRotaryProps().PrecisionTolerance;
			Goal_Ship1D_MoveToPosition *goal_arm=NULL;
			const double position=Angle_Deg;
			goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position,PrecisionTolerance);
			return goal_arm;
		}

		static Goal * Move_ArmXYPosition(Curivator_Goals_Impl *Parent,double length_in,double height_in)
		{
			MultitaskGoal *goal=new MultitaskGoal(true);
			goal->AddGoal(Move_ArmXPosition(Parent,length_in));
			goal->AddGoal(Move_ArmYPosition(Parent,height_in));
			return goal;
		}

		static Goal * Move_BucketClaspAngle(Curivator_Goals_Impl *Parent,double Bucket_Angle_Deg,double Clasp_Angle_Deg)
		{
			MultitaskGoal *goal=new MultitaskGoal(true);
			goal->AddGoal(Move_BucketAngle(Parent,Bucket_Angle_Deg));
			goal->AddGoal(Move_ClaspAngle(Parent,Clasp_Angle_Deg));
			return goal;
		}

		static Goal * Move_ArmAndBucket(Curivator_Goals_Impl *Parent,double length_in,double height_in,double Bucket_Angle_Deg,double Clasp_Angle_Deg)
		{
			MultitaskGoal *goal=new MultitaskGoal(true);
			//I could have added both multi task goals here, but its easier to debug keeping it more flat lined
			goal->AddGoal(Move_ArmXPosition(Parent,length_in));
			goal->AddGoal(Move_ArmYPosition(Parent,height_in));
			goal->AddGoal(Move_BucketAngle(Parent,Bucket_Angle_Deg));
			goal->AddGoal(Move_ClaspAngle(Parent,Clasp_Angle_Deg));
			return goal;
		}

		class RobotQuickNotify : public AtomicGoal, public SetUpProps
		{
		private:
			std::string m_EventName;
			bool m_IsOn;
		public:
			RobotQuickNotify(Curivator_Goals_Impl *Parent,char *EventName, bool On)	: SetUpProps(Parent),m_EventName(EventName),m_IsOn(On) 
				{	m_Status=eInactive;	
				}
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s)
			{
				ActivateIfInactive();
				m_EventMap.EventOnOff_Map[m_EventName.c_str()].Fire(m_IsOn);
				m_Status=eCompleted;
				return m_Status;
			}
		};

		class RobotArmHoldStill : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			RobotArmHoldStill(Curivator_Goals_Impl *Parent)	: Generic_CompositeGoal(true),SetUpProps(Parent) {	m_Status=eInactive;	}
			virtual void Activate()
			{
				AddSubgoal(new RobotQuickNotify(m_Parent,"Robot_LockPosition",false));
				AddSubgoal(new RobotQuickNotify(m_Parent,"Robot_FreezeArm",false));
				AddSubgoal(new Goal_Wait(0.2));
				AddSubgoal(new RobotQuickNotify(m_Parent,"Robot_FreezeArm",true));
				AddSubgoal(new Goal_Wait(0.4));
				AddSubgoal(new RobotQuickNotify(m_Parent,"Robot_LockPosition",true));
				m_Status=eActive;
			}
		};

		//Drive Tests----------------------------------------------------------------------
		class MoveForward : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			MoveForward(Curivator_Goals_Impl *Parent, bool AutoActivate=false)	: Generic_CompositeGoal(AutoActivate),SetUpProps(Parent) 
			{	
				if(!AutoActivate) 
					m_Status=eActive;	
			}
			virtual void Activate()
			{
				double DistanceFeet=1.0; //should be a safe default
				const char * const RotateSmartVar="TestMove";
				#if defined Robot_TesterCode
				try
				{
					DistanceFeet=SmartDashboard::GetNumber(RotateSmartVar);
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber(RotateSmartVar,DistanceFeet);
				}
				#else
				//Just set it for cRIO
				SmartDashboard::PutNumber(RotateSmartVar,DistanceFeet);
				#endif

				AddSubgoal(new Goal_Wait(0.500));
				AddSubgoal(Move_Straight(m_Parent,DistanceFeet));
				AddSubgoal(new Goal_Wait(0.500));  //allow time for mass to settle
				m_Status=eActive;
			}
		};

		class RotateWithWait : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			RotateWithWait(Curivator_Goals_Impl *Parent, bool AutoActivate=false)	: Generic_CompositeGoal(AutoActivate),SetUpProps(Parent)
			{	
				if(!AutoActivate) 
					m_Status=eActive;	
			}
			virtual void Activate()
			{
				double RotateDegrees=45.0; //should be a safe default
				const char * const RotateSmartVar="TestRotate";
				#if defined Robot_TesterCode
				try
				{
					RotateDegrees=SmartDashboard::GetNumber(RotateSmartVar);
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber(RotateSmartVar,RotateDegrees);
				}
				#else
				//Just set it for cRIO
				SmartDashboard::PutNumber(RotateSmartVar,RotateDegrees);
				#endif

				AddSubgoal(new Goal_Wait(0.500));
				AddSubgoal(Rotate(m_Parent,RotateDegrees));
				AddSubgoal(new Goal_Wait(0.500));  //allow time for mass to settle
				m_Status=eActive;
			}
		};

		class TestMoveRotateSequence : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			TestMoveRotateSequence(Curivator_Goals_Impl *Parent)	: m_pParent(Parent),SetUpProps(Parent) {	m_Status=eActive;	}
			virtual void Activate()
			{
				size_t NoIterations=4;
				#if defined Robot_TesterCode
				try
				{
					NoIterations=SmartDashboard::GetNumber("TestMoveRotateIter");
				}
				catch (...)
				{
					//set up some good defaults for a small box
					SmartDashboard::PutNumber("TestRotate",90.0);
					SmartDashboard::PutNumber("TestMove",1.0);
					SmartDashboard::PutNumber("TestMoveRotateIter",4.0);
				}
				#endif
				for (size_t i=0;i<NoIterations;i++)
				{
					AddSubgoal(new MoveForward(m_pParent,true));
					AddSubgoal(new RotateWithWait(m_pParent,true));
				}
				m_Status=eActive;
			}
		private:
			Curivator_Goals_Impl *m_pParent;
		};

		static Goal * GiveRobotSquareWayPointGoal(Curivator_Goals_Impl *Parent)
		{
			Curivator_Robot *Robot=&Parent->m_Robot;
			const char * const LengthSetting="TestDistance_ft";
			double Length_m=Feet2Meters(1);
			#if defined Robot_TesterCode
			try
			{
				Length_m=Feet2Meters(SmartDashboard::GetNumber(LengthSetting));
			}
			catch (...)
			{
				//set up some good defaults for a small box
				SmartDashboard::PutNumber(LengthSetting,Meters2Feet(Length_m));
			}
			#endif

			std::list <WayPoint> points;
			struct Locations
			{
				double x,y;
			} test[]=
			{
				{Length_m,Length_m},
				{Length_m,-Length_m},
				{-Length_m,-Length_m},
				{-Length_m,Length_m},
				{0,0}
			};
			for (size_t i=0;i<_countof(test);i++)
			{
				WayPoint wp;
				wp.Position[0]=test[i].x;
				wp.Position[1]=test[i].y;
				wp.Power=0.5;
				points.push_back(wp);
			}
			//Now to setup the goal
			Goal_Ship_FollowPath *goal=new Goal_Ship_FollowPath(Robot->GetController(),points,false,true);
			return goal;
		}


		//Arm Tests----------------------------------------------------------------------
		class SetArmWaypoint : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			SetArmWaypoint(Curivator_Goals_Impl *Parent,double length_in,double height_in,double bucket_Angle_deg,double clasp_Angle_deg) : 
			  SetUpProps(Parent),m_length_in(length_in),m_height_in(height_in),m_bucket_Angle_deg(bucket_Angle_deg),m_clasp_Angle_deg(clasp_Angle_deg)
			  {		Activate();  //we can set it up ahead of time
			  }
			virtual void Activate()
			{
				if (m_Status==eActive) return;  //allow for multiple calls
				#if 0
				//Note: order is reversed
				AddSubgoal(new RobotArmHoldStill(m_Parent));
				AddSubgoal(Move_ArmXYPosition(m_Parent,length_in,height_in));
				AddSubgoal(Move_BucketClaspAngle(m_Parent,bucket_Angle_deg,clasp_Angle_deg));
				#else
				AddSubgoal(new RobotArmHoldStill(m_Parent));
				AddSubgoal(Move_ArmAndBucket(m_Parent,m_length_in,m_height_in,m_bucket_Angle_deg,m_clasp_Angle_deg));
				#endif
				m_Status=eActive;
			}
		private:
			const double m_length_in;
			const double m_height_in;
			const double m_bucket_Angle_deg;
			const double m_clasp_Angle_deg;
		};
		class SetTurretWaypoint : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			SetTurretWaypoint(Curivator_Goals_Impl *Parent,double Angle_deg) : SetUpProps(Parent),m_Turret_Angle_deg(Angle_deg)
			{
				Activate(); //go ahead
			}
			virtual void Activate()
			{
				if (m_Status==eActive) return;  //allow for multiple calls
				AddSubgoal(new Goal_Wait(0.500));
				AddSubgoal(Move_TurretPosition(m_Parent,m_Turret_Angle_deg));
				AddSubgoal(new Goal_Wait(0.500));
				m_Status=eActive;
			}
		private:
			const double m_Turret_Angle_deg;
		};
		static Goal * TestArmMove(Curivator_Goals_Impl *Parent)
		{
			double length_in=20.0;
			double height_in=0.0;
			double bucket_Angle_deg=78.0;
			double clasp_Angle_deg=13.0;
			const char * const SmartNames[]={"testarm_length","testarm_height","testarm_bucket","testarm_clasp"};
			double * const SmartVariables[]={&length_in,&height_in,&bucket_Angle_deg,&clasp_Angle_deg};

			//Remember can't do this on cRIO since Thunder RIO has issue with using catch(...)
			#if defined Robot_TesterCode
			for (size_t i=0;i<4;i++)
			{
				try
				{
					*(SmartVariables[i])=SmartDashboard::GetNumber(SmartNames[i]);
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
				}
			}
			#else
			for (size_t i=0;i<_countof(SmartNames);i++)
			{
				if (!SmartDashboard::GetBoolean("TestVariables_set"))
					SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
				else
					*(SmartVariables[i])=SmartDashboard::GetNumber(SmartNames[i]);
			}
			#endif
			return new SetArmWaypoint(Parent,length_in,height_in,bucket_Angle_deg,clasp_Angle_deg);
		}
		static Goal * TestTurretMove(Curivator_Goals_Impl *Parent)
		{
			double clasp_Angle_deg=0.0;
			const char * const SmartName="Test_TurretAngle";

			//Remember can't do this on cRIO since Thunder RIO has issue with using catch(...)
			#if defined Robot_TesterCode
			{
				try
				{
					clasp_Angle_deg=SmartDashboard::GetNumber(SmartName);
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber(SmartName,clasp_Angle_deg);
				}
			}
			#else
			if (!SmartDashboard::GetBoolean("TestVariables_set"))
				SmartDashboard::PutNumber(SmartName,clasp_Angle_deg);
			else
				clasp_Angle_deg=SmartDashboard::GetNumber(SmartName);
			#endif
			return new SetTurretWaypoint(Parent,clasp_Angle_deg);
		}

		class ArmGrabSequence : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			ArmGrabSequence(Curivator_Goals_Impl *Parent,double length_in,double height_in) : 
			  SetUpProps(Parent),m_length_in(length_in),m_height_in(height_in)
			  {		Activate();  //we can set it up ahead of time
			  }
			virtual void Activate()
			{
				if (m_Status==eActive) return;  //allow for multiple calls
				AddSubgoal(new SetArmWaypoint(m_Parent,CurivatorGoal_StartingPosition[0],CurivatorGoal_StartingPosition[1],50.0,-7.0));
				for (double angle=40;angle<=90;angle+=10)
					AddSubgoal(new SetArmWaypoint(m_Parent,m_length_in,m_height_in,angle,-7.0)); //rotate bucket (slowly)
				AddSubgoal(new SetArmWaypoint(m_Parent,m_length_in,m_height_in,CurivatorGoal_PickupPosition[2],-7.0)); //close clasp
				AddSubgoal(new SetArmWaypoint(m_Parent,m_length_in,m_height_in,CurivatorGoal_PickupPosition[2],CurivatorGoal_PickupPosition[3]));  //pickup position
				AddSubgoal(new SetArmWaypoint(m_Parent,m_length_in,CurivatorGoal_HoverPosition[1],CurivatorGoal_HoverPosition[2],CurivatorGoal_HoverPosition[3]));
				//TODO move this to another goal once we start working with the turret
				AddSubgoal(new SetArmWaypoint(m_Parent,CurivatorGoal_StartingPosition[0],CurivatorGoal_StartingPosition[1],CurivatorGoal_StartingPosition[2],CurivatorGoal_StartingPosition[3]));
				m_Status=eActive;
			}
		private:
			const double m_length_in;
			const double m_height_in;
		};

		static Goal * TestArmMove2(Curivator_Goals_Impl *Parent)
		{
			double length_in=38.0;
			double height_in=-20.0;
			const char * const SmartNames[]={"testarm_length","testarm_height"};
			double * const SmartVariables[]={&length_in,&height_in};

			//Remember can't do this on cRIO since Thunder RIO has issue with using catch(...)
			#if defined Robot_TesterCode
			for (size_t i=0;i<2;i++)
			{
				try
				{
					*(SmartVariables[i])=SmartDashboard::GetNumber(SmartNames[i]);
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
				}
			}
			#else
			for (size_t i=0;i<_countof(SmartNames);i++)
			{
				//I may need to prime the pump here
				SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
			}
			#endif
			return new ArmGrabSequence(Parent,length_in,height_in);
		}

		class ArmTurretGrabSequence : public Generic_CompositeGoal, public SetUpProps
		{
		public:
			ArmTurretGrabSequence(Curivator_Goals_Impl *Parent,double length_in,double height_in,double turret_start_deg,double turret_grab_deg) : 
			  SetUpProps(Parent),m_length_in(length_in),m_height_in(height_in),m_turret_start_deg(turret_start_deg),m_turret_grab_deg(turret_grab_deg)
			  {		Activate();  //we can set it up ahead of time
			  }
			  virtual void Activate()
			  {
				  if (m_Status==eActive) return;  //allow for multiple calls
				  AddSubgoal(new SetTurretWaypoint(m_Parent,m_turret_start_deg));
				  AddSubgoal(new ArmGrabSequence(m_Parent,m_length_in,m_height_in));
				  AddSubgoal(new SetTurretWaypoint(m_Parent,m_turret_grab_deg));
				  AddSubgoal(new SetTurretWaypoint(m_Parent,m_turret_start_deg));
				  AddSubgoal(new SetArmWaypoint(m_Parent,CurivatorGoal_StartingPosition[0],CurivatorGoal_StartingPosition[1],CurivatorGoal_StartingPosition[2],CurivatorGoal_StartingPosition[3]));
				  m_Status=eActive;
			  }
		private:
			const double m_length_in;
			const double m_height_in;
			const double m_turret_start_deg;
			const double m_turret_grab_deg;
		};

		static Goal * TestArmAndTurret(Curivator_Goals_Impl * Parent)
		{
			double turret_start_in=0;
			double turret_grab_in=-70.0;
			double length_in=30.0;
			double height_in=-10.0;
			const char * const SmartNames[]={"testTurret_Start","testTurret_Grab","testarm_length","testarm_height"};
			double * const SmartVariables[]={&turret_start_in,&turret_grab_in,&length_in,&height_in};

			//Remember can't do this on cRIO since Thunder RIO has issue with using catch(...)
			#if defined Robot_TesterCode
			for (size_t i=0;i<_countof(SmartNames);i++)
			{
				try
				{
					*(SmartVariables[i])=SmartDashboard::GetNumber(SmartNames[i]);
				}
				catch (...)
				{
					//I may need to prime the pump here
					SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
				}
			}
			#else
			for (size_t i=0;i<_countof(SmartNames);i++)
			{
				//I may need to prime the pump here
				SmartDashboard::PutNumber(SmartNames[i],*(SmartVariables[i]));
			}
			#endif
			return new ArmTurretGrabSequence(Parent,length_in,height_in,turret_start_in,turret_grab_in);
		}
	public:
		Curivator_Goals_Impl(Curivator_Robot &robot) : m_Robot(robot), m_Timer(0.0), 
			m_Primer(false)  //who ever is done first on this will complete the goals (i.e. if time runs out)
		{
			m_Status=eInactive;
		}
		void Activate() 
		{
			//ensure arm is unfrozen... as we are about to move it
			m_Robot.GetEventMap()->EventOnOff_Map["Robot_FreezeArm"].Fire(false);
			m_Primer.AsGoal().Terminate();  //sanity check clear previous session
			typedef Curivator_Robot_Props::Autonomous_Properties Autonomous_Properties;
			//pull parameters from SmartDashboard
			Autonomous_Properties &auton=m_Robot.GetAutonProps();
			//auton.ShowAutonParameters();  //Grab again now in case user has tweaked values

			AutonType AutonTest = (AutonType)auton.AutonTest;
			const char * const AutonTestSelection="AutonTest";
			#if defined Robot_TesterCode
			try
			{
				AutonTest=(AutonType)((size_t)SmartDashboard::GetNumber(AutonTestSelection));
			}
			catch (...)
			{
				//set up some good defaults for a small box
				SmartDashboard::PutNumber(AutonTestSelection,(double)auton.AutonTest);
			}
			#else
			//for cRIO checked in using zero in lua (default) to prompt the variable and then change to -1 to use it
			if (auton.AutonTest!=(size_t)-1)
			{
				SmartDashboard::PutNumber(AutonTestSelection,(double)0.0);
				SmartDashboard::PutBoolean("TestVariables_set",false);
			}
			else
				AutonTest=(AutonType)((size_t)SmartDashboard::GetNumber(AutonTestSelection));
			#endif

			printf("Testing=%d \n",AutonTest);
			switch(AutonTest)
			{
			case eJustMoveForward:
				m_Primer.AddGoal(new MoveForward(this));
				break;
			case eJustRotate:
				m_Primer.AddGoal(new RotateWithWait(this));
				break;
			case eSimpleMoveRotateSequence:
				m_Primer.AddGoal(new TestMoveRotateSequence(this));
				break;
			case eTestBoxWayPoints:
				m_Primer.AddGoal(GiveRobotSquareWayPointGoal(this));
				break;
			case eTestArm:
				m_Primer.AddGoal(TestArmMove(this));
				break;
			case eArmGrabSequence:
				m_Primer.AddGoal(TestArmMove2(this));
				break;
			case eTestTurret:
				m_Primer.AddGoal(TestTurretMove(this));
				break;
			case eArmAndTurretTest:
				m_Primer.AddGoal(TestArmAndTurret(this));
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
	//SmartDashboard::PutNumber("Sequence",1.0);  //ensure we are on the right sequence
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(new Curivator_Goals_Impl(*Robot));
	//MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

