#pragma once
#define __DisableCamera__

class FRC_2012_Robot_Control : public FRC_2012_Control_Interface
{
	protected: //from Robot_Control_Interface
		FRC_2012_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members

		Victor m_Turret_Victor,m_PowerWheel_Victor,m_Flipper_Victor;
		//pitch ramp is using i2c
		Compressor m_Compress;
		Solenoid m_OnLowGear,m_OffLowGear;
		Solenoid m_FlipperDown,m_FlipperUp;
		Solenoid m_OnRampDeployment,m_OffRampDeployment;
		Relay m_LowerConveyor_Relay,m_MiddleConveyor_Relay,m_FireConveyor_Relay;
		
		Encoder2 m_Turret_Encoder, m_PowerWheel_Encoder;
		DigitalInput m_Intake_Limit,m_Middle_Limit,m_Fire_Limit;
		DigitalOutput m_UseBreakDrive_A,m_UseBreakDrive_B;  //It does not matter which side these are on
		//TODO see if we will need limit switches for the turret or pitch
		//TODO see if we'll have pot for flippers
		//AnalogChannel m_Potentiometer;
		//Cached from properties
		//double m_ArmMaxSpeed;

		#ifndef __DisableCamera__
		FRC_2012_CameraProcessing m_Camera;
		#endif
		
	private:
		//probably will not need these
		//KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		KalmanFilter m_PowerWheelFilter;
		Averager<double,5> m_PowerWheelAverager;
		Priority_Averager m_PowerWheel_PriorityAverager;
		
		//Note these are temporary to avoid flooding, and should be removed once they are no longer needed
		double m_TurretVoltage,m_PitchRampVoltage,m_PowerWheelVoltage,m_FlipperVoltage;
		double m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage;
	public:
		FRC_2012_Robot_Control(bool UseSafety);
		virtual ~FRC_2012_Robot_Control();
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		FRC_2012_Control_Interface &AsControlInterface() {return *this;}

		const FRC_2012_Robot_Properties &GetRobotProps() {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void CloseSolenoid(size_t index,bool Close) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		//Note: If the motors are reversed, this is now solved in LUA
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
};
