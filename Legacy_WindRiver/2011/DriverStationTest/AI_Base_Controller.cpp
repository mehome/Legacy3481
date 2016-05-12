#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"

using namespace Framework::Base;

const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*														AI_Base_Controller															*/
/***********************************************************************************************************************************/

AI_Base_Controller::AI_Base_Controller(Ship_2D &ship) : m_Goal(NULL),m_ship(ship),m_UI_Controller(NULL)
{
}

void AI_Base_Controller::UpdateController(double dTime_s)
{
	// Is the player controlling this ship, or is the AI?
	if (m_UI_Controller)
	{
		// The player is controlling with a UI, tell my AI_Reaction to reset so it starts over next time
		m_UI_Controller->UpdateController(dTime_s);
	}
	else
	{
		//Attempt to update a goal (if one exists)
		if (m_Goal)
			m_Goal->Process(dTime_s);
	}
}

void AI_Base_Controller::UpdateUI(double dTime_s)
{
	//UpdateTargetLeadPoint();
	if (m_UI_Controller)
		m_UI_Controller->UpdateUI(dTime_s);
}

bool AI_Base_Controller::Try_SetUIController(UI_Controller *controller)
{
	//So far we have had access to all.  We may want to derive some logic here or establish it at this level (to be determined). 
	m_UI_Controller=controller;
	return true;
}

void AI_Base_Controller::DriveToLocation(Vec2d TrajectoryPoint,Vec2d PositionPoint, double power, double dTime_s,Vec2d* matchVel,bool LockOrientation)
{
	//Supposedly _isnan should be available, but isn't defined in math.h... Oh well I don't need this overhead anyhow
	#if 0
	if (	_isnan(TrajectoryPoint[0]) ||
			_isnan(TrajectoryPoint[1]) ||
			_isnan(PositionPoint[0]) ||
			_isnan(PositionPoint[1]) ||
			_isnan(power) ||
			_isnan(dTime_s) ||
			(matchVel && (
				_isnan((*matchVel)[0]) ||
				_isnan((*matchVel)[1]))))
	{
		printf("TrajectoryPoint = (%f,%f)\n", TrajectoryPoint[0], TrajectoryPoint[1]);
		printf("PositionPoint = (%f,%f)\n", PositionPoint[0], PositionPoint[1]);
		if (matchVel)
			printf("matchVel = (%f,%f)\n", (*matchVel)[0], (*matchVel)[1] );
		printf("dTime_s = %f, power = %f\n", dTime_s, power);
		assert(false);
	}
	#endif

	Vec2d VectorOffset=TrajectoryPoint-m_ship.GetPos_m();

	if (!LockOrientation)
	{
		double AngularDistance=m_ship.m_IntendedOrientationPhysics.ComputeAngularDistance(VectorOffset);
		//printf("\r %f          ",RAD_2_DEG(AngularDistance));
		m_ship.SetCurrentAngularAcceleration(-AngularDistance,false);
	}

	//first negotiate the max speed given the power
	double MaxSpeed=m_ship.ENGAGED_MAX_SPEED;
	double ScaledSpeed=MaxSpeed;
	{
		if ((power >= 0.0) && (power <= 1.0))
		{
			//Now to compute the speed based on MAX (Note it is up to the designer to make the power smaller in tight turns
			ScaledSpeed= MaxSpeed * power;
			//DEBUG_AUTO_PILOT_SPEED("\rRamora Speeds: Max=%4.1f, Power = %3.1f, Curr = %3.1f",MaxSpeed, power, m_ship.m_Physics.GetLinearVelocity().length());
		}
		else if (power>1.0)
			SetShipVelocity(MIN(power, MaxSpeed));
	}

	if (matchVel)
	{
		VectorOffset=PositionPoint-m_ship.GetPos_m();
		//Vec2d LocalVectorOffset(m_ship.GetAtt_quat().conj() * VectorOffset);
		Vec2d LocalVectorOffset=GlobalToLocal(m_ship.GetAtt_r(),VectorOffset);
		//Vec2d LocalMatchVel(m_ship.GetAtt_quat().conj() * (*matchVel));
		Vec2d LocalMatchVel=GlobalToLocal(m_ship.GetAtt_r(),*matchVel);

		Vec2d ForceRestraintPositive(m_ship.MaxAccelRight*m_ship.Mass,m_ship.MaxAccelForward*m_ship.Mass);
		Vec2d ForceRestraintNegative(m_ship.MaxAccelLeft*m_ship.Mass,m_ship.MaxAccelReverse*m_ship.Mass);
		//Note: it is possible to overflow in extreme distances, if we challenge this then I should have an overflow check in physics
		Vec2d LocalVelocity=m_ship.m_Physics.GetVelocityFromDistance_Linear(LocalVectorOffset,ForceRestraintPositive,ForceRestraintNegative,dTime_s, LocalMatchVel);

		//The logic here should make use of making coordinated turns anytime the forward/reverse velocity has a greater distance than the sides or up/down.
		//Usually if the trajectory point is the same as the position point it will perform coordinated turns most of the time while the nose is pointing
		//towards its goal.  If the nose trajectory is different it may well indeed use the strafing technique more so.

		if (fabs(LocalVelocity[0])<fabs(LocalVelocity[1]))
		{
			//This first technique only works with the forward and partial reverse thrusters (may be useful for some ships)
			//Note: Even though this controls forward and reverse thrusters, the strafe thrusters are still working implicitly to correct turn velocity

			//Now we simply use the positive forward thruster 
			if (LocalVelocity[1]>0.0)  //only forward not reverse...
				SetShipVelocity(MIN(LocalVelocity[1],ScaledSpeed));
			else
				SetShipVelocity(MAX(LocalVelocity[1],-ScaledSpeed));  //Fortunately the ships do not go in reverse that much  :)
		}
		
		else
		{  //This technique makes use of strafe thrusters.  (Currently we can do coordinated turns with this)
			//It is useful for certain situations.  One thing is for sure, it can get the ship
			//to a point more efficiently than the above method, which may be useful for an advanced tactic.
			//Vec2d GlobalVelocity(m_ship.GetAtt_quat() * LocalVelocity); 
			Vec2d GlobalVelocity=LocalToGlobal(m_ship.GetAtt_r(),LocalVelocity); 
			//now to cap off the velocity speeds
			for (size_t i=0;i<2;i++)
			{
				if (GlobalVelocity[i]>ScaledSpeed)
					GlobalVelocity[i]=ScaledSpeed;
				else if (GlobalVelocity[i]<-ScaledSpeed)
					GlobalVelocity[i]=-ScaledSpeed;
			}
			//Ideally GetForceFromVelocity could work with local orientation for FlightDynmic types, but for now we convert
			Vec2d GlobalForce(m_ship.m_Physics.GetForceFromVelocity(GlobalVelocity,dTime_s));
			//Vec2d LocalForce(m_ship.GetAtt_quat().conj() * GlobalForce);
			Vec2d LocalForce=GlobalToLocal(m_ship.GetAtt_r(),GlobalForce); //First get the local force
			//Now to fire all the thrusters given the acceleration
			m_ship.SetCurrentLinearAcceleration(LocalForce/m_ship.Mass);
		}
	}
	else
		SetShipVelocity(ScaledSpeed);
}

  /***********************************************************************************************************************************/
 /*												Goal_Ship_RotateToPosition															*/
/***********************************************************************************************************************************/

Goal_Ship_RotateToPosition::Goal_Ship_RotateToPosition(AI_Base_Controller *controller, double Heading) : m_Controller(controller), m_Heading(Heading),
	m_ship(controller->GetShip()),m_Terminate(false)
{
	m_Status=eInactive;
}
Goal_Ship_RotateToPosition::~Goal_Ship_RotateToPosition()
{
	Terminate(); //more for completion
}

void Goal_Ship_RotateToPosition::Activate() 
{
	m_Status=eActive;
	//During the activation we'll set the requested intended orientation
	m_Controller->SetIntendedOrientation(m_Heading);
}

Goal::Goal_Status Goal_Ship_RotateToPosition::Process(double dTime_s)
{
	//TODO this may be an inline check
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	if (m_Status==eActive)
	{
		if (m_ship.GetIntendedOrientation()==m_Heading)
		{
			double rotation_delta=m_ship.GetAtt_r()-m_Heading;
			NormalizeRotation(rotation_delta);
			//TODO check IsStuck for failed case
			if (IsZero(rotation_delta))
				m_Status=eCompleted;
		}
		else
			m_Status=eFailed;  //Some thing else took control of the ship
	}
	return m_Status;
}



//TODO this needs to be somewhat re-factored into states from which it will decide to execute
//The base version is some kind of auto pilot, which for the tester I don't really care to implement
//I believe I'll want to AI Controller to override, but call methods from the base as at that level it can decide
//which state to implement

//I think what I'll do at this level is a simple patrol where it simply only does the fly to next location that should be adequate


  /***********************************************************************************************************************************/
 /*												Goal_Ship_MoveToPosition															*/
/***********************************************************************************************************************************/

Goal_Ship_MoveToPosition::Goal_Ship_MoveToPosition(AI_Base_Controller *controller,const WayPoint &waypoint,bool UseSafeStop,bool LockOrientation) : m_Point(waypoint), m_Controller(controller),
	m_ship(controller->GetShip()),m_Terminate(false),m_UseSafeStop(UseSafeStop),m_LockOrientation(LockOrientation)
{
	m_Status=eInactive;
}
Goal_Ship_MoveToPosition::~Goal_Ship_MoveToPosition()
{
	Terminate(); //more for completion
}

void Goal_Ship_MoveToPosition::Activate() 
{
	m_Status=eActive;
}

bool Goal_Ship_MoveToPosition::HitWayPoint()
{
	// Base a tolerance2 for how close we want to get to the way point based on the current velocity,
	// within a second of reaching the way point, just move to the next one
	//Note for FRC... moving at 2mps it will come within an inch of its point with this tolerance
	double tolerance2 = m_UseSafeStop ? 0.0001 : (m_ship.GetPhysics().GetLinearVelocity().length2() * 1.0) + 0.1; // (will keep it within one meter even if not moving)
	Vec2d currPos = m_ship.GetPos_m();
	return ((m_Point.Position-currPos).length2() < tolerance2);
}

Goal::Goal_Status Goal_Ship_MoveToPosition::Process(double dTime_s)
{
	//TODO this may be an inline check
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	if (m_Status==eActive)
	{
		//TODO check IsStuck for failed case
		if (!HitWayPoint())
		{
			Vec2d Temp(0,0);
			m_Controller->DriveToLocation(m_Point.Position, m_Point.Position, m_Point.Power, dTime_s,m_UseSafeStop? &Temp:NULL,m_LockOrientation);
		}
		else
		{
			//for now just stop, but we may want to have a way to identify if we are in a series, perhaps the derived class overrides this and can be more intelligent
			m_Controller->SetShipVelocity(0.0);
			m_Status=eCompleted;
		}
	}
	return m_Status;
}
void Goal_Ship_MoveToPosition::Terminate()
{
	//TODO this may be an inline check
	m_Terminate=true;
}

  /***********************************************************************************************************************************/
 /*													Goal_Ship_FollowPath															*/
/***********************************************************************************************************************************/

Goal_Ship_FollowPath::Goal_Ship_FollowPath(AI_Base_Controller *controller,std::list<WayPoint> path,bool LoopMode) : 
	m_Controller(controller),m_Path(path),m_PathCopy(path),m_LoopMode(LoopMode)
{
	m_Status=eInactive;
}
void Goal_Ship_FollowPath::Activate()
{
	RemoveAllSubgoals();
	if (!m_Path.empty())
	{
		m_Status=eActive;
		WayPoint point=m_Path.front();
		m_Path.pop_front();
		AddSubgoal(new Goal_Ship_MoveToPosition(m_Controller,point,false));
	}
	else
		m_Status=eFailed;
}

Goal::Goal_Status Goal_Ship_FollowPath::Process(double dTime_s)
{
	ActivateIfInactive();
	if (m_Status==eActive)
	{
		m_Status=ProcessSubgoals(dTime_s);
		if (m_Status==eCompleted)
		{
			if (m_Path.empty()&&(m_LoopMode))
					m_Path=m_PathCopy;
			if (!m_Path.empty())
				Activate();
		}
	}
	return m_Status;
}

void Goal_Ship_FollowPath::Terminate()
{
}

  /***********************************************************************************************************************************/
 /*													Goal_Ship_FollowShip															*/
/***********************************************************************************************************************************/

void Goal_Ship_FollowShip::SetRelPosition(const Vec2d &RelPosition)  
{
	m_RelPosition=RelPosition;
	m_TrajectoryPosition=RelPosition;
	m_TrajectoryPosition[1] += 100.0;	// Just point forward
}

Goal_Ship_FollowShip::Goal_Ship_FollowShip(AI_Base_Controller *controller,const Ship_2D &Followship,const Vec2d &RelPosition) : m_Controller(controller),
	m_Followship(Followship),m_ship(controller->GetShip()),m_Terminate(false)
{
	SetRelPosition(RelPosition);
	m_Status=eInactive;
}
Goal_Ship_FollowShip::~Goal_Ship_FollowShip()
{
	Terminate(); //more for completion
}

void Goal_Ship_FollowShip::Activate() 
{
	m_Status=eActive;
}

Goal::Goal_Status Goal_Ship_FollowShip::Process(double dTime_s)
{
	//TODO this may be an inline check
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	if (m_Status==eActive)
	{
		//TODO add these methods to my ship
		//if (followMe.Ship->IsShowing() && !followMe.Ship->IsBeingDestroyed())
		if (true)
		{
			// This is the "correct" offset position
			Vec2d globalGoalPositionPoint;
			Vec2d globalGoalTrajectoryPoint;
			Vec2d globalGoalVelocity;
			globalGoalPositionPoint=globalGoalTrajectoryPoint=m_Followship.GetPos_m();
			globalGoalVelocity = m_Followship.GetPhysics().GetLinearVelocity();


			//globalGoalPositionPoint+=followMe.Ship->GetAtt_quat()*followMe.RelPosition;
			globalGoalPositionPoint+=LocalToGlobal(m_Followship.GetAtt_r(),m_RelPosition);
			//globalGoalTrajectoryPoint  +=followMe.Ship->GetAtt_quat()*followMe.TrajectoryPosition;
			globalGoalTrajectoryPoint+=LocalToGlobal(m_Followship.GetAtt_r(),m_TrajectoryPosition);

			// The globalGoalGlobalOrient position is either in front so we can turn to it
			// AND/OR it is far enough away where we SHOULD make a heavy turn into it.
			//UpdateIntendedLocation(globalGoalTrajectoryPoint,globalGoalPositionPoint, 1.0, &globalGoalVelocity);
			m_Controller->DriveToLocation(globalGoalTrajectoryPoint, globalGoalPositionPoint, 1.0, dTime_s,&globalGoalVelocity);
		}
		else
		{
			// Just drive straight so we can deal with avoidance
			Vec2d globalGoalPositionPoint = m_Followship.GetPos_m() + m_Followship.GetPhysics().GetLinearVelocity();
			//UpdateIntendedLocation(globalGoalPositionPoint,globalGoalPositionPoint, 1.0, NULL);
			m_Controller->DriveToLocation(globalGoalPositionPoint, globalGoalPositionPoint, 1.0, dTime_s, NULL);
			m_Status=eFailed;
		}
	}
	return m_Status;
}
void Goal_Ship_FollowShip::Terminate()
{
	//TODO this may be an inline check
	m_Terminate=true;
}

  /***********************************************************************************************************************************/
 /*															Goal_Wait																*/
/***********************************************************************************************************************************/

Goal_Wait::Goal_Wait(double seconds) : m_TimeToWait(seconds)
{
	m_Status=eInactive;
}

void Goal_Wait::Activate()
{
	m_Status=eActive; 
	m_TimeAccrued=0.0;
}
Goal::Goal_Status Goal_Wait::Process(double dTime_s)
{
	ActivateIfInactive();
	m_TimeAccrued+=dTime_s;
	if (m_TimeAccrued>m_TimeToWait)
		m_Status=eCompleted;
	return m_Status;
}
void Goal_Wait::Terminate()
{
	m_Status=eInactive;
}


  /***********************************************************************************************************************************/
 /*													Goal_NotifyWhenComplete															*/
/***********************************************************************************************************************************/


Goal_NotifyWhenComplete::Goal_NotifyWhenComplete(EventMap &em,char *EventName) : m_EventName(EventName),m_EventMap(em)
{
	m_Status=eInactive;
}

void Goal_NotifyWhenComplete::Activate()
{
	m_Status=eActive; 
}

Goal::Goal_Status Goal_NotifyWhenComplete::Process(double dTime_s)
{
	//Client will activate
	if (m_Status==eInactive)
		return m_Status;

	if (m_Status==eActive)
	{
		m_Status=ProcessSubgoals(dTime_s);
		if (m_Status==eCompleted)
		{
			m_EventMap.Event_Map[m_EventName].Fire(); //Fire the event
			Terminate();
		}
	}
	return m_Status;
}

void Goal_NotifyWhenComplete::Terminate()
{
	//ensure its all clean
	RemoveAllSubgoals();
	m_Status=eInactive; //make this inactive
}
