#pragma once

//This is only needed when needing to isolate the drive motors
#define __EnableSafetyOnDrive__

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
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen_Wheel,IsOpen_Swivel;  //give ability to open or close loop for wheel or swivel system
};

#ifndef Robot_TesterCode
#define DRIVE_API
typedef Ship_Properties UI_Ship_Properties;
#endif

class DRIVE_API Swerve_Robot_Properties : public UI_Ship_Properties
{
	public:
		Swerve_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);

		//where the index matches the enumeration of each rotary system
		const Rotary_Pot_Properties &GetRotaryProps(size_t index) const {return m_RotaryProps[index];}

		//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
		//the width is a measurement of the the center of the wheel width to the other wheel
		const Vec2D &GetWheelDimensions() const {return m_SwerveRobotProps.WheelDimensions;}
		const Swerve_Robot_Props &GetSwerveRobotProps() const {return m_SwerveRobotProps;}
		#ifdef Robot_TesterCode
		const EncoderSimulation_Props &GetEncoderSimulationProps() const {return m_EncoderSimulation.GetEncoderSimulationProps();}
		#endif
		//note derived class will populate these properties because of where it is in the script 
		const Control_Assignment_Properties &Get_ControlAssignmentProps() const {return m_ControlAssignmentProps;}
	protected:
		Control_Assignment_Properties m_ControlAssignmentProps;
		Swerve_Robot_Props m_SwerveRobotProps;
	private:
		//typedef Ship_Properties __super;
		Rotary_Pot_Properties m_RotaryProps[8];  //see Swerve_Robot_SpeedControllerDevices for assignments
		Rotary_Pot_Properties m_CommonRotary; //Tried to make this local but it would not succeed
		#ifdef Robot_TesterCode
		EncoderSimulation_Properties m_EncoderSimulation;
		#endif
		#ifndef Robot_TesterCode
		typedef UI_Ship_Properties __super;
		#endif
};


const char * const csz_Swerve_Robot_SpeedControllerDevices_Enum[] =
{
	"wheel_fl","wheel_fr","wheel_rl","wheel_rr",
	"swivel_fl","swivel_fr","swivel_rl","swivel_rr"
};

//Note: rotary systems share the same index as their speed controller counterpart
const char * const csz_Swerve_Robot_AnalogInputs_Enum[] =
{
	"wheel_fl_enc","wheel_fr_enc","wheel_rl_enc","wheel_rr_enc",
	"swivel_fl_pot","swivel_fr_pot","swivel_rl_pot","swivel_rr_pot"
};

class Swerve_Robot_UI;

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class DRIVE_API Swerve_Robot : public Ship_Tester,
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

		static Swerve_Robot_SpeedControllerDevices GetSpeedControllerDevices_Enum (const char *value)
		{	return Enum_GetValue<Swerve_Robot_SpeedControllerDevices> (value,csz_Swerve_Robot_SpeedControllerDevices_Enum,_countof(csz_Swerve_Robot_SpeedControllerDevices_Enum));
		}

		enum AnalogInputs
		{
			eWheel_FLEnc,eWheel_FREnc,eWheel_RLEnc,eWheel_RREnc,
			eSwivel_FLPot,eSwivel_FRPot,eSwivel_RLPot,eSwivel_RRPot
		};

		static AnalogInputs GetAnalogInputs_Enum (const char *value)
		{	return Enum_GetValue<AnalogInputs> (value,csz_Swerve_Robot_AnalogInputs_Enum,_countof(csz_Swerve_Robot_AnalogInputs_Enum));
		}

		Swerve_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,size_t EnumOffset=0,bool IsAutonomous=false);
		~Swerve_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		/// \param ResetPos typically true for autonomous and false for dynamic use
		void SetUseEncoders(bool UseEncoders,bool ResetPosition=true);
		void SetIsAutonomous(bool IsAutonomous);
		virtual void TimeChange(double dTime_s);

		const Swerve_Robot_Props &GetSwerveRobotProps() const {return m_SwerveRobotProps;}
		const Swerve_Robot_Properties &GetSwerveRobotProperties() const {return m_SwerveProperties;}
		const Swerve_Drive_Control_Interface &GetRobotControl() const {return *m_RobotControl;}
		//Give ability to change properties
		void UpdateDriveProps(const Rotary_Props &DriveProps,const Ship_1D_Props &ShipProps,size_t index);
		//For now offer this as a public method, but eventually this needs to be managed internally
		void Swerve_Robot_SetAggresiveStop(bool UseAggresiveStop) 
		{	m_DrivingModule[0]->SetAggresiveStop(UseAggresiveStop),
			m_DrivingModule[1]->SetAggresiveStop(UseAggresiveStop),
			m_DrivingModule[2]->SetAggresiveStop(UseAggresiveStop),
			m_DrivingModule[3]->SetAggresiveStop(UseAggresiveStop);
		}
	protected:
		friend Swerve_Robot_UI;

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

		double GetIntendedSwivelDirection(size_t index) const {return m_DrivingModule[index]->GetIntendedSwivelDirection();}
		double GetIntendedDriveVelocity(size_t index) const {return m_DrivingModule[index]->GetIntendedDriveVelocity();}
	protected:  //from Vehicle_Drive_Common_Interface
		virtual const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
		virtual double GetWheelTurningDiameter() const {return m_WheelDimensions.length();}
		virtual double Vehicle_Drive_GetAtt_r() const {return GetAtt_r();}
		virtual const PhysicsEntity_2D &Vehicle_Drive_GetPhysics() const {return GetPhysics();}
		virtual PhysicsEntity_2D &Vehicle_Drive_GetPhysics_RW() {return GetPhysics();}

		Swerve_Drive_Control_Interface * const m_RobotControl;
		bool m_IsAutonomous;
	private:
		#ifndef Robot_TesterCode
		typedef Ship_Tester __super;
		#endif
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
				virtual void Initialize(Entity2D_Kind::EventMap& em,const DrivingModule_Props *props=NULL);
				virtual void TimeChange(double dTime_s);
				void SetIntendedSwivelDirection(double direction) {m_IntendedSwivelDirection=direction;}
				double GetIntendedSwivelDirection() const {return m_IntendedSwivelDirection;}
				void SetIntendedDriveVelocity(double Velocity) {m_IntendedDriveVelocity=Velocity;}
				double GetIntendedDriveVelocity() const {return m_IntendedDriveVelocity;}
				//I have no problem exposing read-only access to these :)
				const Rotary_Position_Control &GetSwivel() const {return m_Swivel;}
				const Rotary_Velocity_Control &GetDrive() const {return m_Drive;}
				//Get and Set the Drive properties
				Rotary_Velocity_Control &Drive() {return m_Drive;}
				void ResetPos() {m_Drive.ResetPos(),m_Swivel.ResetPos();}
				void SetAggresiveStop(bool UseAggresiveStop) {m_Drive.SetAggresiveStop(UseAggresiveStop);}
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
		Swerve_Robot_Properties m_SwerveProperties;

		//These help to manage the latency, where the heading will only reflect injection changes on the latency intervals
		double m_Heading;  //We take over the heading from physics
		double m_HeadingUpdateTimer;
		Tank_Steering m_TankSteering;  //adding controls for tank steering
		size_t m_RotaryEnumOffset;  //cached from the constructor to shift where the rotary index enum begins
	public:
		double GetSwerveVelocitiesFromIndex(size_t index) const {return m_VehicleDrive->GetSwerveVelocitiesFromIndex(index);}
};


///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class DRIVE_API Swerve_Robot_Control : public RobotControlCommon, public Swerve_Drive_Control_Interface
{
	public:
		Swerve_Robot_Control(bool UseSafety=true);
		~Swerve_Robot_Control();
		void SetDisplayVoltage(bool display) {m_DisplayVoltage=display;}
	protected: //from RobotControlCommon
		virtual size_t RobotControlCommon_Get_Victor_EnumValue(const char *name) const
		{	return Swerve_Robot::GetSpeedControllerDevices_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_DigitalInput_EnumValue(const char *name) const  	{	return (size_t)-1;	}

		virtual size_t RobotControlCommon_Get_AnalogInput_EnumValue(const char *name) const  
		{	return Swerve_Robot::GetAnalogInputs_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_DoubleSolenoid_EnumValue(const char *name) const 	{	return (size_t)-1;	}

		//This is only needed for simulation
	protected: //from Rotary_Control_Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);

		//from Swerve_Drive_Control_Interface
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		virtual void Reset_Encoders();

	public:  //allow this conversion public in case a robot needs to add more wheels with encoders
		double RPS_To_LinearVelocity(double RPS,double GearRatio);
		double RPS_To_LinearVelocity(double RPS,size_t index) const;
		//virtual void DisplayVoltage();  //allow to override
	protected:
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		#ifdef Robot_TesterCode
		Potentiometer_Tester2 m_Potentiometers[4]; //simulate a real potentiometer for calibration testing
		Encoder_Simulator2 m_Encoders[4];
		//cache voltage values for display
		double m_EncoderVoltage[4],m_PotentiometerVoltage[4];
		#endif
		Swerve_Robot_Properties m_SwerveRobotProps; //cached in the Initialize from specific robot
		bool m_DisplayVoltage;
	private:
		__inline double Pot_GetRawValue(size_t index);

		PolynomialEquation_forth m_PotPoly[8];
		KalmanFilter m_KalFilter[8];
		Averager<double,4> m_Averager[8];

};
