#include "WPILib.h"

#include "Base/src/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/src/Vec2d.h"
#include "Base/src/Misc.h"
#include "Base/src/Event.h"
#include "Base/src/EventMap.h"
#include "Base/src/Script.h"
#include "Common/src/Entity_Properties.h"
#include "Common/src/Physics_1D.h"
#include "Common/src/Physics_2D.h"
#include "Common/src/Entity2D.h"
#include "Common/src/Goal.h"
#include "Common/src/Ship_1D.h"
#include "Common/src/Ship.h"
#include "Common/src/AI_Base_Controller.h"
#include "Common/src/Vehicle_Drive.h"
#include "Common/src/PIDController.h"
#include "Common/src/Poly.h"
#include "Common/src/Robot_Control_Interface.h"
#include "Common/src/Rotary_System.h"

#include "Base/src/Joystick.h"
#include "Base/src/JoystickBinder.h"
#include "Common/src/UI_Controller.h"
#include "Common/src/InOut_Interface.h"
#include "Common/src/Debug.h"
#include "Common/src/Robot_Control_Common.h"
#include "Swerve_Robot.h"

#include "SmartDashboard/SmartDashboard.h"


#ifdef Robot_TesterCode
using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi2=M_PI*2.0;
const double Pi=M_PI;
#else
using namespace Framework::Base;
using namespace std;
#endif



  /***********************************************************************************************************************************/
 /*													Swerve_Robot::DrivingModule														*/
/***********************************************************************************************************************************/

Swerve_Robot::DrivingModule::DrivingModule(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,size_t SectionOrder) : m_ModuleName(EntityName),
	m_SwivelName("Swivel"),m_DriveName("Drive"),
	m_Swivel(m_SwivelName.c_str(),robot_control,SectionOrder+4),m_Drive(m_DriveName.c_str(),robot_control,SectionOrder),
	m_IntendedSwivelDirection(0.0),m_IntendedDriveVelocity(0.0),
	m_RobotControl(robot_control)
{
}

void Swerve_Robot::DrivingModule::Initialize(Entity2D_Kind::EventMap& em,const DrivingModule_Props *props)
{
	m_Swivel.Initialize(em,props->Swivel_Props);
	m_Drive.Initialize(em,props->Drive_Props);
}

void Swerve_Robot::DrivingModule::TimeChange(double dTime_s)
{
	//manage the swivel angle and drive velocity
	m_Swivel.SetIntendedPosition(m_IntendedSwivelDirection);
	m_Drive.SetRequestedVelocity(m_IntendedDriveVelocity);

	//TODO determine why this sticks past 1.5
	//m_Swivel.SetMatchVelocity(m_IntendedSwivelDirection);
	//m_Drive.SetMatchVelocity(m_IntendedDriveVelocity);

	//Update the swivel and drive times
	m_Swivel.AsEntity1D().TimeChange(dTime_s);
	m_Drive.AsEntity1D().TimeChange(dTime_s);
}

  /***********************************************************************************************************************************/
 /*															Swerve_Robot															*/
/***********************************************************************************************************************************/
Swerve_Robot::Swerve_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,size_t EnumOffset,bool IsAutonomous) : 
	Ship_Tester(EntityName), m_RobotControl(robot_control), m_IsAutonomous(IsAutonomous), m_VehicleDrive(NULL),
	m_UsingEncoders(IsAutonomous), //,m_VoltageOverride(false),m_UseDeadZoneSkip(true)
	m_EncoderAngularVelocity(0.0),
	m_Heading(0.0), m_HeadingUpdateTimer(0.0),m_TankSteering(this),
	m_RotaryEnumOffset(EnumOffset)
{
	m_Physics.SetHeadingToUse(&m_Heading);  //We manage the heading
	const char * const ModuleName[]=
	{
		"ModuleLF","ModuleRF","ModuleLR","ModuleRR"
	};
	for (size_t i=0;i<4;i++)
		m_DrivingModule[i]=new DrivingModule(ModuleName[i],m_RobotControl,i+EnumOffset);
}

void Swerve_Robot::DestroyDrive() 
{
	delete m_VehicleDrive;
	m_VehicleDrive=NULL;
}

Swerve_Robot::~Swerve_Robot()
{
	for (size_t i=0;i<4;i++)
	{
		delete m_DrivingModule[i];
		m_DrivingModule[i]=NULL;
	}
	DestroyDrive();
}

void Swerve_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	m_VehicleDrive=CreateDrive();
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const Swerve_Robot_Properties *RobotProps=dynamic_cast<const Swerve_Robot_Properties *>(props);
	if (RobotProps)
	{
		//This will copy all the props
		m_SwerveRobotProps=RobotProps->GetSwerveRobotProps();
		m_SwerveProperties=*RobotProps;
		m_WheelDimensions=RobotProps->GetWheelDimensions();
		//depreciated
		//m_TankSteering.SetStraightDeadZone_Tolerance(RobotProps->GetSwerveRobotProps().TankSteering_Tolerance);
		for (size_t i=0;i<4;i++)
		{
			DrivingModule::DrivingModule_Props props;
			Rotary_Properties drive=RobotProps->GetRotaryProps(i);
			Rotary_Properties swivel=RobotProps->GetRotaryProps(i+eSwivel_FL);
			props.Swivel_Props=&swivel;
			props.Drive_Props=&drive;
			//Now to copy the custom per section properties over for this element
			swivel.RotaryProps().UseAggressiveStop=true;
			#ifdef Robot_TesterCode
			drive.EncoderSimulationProps()=RobotProps->GetEncoderSimulationProps();
			#endif
			//TODO drive.RotaryProps().EncoderReversed
			m_DrivingModule[i]->Initialize(em,&props);
		}

		//This can be dynamically called so we always call it
		SetUseEncoders(!m_SwerveRobotProps.IsOpen_Wheel);
	}
}

void Swerve_Robot::UpdateDriveProps(const Rotary_Props &DriveProps,const Ship_1D_Props &ShipProps,size_t index)
{
	m_DrivingModule[index]->Drive().UpdateShip1DProperties(ShipProps); //this needs to be first
	m_DrivingModule[index]->Drive().UpdateRotaryProps(DriveProps);
}

void Swerve_Robot::ResetPos()
{
	m_Heading=0.0;
	m_VehicleDrive->ResetPos();
	__super::ResetPos();
	m_RobotControl->Reset_Encoders();
	for (size_t i=0;i<4;i++)
	{
		m_DrivingModule[i]->ResetPos();
		m_Swerve_Robot_Velocities.Velocity.AsArray[i+4]=0.0;
		m_Swerve_Robot_Velocities.Velocity.AsArray[i]=0.0;

	}
}


void Swerve_Robot::SetUseEncoders(bool UseEncoders,bool ResetPosition) 
{
	if (!UseEncoders)
	{
		if (m_UsingEncoders)
		{
			//first disable it
			m_UsingEncoders=false;
			printf("Disabling encoders for %s\n",GetName().c_str());
			//Now to reset stuff
			//Reset(ResetPosition);
			ResetPos();
			m_EncoderGlobalVelocity=Vec2d(0.0,0.0);
		}
	}
	else
	{
		if (!m_UsingEncoders)
		{
			m_UsingEncoders=true;
			printf("Enabling encoders for %s\n",GetName().c_str());
			//setup the initial value with the encoders value
			//Reset(ResetPosition);
			ResetPos();
		}
	}
}

void Swerve_Robot::SetIsAutonomous(bool IsAutonomous)
{
	m_IsAutonomous=IsAutonomous;  //this is important (to disable joystick controls etc)
	//We only explicitly turn them on... not off (that will be configured else where)
	if (IsAutonomous)
		SetUseEncoders(true);
}

void Swerve_Robot::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	SwerveVelocities encoders;
	if (m_SwerveRobotProps.IsOpen_Wheel)
	{
		encoders.Velocity.Named.sFL=m_Swerve_Robot_Velocities.Velocity.Named.sFL;
		encoders.Velocity.Named.sFR=m_Swerve_Robot_Velocities.Velocity.Named.sFR;
		encoders.Velocity.Named.sRL=m_Swerve_Robot_Velocities.Velocity.Named.sRL;
		encoders.Velocity.Named.sRR=m_Swerve_Robot_Velocities.Velocity.Named.sRR;
	}
	else
	{
		encoders.Velocity.Named.sFL=m_RobotControl->GetRotaryCurrentPorV(eWheel_FL+m_RotaryEnumOffset);
		encoders.Velocity.Named.sFR=m_RobotControl->GetRotaryCurrentPorV(eWheel_FR+m_RotaryEnumOffset);
		encoders.Velocity.Named.sRL=m_RobotControl->GetRotaryCurrentPorV(eWheel_RL+m_RotaryEnumOffset);
		encoders.Velocity.Named.sRR=m_RobotControl->GetRotaryCurrentPorV(eWheel_RR+m_RotaryEnumOffset);
	}

	if (m_SwerveRobotProps.IsOpen_Swivel)
	{
		encoders.Velocity.Named.aFL=m_Swerve_Robot_Velocities.Velocity.Named.aFL;
		encoders.Velocity.Named.aFR=m_Swerve_Robot_Velocities.Velocity.Named.aFR;
		encoders.Velocity.Named.aRL=m_Swerve_Robot_Velocities.Velocity.Named.aRL;
		encoders.Velocity.Named.aRR=m_Swerve_Robot_Velocities.Velocity.Named.aRR;
	}
	else
	{
		encoders.Velocity.Named.aFL=m_RobotControl->GetRotaryCurrentPorV(eSwivel_FL+m_RotaryEnumOffset);
		encoders.Velocity.Named.aFR=m_RobotControl->GetRotaryCurrentPorV(eSwivel_FR+m_RotaryEnumOffset);
		encoders.Velocity.Named.aRL=m_RobotControl->GetRotaryCurrentPorV(eSwivel_RL+m_RotaryEnumOffset);
		encoders.Velocity.Named.aRR=m_RobotControl->GetRotaryCurrentPorV(eSwivel_RR+m_RotaryEnumOffset);
	}

	double aSwivelDirection[4];
	double VelocityToUse[4];
	double aDistanceToIntendedSwivel[4];
	bool RestrictMotion=false;  //flagged if any of the wheels do not reach a threshold of their position
	//Now the new UpdateVelocities was just called... work with these intended velocities
	for (size_t i=0;i<4;i++)
	{
		const double IntendedDirection=m_VehicleDrive->GetIntendedVelocitiesFromIndex(i+4);
		double SwivelDirection=IntendedDirection;  //this is either the intended direction or the reverse of it
		const Ship_1D &Swivel=m_DrivingModule[i]->GetSwivel();

		//This is normalized implicitly
		//const double LastSwivelDirection=Swivel.GetPos_m();
		const double LastSwivelDirection=encoders.Velocity.AsArray[i+4];

		double DirectionMultiplier=1.0; //changes to -1 when reversed
		//Anything above 180 will need to be flipped favorably to the least traveled angle
		if (fabs(SwivelDirection)>PI_2)
		{
			const double TestOtherDirection=NormalizeRotation_HalfPi(SwivelDirection);
			if (fabs(TestOtherDirection)<fabs(SwivelDirection))
			{
				SwivelDirection=TestOtherDirection;
				DirectionMultiplier=-1;
			}
		}

		//if we are using range... clip to the max range available
		if (Swivel.GetUsingRange())
		{
			if (SwivelDirection>Swivel.GetMaxRange())
				SwivelDirection=Swivel.GetMaxRange();
			else if (SwivelDirection<Swivel.GetMinRange())
				SwivelDirection=Swivel.GetMinRange();
		}

		aSwivelDirection[i]=SwivelDirection;

		//recompute as SwivelDirection may be reduced
		const double DistanceToIntendedSwivel=fabs(NormalizeRotation2(LastSwivelDirection-SwivelDirection));
		aDistanceToIntendedSwivel[i]=DistanceToIntendedSwivel;  //export to other loop

		//Note the velocity is checked once before the time change here, and once after for the current
		//Only apply swivel adjustments if we have significant movement (this matters in targeting tests)
		//if ((fabs(LocalForce[0])>1.5)||(fabs(LocalForce[1])>1.5)||(fabs(m_DrivingModule[i]->GetDrive().GetPhysics().GetVelocity()) > 0.05))
		//This logic fails when driving a direct strafe from a stop

		const double IntendedSpeed=m_VehicleDrive->GetIntendedVelocitiesFromIndex(i)*DirectionMultiplier;

		//To minimize error only apply the Y component amount to the velocity
		//The less the difference between the current and actual swivel direction the greater the full amount can be applied
		//On slower swerve drive mechanisms a check for a threshold of distance is included to minimize skid
		VelocityToUse[i]=sin(DistanceToIntendedSwivel)<DEG_2_RAD(25.0)?cos(DistanceToIntendedSwivel)*IntendedSpeed:0.0;
		if (sin(DistanceToIntendedSwivel)>=DEG_2_RAD(25.0))
			RestrictMotion=true;
		#if 0
		string sm_name="a2_";
		const char * const sm_name_suffix[]={"FL","FR","RL","RR"};
		sm_name+=sm_name_suffix[i];
		SmartDashboard::PutNumber(sm_name.c_str(),RAD_2_DEG(SwivelDirection));
		if (i==0)
		{
			SmartDashboard::PutNumber("TestPredicted",RAD_2_DEG(LastSwivelDirection));
			SmartDashboard::PutNumber("TestActual",RAD_2_DEG(encoders.Velocity.AsArray[i+4]));
			SmartDashboard::PutNumber("TestDistance",RAD_2_DEG(sin(DistanceToIntendedSwivel)));
			SmartDashboard::PutNumber("TestVTU",Meters2Feet(VelocityToUse));
		}
		#endif
	}

	//Note: This is more conservative and is favorable for autonomous navigation, but may not be favorable for teleop as it restricts movement
	if (RestrictMotion)
		for (size_t i=0;i<4;i++)
			VelocityToUse[i]=0.0;

	for (size_t i=0;i<4;i++)
	{
		const Ship_1D &Swivel=m_DrivingModule[i]->GetSwivel();
		const double IntendedDirection=m_VehicleDrive->GetIntendedVelocitiesFromIndex(i+4);
		m_DrivingModule[i]->SetIntendedSwivelDirection(aSwivelDirection[i]);
		#ifdef __DebugLUA__
		if (m_SwerveProperties.GetRotaryProps(i).GetRotaryProps().PID_Console_Dump && (m_RobotControl->GetRotaryCurrentPorV(i)!=0.0))
		{
			double PosY=GetPos_m()[1];
			printf("y=%.2f ",PosY);
		}
		#endif

		m_DrivingModule[i]->SetIntendedDriveVelocity(VelocityToUse[i]);
		m_DrivingModule[i]->TimeChange(dTime_s);

		const double CurrentVelocity=m_DrivingModule[i]->GetDrive().GetPhysics().GetVelocity();
		const double CurrentSwivelDirection=Swivel.GetPos_m();
		//if (i==0)
		//	DOUT4("S= %f %f V= %f %f",CurrentSwivelDirection,SwivelDirection,CurrentVelocity,VelocityToUse);

		//Now to grab and update the actual swerve velocities
		//Note: using GetIntendedVelocities() is a lesser stress for debug purposes
		#if 1
		//This is kind of a hack, but since there a threshold on the angular distance tolerance... for open loop we can evaluate when the intended direction is straight ahead and
		//if the current position distance is substantially small then lock it to zero.  This is no impact on closed loop which can solve this using 'I' in PID, and has no impact
		//on Nona drive
		//  [9/9/2012 Terminator]
		m_Swerve_Robot_Velocities.Velocity.AsArray[i+4]=CurrentSwivelDirection;
		if ((m_SwerveRobotProps.IsOpen_Swivel) && (IntendedDirection==0.0) && (aDistanceToIntendedSwivel[i]<0.005))
			m_Swerve_Robot_Velocities.Velocity.AsArray[i+4]=0.0;
		m_Swerve_Robot_Velocities.Velocity.AsArray[i]=CurrentVelocity;
		#else
		m_Swerve_Robot_Velocities=GetIntendedVelocities();
		#endif
	}

	{
		Vec2d LocalVelocity;
		double AngularVelocity;
		m_VehicleDrive->InterpolateVelocities(encoders,LocalVelocity,AngularVelocity,dTime_s);
		//TODO add gyro's yaw readings for Angular velocity here
		//Store the value here to be picked up in GetOldVelocity()
		m_EncoderGlobalVelocity=LocalToGlobal(GetAtt_r(),LocalVelocity);
		m_EncoderAngularVelocity=AngularVelocity;
		if (!m_SwerveRobotProps.IsOpen_Wheel)
		{
			//Vec2d Velocity=GetLinearVelocity_ToDisplay();
			SmartDashboard::PutNumber("Velocity",Meters2Feet(LocalVelocity[1]));
			SmartDashboard::PutNumber("Rotation Velocity",AngularVelocity);
		}
	}

	m_VehicleDrive->InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Swerve_Robot::TimeChange(double dTime_s)
{
	m_RobotControl->Swerve_Drive_Control_TimeChange(dTime_s);

	__super::TimeChange(dTime_s);
	#if 1
	//For open loop we'll use the internal methods to account for velocities
	if (m_SwerveRobotProps.IsOpen_Wheel)
	{
		Vec2d Velocity=GetLinearVelocity_ToDisplay();
		SmartDashboard::PutNumber("Velocity",Meters2Feet(Velocity[1]));
		SmartDashboard::PutNumber("Rotation Velocity",GetAngularVelocity_ToDisplay());
	}
	#endif
}

bool Swerve_Robot::InjectDisplacement(double DeltaTime_s,Vec2d &PositionDisplacement,double &RotationDisplacement)
{
	bool ret=false;
	//Note: for now there is no passive setting, which would be great for open loop driving while maintaining the position as it was for rebound rumble
	//Instead we can keep the logic simple and only apply displacement if we are using the encoders... this way the simulations of the open loop (lesser stress)
	//will work properly without adding this extra complexity
	//  [8/27/2012 Terminator]
	if (m_UsingEncoders)
	{
		Vec2D EncoderGlobalVelocity=m_EncoderGlobalVelocity;
		double EncoderAngularVelocity=m_EncoderAngularVelocity;
		Vec2d computedVelocity=m_Physics.GetLinearVelocity();
		double computedAngularVelocity=m_Physics.GetAngularVelocity();
		m_Physics.SetLinearVelocity(EncoderGlobalVelocity);
		m_Physics.SetAngularVelocity(EncoderAngularVelocity);
		m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
		const double &HeadingLatency=m_SwerveRobotProps.HeadingLatency;
		if (HeadingLatency!=0.0)
		{	//manage the heading update... 
			m_HeadingUpdateTimer+=DeltaTime_s;
			if (m_HeadingUpdateTimer>HeadingLatency)
			{
				m_HeadingUpdateTimer-=HeadingLatency;
				//This should never happen unless we had a huge delta time (e.g. breakpoint)
				if (m_HeadingUpdateTimer>HeadingLatency)
				{
					m_HeadingUpdateTimer=0.0; //Just reset... it is not critical to have exact interval
					printf("Warning: m_HeadingUpdateTimer>m_TankRobotProps.InputLatency\n");
				}
				//Sync up with our entities heading on this latency interval
				m_Heading=GetAtt_r();
			}
			else
			{
				//Use our original angular velocity for the heading update to avoid having to make corrections
				m_Heading+=computedAngularVelocity*DeltaTime_s;
			}
		}
		else
		{
			m_Heading+=RotationDisplacement;  // else always pull heading from the injected displacement (always in sync with entity)
		}
		//We must set this back so that the PID can compute the entire error
		m_Physics.SetLinearVelocity(computedVelocity);
		m_Physics.SetAngularVelocity(computedAngularVelocity);
		ret=true;
	}
	else
		m_Heading=GetAtt_r();
	if (!ret)
		ret=m_VehicleDrive->InjectDisplacement(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	return ret;
}


void Swerve_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	m_VehicleDrive->UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
}

void Swerve_Robot::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	m_VehicleDrive->ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	//We are not going to use these interpolated values in the control (it would corrupt it)... however we can monitor them here, or choose to
	//view them here as needed
	Vec2D force;
	double torque;
	InterpolateThrusterChanges(force,torque,dTime_s);
	__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
}

void Swerve_Robot::SetAttitude(double radians)
{
	m_Heading=radians;
	__super::SetAttitude(radians);
}

  /***********************************************************************************************************************************/
 /*														Swerve_Robot_Properties														*/
/***********************************************************************************************************************************/

Swerve_Robot_Properties::Swerve_Robot_Properties() 
{
	Swerve_Robot_Props props;
	memset(&props,0,sizeof(Swerve_Robot_Props));

	//Late assign this to override the initial default
	props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side (can only go 390 degrees a second)
	//props.WheelDimensions=Vec2D(0.3758,0.53);  //This is trimmed down to turn 514 degrees a second with a 2.914 speed

	//const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);  //We may limit swerve to this
	//const double c_GearHeightOffset=1.397;  //55 inches
	//const double c_MotorToWheelGearRatio=12.0/36.0;

	const double c_WheelDiameter=0.1524;  //6 inches
	props.WheelDiameter=c_WheelDiameter;
	props.Polynomial_Wheel[0]=0.0;
	props.Polynomial_Wheel[1]=1.0;
	props.Polynomial_Wheel[2]=0.0;
	props.Polynomial_Wheel[3]=0.0;
	props.Polynomial_Wheel[4]=0.0;
	*props.Polynomial_Swivel=*props.Polynomial_Wheel;
	props.Wheel_PID[0]=props.Swivel_PID[0]=100.0; //setting to a safe 1.0
	props.InputLatency=0.0;
	props.HeadingLatency=0.0;
	props.MotorToWheelGearRatio=1.0;  //most-likely this will be overridden
	props.VoltageScalar=1.0;  //May need to be reversed
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.IsOpen_Wheel=true;  //Always true by default until control is fully functional
	props.IsOpen_Swivel=true;
	for (size_t i=0;i<4;i++)
	{
		props.MaxSpeedOffset[i]=0.0;
	}
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.DriveTo_ForceDegradeScalar=Vec2d(1.0,1.0);
	props.SwivelRange=0.0;
	//depreciated
	//props.TankSteering_Tolerance=0.05;
	m_SwerveRobotProps=props;
	//Always use aggressive stop for driving
	for (size_t i=0;i<8;i++)
	{
		m_RotaryProps[i].RotaryProps().UseAggressiveStop=true;
	}
}

void Swerve_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("swerve_drive");
	if (!err) 
	{
		for (size_t i=0;i<8;i++)
		{
			err = script.GetFieldTable(csz_Swerve_Robot_SpeedControllerDevices_Enum[i]);
			if (!err)
			{
				m_RotaryProps[i].LoadFromScript(script);
				script.Pop();
			}
		}
		//Quick snap shot of all the properties
		//Vec2D WheelDimensions;
		//double WheelDiameter;
		//double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
		//double LeftPID[3]; //p,i,d
		//double RightPID[3]; //p,i,d
		//Get the ship dimensions
		err = script.GetFieldTable("wheel_base_dimensions");
		if (!err)
		{
			double length_in, width_in;	
			//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
			err = script.GetField("length_in", NULL, NULL,&length_in);
			ASSERT_MSG(!err, err);
			err = script.GetField("width_in", NULL, NULL,&width_in);
			ASSERT_MSG(!err, err);
			m_SwerveRobotProps.WheelDimensions=Vec2D(Inches2Meters(width_in),Inches2Meters(length_in));  //x,y  where x=width
			script.Pop();
		}

		double wheel_diameter;
		//Note: there shouldn't be a rotary system equivalent of this to pass down
		err=script.GetField("wheel_diameter_in", NULL, NULL, &wheel_diameter);
		if (!err)
			m_SwerveRobotProps.WheelDiameter=Inches2Meters(wheel_diameter);

		script.GetField("encoder_to_wheel_ratio", NULL, NULL, &m_SwerveRobotProps.MotorToWheelGearRatio);
		script.GetField("voltage_multiply", NULL, NULL, &m_SwerveRobotProps.VoltageScalar);
		script.GetField("tolerance", NULL, NULL, &m_SwerveRobotProps.PrecisionTolerance);

		err = script.GetField("heading_latency", NULL, NULL, &m_SwerveRobotProps.HeadingLatency);
		if (err)
			m_SwerveRobotProps.HeadingLatency=m_SwerveRobotProps.InputLatency+0.100;  //Give a good default without needing to add this property

		const char * const section_table[]=
		{
			"fl","fr","rl","rr"
		};

		err = script.GetFieldTable("max_offset");
		if (!err)
		{
			for (size_t i=0;i<4;i++)
			{
				err = script.GetField(section_table[i], NULL, NULL,&m_SwerveRobotProps.MaxSpeedOffset[i]);
				ASSERT_MSG(!err, err);
			}
			script.Pop();
		}

		script.GetField("drive_to_scale", NULL, NULL, &m_SwerveRobotProps.DriveTo_ForceDegradeScalar[1]);
		script.GetField("strafe_to_scale", NULL, NULL, &m_SwerveRobotProps.DriveTo_ForceDegradeScalar[0]);
		

		string sTest;
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
			{
				m_SwerveRobotProps.IsOpen_Wheel=true;
			}
			else
			{
				m_SwerveRobotProps.IsOpen_Wheel=false;
			}
		}
		err = script.GetField("is_closed_swivel",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
			{
				m_SwerveRobotProps.IsOpen_Swivel=true;
			}
			else
			{
				m_SwerveRobotProps.IsOpen_Swivel=false;
			}
		}


		err = script.GetFieldTable("encoder_reversed_wheel");
		if (!err)
		{
			for (size_t i=0;i<4;i++)
			{
				err = script.GetField(section_table[i],&sTest , NULL, NULL);
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					m_SwerveRobotProps.EncoderReversed_Wheel[i]=true;
				ASSERT_MSG(!err, err);
			}
			script.Pop();
		}
		#ifdef Robot_TesterCode
		err = script.GetFieldTable("motor_specs");
		if (!err)
		{
			m_EncoderSimulation.LoadFromScript(script);
			script.Pop();
		}
		#endif
		//The main script pop of swerve_drive field table
		script.Pop(); 
	}

	//depreciated
	//err = script.GetFieldTable("controls");
	//if (!err)
	//{
	//	script.GetField("tank_steering_tolerance", NULL, NULL,&m_SwerveRobotProps.TankSteering_Tolerance);
	//	script.Pop();
	//}

	__super::LoadFromScript(script);
}

  /***********************************************************************************************************************************/
 /*														Swerve_Robot_Control														*/
/***********************************************************************************************************************************/

Swerve_Robot_Control::Swerve_Robot_Control(bool UseSafety) : m_RobotMaxSpeed(0.0),m_DisplayVoltage(true)
{
	#ifdef Robot_TesterCode
	for (size_t i=0;i<4;i++)
	{
		m_EncoderVoltage[i]=0;
		m_PotentiometerVoltage[i]=0;
	}
	#endif
}

Swerve_Robot_Control::~Swerve_Robot_Control()
{
	Encoder_Stop(Swerve_Robot::eWheel_FL),Encoder_Stop(Swerve_Robot::eWheel_FR);  //TODO Move for autonomous mode only
	Encoder_Stop(Swerve_Robot::eWheel_RL),Encoder_Stop(Swerve_Robot::eWheel_RR);
}

void Swerve_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Swerve_Robot_Properties *robot_props=dynamic_cast<const Swerve_Robot_Properties *>(props);

	static bool RunOnce=false;
	if (!RunOnce)
	{
		RunOnce=true;

		//This one one must also be called for the lists that are specific to the robot
		RobotControlCommon_Initialize(robot_props->Get_ControlAssignmentProps());

		const double EncoderPulseRate=(1.0/360.0);
		for (size_t i=Swerve_Robot::eWheel_FL;i<Swerve_Robot::eSwivel_FL;i++)
		{
			Encoder_SetReverseDirection(i,m_SwerveRobotProps.GetSwerveRobotProps().EncoderReversed_Wheel[i]);
			Encoder_SetDistancePerPulse(i,EncoderPulseRate);
			Encoder_Start(i);
		}

		#ifdef Robot_TesterCode
		SmartDashboard::PutBoolean("SafetyLock_Drive",false);
		#else
		SmartDashboard::PutBoolean("SafetyLock_Drive",true);
		#endif
		#ifdef __EnableSafetyOnDrive__
		const size_t SafetyEnumStart=0;
		#else
		const size_t SafetyEnumStart=Swerve_Robot::eSwivel_FL;
		#endif
		for (size_t i=SafetyEnumStart;i<Swerve_Robot::eNoSwerveRobotSpeedControllerDevices;i++)
		{
			const char * const Prefix=csz_Swerve_Robot_SpeedControllerDevices_Enum[i];
			string ContructedName;
			ContructedName=Prefix;
			ContructedName[0]-=32; //Make first letter uppercase
			ContructedName+="Disable";
			#ifdef Robot_TesterCode
			const bool DisableDefault=false;
			#else
			const bool DisableDefault=true;
			#endif
			SmartDashboard::PutBoolean(ContructedName.c_str(),DisableDefault);
		}
	}

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();

		//This will copy all the props
		m_SwerveRobotProps=*robot_props;
		//We'll try to construct the props to match our properties
		//Note: for max accel it needs to be powerful enough to handle curve equations
		//Ship_1D_Properties props("SwerveEncoder",2.0,0.0,m_RobotMaxSpeed,1.0,1.0,robot_props->GetMaxAccelForward() * 3.0,robot_props->GetMaxAccelReverse() * 3.0);
		for (size_t i=0;i<4;i++)
		{
			Rotary_Properties drive=robot_props->GetRotaryProps(i);
			#ifdef Robot_TesterCode
			drive.EncoderSimulationProps()=robot_props->GetEncoderSimulationProps();
			m_Encoders[i].Initialize(&drive);
			m_Encoders[i].SetReverseDirection(robot_props->GetSwerveRobotProps().EncoderReversed_Wheel[i]);
			m_Potentiometers[i].Initialize(&robot_props->GetRotaryProps(i));
			//potentiometers have this solved within their class
			#endif
		}
		for (size_t i=Swerve_Robot::eSwivel_FL;i<Swerve_Robot::eNoSwerveRobotSpeedControllerDevices;i++)
		{
			const Rotary_Pot_Properties &swerve=robot_props->GetRotaryProps(i);
			const Rotary_Pot_Props &props=swerve.GetRotary_Pot_Properties();
			m_PotPoly[i].Initialize(&props.PotPolyTerms);
		}
	}
}

void Swerve_Robot_Control::Reset_Encoders()
{
	//Yup... method driven ;)
	for (size_t i=0;i<Swerve_Robot::eNoSwerveRobotSpeedControllerDevices;i++)
		Reset_Rotary(i);
}

#if 0
void Swerve_Robot_Control::DisplayVoltage()
{
	if (m_DisplayVoltage)
	{
		//display voltages
		DOUT2("fl=%.2f fr=%.2f rl=%.2f rr=%.2f\n",m_EncoderVoltage[Swerve_Robot::eWheel_FL],m_EncoderVoltage[Swerve_Robot::eWheel_FR],
			m_EncoderVoltage[Swerve_Robot::eWheel_RL],m_EncoderVoltage[Swerve_Robot::eWheel_RR]);
		//display voltages  (using wheel enums since the order is consistent)
		DOUT4("fl=%.2f fr=%.2f rl=%.2f rr=%.2f\n",m_PotentiometerVoltage[Swerve_Robot::eWheel_FL],m_PotentiometerVoltage[Swerve_Robot::eWheel_FR],
			m_PotentiometerVoltage[Swerve_Robot::eWheel_RL],m_PotentiometerVoltage[Swerve_Robot::eWheel_RR]);
	}
}
#endif

void Swerve_Robot_Control::Swerve_Drive_Control_TimeChange(double dTime_s)
{
	#ifdef Robot_TesterCode
	for (size_t i=0;i<4;i++)
	{
		m_Encoders[i].SetTimeDelta(dTime_s);
		m_Potentiometers[i].SetTimeDelta(dTime_s);
	}
	#endif
	//DisplayVoltage();
}

void Swerve_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case Swerve_Robot::eWheel_FL:
		case Swerve_Robot::eWheel_FR:
		case Swerve_Robot::eWheel_RL:
		case Swerve_Robot::eWheel_RR:
			m_KalFilter[index].Reset();
			Encoder_SetReverseDirection(index,m_SwerveRobotProps.GetSwerveRobotProps().EncoderReversed_Wheel[index]);
			#ifdef Robot_TesterCode
			m_Encoders[index].ResetPos();
			#endif
			break;
		case Swerve_Robot::eSwivel_FL:
		case Swerve_Robot::eSwivel_FR:
		case Swerve_Robot::eSwivel_RL:
		case Swerve_Robot::eSwivel_RR:
			m_KalFilter[index].Reset();
			#ifdef Robot_TesterCode
			m_Potentiometers[index-4].ResetPos();
			#endif
			break;
	}
}

__inline double Swerve_Robot_Control::Pot_GetRawValue(size_t index)
{
	//double raw_value = (double)m_Potentiometer.GetAverageValue();
	double raw_value=(double)Analog_GetAverageValue(index);
	raw_value = m_KalFilter[index](raw_value);  //apply the Kalman filter
	raw_value=m_Averager[index].GetAverage(raw_value); //and Ricks x element averager
	//Note: we keep the raw value in its native form... just averaging at most for less noise
	return raw_value;
}

double Swerve_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case Swerve_Robot::eWheel_FL:
		case Swerve_Robot::eWheel_FR:
		case Swerve_Robot::eWheel_RL:
		case Swerve_Robot::eWheel_RR:
			{
				//double EncRate=Encoder_GetRate2(m_dTime_s);
				double EncRate=Encoder_GetRate(index);
				EncRate=m_KalFilter[index](EncRate);
				EncRate=m_Averager[index].GetAverage(EncRate);
				EncRate=IsZero(EncRate)?0.0:EncRate;

				const double EncVelocity=RPS_To_LinearVelocity(EncRate);
				//Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", EncVelocity,RightVelocity);
				#ifdef Robot_TesterCode
				result=m_Encoders[index].GetEncoderVelocity();
				#else
				result= EncVelocity;
				#endif
				const char * const Prefix=csz_Swerve_Robot_SpeedControllerDevices_Enum[index];
				string ContructedName;
				ContructedName=Prefix,ContructedName+="_Encoder";
				SmartDashboard::PutNumber(ContructedName.c_str(),result);
			}
			break;
		case Swerve_Robot::eSwivel_FL:
		case Swerve_Robot::eSwivel_FR:
		case Swerve_Robot::eSwivel_RL:
		case Swerve_Robot::eSwivel_RR:
			{
				#ifndef Robot_TesterCode
				const double raw_value=Pot_GetRawValue(index);
				double adjusted_raw_value = m_PotPoly[index](raw_value); //apply custom curve to make linear
				double PotentiometerRaw_To_Arm;

				const double HiRange=m_SwerveRobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotMaxValue;
				const double LowRange=m_SwerveRobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotMinValue;
				//If this is true, the value is inverted with the negative operator
				const bool FlipRange=m_SwerveRobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().IsFlipped;

				PotentiometerRaw_To_Arm = adjusted_raw_value-LowRange;//zeros the potentiometer
				PotentiometerRaw_To_Arm = PotentiometerRaw_To_Arm/(HiRange-LowRange);//scales values from 0 to 1 with +- .001

				//Clip Range
				//I imagine .001 corrections will not be harmful for when in use.
				if (PotentiometerRaw_To_Arm < 0) PotentiometerRaw_To_Arm = 0;//corrects .001 or less causing a negative value
				if (PotentiometerRaw_To_Arm > 1 || PotentiometerRaw_To_Arm > .999) PotentiometerRaw_To_Arm = 1;//corrects .001 or lass causing value greater than 1

				if (FlipRange)
					PotentiometerRaw_To_Arm=1.0-PotentiometerRaw_To_Arm;

				const char * const Prefix=csz_Swerve_Robot_SpeedControllerDevices_Enum[index];
				string ContructedName;
				ContructedName=Prefix,ContructedName+="_Raw";
				SmartDashboard::PutNumber(ContructedName.c_str(),raw_value);
				ContructedName=Prefix,ContructedName+="_Pot_Raw";
				SmartDashboard::PutNumber(ContructedName.c_str(),PotentiometerRaw_To_Arm);
				const double Tolerance=m_SwerveRobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotLimitTolerance;
				//Potentiometer safety, if we lose wire connection it will be out of range in which case we turn on the safety (we'll see it turned on)
				if (raw_value>(HiRange+Tolerance) || raw_value<(LowRange-Tolerance))
				{
					std::string SmartLabel=csz_Swerve_Robot_SpeedControllerDevices_Enum[index];
					SmartLabel[0]-=32; //Make first letter uppercase
					ContructedName=SmartLabel+"Disable";
					SmartDashboard::PutBoolean(ContructedName.c_str(),true);
				}

				//Now to compute the result... we start with the normalized value and give it the appropriate offset and scale
				//the offset is delegated in script in the final scale units, and the scale is the total range in radians
				result=PotentiometerRaw_To_Arm;
				//get scale
				const Ship_1D_Props &shipprops=m_SwerveRobotProps.GetRotaryProps(index).GetShip_1D_Props();
				result*=shipprops.MaxRange-shipprops.MinRange;  //compute the total distance in radians
				result*=m_SwerveRobotProps.GetRotaryProps(index).GetRotaryProps().EncoderToRS_Ratio;
				//get offset... Note: scale comes first since the offset is of that scale
				result+=m_SwerveRobotProps.GetRotaryProps(index).GetRotary_Pot_Properties().PotentiometerOffset;
				#else
				result=NormalizeRotation2(m_Potentiometers[index-4].GetPotentiometerCurrentPosition());
				//Now to normalize it
				const Ship_1D_Props &shipprops=m_SwerveRobotProps.GetRotaryProps(index).GetShip_1D_Props();
				const double NormalizedResult= (result - shipprops.MinRange)  / (shipprops.MaxRange - shipprops.MinRange);
				const char * const Prefix=csz_Swerve_Robot_SpeedControllerDevices_Enum[index];
				string ContructedName;
				ContructedName=Prefix,ContructedName+="_Raw";
				SmartDashboard::PutNumber(ContructedName.c_str(),result);  //this one is a bit different as it is the selected units we use
				ContructedName=Prefix,ContructedName+="_Pot_Raw";
				SmartDashboard::PutNumber(ContructedName.c_str(),NormalizedResult);
				#endif
			}
			break;
	}

	return result;
}


__inline void CheckDisableSafety(size_t index,bool &SafetyLock)
{
	//return;
	std::string SmartLabel=csz_Swerve_Robot_SpeedControllerDevices_Enum[index];
	SmartLabel[0]-=32; //Make first letter uppercase
	//This section is extra control of each system while 3D positioning is operational... enable for diagnostics
	std::string VoltageArmSafety=SmartLabel+"Disable";
	const bool bVoltageArmDisable=SmartDashboard::GetBoolean(VoltageArmSafety.c_str());
	if (bVoltageArmDisable)
		SafetyLock=true;
}

void Swerve_Robot_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	bool SafetyLock=SmartDashboard::GetBoolean("SafetyLock_Drive");
	double VoltageScalar=1.0;

	switch (index)
	{
	case Swerve_Robot::eWheel_FL:
	case Swerve_Robot::eWheel_FR:
	case Swerve_Robot::eWheel_RL:
	case Swerve_Robot::eWheel_RR:
		#ifdef __EnableSafetyOnDrive__
		CheckDisableSafety(index,SafetyLock);
		#endif
		#ifdef Robot_TesterCode
		//if (m_SlowWheel) Voltage=0.0;
		m_EncoderVoltage[index]=Voltage;
		if (SafetyLock)
			Voltage=0.0;
		m_Encoders[index].UpdateEncoderVoltage(Voltage);
		m_Encoders[index].TimeChange();
		#endif
		break;
	case Swerve_Robot::eSwivel_FL:
	case Swerve_Robot::eSwivel_FR:
	case Swerve_Robot::eSwivel_RL:
	case Swerve_Robot::eSwivel_RR:
		CheckDisableSafety(index,SafetyLock);
		#ifdef Robot_TesterCode
		{
			size_t i=index-4;
			m_PotentiometerVoltage[i]=Voltage;
			m_Potentiometers[i].UpdatePotentiometerVoltage(SafetyLock?0.0:Voltage);
			m_Potentiometers[i].TimeChange();  //have this velocity immediately take effect
		}
		#endif
		break;
	}
	VoltageScalar=m_SwerveRobotProps.GetRotaryProps(index).GetRotaryProps().VoltageScalar;
	Voltage*=VoltageScalar;
	std::string SmartLabel=(index<8)?csz_Swerve_Robot_SpeedControllerDevices_Enum[index]:"OutOfRange";
	SmartLabel[0]-=32; //Make first letter uppercase
	SmartLabel+="_Voltage";
	SmartDashboard::PutNumber(SmartLabel.c_str(),Voltage);
	if (SafetyLock)
		Voltage=0.0;
	Victor_UpdateVoltage(index,Voltage);
}

double Swerve_Robot_Control::RPS_To_LinearVelocity(double RPS)
{
	const Swerve_Robot_Props &SwerveRobotProps=m_SwerveRobotProps.GetSwerveRobotProps();
	return RPS * SwerveRobotProps.MotorToWheelGearRatio * M_PI * SwerveRobotProps.WheelDiameter; 
}
