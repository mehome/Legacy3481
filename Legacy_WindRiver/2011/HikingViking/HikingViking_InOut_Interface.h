#pragma once

class HikingViking_Robot_Control : public HikingViking_Control_Interface
{
	protected: //from Robot_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		#ifdef __UsingTestingKit__
		Servo_Robot_Control m_TankRobotControl;  //for x-axis control
		#else
		Tank_Robot_Control m_TankRobotControl;
		#endif

		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
	
		Victor m_ArmMotor,m_RollerMotor;
		Compressor m_Compress;
		Solenoid m_OnRist,m_OffRist;
		Solenoid m_OnClaw,m_OffClaw;
		Solenoid m_OnDeploy,m_OffDeploy;
		
		//Servo m_DeployDoor,m_LazySusan;
		AnalogChannel m_Potentiometer;
		double m_ArmMaxSpeed;
	private:
		HikingViking_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		Averager<double,5> m_ArmAverager;
	public:
		HikingViking_Robot_Control(bool UseSafety);
		virtual ~HikingViking_Robot_Control(); 
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		HikingViking_Control_Interface &AsControlInterface() {return *this;}
	protected: //from Robot_Control_Interface
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Close) {CloseSolenoid(index,!Close);}
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
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
};

