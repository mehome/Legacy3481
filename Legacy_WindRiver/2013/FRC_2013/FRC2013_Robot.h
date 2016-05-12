#pragma once


class FRC_2013_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface,
									public Servo_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
	#ifdef Robot_TesterCode
	virtual void BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)=0;
	#endif
};

//We know that from ground to first rung distance vs. first to second rung will be different lengths since the angle approach is different, so we'll need at least
//two different distance settings.  The second to third rung should be equal, but we can add more if needed here.
const size_t c_NoClimbPropertyElements=2;

struct FRC_2013_Robot_Props
{
public:	
	Vec2D PresetPositions[3];
	Vec2D KeyGrid[6][3];
	struct DeliveryCorrectionFields
	{
		double PowerCorrection;
		double PitchCorrection;
	};
	DeliveryCorrectionFields KeyCorrections[6][3];
	double FireTriggerDelay;		//Time for stable signal before triggering the fire
	double FireButtonStayOn_Time;   //Time to stay on before stopping the conveyors
	size_t Coordinates_DiplayRow;
	size_t TargetVars_DisplayRow;
	size_t Power1Velocity_DisplayRow,Power2Velocity_DisplayRow;
	double YawTolerance;			//Used for drive yaw targeting (the drive is the turret) to avoid oscillation
	double Min_IntakeDrop;			//Used to determine the minimum drop of the intake during a fire operation
	double SmoothingPitch,SmoothingYaw;  //Used to help camera not overshoot the delta adjustments

	struct Climb_Properties
	{
		//In theory lift and drop should be the same they can be negative in direction as well.  They may work where lift goes in one directions and the drop goes in the opposite
		double LiftDistance;
		double DropDistance;
	} Climb_Props[c_NoClimbPropertyElements];

	struct Autonomous_Properties
	{
		//These are first iteration open loop based on time... fine if nothing else is needed
		double InitalRevTime;
		double WaitOnTime;
		double WaitOffTime;
		//solved empirically
		double FirstStageVelocity, SecondStageVelocity;

		//May want to use something like this for next iteration... closed loop will make most efficient time to do other tasks
		struct WaitForBall_Info
		{
			double InitialWait;
			double TimeOutWait;			//If -1 then it is infinite and will not multi task a wait time (great for testing)
			double ToleranceThreshold;  //If zero then only the initial wait is used for each ball (or not using the wait for ball feature)
		} FirstBall_Wait,SecondBall_Wait; //We'll want to tweak the second ball a bit differently
		double DriveToY; //waypoint Y to drive in meters
	} Autonomous_Props;
};

class FRC_2013_Robot_Properties : public Tank_Robot_Properties
{
	public:
		FRC_2013_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);

		const Servo_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Servo_Properties &GetTurretProps() const {return m_TurretProps;}
		const Rotary_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}
		const Rotary_Properties &GetPowerSlowWheelProps() const {return m_PowerSlowWheelProps;}
		const Rotary_Properties &GetHelixProps() const {return m_HelixProps;}
		const Rotary_Properties &GetRollersProps() const {return m_RollersProps;}
		const Rotary_Properties &GetIntakeDeploymentProps() const {return m_IntakeDeploymentProps;}
		const Tank_Robot_Properties &GetClimbGearLiftProps() const {return m_ClimbGearLiftProps;}
		const Tank_Robot_Properties &GetClimbGearDropProps() const {return m_ClimbGearDropProps;}
		const FRC_2013_Robot_Props &GetFRC2013RobotProps() const {return m_FRC2013RobotProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		#ifndef Robot_TesterCode
		typedef Tank_Robot_Properties __super;
		#endif
		Servo_Properties m_PitchRampProps,m_TurretProps;
		Rotary_Properties m_PowerWheelProps,m_PowerSlowWheelProps,m_HelixProps,m_RollersProps,m_IntakeDeploymentProps;
		Tank_Robot_Properties m_ClimbGearLiftProps;
		Tank_Robot_Properties m_ClimbGearDropProps;
		FRC_2013_Robot_Props m_FRC2013RobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;
};

class FRC_2013_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			ePowerWheelFirstStage,
			ePowerWheelSecondStage,
			eHelix,
			eIntake_Deployment,
			eRollers
		};

		enum ServoDevices
		{
			ePitchRamp,
			eTurret
		};

		//Most likely will not need IR sensors
		enum BoolSensorDevices
		{
			eIntake_DeployedLimit_Sensor
		};

		//Note: these shouldn't be written to directly but instead set the climb state which then will write to these
		//This will guarantee that they will be preserved in a mutually exclusive state
		enum SolenoidDevices
		{
			//These 3 cylinders work together:
			//neutral state is when all all neutral, and will be a transitional state; otherwise these are mutually exclusive
			eEngageDriveTrain,		//Cylinder 1 is on the drive train gear box
			eEngageLiftWinch,		//Cylinder 2 is on the lift winch
			eEngageDropWinch,		//Cylinder 3 is on the drop winch
			eFirePiston				//pneumatic 4... will engage on fire button per press
		};

		//You use a variation of selected states to do things
		//so driving would be:

		//cylinder 1 = drive
		//2 = neutral
		//3 = neutral

		//raising lift is
		//1 = neutral
		//2 = engaged
		//3 = neutral

		//dropping lift for climb
		//1 = neutral
		//2 = neutral
		//3= engaged
		enum ClimbState
		{
			eClimbState_Neutral,
			eClimbState_Drive,
			eClimbState_RaiseLift,
			//Drop lift has been divided into two phases... we'll want to give time for it to engage which the lift winch is still engaged, before releasing the lift winch
			eClimbState_DropLift,
			eClimbState_DropLift2
		};

		enum Targets
		{
			eCenterHighGoal,
			eLeftGoal,
			eRightGoal,
			eFrisbee,
			eDefensiveKey
		};
		FRC_2013_Robot(const char EntityName[],FRC_2013_Control_Interface *robot_control,bool IsAutonomous=false);
		virtual~FRC_2013_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:

		class AxisControl : public Servo_Position_Control
		{
			public:
				AxisControl(FRC_2013_Robot *pParent,const char EntityName[],Servo_Control_Interface *robot_control,size_t InstanceIndex);
				IEvent::HandlerList ehl;
			protected:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				virtual void SetIntendedPosition_Plus(double Position);
				FRC_2013_Robot * const m_pParent;
		};

		class PitchRamp : public AxisControl
		{
			private:
				#ifndef Robot_TesterCode
				typedef AxisControl __super;
				#endif
			public:
				PitchRamp(FRC_2013_Robot *pParent,Servo_Control_Interface *robot_control);
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void TimeChange(double dTime_s);
			protected:
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition_Plus(double Position) {__super::SetIntendedPosition_Plus(Position);}
		};

		class Turret : public AxisControl
		{
			private:

			#ifndef Robot_TesterCode
			typedef AxisControl __super;
			#endif
			public:
				Turret(FRC_2013_Robot *pParent,Servo_Control_Interface *robot_control);
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void TimeChange(double dTime_s);
			protected:
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition_Plus(double Position) {__super::SetIntendedPosition_Plus(Position);}
		};

		class PowerWheels
		{
			public:
				PowerWheels(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				void Initialize(Base::EventMap& em,const Entity1D_Properties *props=NULL);

				void BindAdditionalEventControls(bool Bind);
				void ResetPos();
				const Rotary_Velocity_Control &GetFirstStageShooter() const {return m_FirstStage;}
				const Rotary_Velocity_Control &GetSecondStageShooter() const {return m_SecondStage;}
				void TimeChange(double dTime_s);
				bool GetIsRunning() const {return m_IsRunning;}
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
			protected:
				void SetRequestedVelocity_FromNormalized(double Velocity) {m_ManualVelocity=Velocity;}
				void SetRequestedVelocity_Axis_FromNormalized(double Velocity) {m_ManualAcceleration=Velocity;}
				void Set_FirstStage_RequestedVelocity_FromNormalized(double Velocity) {m_FirstStageManualVelocity=Velocity;}
				void SetEncoderSafety(bool DisableFeedback);
			private:
				FRC_2013_Robot * const m_pParent;
				Rotary_Velocity_Control m_SecondStage,m_FirstStage;
				double m_ManualVelocity,m_FirstStageManualVelocity;
				double m_ManualAcceleration;
				bool m_IsRunning;
		};

		class IntakeSystem
		{
			private:
				FRC_2013_Robot * const m_pParent;
				Rotary_Velocity_Control m_Helix,m_Rollers;
				class Intake_Deployment : public Rotary_Position_Control
				{
				private:
					#ifndef Robot_TesterCode
					typedef Rotary_Position_Control __super;
					#endif
					FRC_2013_Robot * const m_pParent;
					bool m_Advance,m_Retract;
					bool m_ChooseDropped; //cache last state that was used
				public:
					Intake_Deployment(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
					IEvent::HandlerList ehl;
					virtual void BindAdditionalEventControls(bool Bind);
					void SetIntendedPosition(double Position);
					bool GetChooseDropped() const {return m_ChooseDropped;}
				protected:
					void Advance();
					void Retract();
					//events are a bit picky on what to subscribe so we'll just wrap from here
					void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

					void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
					virtual void TimeChange(double dTime_s);
				} m_IntakeDeployment;

				double m_FireDelayTrigger_Time; //Time counter of the value remaining in the on-to-delay state
				double m_FireStayOn_Time;  //Time counter of the value remaining in the on state
				bool m_FireDelayTriggerOn; //A valve mechanism that must meet time requirement to disable the delay
				bool m_FireStayOn;		   //A valve mechanism that must time out to turn off
				union ControlSignals
				{
					struct ControlSignals_rw
					{
						unsigned char Grip   : 1;
						unsigned char Squirt : 1;
						unsigned char Fire   : 1;
						unsigned char GripH  : 1;
					} bits;
					unsigned char raw;
				} m_ControlSignals;
			public:
				IntakeSystem(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
				void Initialize(Base::EventMap& em,const Entity1D_Properties *props=NULL);
				bool GetIsFireRequested() const {return m_ControlSignals.bits.Fire==1;}
				IEvent::HandlerList ehl;

				void ResetPos();
				//Intercept the time change to send out voltage
				void TimeChange(double dTime_s);
				void BindAdditionalEventControls(bool Bind);
				//Expose for goals
				void Fire(bool on) {m_ControlSignals.bits.Fire=on;}
				void Squirt(bool on) {m_ControlSignals.bits.Squirt=on;}

			protected:
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on) {m_ControlSignals.bits.Grip=on;}

				void SetRequestedVelocity_FromNormalized(double Velocity);
		};

	public: //Autonomous public access (wind river has problems with friend technique)
		IntakeSystem &GetIntakeSystem();
		PowerWheels &GetPowerWheels();
		void SetTarget(Targets target);
		const FRC_2013_Robot_Properties &GetRobotProps() const;
		void SetClimbState(ClimbState climb_state);
		bool IsStopped() const;  //returns true if both encoders read zero on this iteration
	protected:
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);
	private:
		void ApplyErrorCorrection();
		#ifndef Robot_TesterCode
		typedef  Tank_Robot __super;
		#endif
		FRC_2013_Control_Interface * const m_RobotControl;
		PitchRamp m_PitchRamp;
		Turret m_Turret;
		PowerWheels m_PowerWheels;
		IntakeSystem m_IntakeSystem;
		FRC_2013_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Targets m_Target;		//This allows us to change our target
		Vec2D m_DefensiveKeyPosition;
		void *m_UDP_Listener;

		//This is adjusted depending on location for correct bank-shot angle trajectory, note: the coordinate system is based where 0,0 is the 
		//middle of the game playing field
		Vec2D m_TargetOffset;  //2d top view x,y of the target
		//This is adjusted depending on doing a bank shot or swishing 
		double m_TargetHeight;  //1d z height (front view) of the target
		//cached during robot time change and applied to other systems when targeting is true
		double m_PitchAngle,m_YawAngle;
		//TODO remove these
		double m_LinearVelocity,m_HangTime;
		double m_PitchErrorCorrection,m_PowerErrorCorrection;
		double m_DefensiveKeyNormalizedDistance;
		size_t m_DefaultPresetIndex;
		size_t m_AutonPresetIndex;  //used only because encoder tracking is disabled
		bool m_POVSetValve;

		bool m_IsTargeting;
		bool IsTargeting() const {return m_IsTargeting;}
		void SetTargeting(bool on) {m_IsTargeting=on;}
		void SetTargeting_Off(bool off) {SetTargeting(!off);}
		void SetTargetingOn() {SetTargeting(true);}
		void SetTargetingOff() {SetTargeting(false);}
		void SetTargetingValue(double Value);

		void SetClimbDriveEngaged() {SetClimbState(eClimbState_Drive);}
		void SetClimbRaiseLift() {SetClimbState(eClimbState_RaiseLift);}
		void SetClimbDropLift() {SetClimbState(eClimbState_DropLift);}
		void SetClimbDropLift2() {SetClimbState(eClimbState_DropLift2);}
		void SetClimbSpeed(double Speed);
		ClimbState m_ClimbState;

		enum AutoDriveState
		{
			eAutoDrive_Disabled,
			eAutoDrive_YawOnly,  //as name implies this only rotates for targets
			eAutoDrive_FullAuto  //This does full drive to way-point for Frisbees
		};
		AutoDriveState m_AutoDriveState;
		AutoDriveState GetAutoDriveState() const {return m_AutoDriveState;}
		void SetAutoDriveYaw(bool on) {m_AutoDriveState=on?eAutoDrive_YawOnly:eAutoDrive_Disabled;}
		void SetAutoDriveYawOn() {SetAutoDriveYaw(true);}
		void SetAutoDriveYawOff() {SetAutoDriveYaw(false);}
		void SetAutoDriveFull(bool on);
		void SetAutoDriveFullOn() {SetAutoDriveFull(true);}
		void SetAutoDriveFullOff() {SetAutoDriveFull(false);}

		size_t m_ClimbCounter;  //keep track of which iteration count we are on (saturates to c_NoClimbPropertyElements)
		bool m_SetClimbGear;
		bool m_SetClimbLeft,m_SetClimbRight;
		void SetClimbGear(bool on);
		void SetClimbGear_LeftButton(bool on);
		void SetClimbGear_RightButton(bool on);
		void SetClimbGearOn() {SetClimbGear(true);}
		void SetClimbGearOff() {SetClimbGear(false);}
		void Set10PointHang() {m_RobotControl->GetIsSolenoidClosed(eEngageDriveTrain)?
			m_RobotControl->OpenSolenoid(eEngageDriveTrain):m_RobotControl->CloseSolenoid(eEngageDriveTrain);}
		
		void SetPresetPOV (double value);

		void SetDefensiveKeyPosition(double NormalizedDistance) {m_DefensiveKeyNormalizedDistance=NormalizedDistance;}
		void SetDefensiveKeyOn();
		void SetDefensiveKeyOff() {m_Target=eCenterHighGoal;}

		void Robot_SetCreepMode(bool on);
};

class FRC_2013_Goals
{
	public:
		static Goal *Get_ShootFrisbees(FRC_2013_Robot *Robot,bool EnableDriveBack);
		//static Goal *Get_FRC2013_Autonomous(FRC_2013_Robot *Robot,size_t KeyIndex,size_t TargetIndex,size_t RampIndex);
		/// \param iteration this is a simple count of how many climbs which have been made.  This is used to pick the correct distance properties to use per iteration
		static Goal *Climb(FRC_2013_Robot *Robot,size_t iteration);
	private:
		class Fire : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			bool m_Terminate;
			bool m_IsOn;
			bool m_AimOnly;  //If True it does the feed instead of fire
		public:
			Fire(FRC_2013_Robot &robot, bool On, bool AimOnly=false);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class WaitForBall : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			double m_Tolerance;
			bool m_Terminate;
		public:
			WaitForBall(FRC_2013_Robot &robot,double Tolerance);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class ResetPosition : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			double m_Timer;  //keep track of how much time has elapsed
			double m_TimeStopped;  //keep track of how much *consecutive* time it has been stopped
			bool m_Terminate;
		public:
			ResetPosition(FRC_2013_Robot &robot);
			virtual void Activate();
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class ChangeClimbState : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			const FRC_2013_Robot::ClimbState m_ClimbState;
			double m_TimeAccrued;
			double m_TimeToWait;
			bool m_Terminate;
		public:
			ChangeClimbState(FRC_2013_Robot &robot,FRC_2013_Robot::ClimbState climb_state,double TimeToTakeEffect_s=1.00);
			virtual void Activate();
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};
};

#ifdef Robot_TesterCode

#undef __TestXAxisServoDump__
#define __TestPotsOnEncoder__
class FRC_2013_Robot_Control : public FRC_2013_Control_Interface
{
	public:
		FRC_2013_Robot_Control();
		const FRC_2013_Robot_Properties &GetRobotProps() const {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void OpenSolenoid(size_t index,bool Open);
		virtual bool GetIsSolenoidOpen(size_t index) const;
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}

		#ifndef __TestXAxisServoDump__
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);  //Needed to intercept for climb case
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		#else
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		#endif

		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}

	protected: //from Servo Interface
		virtual void Reset_Servo(size_t index=0); 
		virtual double GetServoAngle(size_t index=0);
		virtual void SetServoAngle(size_t index,double radians);

	protected: //from FRC_2013_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		//Note: This is only for Robot Tester
		virtual void BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl);

		void TriggerIntakeDeployedLimit(bool on) {m_DeployedLimit=on;}
		void SlowWheel(bool on) {m_SlowWheel=on;}

	protected:
		FRC_2013_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		#ifndef __TestPotsOnEncoder__
		Potentiometer_Tester2 m_Pitch_Pot,m_IntakeDeployment_Pot; //simulate the potentiometer and motor
		#else
		Encoder_Simulator2 m_Pitch_Pot,m_IntakeDeployment_Pot;
		#endif
		Encoder_Simulator m_PowerWheel_Enc,m_PowerSlowWheel_Enc,m_Helix_Enc,m_Rollers_Enc;  //simulate the encoder and motor
		KalmanFilter m_KalFilter_Arm;
		#ifdef __TestXAxisServoDump__
		double m_LastYawAxisSetting;  //needed to creep up the angle to position smoothly when testing servo code
		double m_LastLeftVelocity,m_LastRightVelocity;
		#endif
		//cache voltage values for display
		double m_PitchRampAngle,m_TurretAngle;
		double m_PowerWheelVoltage,m_PowerSlowWheelVoltage,m_IntakeDeploymentVoltage;
		double m_HelixVoltage,m_RollersVoltage;
		double m_IntakeDeploymentOffset;  //used to keep 90-0 range once limit switch has been triggered
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use
		bool m_IsDriveEngaged;  //Cache when the drive is engaged to avoid excessive I/O reads
		bool m_DeployedLimit;
		bool m_SlowWheel;
		bool m_FirePiston;
};

class FRC_2013_Power_Wheel_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Power_Wheel_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
		double m_PowerWheelMaxSpeed;  //cache to avoid all the hoops of getting it (it's constant)
};

class FRC_2013_Power_Slow_Wheel_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Power_Slow_Wheel_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
		double m_PowerWheelMaxSpeed;  //cache to avoid all the hoops of getting it (it's constant)
};

class FRC_2013_Rollers_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Rollers_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
};

class FRC_2013_Fire_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Fire_Conveyor_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
};

class Axis_UI : public Swivel_Wheel_UI
{
public:
	virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.0,0.6,0.2,1.0);}
	virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(0.1,0.1,1.0,1.0);}
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2013_Robot_UI : public FRC_2013_Robot, public FRC_2013_Robot_Control
{
	public:
		FRC_2013_Robot_UI(const char EntityName[]);
	protected:
		virtual void TimeChange(double dTime_s);
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos); 
		virtual void Text_SizeToUse(double SizeToUse);
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

	private:
		Tank_Robot_UI m_TankUI;
		FRC_2013_Power_Wheel_UI m_PowerWheelUI;
		FRC_2013_Power_Slow_Wheel_UI m_PowerSlowWheelUI;
		FRC_2013_Rollers_UI m_Rollers;
		FRC_2013_Fire_Conveyor_UI m_Helix;
		Axis_UI m_AxisCamera;
};

#endif //Robot_TesterCode
