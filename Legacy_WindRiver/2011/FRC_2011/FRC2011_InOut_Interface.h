#pragma once

class FRC_2011_Robot_Control : public FRC_2011_Control_Interface
{
	protected: //from Robot_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
	
		Victor m_ArmMotor,m_RollerMotor;
		Compressor m_Compress;
		Solenoid m_OnRist,m_OffRist;
		Solenoid m_OnClaw,m_OffClaw;
		Solenoid m_OnDeploy,m_OffDeploy;
		
		//Servo m_DeployDoor,m_LazySusan;
		AnalogChannel m_Potentiometer;
		#ifdef  __2011_TestCamera__
		FRC_2012_CameraProcessing m_Camera;
		#else
		AxisCamera *m_Camera;
		#endif
		
		double m_ArmMaxSpeed;
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;

	public:
		FRC_2011_Robot_Control(bool UseSafety);
		virtual ~FRC_2011_Robot_Control(); 
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		FRC_2011_Control_Interface &AsControlInterface() {return *this;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Close) {CloseSolenoid(index,!Close);}
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Arm Interface
		virtual void Reset_Arm(size_t index=0); 
		virtual void UpdateArmVoltage(size_t index,double Voltage) {UpdateVoltage(FRC_2011_Robot::eArm,Voltage);}
		//pacify this by returning its current value
		virtual double GetArmCurrentPosition(size_t index);
		virtual void CloseRist(bool Close) {CloseSolenoid(FRC_2011_Robot::eRist,Close);}
		virtual void OpenRist(bool Close) {CloseSolenoid(FRC_2011_Robot::eRist,!Close);}
	protected: //from FRC_2011_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
};

