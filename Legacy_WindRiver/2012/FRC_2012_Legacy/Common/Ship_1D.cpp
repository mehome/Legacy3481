#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"

using namespace Framework::Base;

#undef __DisableShipSpeedBoost__
#undef __DisableSpeedControl__  //This one is great for test purposes


  /***************************************************************************************************************/
 /*													Ship_1D														*/
/***************************************************************************************************************/

Ship_1D::Ship_1D(const char EntityName[]) : Entity1D(EntityName)
	
{
	SetSimFlightMode(true);  //this sets up the initial speed as well
	m_LockShipToPosition=false;  //usually this is false (especially for AI and Remote controllers)
	ResetPos();
}

Ship_1D::~Ship_1D()
{
}


void Ship_1D::ResetPos()
{
	__super::ResetPos();

	m_RequestedVelocity = m_currAccel =0.0;
	//See case 397... for the 1D this is not really an issue as once we get it started it works itself out... it just cannot be zero to get it started
	m_Last_RequestedVelocity=-1.0;
	m_IntendedPosition=0.0;
	m_IntendedPositionPhysics.ResetVectors();
	m_LastNormalizedVelocity=0.0;
	//m_Physics.ResetVectors(); called from entity 1D's reset
	SetSimFlightMode(true);  //This one is a tough call... probably should do it on reset
}

bool Ship_1D::GetLockShipToPosition() const
{
	return m_LockShipToPosition;
}

void Ship_1D::SetSimFlightMode(bool SimFlightMode)	
{
	//It seems that some people want/need to call this function repeatedly so I have included a valve branch here to prevent the debug flooding
	//And to not do extra work on the m_RequestedVelocity.
	if (m_SimFlightMode!=SimFlightMode)
	{
		m_RequestedVelocity=m_Physics.GetVelocity();
		m_SimFlightMode=SimFlightMode;	
		DebugOutput("SimFlightMode=%d\n",SimFlightMode);
	}
}

void Ship_1D::SetRequestedVelocity(double Velocity)
{
	SetSimFlightMode(true);
	m_LockShipToPosition=true;  //unlike in 2D/3D setting this has an impact on the locking management
	if (Velocity>0.0)
		m_RequestedVelocity=MIN(Velocity,GetMaxSpeed());
	else
		m_RequestedVelocity=MAX(Velocity,-GetMaxSpeed());
}

void Ship_1D::SetRequestedVelocity_FromNormalized(double Velocity)
{
	//we must have flood control so that other controls may work (the joystick will call this on every time slice!)
	if (Velocity!=m_LastNormalizedVelocity)
	{
		//scale the velocity to the max speed's magnitude
		double VelocityScaled=Velocity*GetMaxSpeed();
		SetRequestedVelocity(VelocityScaled);
		m_LastNormalizedVelocity=Velocity;
	}
}

void Ship_1D::Initialize(EventMap& em,const Entity1D_Properties *props)
{
	__super::Initialize(em,props);
	const Ship_1D_Properties *ship_props=static_cast<const Ship_1D_Properties *>(props);
	if (ship_props)
	{
		ship_props->Initialize(this);
	}
	else
	{
		MAX_SPEED = 1.0;
		ACCEL = 1.0;
		BRAKE = 1.0;

		MaxAccelForward=1.0;
		MaxAccelReverse=1.0;
		m_UsingRange=false;
		m_MinRange=m_MaxRange=0;
		m_IsAngular=false;
	}
	Mass  = m_Physics.GetMass();

	m_IntendedPosition = 0.0;
	m_IntendedPositionPhysics.SetMass(Mass);
}


void Ship_1D::UpdateIntendedPosition(double dTime_s)
{
	if (m_LockShipToPosition)
	{
		//Keep the intended position locked to the current position, since we are not managing it like we do in 2D/3D.
		//once the mouse kicks in it will be in the correct starting place to switch modes
		m_IntendedPosition=GetPos_m();
	}
	else
		m_IntendedPosition+=m_currAccel;
}


void Ship_1D::TimeChange(double dTime_s)
{

	// Find the current velocity and use to determine the flight characteristics we will WANT to us
	double LocalVelocity=m_Physics.GetVelocity();
	double currFwdVel = LocalVelocity;
	bool manualMode = !((m_SimFlightMode)&&(m_currAccel==0));

	double ForceToApply;
	double posDisplacement_m=0;

	{
		UpdateIntendedPosition(dTime_s);
		//Determine the angular distance from the intended orientation
		posDisplacement_m=m_IntendedPosition-GetPos_m();
		PosDisplacementCallback(posDisplacement_m);  //call the callback with this value
	}

	//Apply the restraints now... I need this to compute my roll offset
	double AccRestraintPositive=MaxAccelForward;
	double AccRestraintNegative=MaxAccelReverse;

	//Unlike in 2D the intended position and velocity control now resides in the same vector to apply force.  To implement, we'll branch depending on
	//which last LockShipToPosition was used.  Typically speaking the mouse, AI, or SetIntendedPosition() will branch to the non locked mode, while the
	//joystick and keyboard will conduct the locked mode
	if (m_LockShipToPosition)
	{
		if (!manualMode)
		{
			//This first system combined the speed request and the accel delta's as one but this runs into undesired effects with the accel deltas
			//The most undesired effect is that when no delta is applied neither should any extra force be applied.  There is indeed a distinction
			//between cruise control (e.g. slider) and using a Key button entry in this regard.  The else case here keeps these more separated where
			//you are either using one mode or the other
			double VelocityDelta=m_currAccel*dTime_s;

			bool UsingRequestedVelocity=false;

			//Note: m_RequestedVelocity is not altered with the velocity delta, but it will keep up to date
			if (VelocityDelta!=0) //if user is changing his adjustments then reset the velocity to current velocity
				m_RequestedVelocity=m_Last_RequestedVelocity=currFwdVel+VelocityDelta;
			else
				UsingRequestedVelocity=(m_RequestedVelocity!=m_Last_RequestedVelocity);

			//Just transfer the acceleration directly into our velocity to use variable
			double VelocityToUse=(UsingRequestedVelocity)? m_RequestedVelocity:currFwdVel;


			#ifndef __DisableSpeedControl__
			{
				if (m_currAccel<0) // Watch for braking too far backwards, we do not want to go beyond -ENGAGED_MAX_SPEED
				{
					if ((VelocityToUse) < -MAX_SPEED)
					{
						m_RequestedVelocity = VelocityToUse = -MAX_SPEED;
						m_currAccel=0.0;
					}
				}
				else 
				{
					double MaxSpeed=MAX_SPEED;
					if ((VelocityToUse) > MaxSpeed)
					{
						m_RequestedVelocity = VelocityToUse=MaxSpeed;
						m_currAccel=0.0;
					}
				}
			}
			#endif

			if (m_UsingRange)
			{
				double Position=GetPos_m();
				//check to see if we are going reach limit
				if ((VelocityToUse + Position) > m_MaxRange)
					VelocityToUse=m_MaxRange-Position;
				else if ((VelocityToUse + Position) < m_MinRange)
					VelocityToUse=m_MinRange-Position;
			}
			ForceToApply=m_Physics.GetForceFromVelocity(VelocityToUse,dTime_s);
			if (!UsingRequestedVelocity)
				ForceToApply+=m_currAccel * Mass;
			//Allow subclass to evaluate the requested velocity in use;
			RequestedVelocityCallback(VelocityToUse,dTime_s);
		}
		else   //Manual mode
		{
			#ifndef __DisableSpeedControl__
			{
				{
					double VelocityDelta=m_currAccel*dTime_s;
					if ((LocalVelocity+VelocityDelta>MAX_SPEED)&&(m_currAccel>0))
							m_currAccel=0.0;
					else if ((LocalVelocity+VelocityDelta<-MAX_SPEED)&&(m_currAccel<0))
						m_currAccel=0.0;
				}
			}
			#endif
			ForceToApply=m_currAccel*Mass;
		}
		ForceToApply=m_Physics.ComputeRestrainedForce(ForceToApply,AccRestraintPositive*Mass,AccRestraintNegative*Mass,dTime_s);
	}
	else
	{
		double Vel;

		{
			double DistanceToUse=posDisplacement_m;
			double MatchVelocity=GetMatchVelocity();
			//Most likely these should never get triggered unless there is some kind of control like the mouse that can go beyond the limit
			if (m_UsingRange)
			{
				if (m_IntendedPosition>m_MaxRange)
					DistanceToUse=m_MaxRange-GetPos_m();
				else if(m_IntendedPosition<m_MinRange)
					DistanceToUse=m_MinRange-GetPos_m();
			}
			if (!m_IsAngular)
			{
				//The match velocity needs to be in the same direction as the distance (It will not be if the ship is banking)
				Vel=m_Physics.GetVelocityFromDistance_Linear(DistanceToUse,AccRestraintPositive*Mass,AccRestraintNegative*Mass,dTime_s,MatchVelocity);
			}
			else
				Vel=m_Physics.GetVelocityFromDistance_Angular(DistanceToUse,AccRestraintPositive*Mass,dTime_s,MatchVelocity);
		}

		#ifndef __DisableSpeedControl__
		{
			if ((Vel) < -MAX_SPEED)
			{
				Vel = -MAX_SPEED;
				m_currAccel=0.0;
			}
			else if ((Vel) > MAX_SPEED) 
			{
				Vel=MAX_SPEED;
				m_RequestedVelocity=MAX_SPEED;
				m_currAccel=0.0;
			}
		}
		#endif

		ForceToApply=m_Physics.GetForceFromVelocity(Vel,dTime_s);
	}


	//To be safe we reset this to zero (I'd put a critical section around this line of code if there are thread issues
	posDisplacement_m=0.0;

	//ApplyThrusters(m_Physics,ForceToApply,TorqueToApply,Ships_TorqueRestraint,dTime_s);
	m_Physics.ApplyFractionalForce(ForceToApply,dTime_s);


	// Now to run the time updates (displacement plus application of it)
	Entity1D::TimeChange(dTime_s);

	m_currAccel=0.0;
}

  /***********************************************************************************************************************************/
 /*												Goal_Ship1D_MoveToPosition															*/
/***********************************************************************************************************************************/

Goal_Ship1D_MoveToPosition::Goal_Ship1D_MoveToPosition(Ship_1D &ship,double position,double tolerance) :
	m_ship(ship),m_Position(position),m_Tolerance(tolerance),m_Terminate(false)
{
	m_Status=eInactive;
}
Goal_Ship1D_MoveToPosition::~Goal_Ship1D_MoveToPosition()
{
	Terminate(); //more for completion
}

void Goal_Ship1D_MoveToPosition::Activate() 
{
	m_Status=eActive;
	//During the activation we'll set the requested position
	m_ship.SetIntendedPosition(m_Position);
}

Goal::Goal_Status Goal_Ship1D_MoveToPosition::Process(double dTime_s)
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
		if (m_ship.GetIntendedPosition()==m_Position)
		{
			double position_delta=m_ship.GetPos_m()-m_Position;
			//TODO check IsStuck for failed case
			//printf("\r%f        ",position_delta);
			if (fabs(position_delta)<m_Tolerance)  //When testing the arm it would idle around 0.02825
			{
				//printf("completed %f\n",position_delta);
				m_Status=eCompleted;
			}
		}
		else
			m_Status=eFailed;  //Some thing else took control of the ship
	}
	return m_Status;
}
