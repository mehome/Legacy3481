#if 0
#pragma once
//TODO this may be omitted once we confirm encoder directions are working properly
#undef __UseOwnEncoderScalar__

class Tank_Robot_Control : public Tank_Drive_Control_Interface
{
	public:
		Tank_Robot_Control(bool UseSafety);
		virtual ~Tank_Robot_Control(); 
		void SetSafety(bool UseSafety);

		//This is only needed for simulation
		virtual void Tank_Drive_Control_TimeChange(double dTime_s);
		//double GetLeftVoltage() const {return m_LeftVoltage;}
		//double GetRightVoltage() const {return m_RightVoltage;}
		//void SetDisplayVoltage(bool display) {m_DisplayVoltage=display;}
	protected: //from Robot_Control_Interface
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		double RPS_To_LinearVelocity(double RPS);
	protected:
		
		Victor m_fl_1,m_rl_2,m_fr_3,m_rr_4;  //explicitly specify victor speed controllers for the robot drive
		RobotDrive m_RobotDrive;
		Encoder2 m_LeftEncoder,m_RightEncoder;

		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use
		
		#ifdef __UseOwnEncoderScalar__
		double m_EncoderLeftScalar, m_EncoderRightScalar;
		#endif
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		Averager<double,4> m_Averager_EncoderLeft, m_Averager_EncodeRight;
	private:
		//Used for diagnostics, but also may be used for path align information
		void InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
	public:
		double Get_dTime_s() const {return m_dTime_s;}
};
#endif
