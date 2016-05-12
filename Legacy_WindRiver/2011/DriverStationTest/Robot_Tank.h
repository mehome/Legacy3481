#pragma once

///This kind of ship will convert the torque and force into a two fixed point force/thrust system like that of a tank.  It also interpolates the left
///right velocities back into the torque and force.  Unlike a ship, it will always absorb any lateral forces as it is assumed that the entity will not
///skid in those directions.  This means it cannot strafe, and will behave much like a vehicle on land rather than a ship in space.
class Robot_Tank : public Ship_Tester
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		Robot_Tank(const char EntityName[]);
		double GetLeftVelocity() const {return m_LeftLinearVelocity;}
		double GetRightVelocity() const {return m_RightLinearVelocity;}
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();
	protected:
		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//This method converts the given left right velocities into a form local linear velocity and angular velocity
		void InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
	private:
		//typedef Ship_2D __super;
		void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		double m_LeftLinearVelocity,m_RightLinearVelocity;
};

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Robot_Control_Interface
{
	public:
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
		virtual void UpdateArmVoltage(double Voltage)=0;
		///This is a implemented by reading the potentiometer and converting its value to correspond to the arm's current angle
		///This is in radians of the arm's gear ratio
		virtual double GetArmCurrentPosition()=0;
		virtual void CloseClaw(bool Close)=0;  //true=close false=open
		virtual void CloseDeploymentDoor(bool Open)=0;
		//virtual void ReleaseLazySusan(bool Release)=0;
};

