#pragma once
//#ifndef __UsingTankDrive__
//#define __UsingTankDrive__
//#endif

class Curivator_Control_Interface :	
									#ifdef __UsingTankDrive__
									public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
									#else
									public Swerve_Drive_Control_Interface
									#endif
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

struct Curivator_Robot_Props
{
public:
	//TODO evaluate how this translates into the dart
	//everything in meters and radians
	double OptimalAngleUp;
	double OptimalAngleDn;
	double ArmLength;
	double ArmToGearRatio;
	double PotentiometerToArmRatio;
	double PotentiometerMaxRotation;
	double GearHeightOffset;
	double MotorToWheelGearRatio;
	bool EnableArmAutoPosition;
	struct Autonomous_Properties
	{
		size_t AutonTest;
		//void ShowAutonParameters(); //This will show SmartDashboard variables if ShowParameters is true
		bool ShowParameters;   //If true ShowAutonParameters will populate SmartDashboard with autonomous parameters
	} Autonomous_Props;
};

#ifdef __UsingTankDrive__
class Curivator_Robot_Properties : public Tank_Robot_Properties
#else
class Curivator_Robot_Properties : public Swerve_Robot_Properties
#endif
{
	public:
		Curivator_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);

		//where the index matches the enumeration of each rotary system
		const Rotary_Pot_Properties &GetRotaryProps(size_t index) const {return m_RotaryProps[index];}

		const Curivator_Robot_Props &GetCurivatorRobotProps() const {return m_CurivatorRobotProps;}
		Curivator_Robot_Props &GetCurivatorRobotProps_rw() {return m_CurivatorRobotProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
		const Control_Assignment_Properties &Get_ControlAssignmentProps() const {return m_ControlAssignmentProps;}
	private:
		#ifndef Robot_TesterCode
		#ifdef __UsingTankDrive__
		typedef Tank_Robot_Properties __super;
		#else
		typedef Swerve_Robot_Properties __super;
		#endif
		#endif
		Rotary_Pot_Properties m_RotaryProps[11];
		Rotary_Pot_Properties m_CommonRotary;   //Like with swerve... preferably would like to make this local
		Curivator_Robot_Props m_CurivatorRobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;
};

//Note: names and assignments after clasp are in the order as they are in Swerve_Robot.h Swerve_Robot_SpeedControllerDevices
//They shouldn't be used in Curivator_Robot, but are added for completeness and error checking
const char * const csz_Curivator_Robot_SpeedControllerDevices_Enum[] =
{
	"turret","arm","boom","bucket","clasp","arm_xpos","arm_ypos","bucket_angle","clasp_angle",
	"wheel_cl","wheel_cr"
};

//const char * const csz_Curivator_Robot_BoolSensorDevices_Enum[] =
//{
//	"dart_upper_limit",	"dart_lower_limit"
//};

//Note: rotary systems share the same index as their speed controller counterpart
//Note: names and assignments after clasp are in the order as they are in Swerve_Robot.h Swerve_Robot_SpeedControllerDevices
//They shouldn't be used in Curivator_Robot, but are added for completeness and error checking
const char * const csz_Curivator_Robot_AnalogInputs_Enum[] =
{
	"turret_pot","arm_pot","boom_pot","bucket_pot","clasp_pot","arm_xpos_pot","arm_ypos_pot","bucket_angle_pot","clasp_angle_pot",
	"rocker_left_enc","rocker_right_enc","bogie_left_enc","bogie_right_enc","rocker_left_pot","rocker_right_pot","bogie_left_pot","bogie_right_pot"
};

const size_t Curivator_Robot_NoArmRotarySystems=5;  //This reflects Robot_Arm count, excluding the 3D position enumerations
const size_t Curivator_Robot_NoRobotArm=9;  //This reflects Robot_Arm count, which does not include the drive speed controller devices

#ifdef __UsingTankDrive__
class Curivator_Robot : public Tank_Robot
#else
class Curivator_Robot : public Swerve_Robot
#endif
{
	public:
		enum SpeedControllerDevices
		{
			eTurret,eArm,eBoom,eBucket,eClasp,eArm_Xpos,eArm_Ypos,eBucket_Angle,eClasp_Angle,eWheel_CL,eWheel_CR,eDriveOffset
		};

		static SpeedControllerDevices GetSpeedControllerDevices_Enum (const char *value)
		{	return Enum_GetValue<SpeedControllerDevices> (value,csz_Curivator_Robot_SpeedControllerDevices_Enum,_countof(csz_Curivator_Robot_SpeedControllerDevices_Enum));
		}

		//static SolenoidDevices GetSolenoidDevices_Enum (const char *value)
		//{	return 0;
		//}

		enum BoolSensorDevices
		{
			eDartUpper,eDartLower
		};

		static BoolSensorDevices GetBoolSensorDevices_Enum (const char *value)
		{	//return Enum_GetValue<BoolSensorDevices> (value,csz_Curivator_Robot_BoolSensorDevices_Enum,_countof(csz_Curivator_Robot_BoolSensorDevices_Enum));
			return (BoolSensorDevices)0;
		}

		enum AnalogInputs
		{
			eTurretPot,eArmPot,eBoomPot,eBucketPot,eClaspPot,eArmXposPot,eArmYposPot,eBucketAngle,eClaspAngle
		};

		static AnalogInputs GetAnalogInputs_Enum (const char *value)
		{	return Enum_GetValue<AnalogInputs> (value,csz_Curivator_Robot_AnalogInputs_Enum,_countof(csz_Curivator_Robot_AnalogInputs_Enum));
		}

		Curivator_Robot(const char EntityName[],Curivator_Control_Interface *robot_control,bool IsAutonomous=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);
		void FreezeArm(bool isOn) {m_FreezeArm=isOn;}
		void LockPosition(bool isOn) {m_LockPosition=isOn;}
		void StopAuton(bool isOn);  //just like freeze arm but puts things back in teleop
		//simple computation between the target angle against the actual angle measured
		double GetBucketAngleContinuity();

	protected:
		class Turret
		{
			private:
				Curivator_Robot * const m_pParent;
				double m_Velocity; //adds all axis velocities then assigns on the time change
			public:
				Turret(Curivator_Robot *parent,Rotary_Control_Interface *robot_control);
				virtual ~Turret() {}
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void ResetPos();
				double GetCurrentVelocity() const {return m_Velocity;}
				virtual void TimeChange(double dTime_s);
			protected:
				void Turret_SetRequestedVelocity(double Velocity) {m_Velocity+=Velocity;}
		};

	public: //Autonomous public access (wind river has problems with friend technique)
		class Robot_Arm : public Rotary_Position_Control
		{
			public:
				Robot_Arm(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
			protected:
				void SetIntendedPosition_Plus(double Position);
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void BindAdditionalEventControls(bool Bind);
				void Advance(bool on);
				void Retract(bool on);
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);

				//override from rotary system... will implicitly manage limit switch support
				virtual bool DidHitMinLimit() const;
				virtual bool DidHitMaxLimit() const;
			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				const size_t m_Index;
				Curivator_Robot * const m_pParent;
				double m_LastIntendedPosition;
				bool m_Advance, m_Retract;
		};

		class BigArm : public Robot_Arm
		{
			public:
				BigArm(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control);
				double GetBigArmLength() const;
				double GetBigArmHeight() const;
				double GetBigArmAngle() const {return m_BigArmAngle;}
			protected:
				virtual void TimeChange(double dTime_s);
			private:
				double m_BigArmAngle;
				#ifndef Robot_TesterCode
				typedef Robot_Arm __super;
				#endif
		};

		class Boom : public Robot_Arm
		{
			public:
				Boom(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control, BigArm &bigarm);
				double GetBoomLength() const;
				double GetBoomHeight() const;
				double GetBoomAngle() const {return m_BoomAngle;}
			protected:
				virtual void TimeChange(double dTime_s);
			private:
				BigArm &m_BigArm;
				double m_BoomAngle;
				#ifndef Robot_TesterCode
				typedef Robot_Arm __super;
				#endif
		};

		class Bucket : public Robot_Arm
		{
			public:
				Bucket(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control, Boom &boom);
				double GetBucketLength() const;
				double GetBucketTipHeight() const {return m_GlobalTipHeight;}
				double GetCoMHeight() const;
				double GetCoMDistance() const;
				double GetBucketRoundEndHeight() const;
				double GetBucketAngle() const;
				//Get the global height of the bucket rocker to bucket pivot point (3 inch separate holes rotated globally)
				double GetBucket_globalBRP_BP_height() const {return m_Bucket_globalBRP_BP_height;}
				double GetBucket_globalBRP_BP_distance() const {return m_Bucket_globalBRP_BP_distance;}
				Boom &GetBoom() {return m_Boom;}  //avoid need of clasp having to aggregate a boom member variable
			protected:
				virtual void TimeChange(double dTime_s);
			private:
				Boom &m_Boom;
				double m_GlobalCoMHeight,m_GlobalCoMDistance;
				double m_GlobalTipHeight;
				double m_LocalBucketAngle;
				double m_GlobalDistance;

				//clasp uses these as well
				double m_Bucket_globalBRP_BP_height;
				double m_Bucket_globalBRP_BP_distance;
				#ifndef Robot_TesterCode
				typedef Robot_Arm __super;
				#endif
		};

		class Clasp : public Robot_Arm
		{
			public:
				Clasp(size_t index,Curivator_Robot *parent,Rotary_Control_Interface *robot_control, Bucket &bucket);
				double GetClaspLength() const {return m_GlobalMidlineDistance;}
				double GetClaspMidlineHeight() const {return m_GlobalMidlineHeight;}
				double GetMinHeight() const;
				double GetClaspAngle() const {return m_GlobalClaspAngle;}
				double GetInnerTipHieght() const;  //tip closest to bucket
				double GetOuterTipHieght() const;  //tip furthest from bucket
			protected:
				virtual void TimeChange(double dTime_s);
			private:
				Bucket &m_Bucket;
				double m_GlobalMidlineHeight;
				double m_GlobalMidlineDistance;
				double m_GlobalClaspAngle;
				double m_Clasp_MidlineToEdge_Angle_Horizontal;
				#ifndef Robot_TesterCode
				typedef Robot_Arm __super;
				#endif
		};

		const Curivator_Robot_Properties &GetRobotProps() const;
		Curivator_Robot_Props::Autonomous_Properties &GetAutonProps();
		//Accessors needed for setting goals
		Robot_Arm &GetTurret() {return m_Turret;}
		Robot_Arm &GetArmXpos() {return m_ArmXpos;}
		Robot_Arm &GetArmYpos() {return m_ArmYpos;}
		Robot_Arm &GetBucketAngle() {return m_BucketAngle;}
		Robot_Arm &GetClaspAngle() {return m_ClaspAngle;}
		///Probably one of the most important computations... given a desired position and angle of the bucket, will provide the computations
		///for each linear actuator.  Note the following is based from the point of origin which is where the big arm pivots.
		/// \param GlobalHeight height positive number is above origin... negative is below (down)
		/// \note The height is based from the lowest point of bucket... whether it is the tip or the round end... if necessary
		/// we can have more control over which to use.
		/// \param GlobalDistance from origin to tip of bucket (always positive)
		/// \param BucketAngle  From buckets edge of intake to horizontal plane (e.g. floor)
		/// \param ClaspOpenAngle angle between bucket edge and clasp edge... 0 is not quite shut so using negative (e.g. -7 degrees) 
		void ComputeArmPosition(double GlobalHeight,double GlobalDistance,double BucketAngle_deg,double ClaspOpeningAngle_deg,
			double &BigArm_ShaftLength,double &Boom_ShaftLength,double &BucketShaftLength,double &ClaspShaftLength);
	protected:
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);
		//used to blend turret and pitch controls into the drive itself
		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s);
	private:
		#ifndef Robot_TesterCode
		typedef  Swerve_Robot __super;
		#endif
		Curivator_Control_Interface * const m_RobotControl;
		Robot_Arm m_Turret;
	protected:
		BigArm m_Arm;
		Boom m_Boom;
		Bucket m_Bucket;
		Clasp m_Clasp;
		Robot_Arm m_ArmXpos;
		Robot_Arm m_ArmYpos;
		Robot_Arm m_BucketAngle;
	private:
		Robot_Arm m_ClaspAngle;
		Robot_Arm *mp_Arm[Curivator_Robot_NoRobotArm];  //A handy work-around to treat these as an array, by pointing to them

		Rotary_Velocity_Control m_CenterLeftWheel,m_CenterRightWheel;
		Curivator_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		double m_LatencyCounter;

		double m_Last_xpos;
		double m_Last_ypos;
		double m_Last_bucket_angle;
		double m_Last_clasp_angle;

		double m_YawErrorCorrection,m_PowerErrorCorrection;
		size_t m_DefaultPresetIndex;
		size_t m_AutonPresetIndex;  //used only because encoder tracking is disabled
		bool m_FreezeArm;  //used mostly during calibration to have ability to stop movement, also to freeze to a way point in auton
		bool m_LockPosition; //used to allow a position to remain locked to allow arm to catch up and stabilize

		#ifdef Robot_TesterCode
		void TestAutonomous();
		void GoalComplete();
		#endif
};

namespace Curivator_Goals
{
	Goal *Get_Curivator_Autonomous(Curivator_Robot *Robot);
};

class Curivator_Robot_Control : public RobotControlCommon, public Curivator_Control_Interface
{
	public:
		Curivator_Robot_Control(bool UseSafety=true);
		virtual ~Curivator_Robot_Control();

		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		#ifndef Robot_TesterCode
		//Note: Swerve drive is not using drive
		//void SetSafety(bool UseSafety) {m_DriveRobotControl.SetSafety(UseSafety);}
		#endif

		Curivator_Control_Interface &AsControlInterface() {return *this;}

		const Curivator_Robot_Properties &GetRobotProps() const {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		//virtual bool GetBoolSensorState(size_t index) const;
		virtual void CloseSolenoid(size_t index,bool Close) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
	#ifdef __UsingTankDrive__
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pDriveRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pDriveRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pDriveRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	#else
	protected: //from Swerve_Drive_Control_Interface
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s) {m_pDriveRobotControl->Swerve_Drive_Control_TimeChange(dTime_s);}
		virtual void Reset_Encoders() {m_pDriveRobotControl->Reset_Encoders();}
	#endif
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}
	protected: //from RobotControlCommon
		virtual size_t RobotControlCommon_Get_Victor_EnumValue(const char *name) const
		{	return Curivator_Robot::GetSpeedControllerDevices_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_DigitalInput_EnumValue(const char *name) const  
		{	return Curivator_Robot::GetBoolSensorDevices_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_AnalogInput_EnumValue(const char *name) const  
		{	return Curivator_Robot::GetAnalogInputs_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_DoubleSolenoid_EnumValue(const char *name) const  
		{	//return Curivator_Robot::GetSolenoidDevices_Enum(name);
			return 0;
		}
	protected: //from Curivator_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		#ifdef Robot_TesterCode
		virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl);
		#endif

	protected:
		Curivator_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		#ifdef __UsingTankDrive__
		Tank_Robot_Control m_DriveRobotControl;
		Tank_Drive_Control_Interface * const m_pDriveRobotControl;  //This allows access to protected members
		#else
		Swerve_Robot_Control m_DriveRobotControl;
		Swerve_Drive_Control_Interface * const m_pDriveRobotControl;  //This allows access to protected members
		#endif
		Compressor *m_Compressor;
		Accelerometer *m_RoboRIO_Accelerometer;
		//All digital input reads are done on time change and cached to avoid multiple reads to the FPGA
		bool m_Limit_IntakeMin1,m_Limit_IntakeMin2,m_Limit_IntakeMax1,m_Limit_IntakeMax2;
	private:
		__inline double Pot_GetRawValue(size_t index);

		KalmanFilter m_KalFilter[12];
		Averager<double,5> m_Averager[12];
		#ifdef Robot_TesterCode
		Potentiometer_Tester2 m_Potentiometer[10]; //simulate a real potentiometer for calibration testing
		Encoder_Simulator2 m_Encoders[2];  //for the center wheels
		#endif
};

#ifdef Robot_TesterCode
///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Curivator_Robot_UI : public Curivator_Robot, public Curivator_Robot_Control
{
	public:
		Curivator_Robot_UI(const char EntityName[]);
	protected:
		virtual void TimeChange(double dTime_s);
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos); 
		virtual void Text_SizeToUse(double SizeToUse);
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

	private:
		class LinesUpdate : public osg::Drawable::UpdateCallback
		{
		public:
			LinesUpdate(Curivator_Robot_UI *parent) : m_pParent(parent) {}
		protected:
			virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw);
		private:
			Curivator_Robot_UI *m_pParent;
		} *m_LinesUpdate;
		Actor_Text *m_UI_Parent;
		#ifdef __UsingTankDrive__
		Tank_Robot_UI m_DriveUI;
		#else
		Swerve_Robot_UI m_DriveUI;
		#endif
		osg::ref_ptr<osg::Vec3Array> m_VertexData;
		osg::ref_ptr<osg::Vec4Array> m_ColorData;
		osg::ref_ptr<osg::Geometry> m_Circle,m_Goal;
		osg::ref_ptr<osg::PositionAttitudeTransform> m_CircleTransform,m_ArmTransform,m_GoalTransform;
};
#endif //Robot_TesterCode
