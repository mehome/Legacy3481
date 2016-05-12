
///This is really stripped down from the Ship_2D.  Not only is their one dimension (given), but there is also no controller.  This class is intended
///To be controlled directly from a parent class which has controller support.
class Ship_1D : public Entity1D
{
	public:
		Ship_1D(const char EntityName[]);
		virtual void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual ~Ship_1D();

		///This implicitly will place back in auto mode with a speed of zero
		void Stop(){SetRequestedVelocity(0.0);}
		void SetRequestedVelocity(double Velocity);
		double GetRequestedVelocity(){return m_RequestedVelocity;}

		/// \param LockShipHeadingToPosition for this given time slice if this is true the intended orientation is restrained
		/// to the ships restraints and the ship is locked to the orientation (Joy/Key mode).  If false (Mouse/AI) the intended orientation
		/// is not restrained and the ship applies its restraints to catch up to the orientation. \note this defaults to true since this is 
		/// most likely never going to be used with a mouse or AI
		void SetCurrentLinearAcceleration(double Acceleration,bool LockShipToPosition=true) 
		{ m_LockShipToPosition=LockShipToPosition,m_currAccel=Acceleration;
		}
		/// This is like setting a way point since there is one dimension there is only one setting to use here
		void SetIntendedPosition(double Position) 
		{ m_LockShipToPosition=false,m_IntendedPosition=Position;
		}

		// This is where both the vehicle entity and camera need to align to
		virtual const double &GetIntendedPosition() const {return m_IntendedPosition;}
		void SetSimFlightMode(bool SimFlightMode);

		// virtual void ResetPos();
		bool GetAlterTrajectory() const { return m_SimFlightMode;}
		double GetMaxSpeed() const		    {return MAX_SPEED;}

		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		//The UI controller will call this when attaching or detaching control.  The Bind parameter will either bind or unbind.  Since these are 
		//specific controls to a specific ship there is currently no method to transfer these specifics from one ship to the next.  Ideally there
		//should be no member variables needed to implement the bindings
		virtual void BindAdditionalEventControls(bool Bind) {}
	protected:
		///This will apply turn pitch and roll to the intended orientation
		void UpdateIntendedPosition(double dTime_s);

		virtual void TimeChange(double dTime_s);

		// Watch for being made the controlled ship
		virtual bool IsPlayerControllable(){return true;}

		friend class Ship_1D_Properties;

		double MAX_SPEED;

		// Used in Keyboard acceleration and braking
		double ACCEL, BRAKE;

		//Stuff needed for physics
		double Mass;
		double MaxAccelForward,MaxAccelReverse;

		//I don't think we would need this for the game, but it is possible, (certainly not for the robot arm)
		//! We can break this up even more if needed
		//double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
		//double EngineDeceleration,EngineRampStrafe;
	
		//Use this technique when m_AlterTrajectory is true
		double m_RequestedVelocity;
		//All input for turn pitch and roll apply to this, both the camera and ship need to align to it
		double m_IntendedPosition;
		//We need the m_IntendedPosition to work with its own physics
		PhysicsEntity_1D m_IntendedPositionPhysics;

		//For slide mode all strafe is applied here
		double m_currAccel;  //This is the immediate request for thruster levels
		double m_Last_RequestedVelocity;  ///< This monitors the last caught requested velocity  from a speed delta change
		double m_MinRange,m_MaxRange;
		bool m_SimFlightMode;  ///< If true auto strafing will occur to keep ship in line with its position
		bool m_UsingRange; 

	private:
		//typedef Entity1D __super;
		bool m_LockShipToPosition; ///< Locks the ship to intended position (Joystick and Keyboard controls use this)
};


//This is similar to Traverse_Edge in book (not to be confused with its MoveToPosition)
class Goal_Ship1D_MoveToPosition : public AtomicGoal
{
	public:
		Goal_Ship1D_MoveToPosition(Ship_1D &ship,double position);
		~Goal_Ship1D_MoveToPosition();
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate() {m_Terminate=true;}
	private:
		Ship_1D &m_ship;
		double m_Position;
		bool m_Terminate;
};
