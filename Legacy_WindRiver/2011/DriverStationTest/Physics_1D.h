#pragma once

class PhysicsEntity_1D
{
	public:
		PhysicsEntity_1D();
		///You must set the mass otherwise you will get the default
		void SetMass(double mass);
		double GetMass() const;
		///This will zero out all vectors
		virtual void ResetVectors();

		// An overloaded operator for matching the current physics and position
		virtual void CopyFrom(const PhysicsEntity_1D& rhs);

		///This will compute all the displacement. Call this each time slice to obtain the rotation and position displacements.  
		///You must call this to run the physics engine!
		virtual void TimeChangeUpdate(double DeltaTime_s,double &PositionDisplacement);

		inline double GetVelocityFromCollision(double ThisVelocityToUse,double otherEntityMass,double otherEntityVelocity);
		///This simply returns a min operation of speed/time and the maximum speed available to stop within the given distance
		///This can either work with local or global orientation that all depends on the orientation of the restraints typically this works in local
		virtual double GetVelocityFromDistance_Linear(double Distance,double ForceRestraintPositive,double ForceRestraintNegative,double DeltaTime_s,double matchVel);

		///These are coefficients to use when dealing with a force of friction typically 0.8 and 0.2 respectively
		void SetFriction(double StaticFriction, ///<The amount of friction to be applied when object is not moving
			double KineticFriction ///<The amount of friction to be applied when object is moving
			);

		void SetVelocity( double Velocity);
		double GetVelocity() const;

		///These will auto sum for each call made, the forces last for one second during each timer update, so you have to call them repeatedly to 
		///continue to apply force.  If you want to apply a force for a specific amount of time, this can be achieved by calling this
		///during each time slice.  
		/// \note This should offer enough precision in time, but you can get more precise by computing a fraction force if necessary
		//void ApplyForce( double force);

		///These work like the above except that the force applied only happens for a fraction of a second.  For accurate results FrameDuration
		///should be <= 1/framerate.  Ideally these should be used for high precision movements like moving a ship, where the FrameDuration is
		///typically the TimeDelta value
		void ApplyFractionalForce( double force,double FrameDuration);

		///You may prefer to set a desired speed instead of computing the forces.  These values returned are intended to be used with 
		///ApplyFractionalForce, and ApplyFractionalTorque respectively.
		/// For the force, this works with the current linear velocity; therefore the desired velocity and return must work in global orientation
		virtual double GetForceFromVelocity( 
			double vDesiredVelocity,			///< How fast do you want to go (in a specific direction)
			double DeltaTime_s					///< How quickly do you want to get there
			);

		///This is a clean way to compute how much torque that can be applied given the maximum amount available (e.g. thruster capacity)
		///It should be noted that this treats roll as a separate factor, which is best suited for avionic type of context
		/// \Note all restraint parameters are positive (i.e. ForceRestraintNegative)
		double ComputeRestrainedForce(double LocalForce,double ForceRestraintPositive,double ForceRestraintNegative,double dTime_s);

	protected:
		double m_EntityMass;
		double m_StaticFriction,m_KineticFriction;

		double m_Velocity;

		///This variable is factored in but is managed externally 
		double m_SummedExternalForces;
		double m_lastTime_s;
};
