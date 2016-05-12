#pragma once

class FRC_2013_Robot_Control : public FRC_2013_Control_Interface
{
	protected: //from Robot_Control_Interface
		FRC_2013_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		
		#ifdef __UsingTestingKit__
		Servo_Robot_Control m_TankRobotControl;  //for x-axis control
		#else
		Tank_Robot_Control m_TankRobotControl;
		#endif

		Servo m_PitchAxis,m_TurretAxis;

		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Victor m_drive_1,m_drive_2;  //explicitly specify victor speed controllers for the robot drive

		Victor m_PowerWheel_First_Victor,m_PowerWheel_Second_Victor,m_Helix_Victor;
		Victor m_IntakeMotor_Victor,m_Rollers_Victor,m_IntakeDeployment_Victor;
		//pitch ramp is using i2c
		Compressor m_Compress;
		//Note: Unfortunately the Get() accessor does not have const so these have to be mutable
		mutable DoubleSolenoid m_EngageDrive,m_EngageLiftWinch,m_EngageDropWinch,m_EngageFirePiston;
		
		Encoder2 m_IntakeDeployment_Encoder, m_PowerWheel_First_Encoder,m_PowerWheel_Second_Encoder;
		DigitalInput m_Intake_DeployedLimit;
		//AnalogChannel m_Potentiometer;
		//Cached from properties
		//double m_ArmMaxSpeed;
		
	private:
		//probably will not need these
		//KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		KalmanFilter m_PowerWheelFilter;
		Averager<double,5> m_PowerWheelAverager;
		Priority_Averager m_PowerWheel_PriorityAverager;
		double m_IntakeDeploymentOffset;
		//for best results we'll read back what we got to be smooth as they are needed to creep up the angle to position smoothly
		double m_PitchRampAngle,m_TurretAngle;
		bool m_IsDriveEngaged;
	public:
		FRC_2013_Robot_Control(bool UseSafety);
		virtual ~FRC_2013_Robot_Control();
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		FRC_2013_Control_Interface &AsControlInterface() {return *this;}

		const FRC_2013_Robot_Properties &GetRobotProps() {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void CloseSolenoid(size_t index,bool Close=true) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
		virtual bool GetIsSolenoidOpen(size_t index) const;
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		//Note: If the motors are reversed, this is now solved in LUA
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}
	protected: //from Servo Interface
		virtual void Reset_Servo(size_t index=0); 
		virtual double GetServoAngle(size_t index=0);
		virtual void SetServoAngle(size_t index,double radians);

	protected: //from FRC_2012_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
};
