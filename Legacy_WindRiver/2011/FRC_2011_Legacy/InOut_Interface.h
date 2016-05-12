#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

class Robot_Control : public Robot_Control_Interface
{
	public:
		Robot_Control(bool UseSafety);
		virtual ~Robot_Control(); 
		virtual void Initialize(const Entity_Properties *props);
		virtual void TimeChange(double dTime_s);
		void SetSafety(bool UseSafety);
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
	protected: //from Robot_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Reset_Arm(); 
		virtual void Reset_Encoders();
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		//This is moved to robot specific class
		//virtual void UpdateVoltage(size_t index, double Voltage);
		virtual double GetArmCurrentPosition();
		
		Victor m_1,m_2,m_3,m_4;  //explicitly specify victor speed controllers for the robot drive
		RobotDrive m_RobotDrive;
		Victor m_ArmMotor,m_RollerMotor;
		Compressor m_Compress;
		Solenoid m_OnRist,m_OffRist;
		Solenoid m_OnClaw,m_OffClaw;
		Solenoid m_OnDeploy,m_OffDeploy;
		Encoder m_LeftEncoder,m_RightEncoder;
		
		//Servo m_DeployDoor,m_LazySusan;
		AnalogChannel m_Potentiometer;
		AxisCamera *m_Camera;  //This is a singleton, but treated as a member that is optional

		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
};

class Robot_Control_2011 : public Robot_Control
{
	public:
		Robot_Control_2011(bool UseSafety) : Robot_Control(UseSafety) {}
		//See FRC_2011_Robot for enumerations
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual void CloseSolenoid(size_t index,bool Close);
};

class Driver_Station_Joystick : public Framework::Base::IJoystick
{	
	public:
		//Note: this current configuration requires the two joysticks reside in adjacent ports (e.g. 2,3)
		Driver_Station_Joystick(int NoJoysticks,int StartingPort);
		virtual ~Driver_Station_Joystick();
	protected:  //from IJoystick
		virtual size_t GetNoJoysticksFound();
		virtual bool read_joystick (size_t nr, JoyState &Info);
		
		virtual const JoystickInfo &GetJoyInfo(size_t nr) const {return m_JoyInfo[nr];}
	private:
		std::vector<JoystickInfo> m_JoyInfo;
		DriverStation *m_ds;
		int m_NoJoysticks,m_StartingPort;
};

#endif /*INOUT_INTERFACE_H_*/
