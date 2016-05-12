
#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "../Common/Entity_Properties.h"
#include "../Common/Physics_1D.h"
#include "../Common/Physics_2D.h"
#include "../Common/Entity2D.h"
#include "../Common/Goal.h"
#include "../Common/Ship_1D.h"
#include "../Common/Ship.h"
#include "../Common/AI_Base_Controller.h"
#include "../Common/Vehicle_Drive.h"
#include "../Common/PIDController.h"
#include "../Common/Poly.h"
#include "../Common/Robot_Control_Interface.h"
#include "../Common/Rotary_System.h"
#include "Swerve_Robot.h"

using namespace Framework::Base;
using namespace std;

//namespace Scripting=GG_Framework::Logic::Scripting;
namespace Scripting=Framework::Scripting;

//const double Pi2=M_PI*2.0;

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

void Swerve_Robot::DrivingModule::Initialize(Framework::Base::EventMap& em,const DrivingModule_Props *props)
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
Swerve_Robot::Swerve_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
	Ship_Tester(EntityName), m_RobotControl(robot_control), m_IsAutonomous(IsAutonomous), m_VehicleDrive(NULL),
	m_UsingEncoders(IsAutonomous), //,m_VoltageOverride(false),m_UseDeadZoneSkip(true)
	m_Heading(0.0), m_HeadingUpdateTimer(0.0),m_TankSteering(this)
{
	m_Physics.SetHeadingToUse(&m_Heading);  //We manage the heading
	const char * const ModuleName[]=
	{
		"ModuleLF","ModuleRF","ModuleLR","ModuleRR"
	};
	for (size_t i=0;i<4;i++)
		m_DrivingModule[i]=new DrivingModule(ModuleName[i],m_RobotControl,i);
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

void Swerve_Robot::Initialize(Framework::Base::EventMap& em, const Entity_Properties *props)
{
	m_VehicleDrive=CreateDrive();
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const Swerve_Robot_Properties *RobotProps=dynamic_cast<const Swerve_Robot_Properties *>(props);
	if (RobotProps)
	{
		//This will copy all the props
		m_SwerveRobotProps=RobotProps->GetSwerveRobotProps();
		m_WheelDimensions=RobotProps->GetWheelDimensions();
		//depreciated
		//m_TankSteering.SetStraightDeadZone_Tolerance(RobotProps->GetSwerveRobotProps().TankSteering_Tolerance);
		for (size_t i=0;i<4;i++)
		{
			DrivingModule::DrivingModule_Props props;
			Rotary_Properties drive=RobotProps->GetDriveProps();
			Rotary_Properties swivel=RobotProps->GetSwivelProps();
			props.Swivel_Props=&swivel;
			props.Drive_Props=&drive;
			//Now to copy the custom per section properties over for this element
			//TODO max_offset support
			drive.RotaryProps().PID_Console_Dump=m_SwerveRobotProps.PID_Console_Dump_Wheel[i];
			swivel.RotaryProps().PID_Console_Dump=m_SwerveRobotProps.PID_Console_Dump_Swivel[i];
			drive.RotaryProps().InverseMaxAccel=m_SwerveRobotProps.InverseMaxAccel;
			drive.RotaryProps().InverseMaxDecel=m_SwerveRobotProps.InverseMaxDecel;
			drive.RotaryProps().UseAggressiveStop=true;
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
		encoders.Velocity.Named.sFL=m_RobotControl->GetRotaryCurrentPorV(eWheel_FL);
		encoders.Velocity.Named.sFR=m_RobotControl->GetRotaryCurrentPorV(eWheel_FR);
		encoders.Velocity.Named.sRL=m_RobotControl->GetRotaryCurrentPorV(eWheel_RL);
		encoders.Velocity.Named.sRR=m_RobotControl->GetRotaryCurrentPorV(eWheel_RR);
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
		encoders.Velocity.Named.aFL=m_RobotControl->GetRotaryCurrentPorV(eSwivel_FL);
		encoders.Velocity.Named.aFR=m_RobotControl->GetRotaryCurrentPorV(eSwivel_FR);
		encoders.Velocity.Named.aRL=m_RobotControl->GetRotaryCurrentPorV(eSwivel_RL);
		encoders.Velocity.Named.aRR=m_RobotControl->GetRotaryCurrentPorV(eSwivel_RR);
	}

	//Now the new UpdateVelocities was just called... work with these intended velocities
	for (size_t i=0;i<4;i++)
	{
		const double IntendedDirection=m_VehicleDrive->GetIntendedVelocitiesFromIndex(i+4);
		double SwivelDirection=IntendedDirection;  //this is either the intended direction or the reverse of it
		const Ship_1D &Swivel=m_DrivingModule[i]->GetSwivel();
		//This is normalized implicitly
		const double LastSwivelDirection=Swivel.GetPos_m();
		double DistanceToIntendedSwivel=fabs(NormalizeRotation2(LastSwivelDirection-SwivelDirection));

		if ((DistanceToIntendedSwivel>PI_2) || 
			(Swivel.GetUsingRange() &&
			 ((SwivelDirection>Swivel.GetMaxRange()) || (SwivelDirection<Swivel.GetMinRange()))) 
			)
		{
			SwivelDirection=NormalizeRotation2(SwivelDirection+Pi);
			if (Swivel.GetUsingRange())
			{
				double TestIntendedFlipped=NormalizeRotation2(IntendedDirection+Pi);
				//If we flipped because of a huge delta check that the reverse position is in range... and flip it back if it exceed the range
				if ((SwivelDirection>Swivel.GetMaxRange()) || (SwivelDirection<Swivel.GetMinRange()) ||
					(TestIntendedFlipped>Swivel.GetMaxRange()) || (TestIntendedFlipped<Swivel.GetMinRange()))
				{
					SwivelDirection+=Pi;
					NormalizeRotation(SwivelDirection);
				}
			}
		}

		//Note the velocity is checked once before the time change here, and once after for the current
		//Only apply swivel adjustments if we have significant movement (this matters in targeting tests)
		//if ((fabs(LocalForce[0])>1.5)||(fabs(LocalForce[1])>1.5)||(fabs(m_DrivingModule[i]->GetDrive().GetPhysics().GetVelocity()) > 0.05))
		//This logic fails when driving a direct strafe from a stop

		//This logic allows for 2 / 3 degrees of freedom... which help keep it from auto correcting too much
		//if (DistanceToIntendedSwivel>0.034)
		//For now I'll leave this disabled since close loop is doing so well, but once I get the swivel on-line for simulation I may need to reconsider

		m_DrivingModule[i]->SetIntendedSwivelDirection(SwivelDirection);

		const double IntendedSpeed=m_VehicleDrive->GetIntendedVelocitiesFromIndex(i);

		//To minimize error only apply the Y component amount to the velocity
		//The less the difference between the current and actual swivel direction the greater the full amount can be applied
		double VelocityToUse=cos(DistanceToIntendedSwivel)*IntendedSpeed;

		#ifdef __DebugLUA__
		if (m_SwerveRobotProps.PID_Console_Dump_Wheel[i] && (m_RobotControl->GetRotaryCurrentPorV(i)!=0.0))
		{
			double PosY=GetPos_m()[1];
			printf("y=%.2f ",PosY);
		}
		#endif

		m_DrivingModule[i]->SetIntendedDriveVelocity(VelocityToUse);
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
		if ((m_SwerveRobotProps.IsOpen_Swivel) && (IntendedDirection==0.0) && (DistanceToIntendedSwivel<0.005))
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
	}

	m_VehicleDrive->InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Swerve_Robot::TimeChange(double dTime_s)
{
	m_RobotControl->Swerve_Drive_Control_TimeChange(dTime_s);

	__super::TimeChange(dTime_s);
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

Swerve_Robot_Properties::Swerve_Robot_Properties() : m_SwivelProps(
	"Swivel",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	18.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	60.0,60.0, //Max Acceleration Forward/Reverse (try to tune to the average turning speed to minimize error on PID)
	Ship_1D_Props::eSwivel,
	//true,	//Using the range:  for now assuming a 1:1 using a potentiometer with 270 degrees and 10 degrees of padding = 130 degrees each way
	false,  //Or not use the range... it appears it is possible to have unlimited range using the Vishear encoders
	-DEG_2_RAD(130.0),DEG_2_RAD(130.0),
	true  //this is definitely angular
	),
	m_DriveProps(
	"Drive",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//These should match the settings in the script
	2.916,   //Max Speed (This is linear movement speed)
	//10, //TODO find out why I need 8 for turns
	10.0,10.0, //ACCEL, BRAKE  (These can be ignored)
	//Make these as fast as possible without damaging chain or motor
	//These must be fast enough to handle rotation angular acceleration
	60.0,60.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eSimpleMotor,
	false	//No limit ever!
	)
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
		props.PID_Console_Dump_Wheel[i]=false;  //Always false unless you want to analyze PID (only one system at a time!)
		props.PID_Console_Dump_Swivel[i]=false;  //Always false unless you want to analyze PID (only one system at a time!)
		props.MaxSpeedOffset[i]=0.0;
		props.EncoderReversed_Wheel[i]=false;
		props.EncoderReversed_Swivel[i]=false;
	}
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.ReverseSteering=false;
	props.DriveTo_ForceDegradeScalar=Vec2d(1.0,1.0);
	props.SwivelRange=0.0;
	//depreciated
	//props.TankSteering_Tolerance=0.05;
	props.InverseMaxAccel=0.0;
	m_SwerveRobotProps=props;
	//Always use aggressive stop for driving
	m_DriveProps.RotaryProps().UseAggressiveStop=true;
	m_SwivelProps.RotaryProps().UseAggressiveStop=true;
}

void Swerve_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("swerve_drive");
	if (!err) 
	{
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
		m_DriveProps.RotaryProps().EncoderToRS_Ratio=m_SwerveRobotProps.MotorToWheelGearRatio;

		script.GetField("voltage_multiply", NULL, NULL, &m_SwerveRobotProps.VoltageScalar);
		err = script.GetFieldTable("wheel_pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_SwerveRobotProps.Wheel_PID[0]);
			m_DriveProps.RotaryProps().PID[0]=m_SwerveRobotProps.Wheel_PID[0];
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_SwerveRobotProps.Wheel_PID[1]);
			m_DriveProps.RotaryProps().PID[1]=m_SwerveRobotProps.Wheel_PID[1];
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_SwerveRobotProps.Wheel_PID[2]);
			m_DriveProps.RotaryProps().PID[2]=m_SwerveRobotProps.Wheel_PID[2];
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		err = script.GetFieldTable("swivel_pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_SwerveRobotProps.Swivel_PID[0]);
			m_SwivelProps.RotaryProps().PID[0]=m_SwerveRobotProps.Swivel_PID[0];
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_SwerveRobotProps.Swivel_PID[1]);
			m_SwivelProps.RotaryProps().PID[1]=m_SwerveRobotProps.Swivel_PID[1];
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_SwerveRobotProps.Swivel_PID[2]);
			m_SwivelProps.RotaryProps().PID[2]=m_SwerveRobotProps.Swivel_PID[2];
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("tolerance", NULL, NULL, &m_SwerveRobotProps.PrecisionTolerance);
		m_DriveProps.RotaryProps().PrecisionTolerance=m_SwerveRobotProps.PrecisionTolerance;
		//TODO see if we want this for swivel

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
		
		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
		{
			m_SwerveRobotProps.Feedback_DiplayRow=(size_t)fDisplayRow;
			m_DriveProps.RotaryProps().Feedback_DiplayRow=m_SwerveRobotProps.Feedback_DiplayRow;
		}

		string sTest;
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
			{
				m_SwerveRobotProps.IsOpen_Wheel=true;
				m_DriveProps.RotaryProps().LoopState=Rotary_Props::eOpen;
			}
			else
			{
				m_SwerveRobotProps.IsOpen_Wheel=false;
				m_DriveProps.RotaryProps().LoopState=Rotary_Props::eClosed;
			}
		}
		err = script.GetField("is_closed_swivel",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
			{
				m_SwerveRobotProps.IsOpen_Swivel=true;
				m_SwivelProps.RotaryProps().LoopState=Rotary_Props::eOpen;
			}
			else
			{
				m_SwerveRobotProps.IsOpen_Swivel=false;
				m_SwivelProps.RotaryProps().LoopState=Rotary_Props::eClosed;
			}
		}

		err = script.GetFieldTable("show_pid_dump_wheel");
		if (!err)
		{
			for (size_t i=0;i<4;i++)
			{
				err = script.GetField(section_table[i],&sTest , NULL, NULL);
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					m_SwerveRobotProps.PID_Console_Dump_Wheel[i]=true;
				ASSERT_MSG(!err, err);
			}
			script.Pop();
		}

		err = script.GetFieldTable("show_pid_dump_swivel");
		if (!err)
		{
			for (size_t i=0;i<4;i++)
			{
				err = script.GetField(section_table[i],&sTest , NULL, NULL);
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					m_SwerveRobotProps.PID_Console_Dump_Swivel[i]=true;
				ASSERT_MSG(!err, err);
			}
			script.Pop();
		}

		err = script.GetField("reverse_steering",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_SwerveRobotProps.ReverseSteering=true;
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

		err = script.GetFieldTable("encoder_reversed_swivel");
		if (!err)
		{
			for (size_t i=0;i<4;i++)
			{
				err = script.GetField(section_table[i],&sTest , NULL, NULL);
				if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
					m_SwerveRobotProps.EncoderReversed_Swivel[i]=true;
				ASSERT_MSG(!err, err);
			}
			script.Pop();
		}

		err = script.GetFieldTable("curve_voltage_wheel");
		if (!err)
		{
			err = script.GetField("c", NULL, NULL,&m_SwerveRobotProps.Polynomial_Wheel[0]);
			m_DriveProps.RotaryProps().Voltage_Terms.Term[0]=m_SwerveRobotProps.Polynomial_Wheel[0];
			ASSERT_MSG(!err, err);
			err = script.GetField("t1", NULL, NULL,&m_SwerveRobotProps.Polynomial_Wheel[1]);
			m_DriveProps.RotaryProps().Voltage_Terms.Term[1]=m_SwerveRobotProps.Polynomial_Wheel[1];
			ASSERT_MSG(!err, err);
			err = script.GetField("t2", NULL, NULL,&m_SwerveRobotProps.Polynomial_Wheel[2]);
			m_DriveProps.RotaryProps().Voltage_Terms.Term[2]=m_SwerveRobotProps.Polynomial_Wheel[2];
			ASSERT_MSG(!err, err);
			err = script.GetField("t3", NULL, NULL,&m_SwerveRobotProps.Polynomial_Wheel[3]);
			m_DriveProps.RotaryProps().Voltage_Terms.Term[3]=m_SwerveRobotProps.Polynomial_Wheel[3];
			ASSERT_MSG(!err, err);
			err = script.GetField("t4", NULL, NULL,&m_SwerveRobotProps.Polynomial_Wheel[4]);
			m_DriveProps.RotaryProps().Voltage_Terms.Term[4]=m_SwerveRobotProps.Polynomial_Wheel[4];
			ASSERT_MSG(!err, err);
			script.Pop();
		}

		err = script.GetFieldTable("curve_voltage_wheel");
		if (!err)
		{
			err = script.GetField("c", NULL, NULL,&m_SwerveRobotProps.Polynomial_Swivel[0]);
			m_SwivelProps.RotaryProps().Voltage_Terms.Term[0]=m_SwerveRobotProps.Polynomial_Wheel[0];
			ASSERT_MSG(!err, err);
			err = script.GetField("t1", NULL, NULL,&m_SwerveRobotProps.Polynomial_Swivel[1]);
			m_SwivelProps.RotaryProps().Voltage_Terms.Term[1]=m_SwerveRobotProps.Polynomial_Wheel[1];
			ASSERT_MSG(!err, err);
			err = script.GetField("t2", NULL, NULL,&m_SwerveRobotProps.Polynomial_Swivel[2]);
			m_SwivelProps.RotaryProps().Voltage_Terms.Term[2]=m_SwerveRobotProps.Polynomial_Wheel[2];
			ASSERT_MSG(!err, err);
			err = script.GetField("t3", NULL, NULL,&m_SwerveRobotProps.Polynomial_Swivel[3]);
			m_SwivelProps.RotaryProps().Voltage_Terms.Term[3]=m_SwerveRobotProps.Polynomial_Wheel[3];
			ASSERT_MSG(!err, err);
			err = script.GetField("t4", NULL, NULL,&m_SwerveRobotProps.Polynomial_Swivel[4]);
			m_SwivelProps.RotaryProps().Voltage_Terms.Term[4]=m_SwerveRobotProps.Polynomial_Wheel[4];
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("inv_max_accel", NULL, NULL, &m_SwerveRobotProps.InverseMaxAccel);
		m_SwerveRobotProps.InverseMaxDecel=m_SwerveRobotProps.InverseMaxAccel;	//set up deceleration to be the same value by default
		script.GetField("inv_max_decel", NULL, NULL, &m_SwerveRobotProps.InverseMaxDecel);
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
	m_DriveProps.SetFromShip_Properties(GetShipProps());
}
