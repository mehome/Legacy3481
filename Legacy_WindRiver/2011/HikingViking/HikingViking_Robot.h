#pragma once

class HikingViking_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
};

struct HikingViking_Robot_Props
{
	//everything in meters and radians
	double OptimalAngleUp;
	double OptimalAngleDn;
	double ArmLength;
	double ArmToGearRatio;
	double PotentiometerToArmRatio;
	double PotentiometerMaxRotation;
	double GearHeightOffset;
	double MotorToWheelGearRatio;
};

class HikingViking_Robot_Properties : public Tank_Robot_Properties
{
	public:
		#ifndef Robot_TesterCode
		typedef Tank_Robot_Properties __super;
		#endif

		HikingViking_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);
		
		const Rotary_Properties &GetArmProps() const {return m_ArmProps;}
		const Rotary_Properties &GetClawProps() const {return m_ClawProps;}
		const HikingViking_Robot_Props &GetHikingVikingRobotProps() const {return m_HikingVikingRobotProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		Rotary_Properties m_ArmProps,m_ClawProps;
		HikingViking_Robot_Props m_HikingVikingRobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;

};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class HikingViking_Robot : public Tank_Robot
{
	public:
		enum SolenoidDevices
		{
			eDeployment,
			eClaw,
			eRist
		};
		enum SpeedControllerDevices
		{
			eArm,
			eRollers
		};

		HikingViking_Robot(const char EntityName[],HikingViking_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);
		void CloseDeploymentDoor(bool Close);

		const HikingViking_Robot_Properties &GetRobotProps() const;

		//TODO test roller using is angular to be true
		class Robot_Claw : public Rotary_Velocity_Control
		{
			public:
				Robot_Claw(HikingViking_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//public access needed for goals
				void CloseClaw(bool Close);
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on);
				void Squirt(bool on);
			protected:
				//Intercept the time change to send out voltage
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Velocity_Control __super;
				#endif
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				HikingViking_Robot * const m_pParent;
				bool m_Grip,m_Squirt;
		};
		class Robot_Arm : public Rotary_Position_Control
		{
			public:
				Robot_Arm(HikingViking_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//The parent needs to call initialize
				double HeightToAngle_r(double Height_m) const;
				double Arm_AngleToHeight_m(double Angle_r) const;
				double AngleToHeight_m(double Angle_r) const;
				double GetPosRest();
				//given the raw potentiometer converts to the arm angle
				double PotentiometerRaw_To_Arm_r(double raw) const;
				void CloseRist(bool Close);
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void BindAdditionalEventControls(bool Bind);
				void Advance(bool on);
				void Retract(bool on);
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);

			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				void SetPosRest();
				void SetPos0feet();
				void SetPos3feet();
				void SetPos6feet();
				void SetPos9feet();
				HikingViking_Robot * const m_pParent;
				bool m_Advance, m_Retract;
		};

		//Accessors needed for setting goals
		Robot_Arm &GetArm() {return m_Arm;}
		Robot_Claw &GetClaw() {return m_Claw;}
	protected:
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);
	private:
		HikingViking_Robot_Properties m_RobotProps;
		#ifndef Robot_TesterCode
		typedef  Tank_Robot __super;
		#endif
		HikingViking_Control_Interface * const m_RobotControl;
		Robot_Arm m_Arm;
		Robot_Claw m_Claw;
		bool m_VoltageOverride;  //when true will kill voltage
};


namespace HikingViking_Goals
{
	class Goal_OperateSolenoid : public AtomicGoal
	{
		private:
			HikingViking_Robot &m_Robot;
			const HikingViking_Robot::SolenoidDevices m_SolenoidDevice;
			bool m_Terminate;
			bool m_IsClosed;
		public:
			Goal_OperateSolenoid(HikingViking_Robot &robot,HikingViking_Robot::SolenoidDevices SolenoidDevice,bool Close);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
	};

	Goal *Get_TestLengthGoal(HikingViking_Robot *Robot);
	Goal *Get_UberTubeGoal(HikingViking_Robot *Robot);
}

#ifdef Robot_TesterCode

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class HikingViking_Robot_Control : public HikingViking_Control_Interface
{
	public:
		HikingViking_Robot_Control();
		//This is only needed for simulation
	protected: //from Robot_Control_Interface
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Open) {CloseSolenoid(index,!Open);}
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		//Unfortunately the actual wheels are reversed (resolved here since this is this specific robot)
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);
		//pacify this by returning its current value
		virtual double GetRotaryCurrentPorV(size_t index);
		virtual void CloseRist(bool Close) {CloseSolenoid(HikingViking_Robot::eRist,Close);}
		virtual void OpenRist(bool Close) {CloseSolenoid(HikingViking_Robot::eRist,!Close);}
	protected: //from HikingViking_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);

	protected:
		Tank_Robot_Control m_TankRobotControl;
		HikingViking_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		double m_ArmMaxSpeed;
		Potentiometer_Tester3 m_Potentiometer; //simulate a real potentiometer for calibration testing
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
		double m_ArmVoltage,m_RollerVoltage;
		bool m_Deployment,m_Claw,m_Rist;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class HikingViking_Robot_UI : public HikingViking_Robot, public HikingViking_Robot_Control
{
	public:
		HikingViking_Robot_UI(const char EntityName[]) : HikingViking_Robot(EntityName,this),HikingViking_Robot_Control(),
			m_TankUI(this) {}
	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_TankUI.TimeChange(dTime_s);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_TankUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_TankUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_TankUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_TankUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_TankUI.UpdateScene(geode,AddOrRemove);}

	private:
		Tank_Robot_UI m_TankUI;

};

#endif

