#pragma once


class FRC_2012_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
	#ifdef Robot_TesterCode
	virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl)=0;
	#endif
};

struct FRC_2012_Robot_Props
{
public:
	
	Vec2D PresetPositions[3];
	Vec2D KeyGrid[3][3];
	struct DeliveryCorrectionFields
	{
		double PowerCorrection;
		double YawCorrection;
	};
	DeliveryCorrectionFields KeyCorrections[3][3];
	double FireTriggerDelay;		//Time for stable signal before triggering the fire
	double FireButtonStayOn_Time;   //Time to stay on before stopping the conveyors
	size_t Coordinates_DiplayRow;
	size_t TargetVars_DisplayRow;
	size_t PowerVelocity_DisplayRow;

	struct Autonomous_Properties
	{
		Vec2D RampLeft_ErrorCorrection_Offset;
		Vec2D RampRight_ErrorCorrection_Offset;
		Vec2D RampCenter_ErrorCorrection_Offset;
		double XLeftArc,XRightArc;
		struct WaitForBall_Info
		{
			double InitialWait;
			double TimeOutWait;			//If -1 then it is infinite and will not multi task a wait time (great for testing)
			double ToleranceThreshold;  //If zero then only the initial wait is used for each ball (or not using the wait for ball feature)
		} FirstBall_Wait,SecondBall_Wait; //We'll want to tweak the second ball a bit differently
		double MoveForward;				//Optional to move forward to use less power to shoot
		double TwoShotScaler;			//Scaler used for two point shots
	} Autonomous_Props;
};

class FRC_2012_Robot_Properties : public Tank_Robot_Properties
{
	public:

		FRC_2012_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);

		const Rotary_Properties &GetTurretProps() const {return m_TurretProps;}
		const Rotary_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Rotary_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}
		const Rotary_Properties &GetConveyorProps() const {return m_ConveyorProps;}
		const Rotary_Properties &GetFlipperProps() const {return m_FlipperProps;}
		const Tank_Robot_Properties &GetLowGearProps() const {return m_LowGearProps;}
		const FRC_2012_Robot_Props &GetFRC2012RobotProps() const {return m_FRC2012RobotProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		#ifndef Robot_TesterCode
		typedef Tank_Robot_Properties __super;
		#endif
		Rotary_Properties m_TurretProps,m_PitchRampProps,m_PowerWheelProps,m_ConveyorProps,m_FlipperProps;
		Tank_Robot_Properties m_LowGearProps;
		FRC_2012_Robot_Props m_FRC2012RobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;
};

class FRC_2012_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			eTurret,
			ePitchRamp,
			ePowerWheels,
			eLowerConveyor,
			eMiddleConveyor,
			eFireConveyor,
			eFlippers
		};

		enum BoolSensorDevices
		{
			eLowerConveyor_Sensor,
			eMiddleConveyor_Sensor,
			eFireConveyor_Sensor
		};

		enum SolenoidDevices
		{
			eUseLowGear,		//If the OpenSolenoid() is called with true then it should be in low gear; otherwise high gear
			eFlipperDown,		//If true flipper is down
			eUseBreakDrive,		//OpenSolenoid() is called with true then its in break mode (default is coast) This is really a digital output
			eRampDeployment
		};


		enum Targets
		{
			eCenterHighGoal,
			eLeftGoal,
			eRightGoal,
			eDefensiveKey
		};
		FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool IsAutonomous=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:
		class Turret : public Rotary_Position_Control
		{
			private:
				FRC_2012_Robot * const m_pParent;
				double m_Velocity; //adds all axis velocities then assigns on the time change
				double m_LastIntendedPosition;
			public:
				Turret(FRC_2012_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void ResetPos();
			protected:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				void Turret_SetRequestedVelocity(double Velocity) {m_Velocity+=Velocity;}
				void SetIntendedPosition_Plus(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
		};

		class PitchRamp : public Rotary_Position_Control
		{
			public:
				PitchRamp(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition_Plus(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2012_Robot * const m_pParent;
		};

		class PowerWheels : public Rotary_Velocity_Control
		{
			public:
				PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void ResetPos();
			protected:
				#ifndef Robot_TesterCode
				typedef Rotary_Velocity_Control __super;
				#endif
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetEncoderSafety(bool DisableFeedback) {__super::SetEncoderSafety(DisableFeedback);}
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2012_Robot * const m_pParent;
				double m_ManualVelocity;
				bool m_IsRunning;
		};

		class BallConveyorSystem
		{
			private:
				FRC_2012_Robot * const m_pParent;
				Rotary_Velocity_Control m_LowerConveyor,m_MiddleConveyor,m_FireConveyor;
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
						unsigned char GripL  : 1;	//Manual grip low, medium, and high
						unsigned char GripM  : 1;
						unsigned char GripH  : 1;
					} bits;
					unsigned char raw;
				} m_ControlSignals;
			public:
				BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				void Initialize(Entity2D_Kind::EventMap& em,const Entity1D_Properties *props=NULL);
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
				void GripL(bool on) {m_ControlSignals.bits.GripL=on;}
				void GripM(bool on) {m_ControlSignals.bits.GripM=on;}
				void GripH(bool on) {m_ControlSignals.bits.GripH=on;}

				void SetRequestedVelocity_FromNormalized(double Velocity);
		};

		class Flippers : public Rotary_Position_Control
		{
			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				FRC_2012_Robot * const m_pParent;
				bool m_Advance,m_Retract;
			public:
				Flippers(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:

				void Advance(bool on) {m_Advance=on;}
				void Retract(bool on) {m_Retract=on;}
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
		};

	public: //Autonomous public access (wind river has problems with friend technique)
		BallConveyorSystem &GetBallConveyorSystem();
		PowerWheels &GetPowerWheels();
		void SetPresetPosition(size_t index,bool IgnoreOrientation=false);
		void Set_Auton_PresetPosition(size_t index);
		void SetTarget(Targets target);
		const FRC_2012_Robot_Properties &GetRobotProps() const;
		void SetFlipperPneumatic(bool on) {m_RobotControl->OpenSolenoid(eFlipperDown,on);}
	protected:
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);
	private:
		void ApplyErrorCorrection();
		#ifndef Robot_TesterCode
		typedef  Tank_Robot __super;
		#endif
		FRC_2012_Control_Interface * const m_RobotControl;
		Turret m_Turret;
		PitchRamp m_PitchRamp;
		PowerWheels m_PowerWheels;
		BallConveyorSystem m_BallConveyorSystem;
		Flippers m_Flippers;
		FRC_2012_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Targets m_Target;		//This allows us to change our target
		Vec2D m_DefensiveKeyPosition;

		//This is adjusted depending on location for correct bank-shot angle trajectory, note: the coordinate system is based where 0,0 is the 
		//middle of the game playing field
		Vec2D m_TargetOffset;  //2d top view x,y of the target
		//This is adjusted depending on doing a bank shot or swishing 
		double m_TargetHeight;  //1d z height (front view) of the target
		//cached during robot time change and applied to other systems when targeting is true
		double m_PitchAngle,m_LinearVelocity,m_HangTime;
		double m_YawErrorCorrection,m_PowerErrorCorrection;
		double m_DefensiveKeyNormalizedDistance;
		size_t m_DefaultPresetIndex;
		size_t m_AutonPresetIndex;  //used only because encoder tracking is disabled
		bool m_DisableTurretTargetingValue;
		bool m_POVSetValve;

		bool m_IsTargeting;
		void IsTargeting(bool on) {m_IsTargeting=on;}
		void SetTargetingOn() {IsTargeting(true);}
		void SetTargetingOff() {IsTargeting(false);}
		void SetTurretTargetingOff(bool on) {m_DisableTurretTargetingValue=on;}
		void SetTargetingValue(double Value);

		bool m_SetLowGear;
		void SetLowGear(bool on);
		void SetLowGearOn() {SetLowGear(true);}
		void SetLowGearOff() {SetLowGear(false);}
		void SetLowGearValue(double Value);
		
		void SetPreset1() {SetPresetPosition(0);}
		void SetPreset2() {SetPresetPosition(1);}
		void SetPreset3() {SetPresetPosition(2);}
		void SetPresetPOV (double value);

		void SetDefensiveKeyPosition(double NormalizedDistance) {m_DefensiveKeyNormalizedDistance=NormalizedDistance;}
		void SetDefensiveKeyOn();
		void SetDefensiveKeyOff() {m_Target=eCenterHighGoal;}

		void Robot_SetCreepMode(bool on);
};

class FRC_2012_Goals
{
	public:
		static Goal *Get_ShootBalls(FRC_2012_Robot *Robot,bool DoSquirt=false);
		static Goal *Get_ShootBalls_WithPreset(FRC_2012_Robot *Robot,size_t KeyIndex);
		static Goal *Get_FRC2012_Autonomous(FRC_2012_Robot *Robot,size_t KeyIndex,size_t TargetIndex,size_t RampIndex);
	private:
		class Fire : public AtomicGoal
		{
		private:
			FRC_2012_Robot &m_Robot;
			bool m_Terminate;
			bool m_IsOn;
			bool m_DoSquirt;  //If True it does the feed instead of fire
		public:
			Fire(FRC_2012_Robot &robot, bool On, bool DoSquirt=false);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class WaitForBall : public AtomicGoal
		{
		private:
			FRC_2012_Robot &m_Robot;
			double m_Tolerance;
			bool m_Terminate;
		public:
			WaitForBall(FRC_2012_Robot &robot,double Tolerance);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class OperateSolenoid : public AtomicGoal
		{
		private:
			FRC_2012_Robot &m_Robot;
			const FRC_2012_Robot::SolenoidDevices m_SolenoidDevice;
			bool m_Terminate;
			bool m_IsOpen;
		public:
			OperateSolenoid(FRC_2012_Robot &robot,FRC_2012_Robot::SolenoidDevices SolenoidDevice,bool Open);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};
};

#ifdef Robot_TesterCode

class FRC_2012_Robot_Control : public FRC_2012_Control_Interface
{
	public:
		FRC_2012_Robot_Control();
		const FRC_2012_Robot_Properties &GetRobotProps() const {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void CloseSolenoid(size_t index,bool Close) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}

	protected: //from FRC_2012_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		//Note: This is only for Robot Tester
		virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl);

		void TriggerLower(bool on) {m_LowerSensor=on;}
		void TriggerMiddle(bool on) {m_MiddleSensor=on;}
		void TriggerFire(bool on) {m_FireSensor=on;}
		void SlowWheel(bool on) {m_SlowWheel=on;}

	protected:
		FRC_2012_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Potentiometer_Tester2 m_Turret_Pot,m_Pitch_Pot,m_Flippers_Pot; //simulate the potentiometer and motor
		Encoder_Simulator m_PowerWheel_Enc,m_LowerConveyor_Enc,m_MiddleConveyor_Enc,m_FireConveyor_Enc;  //simulate the encoder and motor
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
		double m_TurretVoltage,m_PitchRampVoltage,m_PowerWheelVoltage,m_FlipperVoltage;
		double m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage;
		bool m_LowerSensor,m_MiddleSensor,m_FireSensor;
		bool m_SlowWheel;
};

class FRC_2012_Turret_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		struct Turret_Properties
		{
			double YOffset;
		};

		FRC_2012_Turret_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		virtual void Initialize(Entity2D::EventMap& em, const Turret_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
		Actor_Text *m_UIParent;
		Turret_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Turret; 
};

class FRC_2012_Power_Wheel_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Power_Wheel_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
		double m_PowerWheelMaxSpeed;  //cache to avoid all the hoops of getting it (its constant)
};

class FRC_2012_Lower_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Lower_Conveyor_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
};

class FRC_2012_Middle_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Middle_Conveyor_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
};

class FRC_2012_Fire_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Fire_Conveyor_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2012_Robot_UI : public FRC_2012_Robot, public FRC_2012_Robot_Control
{
	public:
		FRC_2012_Robot_UI(const char EntityName[]);
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
		FRC_2012_Turret_UI m_TurretUI;
		FRC_2012_Power_Wheel_UI m_PowerWheelUI;
		FRC_2012_Lower_Conveyor_UI m_LowerConveyor;
		FRC_2012_Middle_Conveyor_UI m_MiddleConveyor;
		FRC_2012_Fire_Conveyor_UI m_FireConveyor;
};

#endif

