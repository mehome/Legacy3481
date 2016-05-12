#pragma once

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class FRC_2011_Robot : public Robot_Tank
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);
		static double RPS_To_LinearVelocity(double RPS);

		class Robot_Arm : public Ship_1D
		{
			public:
				Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				static double HeightToAngle_r(double Height_m);
				static double Arm_AngleToHeight_m(double Angle_r);
				static double AngleToHeight_m(double Angle_r);
				//given the raw potentiometer converts to the arm angle
				static double PotentiometerRaw_To_Arm_r(double raw);
				void CloseClaw(bool Close);
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				//typedef Ship_1D __super;
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetPos0feet();
				void SetPos3feet();
				void SetPos6feet();
				void SetPos9feet();
				Robot_Control_Interface * const m_RobotControl;
				double m_LastNormalizedVelocity;  //this is managed direct from being set to avoid need for precision tolerance
				PIDController2 m_PIDController;
				double m_LastPosition;  //used for calibration
				double m_CalibratedScaler; //used for calibration
				double m_LastTime; //used for calibration
				double m_MaxSpeedReference; //used for calibration
		};

		//Accessor needed for setting goals
		Robot_Arm &GetArm() {return m_Arm;}
	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		void CloseDeploymentDoor(bool Close);
		//void ReleaseLazySusan(bool Release);

		//typedef  Robot_Tank __super;
		Robot_Control_Interface * const m_RobotControl;
		Robot_Arm m_Arm;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration
		bool m_UsingEncoders;
};

class FRC_2011_Robot_Properties : public Ship_Properties
{
	public:
		FRC_2011_Robot_Properties();
		//I'm not going to implement script support mainly due to lack of time, but also this is a specific object that
		//most likely is not going to be sub-classed (i.e. sealed)... if this turns out different later we can implement
		//virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Ship_1D_Properties &GetArmProps() const {return m_ArmProps;}
	private:
		Ship_1D_Properties m_ArmProps;
};

class Goal_OperateClaw : public AtomicGoal
{
	private:
		FRC_2011_Robot &m_Robot;
		bool m_Terminate;
		bool m_IsClosed;
	public:
		Goal_OperateClaw(FRC_2011_Robot &robot,bool Close) : m_Robot(robot),m_Terminate(false),m_IsClosed(Close) 
		{	m_Status=eInactive;
		}
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s)
		{
			if (m_Terminate)
			{
				if (m_Status==eActive)
					m_Status=eFailed;
				return m_Status;
			}
			ActivateIfInactive();
			m_Robot.GetArm().CloseClaw(m_IsClosed);
			m_Status=eCompleted;
			return m_Status;
		}
		virtual void Terminate() {m_Terminate=true;}
};
