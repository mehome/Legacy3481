#include <math.h>
#include <assert.h>
#include "../Base/Misc.h"
#include "Physics_1D.h"

using namespace Framework;
using namespace std;
using namespace Framework::Base;

const double Pi2=M_PI*2.0;
//The actual force between two objects are f=(G m1 m2)/ r^2
//For example g = ( G * (Me->5.98E+24)) / (Re->6.38E+6)^2 = 9.8 m/s^2
//G is ideal to compute forces on ships from various planets
const double G=6.673E-11;


inline bool PosBNE(double val,double t)
{
	return !(fabs(val-t)<1E-3);
}

void PhysicsEntity_1D::SetMass(double mass)
{
	m_EntityMass=mass;
}
double PhysicsEntity_1D::GetMass() const
{
	return m_EntityMass;
}

void PhysicsEntity_1D::CopyFrom(const PhysicsEntity_1D& rhs)
{
	m_Velocity= rhs.m_Velocity;
}

void PhysicsEntity_1D::ResetVectors()
{
	m_Velocity= 0.0;
}

PhysicsEntity_1D::PhysicsEntity_1D()
{
	//Plug in some good defaults
	m_EntityMass=5.0;
	m_StaticFriction=0.8;
	m_KineticFriction=0.2;
	m_SummedExternalForces=0.0;
	m_lastTime_s=0.0;

	ResetVectors();
}

void PhysicsEntity_1D::SetFriction(double StaticFriction,double KineticFriction)
{
	m_StaticFriction=StaticFriction;
	m_KineticFriction=KineticFriction;
}

void PhysicsEntity_1D::SetVelocity(double Velocity)
{ 
	m_Velocity=Velocity;
}
double PhysicsEntity_1D::GetVelocity() const
{
	return m_Velocity;
}

void PhysicsEntity_1D::ApplyFractionalForce( double force,double FrameDuration)
{
	//I'm writing this out so I can easily debug
	double AccelerationDelta=force/m_EntityMass;
	double VelocityDelta=AccelerationDelta*FrameDuration;
	m_Velocity+=VelocityDelta;

	//if (AccelerationDelta[1]!=0)
	//	DebugOutput("Acc%f Vel%f\n",AccelerationDelta[1],m_Velocity[1]);
}

double PhysicsEntity_1D::GetForceFromVelocity(double vDesiredVelocity,double DeltaTime_s)
{
	double DeltaVelocity=(vDesiredVelocity-GetVelocity());
	//A=Delta V / Delta T
	double Acceleration=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply Force restrictions later)


	//if (Acceleration!=osg::Vec2d(0,0,0))
	//	printf(" x=%f,y=%f,z=%f\n",Acceleration[0],Acceleration[1],Acceleration[2]);
	//Now that we know what the acceleration needs to be figure out how much force to get it there
	double Force= Acceleration * m_EntityMass;
	//if (PosBNE(Force[0],0)||(PosBNE(Force[1],0))||(PosBNE(Force[2],0)))
	//	printf("tx=%f,ty=%f,tz=%f\n",Force[0],Force[1],Force[2]);

	//if (PosBNE(Heading[2],0.0))
	//	DebugOutput(" s=%f,a=%f,w=%f,h=%f,z=%f,t=%f\n",Distance[2],m_AngularAcceleration[2],m_AngularVelocity[2],Heading[2],CurrentOrientation[2],Force[2]);

	return Force;
}

inline double PhysicsEntity_1D::GetVelocityFromCollision(double ThisVelocityToUse,double otherEntityMass,double otherEntityVelocity)
{
	//almost not quite
	//return (m_Velocity*(m_EntityMass-otherEntityMass)) / (m_EntityMass+otherEntityMass);

	/// en.wikipedia.org/wiki/Elastic_collision
	// Here is the equation
	// ((vel1 ( mass1 - mass2 )) + (2 * mass2 * vel2))) / (m1+m2)
	double ret= (ThisVelocityToUse *(m_EntityMass-otherEntityMass));
	ret+= (otherEntityVelocity*(2 * otherEntityMass));
	ret/= (m_EntityMass+otherEntityMass);
	return ret;
}

double PhysicsEntity_1D::GetVelocityFromDistance_Linear(double Distance,double ForceRestraintPositive,double ForceRestraintNegative,double DeltaTime_s, double matchVel)
{
	double ret;
	double DistToUse=Distance; //save the original distance

	DistToUse-=(matchVel*DeltaTime_s);
	double Distance_Length=fabs(DistToUse);
	if (IsZero(Distance_Length))
		return matchVel;

	//This is how many meters per second the ship is capable to stop for this given time frame
	//Compute the restraint based off of its current direction
	double Restraint=DistToUse>0?ForceRestraintPositive:ForceRestraintNegative;
	double Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	double IdealSpeed=Distance_Length/DeltaTime_s;
	double AccelerationMagnitude=fabs(Acceleration);
	double Time=sqrt(2.0*(Distance_Length/AccelerationMagnitude));

	double MaxSpeed=Distance_Length/Time;
	double SpeedToUse=min(IdealSpeed,MaxSpeed);

	//DebugOutput("d=%f i=%f m=%f\n",Distance[1],IdealSpeed,MaxSpeed);
	//Now to give this magnitude a direction based of the velocity
	double scale=SpeedToUse/Distance_Length;
	ret=DistToUse*scale;
	ret+=matchVel;
	return ret;
}

double PhysicsEntity_1D::GetVelocityFromDistance_Angular(double Distance,double Restraint,double DeltaTime_s,double matchVel)
{
	double ret;

	//This is how many radians the ship is capable to turn for this given time frame
	double Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	{
		//first compute which direction to go
		double DistanceDirection=Distance;
		DistanceDirection-=matchVel*DeltaTime_s;
		if (IsZero(DistanceDirection))
		{
			ret=matchVel;
			return ret;
		}

		//Unlike in the 3D physics, we'll need while loops to ensure all of the accumulated turns are normalized, in the 3D physics the
		//Quat is auto normalized to only require one if check here
		while (DistanceDirection>M_PI)
			DistanceDirection-=Pi2;
		while (DistanceDirection<-M_PI)
			DistanceDirection+=Pi2;
		double DistanceLength=fabs(DistanceDirection);

		//Ideal speed needs to also be normalized
		double IDS=Distance;
		if (IDS>M_PI)
			IDS-=Pi2;
		else if (IDS<-M_PI)
			IDS+=Pi2;

		double IdealSpeed=fabs(IDS/DeltaTime_s);

		if (Restraint!=-1)
		{
			//Given the distance compute the time needed
			//Place the division first keeps the multiply small
			double Time=sqrt(2.0*(DistanceLength/Acceleration));

			//Now compute maximum speed for this time
			double MaxSpeed=DistanceLength/Time;
			ret=min(IdealSpeed,MaxSpeed);

			if (DistanceDirection<0)
				ret=-ret;
			ret+=matchVel;
		}
		else
		{
			ret=IdealSpeed;  //i.e. god speed
			if (IDS<0)
				ret=-ret;
		}
	}
	return ret;
}

double PhysicsEntity_1D::ComputeRestrainedForce(double LocalForce,double ForceRestraintPositive,double ForceRestraintNegative,double dTime_s)
{
	//Note: This could be simplified; however, (at least for now) I am keeping it intact as it is in 2D to see how the function behaves
	double ForceToApply=LocalForce;
	if (ForceRestraintPositive!=-1)
	{
		double SmallestRatio=1.0;
		//Apply Force restraints; This method computes the smallest ratio needed to scale down the vector.  It should give the maximum amount
		//of magnitude available without sacrificing the intended direction
		{
			double Temp;
			//separate the positive and negative coordinates
			if (LocalForce>0)
			{
				if (LocalForce>ForceRestraintPositive)
				{
					Temp=ForceRestraintPositive/LocalForce;
					SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
				}
			}
			else
			{
				double AbsComponent=fabs(LocalForce);
				if (AbsComponent>ForceRestraintNegative)
				{
					Temp=ForceRestraintNegative/AbsComponent;
					SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
				}
			}
		}
		ForceToApply*=SmallestRatio;
		//Test my restraints
		//printf("\r lr %f fr %f ud %f                ",LocalForce[0],LocalForce[1],LocalForce[2]);
	}
	return ForceToApply;
}

void PhysicsEntity_1D::TimeChangeUpdate(double DeltaTime_s,double &PositionDisplacement)
{
	//Transfer the velocity to displacement
	PositionDisplacement = m_Velocity * DeltaTime_s;
}
