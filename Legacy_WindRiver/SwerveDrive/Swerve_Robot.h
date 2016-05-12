#pragma once

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Swerve_Drive_Control_Interface : public Rotary_Control_Interface,
									   public Robot_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Encoders()=0;
};

struct Swerve_Robot_Props
{
	typedef Framework::Base::Vec2d Vec2D;
	//typedef osg::Vec2d Vec2D;

	//Currently supporting 4 terms in polynomial equation
	//Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	double Polynomial_Wheel[5];
	double Polynomial_Swivel[5];  

	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double VoltageScalar;		//Used to handle reversed voltage wiring
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double Wheel_PID[3]; //p,i,d
	double Swivel_PID[3]; //p,i,d
	double InputLatency;  //Used with PID to help avoid oscillation in the error control (We can make one for each if needed)
	double HeadingLatency; //Should be about 100ms + Input Latency... this will establish intervals to sync up the heading with entity
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	double MaxSpeedOffset[4];	//These are used to align max speed to what is reported by encoders (Encoder MaxSpeed - Computed MaxSpeed)
	Vec2D DriveTo_ForceDegradeScalar;  //Used for way point driving in autonomous in conjunction with max force to get better deceleration precision
	double SwivelRange;  //Value in radians of the swivel range 0 is infinite
	//double TankSteering_Tolerance; //used to help controls drive straight
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxAccel;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	double InverseMaxDecel;  //used for deceleration case
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen_Wheel,IsOpen_Swivel;  //give ability to open or close loop for wheel or swivel system  
	//This will dump the console PID info (Only active if __DebugLUA__ is defined)
	bool PID_Console_Dump_Wheel[4];  
	bool PID_Console_Dump_Swivel[4];
	bool ReverseSteering;  //This will fix if the wiring on voltage has been reversed (e.g. voltage to right turns left side)
	//Different robots may have the encoders flipped or not which must represent the same direction of both treads
	bool EncoderReversed_Wheel[4];
	bool EncoderReversed_Swivel[4];
};

class Swerve_Robot_UI;

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class  Swerve_Robot : public Ship_Tester,
					 public Swerve_Drive_Interface
{
	public:
		//Note these order in the same as SwerveVelocities::SectionOrder
		enum Swerve_Robot_SpeedControllerDevices
		{
			eWheel_FL,
			eWheel_FR,
			eWheel_RL,
			eWheel_RR,
			eSwivel_FL,
			eSwivel_FR,
			eSwivel_RL,
			eSwivel_RR,
			eNoSwerveRobotSpeedControllerDevices
		};

		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		Swerve_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		~Swerve_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		/// \param ResetPos typically true for autonomous and false for dynamic use
		void SetUseEncoders(bool UseEncoders,bool ResetPosition=true);
		void SetIsAutonomous(bool IsAutonomous);
		virtual void TimeChange(double dTime_s);

		const Swerve_Robot_Props &GetSwerveRobotProps() const {return m_SwerveRobotProps;}
		const Swerve_Drive_Control_Interface &GetRobotControl() const {return *m_RobotControl;}
		//Give ability to change properties
		void UpdateDriveProps(const Rotary_Props &DriveProps,const Ship_1D_Props &ShipProps,size_t index);
	protected:
		//friend Swerve_Robot_UI;

		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		//virtual void BindAdditionalEventControls(bool Bind);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		virtual void SetAttitude(double radians);  //from ship tester

		//Get the sweet spot between the update and interpolation to avoid oscillation 
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);

		virtual Vec2D Get_DriveTo_ForceDegradeScalar() const {return m_SwerveRobotProps.DriveTo_ForceDegradeScalar;}
		virtual Swerve_Drive *CreateDrive() {return new Swerve_Drive(this);}
		virtual void DestroyDrive();

		//from Swerve_Drive_Interface
		virtual const SwerveVelocities &GetSwerveVelocities() const {return m_Swerve_Robot_Velocities;}
		virtual Vec2D GetLinearVelocity_ToDisplay() {return GlobalToLocal(GetAtt_r(),m_EncoderGlobalVelocity);}
		virtual double GetAngularVelocity_ToDisplay() {return m_EncoderAngularVelocity;}

		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s) 
			{m_TankSteering.UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,*this,LockShipHeadingToOrientation,dTime_s);
			}
		virtual void BindAdditionalEventControls(bool Bind) 
			{m_TankSteering.BindAdditionalEventControls(Bind,GetEventMap(),ehl);
			}
	protected:  //from Vehicle_Drive_Common_Interface
		virtual const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
		virtual double GetWheelTurningDiameter() const {return m_WheelDimensions.length();}
		virtual double Vehicle_Drive_GetAtt_r() const {return GetAtt_r();}
		virtual const PhysicsEntity_2D &Vehicle_Drive_GetPhysics() const {return GetPhysics();}
		virtual PhysicsEntity_2D &Vehicle_Drive_GetPhysics_RW() {return GetPhysics();}

		Swerve_Drive_Control_Interface * const m_RobotControl;
		bool m_IsAutonomous;
	private:
		typedef Ship_Tester __super;

		Swerve_Drive * m_VehicleDrive;
		
		//The driving module consists of a swivel motor and the driving motor for a wheel.  It manages / converts the intended direction and speed to 
		//actual direction and velocity (i.e. works in reverse) as well as working with sensor feedback (e.g. potentiometer, encoder) for error
		//correction of voltage computation.
		class DrivingModule
		{
			public:
				DrivingModule(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,size_t SectionOrder);
				virtual ~DrivingModule() {}
				
				struct DrivingModule_Props
				{
					const  Rotary_Properties *Swivel_Props;
					const  Rotary_Properties *Drive_Props;
				};
				virtual void Initialize(Framework::Base::EventMap& em,const DrivingModule_Props *props=NULL);
				virtual void TimeChange(double dTime_s);
				void SetIntendedSwivelDirection(double direction) {m_IntendedSwivelDirection=direction;}
				void SetIntendedDriveVelocity(double Velocity) {m_IntendedDriveVelocity=Velocity;}
				//I have no problem exposing read-only access to these :)
				const Rotary_Position_Control &GetSwivel() const {return m_Swivel;}
				const Rotary_Velocity_Control &GetDrive() const {return m_Drive;}
				//Get and Set the Drive properties
				Rotary_Velocity_Control &Drive() {return m_Drive;}
				void ResetPos() {m_Drive.ResetPos(),m_Swivel.ResetPos();}
				
			private:
				std::string m_ModuleName,m_SwivelName,m_DriveName;
				Rotary_Position_Control m_Swivel;  //apply control to swivel mechanism
				Rotary_Velocity_Control m_Drive;  //apply control to drive motor
				//Pass along the intended swivel direction and drive velocity
				double m_IntendedSwivelDirection,m_IntendedDriveVelocity;

				Swerve_Drive_Control_Interface * const m_RobotControl;
		} *m_DrivingModule[4]; //FL, FR, RL, RR  The four modules used  (We could put 6 here if we want)

		bool m_UsingEncoders;
		Vec2D m_WheelDimensions; //cached from the Swerve_Robot_Properties
		SwerveVelocities m_Swerve_Robot_Velocities;
		Vec2D m_EncoderGlobalVelocity;
		double m_EncoderAngularVelocity;
		Swerve_Robot_Props m_SwerveRobotProps; //cached in the Initialize from specific robot

		//These help to manage the latency, where the heading will only reflect injection changes on the latency intervals
		double m_Heading;  //We take over the heading from physics
		double m_HeadingUpdateTimer;
		Tank_Steering m_TankSteering;  //adding controls for tank steering
	public:
		double GetSwerveVelocitiesFromIndex(size_t index) const {return m_VehicleDrive->GetSwerveVelocitiesFromIndex(index);}
};

class Swerve_Robot_Properties : public Ship_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		Swerve_Robot_Properties();
		virtual void LoadFromScript(Framework::Scripting::Script& script);

		const Rotary_Properties &GetSwivelProps() const {return m_SwivelProps;}
		const Rotary_Properties &GetDriveProps() const {return m_DriveProps;}
		//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
		//the width is a measurement of the the center of the wheel width to the other wheel
		const Vec2D &GetWheelDimensions() const {return m_SwerveRobotProps.WheelDimensions;}
		const Swerve_Robot_Props &GetSwerveRobotProps() const {return m_SwerveRobotProps;}
		#ifdef AI_TesterCode
		const EncoderSimulation_Props &GetEncoderSimulationProps() const {return m_EncoderSimulation.GetEncoderSimulationProps();}
		#endif
	private:
		typedef Ship_Properties __super;
		
		//Note the drive properties is a measurement of linear movement (not angular velocity)
		Rotary_Properties m_SwivelProps,m_DriveProps;
		Swerve_Robot_Props m_SwerveRobotProps;
		#ifdef AI_TesterCode
		EncoderSimulation_Properties m_EncoderSimulation;
		#endif
};
