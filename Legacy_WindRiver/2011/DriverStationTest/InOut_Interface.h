#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

/**
* Standard hobby style servo.
* 
* The range parameters default to the appropriate values for the Hitec HS-322HD servo provided
* in the FIRST Kit of Parts in 2008.
*/
class Servo
{
public:
	explicit Servo(UINT32 channel) {}
	Servo(UINT32 slot, UINT32 channel) {}
	virtual ~Servo() {}
	void Set(float value) {}
	void SetOffline() {}
	float Get() {return 0.0;}
	void SetAngle(float angle);
	float GetAngle() {return 0.0;}
	static float GetMaxAngle() { return 170.0; };
	static float GetMinAngle() { return 0.0; };
};

class Compressor
{
private:
	bool m_enabled;
public:
	Compressor(UINT32 pressureSwitchChannel, UINT32 compressorRelayChannel) {}
	Compressor(UINT32 pressureSwitchSlot, UINT32 pressureSwitchChannel,UINT32 compresssorRelaySlot, UINT32 compressorRelayChannel) {}
	~Compressor() {}
	void Start() {m_enabled=true;}
	void Stop() {m_enabled=false;}
	bool Enabled() {return m_enabled;}
	UINT32 GetPressureSwitchValue() {}
	//void SetRelayValue(Relay::Value relayValue) {}
};

class AnalogChannel
{
	private:
		UINT32 m_Channel;
	public:
		AnalogChannel(UINT32 channel) : m_Channel(channel) {}
		INT32 GetAverageValue()
		{
			DriverStation *ds=DriverStation::GetInstance();
			return (INT32) ds->GetAnalogIn(m_Channel);
		}
};

class Robot_Control : public Robot_Control_Interface
{
	RobotDrive m_RobotDrive;
	RobotDrive m_ArmMotor;
	Compressor m_Compress;
	Solenoid m_OnClaw,m_OffClaw;
	Solenoid m_OnDeploy,m_OffDeploy;
	Encoder m_LeftEncoder,m_RightEncoder;
	//Servo m_DeployDoor,m_LazySusan;
	AnalogChannel m_Potentiometer;

	double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
	double m_ArmMaxSpeed;
	public:
		Robot_Control(bool UseSafety);
		virtual ~Robot_Control(); 
		virtual void Initialize(const Entity_Properties *props);
		void SetSafety(bool UseSafety);
	protected: //from Robot_Control_Interface
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		virtual void UpdateArmVoltage(double Voltage);
		virtual double GetArmCurrentPosition();
		virtual void CloseClaw(bool Close) {m_OnClaw.Set(Close),m_OffClaw.Set(!Close);}
		virtual void CloseDeploymentDoor(bool Close) {m_OnDeploy.Set(Close),m_OffDeploy.Set(!Close);}
		//virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
		//virtual void ReleaseLazySusan(bool Release) {m_LazySusan.SetAngle(Release?Servo::GetMaxAngle():Servo::GetMinAngle());}
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
