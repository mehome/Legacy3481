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
#include "Ship.h"

using namespace Framework::Base;

  /***********************************************************************************************************************************/
 /*															Entity1D_Properties														*/
/***********************************************************************************************************************************/

Entity1D_Properties::Entity1D_Properties()
{
	m_EntityName="Entity1D";
	m_Mass=10000.0;
	m_Dimension=12.0;
	m_IsAngular=false;
};

Entity1D_Properties::Entity1D_Properties(const char EntityName[],double Mass,double Dimension,bool IsAngular)
{
	m_EntityName=EntityName;
	m_Mass=Mass;
	m_Dimension=Dimension;
	m_IsAngular=IsAngular;
}

void Entity1D_Properties::Initialize(Entity1D *NewEntity) const
{
	NewEntity->m_Dimension=m_Dimension;
	NewEntity->GetPhysics().SetMass(m_Mass);
	NewEntity->m_IsAngular=m_IsAngular;
}

  /***********************************************************************************************************************************/
 /*														Ship_1D_Properties															*/
/***********************************************************************************************************************************/

Ship_1D_Properties::Ship_1D_Properties()
{
	double Scale=0.2;  //we must scale everything down to see on the view
	m_MAX_SPEED = 400.0 * Scale;
	m_ACCEL = 60.0 * Scale;
	m_BRAKE = 50.0 * Scale;

	m_MaxAccelForward=87.0 * Scale;
	m_MaxAccelReverse=70.0 * Scale;
	m_MinRange=m_MaxRange=0.0;
	m_UsingRange=false;
};

Ship_1D_Properties::Ship_1D_Properties(const char EntityName[], double Mass,double Dimension,
				   double MAX_SPEED,
				   double ACCEL,double BRAKE,
				   double MaxAccelForward, double MaxAccelReverse,
				   Ship_Type ShipType,bool UsingRange,
				   double MinRange, double MaxRange,bool IsAngular
				   ) : Entity1D_Properties(EntityName,Mass,Dimension,IsAngular)
{
	m_MAX_SPEED = MAX_SPEED;
	m_ACCEL = ACCEL;
	m_BRAKE = BRAKE;
	m_MaxAccelForward=MaxAccelForward;
	m_MaxAccelReverse=MaxAccelReverse;
	m_ShipType=ShipType;
	m_MinRange=MinRange;
	m_MaxRange=MaxRange;
	m_UsingRange=UsingRange;

}

void Ship_1D_Properties::Initialize(Ship_1D *NewShip) const
{
	NewShip->MAX_SPEED=m_MAX_SPEED;
	NewShip->ACCEL=m_ACCEL;
	NewShip->BRAKE=m_BRAKE;
	NewShip->MaxAccelForward=m_MaxAccelForward;
	NewShip->MaxAccelReverse=m_MaxAccelReverse;
	NewShip->m_UsingRange=m_UsingRange;
	NewShip->m_MinRange=m_MinRange;
	NewShip->m_MaxRange=m_MaxRange;
}

  /***********************************************************************************************************************************/
 /*															Entity_Properties														*/
/***********************************************************************************************************************************/

Entity_Properties::Entity_Properties()
{
	m_EntityName="Entity";
	//m_NAME="default";
	m_Mass=25.0;
	m_Dimensions[0]=0.6477;
	m_Dimensions[1]=0.9525;
};

void Entity_Properties::Initialize(Entity2D *NewEntity) const
{
	NewEntity->m_Dimensions[0]=m_Dimensions[0];
	NewEntity->m_Dimensions[1]=m_Dimensions[1];
	NewEntity->GetPhysics().SetMass(m_Mass);
}

  /***********************************************************************************************************************************/
 /*															Ship_Properties															*/
/***********************************************************************************************************************************/

Ship_Properties::Ship_Properties()
{
	m_dHeading = DEG_2_RAD(514.0);

	m_MAX_SPEED = 2.916;
	m_ENGAGED_MAX_SPEED = 2.916;
	m_ACCEL = m_ENGAGED_MAX_SPEED;
	m_BRAKE = m_ENGAGED_MAX_SPEED;
	m_STRAFE = m_BRAKE;
	m_AFTERBURNER_ACCEL = 60.0;    //we could use these, but I don't think it is necessary 
	m_AFTERBURNER_BRAKE = m_BRAKE;

	//These are the most important that setup the force restraints
	m_MaxAccelLeft=5.0;		//The left and right apply to strafe (are ignored for 2011 robot)
	m_MaxAccelRight=5.0;
	m_MaxAccelForward=5.0;
	m_MaxAccelReverse=5.0;
	m_MaxTorqueYaw=25.0;

	//I'm leaving these in event though they are not going to be used
	double RAMP_UP_DUR = 1.0;
	double RAMP_DOWN_DUR = 1.0;
	m_EngineRampAfterBurner= m_AFTERBURNER_ACCEL/RAMP_UP_DUR;
	m_EngineRampForward= m_ACCEL/RAMP_UP_DUR;
	m_EngineRampReverse= m_BRAKE/RAMP_UP_DUR;
	m_EngineRampStrafe= m_STRAFE/RAMP_UP_DUR;
	m_EngineDeceleration= m_ACCEL/RAMP_DOWN_DUR;
};


void Ship_Properties::Initialize(Ship_2D *NewShip) const
{
	NewShip->dHeading=m_dHeading;
	NewShip->MAX_SPEED=m_MAX_SPEED;
	NewShip->ENGAGED_MAX_SPEED=m_ENGAGED_MAX_SPEED;
	NewShip->ACCEL=m_ACCEL;
	NewShip->BRAKE=m_BRAKE;
	NewShip->STRAFE=m_STRAFE;
	NewShip->AFTERBURNER_ACCEL=m_AFTERBURNER_ACCEL;
	NewShip->AFTERBURNER_BRAKE=m_AFTERBURNER_BRAKE;

	NewShip->EngineRampAfterBurner=m_EngineRampAfterBurner;
	NewShip->EngineRampForward=m_EngineRampForward;
	NewShip->EngineRampReverse=m_EngineRampReverse;
	NewShip->EngineRampStrafe=m_EngineRampStrafe;
	NewShip->EngineDeceleration=m_EngineDeceleration;

	NewShip->MaxAccelLeft=m_MaxAccelLeft;
	NewShip->MaxAccelRight=m_MaxAccelRight;
	NewShip->MaxAccelForward=m_MaxAccelForward;
	NewShip->MaxAccelReverse=m_MaxAccelReverse;
	NewShip->MaxTorqueYaw=m_MaxTorqueYaw;
}
