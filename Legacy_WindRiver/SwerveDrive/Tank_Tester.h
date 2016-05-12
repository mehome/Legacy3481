#pragma once

//This will test nona drive on a tank robot
class Tank_Tester_Control : public Swerve_Drive_Control_Interface
{
	public:
		Tank_Tester_Control(bool UseSafety);
		virtual ~Tank_Tester_Control(); 
		void SetSafety(bool UseSafety);
		
		Swerve_Drive_Control_Interface &AsControlInterface() {return *this;}
	protected: //from Rotary_Control_Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);

		//from Swerve_Drive_Control_Interface
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		virtual void Reset_Encoders();

		double RPS_To_LinearVelocity(double RPS);
	protected:
		Victor m_1,m_2,m_3,m_4;  //explicitly specify victor speed controllers for the robot drive
		RobotDrive m_RobotDrive;
		Encoder2 m_LeftEncoder,m_RightEncoder;

		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use

		Swerve_Robot_Props m_SwerveRobotProps; //cached in the Initialize from specific robot
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		Averager<double,4> m_Averager_EncoderLeft, m_Averager_EncodeRight;
	public:
		double Get_dTime_s() const {return m_dTime_s;}
		//Cache the left side to implement the right side in pairs
		double m_LeftVelocity,m_RightVelocity,m_LeftVoltage;
};

class Tank_Nona_Control : public Tank_Tester_Control
{
	public:
		Tank_Nona_Control(bool UseSafety);
		virtual ~Tank_Nona_Control(); 
		
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void UpdateCompressor();
	protected: //from Rotary_Control_Interface
		//virtual double GetRotaryCurrentPorV(size_t index=0);  //no control available on robot for kicker
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);
		
		//from Swerve_Drive_Control_Interface
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);

		//from Robot_Control_Interface
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Open) {CloseSolenoid(index,!Open);}
	protected:
		Victor m_Kicker_Victor;
		Compressor m_Compress;
		Solenoid m_OnLowGear,m_OffLowGear;
		
		Butterfly_Robot_Properties m_ButterflyProps; //cache to obtain drive props
		Rotary_Props m_Kicker_Props;  //cache the rotary props for the kicker wheel
	private:
		typedef Tank_Tester_Control __super;
		int m_Compressor_FloodCount;
		bool m_CurrentCompressorState;
};
