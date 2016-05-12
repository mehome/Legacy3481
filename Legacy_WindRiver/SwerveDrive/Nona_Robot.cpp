
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
#include "Nona_Robot.h"

using namespace Framework::Base;
using namespace std;

//namespace Scripting=GG_Framework::Logic::Scripting;
namespace Scripting=Framework::Scripting;

//const double PI=M_PI;
//const double Pi2=M_PI*2.0;
//const double Pi_Half=1.57079632679489661923;


  /***********************************************************************************************************/
 /*										Butterfly_Robot::DriveModeManager 									*/
/***********************************************************************************************************/
Butterfly_Robot::DriveModeManager::DriveModeManager(Butterfly_Robot *parent) : m_pParent(parent),m_CurrentMode(eOmniWheelDrive)
{
}

void Butterfly_Robot::DriveModeManager::SetMode(DriveMode Mode)
{
	//flood control
	if (m_CurrentMode!=Mode)
	{
		const TractionModeProps *PropsToUse=(Mode==eTractionDrive)?&m_TractionModeProps:&m_OmniModeProps;
		m_pParent->UpdateShipProperties(PropsToUse->ShipProperties.GetShipProps());
		//init the props (more of a pedantic step to avoid corrupt data)
		Rotary_Props props=m_ButterflyProps.GetDriveProps().GetRotaryProps();
		props.InverseMaxAccel=m_TractionModeProps.InverseMaxAccel;
		props.LoopState=(m_TractionModeProps.IsOpen)?Rotary_Props::eOpen : Rotary_Props::eClosed;
		double *PID=(Mode==eTractionDrive)?m_TractionModeProps.PID : m_OmniModeProps.PID;
		props.PID[0]=PID[0];
		props.PID[1]=PID[1];
		props.PID[2]=PID[2];
		Ship_1D_Props ship_props=m_ButterflyProps.GetDriveProps().GetShip_1D_Props();
		ship_props.SetFromShip_Properties(PropsToUse->ShipProperties.GetShipProps());

		//Now for the hand-picked swerve properties
		for (size_t i=0;i<4;i++)
		{
			props.PID_Console_Dump=m_ButterflyProps.GetSwerveRobotProps().PID_Console_Dump_Wheel[i];
			m_pParent->UpdateDriveProps(props,ship_props,i);
		}

		m_CurrentMode=Mode;
		//Notify parent for further processing
		m_pParent->DriveModeManager_SetMode_Callback(Mode);
		if (Mode==eTractionDrive)
			printf("Now in LowGear Traction Drive\n");
		else
		{
			assert(Mode==eOmniWheelDrive);
			printf("Now in HighGear Omni Drive\n");
		}
	}
}

void Butterfly_Robot::DriveModeManager::Initialize(const Butterfly_Robot_Properties &props)
{
	m_ButterflyProps=props;
	m_TractionModeProps=props.GetTractionModeProps();
	m_OmniModeProps.ShipProperties=m_pParent->m_ShipProps;
	m_OmniModeProps.IsOpen=m_pParent->GetSwerveRobotProps().IsOpen_Wheel;
	m_OmniModeProps.InverseMaxAccel=m_pParent->GetSwerveRobotProps().InverseMaxAccel;
	m_OmniModeProps.PID[0]=m_pParent->GetSwerveRobotProps().Wheel_PID[0];
	m_OmniModeProps.PID[1]=m_pParent->GetSwerveRobotProps().Wheel_PID[1];
	m_OmniModeProps.PID[2]=m_pParent->GetSwerveRobotProps().Wheel_PID[2];
}

void Butterfly_Robot::DriveModeManager::SetLowGear(bool on)
{
	SetMode(on?eTractionDrive:eOmniWheelDrive);
}

void Butterfly_Robot::DriveModeManager::SetLowGearValue(double Value)
{
	if (m_pParent->m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//printf("\r%f       ",Value);
	if (Value > 0.0)
	{
		if (GetMode()==eTractionDrive)
			SetLowGear(false);
	}
	else
	{
		if (GetMode()==eOmniWheelDrive)
			SetLowGear(true);
	}
}

void Butterfly_Robot::DriveModeManager::BindAdditionalEventControls(bool Bind)
{
	Framework::Base::EventMap *em=m_pParent->GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Butterfly_SetLowGear"].Subscribe(m_pParent->ehl, *this, &Butterfly_Robot::DriveModeManager::SetLowGear);
		em->Event_Map["Butterfly_SetLowGearOn"].Subscribe(m_pParent->ehl, *this, &Butterfly_Robot::DriveModeManager::SetLowGearOn);
		em->Event_Map["Butterfly_SetLowGearOff"].Subscribe(m_pParent->ehl, *this, &Butterfly_Robot::DriveModeManager::SetLowGearOff);
		em->EventValue_Map["Butterfly_SetLowGearValue"].Subscribe(m_pParent->ehl,*this, &Butterfly_Robot::DriveModeManager::SetLowGearValue);
	}
	else
	{
		em->EventOnOff_Map["Butterfly_SetLowGear"]  .Remove(*this, &Butterfly_Robot::DriveModeManager::SetLowGear);
		em->Event_Map["Butterfly_SetLowGearOn"]  .Remove(*this, &Butterfly_Robot::DriveModeManager::SetLowGearOn);
		em->Event_Map["Butterfly_SetLowGearOff"]  .Remove(*this, &Butterfly_Robot::DriveModeManager::SetLowGearOff);
		em->EventValue_Map["Butterfly_SetLowGearValue"].Remove(*this, &Butterfly_Robot::DriveModeManager::SetLowGearValue);
	}
}

void Butterfly_Robot::DriveModeManager::BindAdditionalUIControls(bool Bind,void *joy,void *key)
{
	m_ButterflyProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
}

  /***********************************************************************************************************/
 /*												Butterfly_Robot												*/
/***********************************************************************************************************/

Butterfly_Robot::Butterfly_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
	Swerve_Robot(EntityName,robot_control,IsAutonomous),m_DriveModeManager(this)
{

}

Butterfly_Robot::~Butterfly_Robot()
{

}

void Butterfly_Robot::Initialize(Framework::Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	const Butterfly_Robot_Properties *RobotProps=dynamic_cast<const Butterfly_Robot_Properties *>(props);
	if (RobotProps)
		m_DriveModeManager.Initialize(*RobotProps);
}

void Butterfly_Robot::BindAdditionalEventControls(bool Bind)
{
	__super::BindAdditionalEventControls(Bind);
	m_DriveModeManager.BindAdditionalEventControls(Bind);
}

void Butterfly_Robot::BindAdditionalUIControls(bool Bind,void *joy)
{
	m_DriveModeManager.BindAdditionalUIControls(Bind,joy,NULL);
	__super::BindAdditionalUIControls(Bind,joy,NULL);  //call super for more general control assignments
}

void Butterfly_Robot::DriveModeManager_SetMode_Callback(DriveMode Mode) 
{
	m_RobotControl->CloseSolenoid(eUseLowGear,Mode==Butterfly_Robot::eTractionDrive);
}

  /***********************************************************************************************************/
 /*											Butterfly_Robot_Properties										*/
/***********************************************************************************************************/

Butterfly_Robot_Properties::Butterfly_Robot_Properties() : m_RobotControls(&s_ControlsEvents)
{
	memset(&m_TractionModePropsProps,0,sizeof(TractionModeProps));
}

//declared as global to avoid allocation on stack each iteration
const char * const g_Butterfly_Controls_Events[] = 
{
	"Butterfly_SetLowGear","Butterfly_SetLowGearOn","Butterfly_SetLowGearOff","Butterfly_SetLowGearValue"
};

const char *Butterfly_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_Butterfly_Controls_Events))?g_Butterfly_Controls_Events[index] : NULL;
}
Butterfly_Robot_Properties::ControlEvents Butterfly_Robot_Properties::s_ControlsEvents;

void Butterfly_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	__super::LoadFromScript(script);
	m_TractionModePropsProps.ShipProperties=*this;
	const char* err=NULL;
	err = script.GetFieldTable("low_gear");
	if (!err)
	{
		m_TractionModePropsProps.ShipProperties.LoadFromScript(script);

		err = script.GetFieldTable("swerve_drive");
		if (!err) 
		{
			err = script.GetFieldTable("pid");
			if (!err)
			{
				err = script.GetField("p", NULL, NULL,&m_TractionModePropsProps.PID[0]);
				ASSERT_MSG(!err, err);
				err = script.GetField("i", NULL, NULL,&m_TractionModePropsProps.PID[1]);
				ASSERT_MSG(!err, err);
				err = script.GetField("d", NULL, NULL,&m_TractionModePropsProps.PID[2]);
				ASSERT_MSG(!err, err);
				script.Pop();
			}

			string sTest;
			err = script.GetField("is_closed",&sTest,NULL,NULL);
			if (!err)
			{
				if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
					m_TractionModePropsProps.IsOpen=true;
				else
					m_TractionModePropsProps.IsOpen=false;
			}
			err = script.GetField("inv_max_accel", NULL, NULL, &m_TractionModePropsProps.InverseMaxAccel);
			#ifdef AI_TesterCode
			err = script.GetFieldTable("motor_specs");
			if (!err)
			{
				double test;
				err = script.GetField("gear_reduction", NULL, NULL, &test);
				if (!err) m_TractionModePropsProps.GearReduction=test;
				else 
					m_TractionModePropsProps.GearReduction=12.4158;  //a good default
				script.Pop();
			}
			else 
				m_TractionModePropsProps.GearReduction=12.4158;  //a good default
			#endif
			script.Pop();
		}
		script.Pop();
	}
	err = script.GetFieldTable("controls");
	if (!err)
	{
		m_RobotControls.LoadFromScript(script);
		script.Pop();
	}
}


  /***********************************************************************************************************/
 /*													Nona_Robot												*/
/***********************************************************************************************************/

Nona_Robot::Nona_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
Butterfly_Robot(EntityName,robot_control,IsAutonomous),m_KickerWheel("KickerWheel",robot_control,eWheel_Kicker),m_NonaDrive(NULL)
{
}

Swerve_Drive *Nona_Robot::CreateDrive() 
{
	m_NonaDrive=new Nona_Drive(this);
	return m_NonaDrive;
}

Nona_Robot::~Nona_Robot()
{
}

void Nona_Robot::Initialize(Framework::Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	const Nona_Robot_Properties *RobotProps=dynamic_cast<const Nona_Robot_Properties *>(props);
	if (RobotProps)
	{
		const Rotary_Properties &kickerwheel=RobotProps->GetKickerWheelProps();
		m_KickerWheel.Initialize(em,&kickerwheel);
	}
}

void Nona_Robot::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	double encoderVelocity=m_RobotControl->GetRotaryCurrentPorV(eWheel_Kicker);
	const double IntendedVelocity=m_NonaDrive->GetKickerWheelIntendedVelocity();
	m_NonaDrive->SetKickerWheelVelocity(encoderVelocity);
	__super::InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
	m_NonaDrive->SetKickerWheelVelocity(IntendedVelocity);
	m_KickerWheel.SetRequestedVelocity(IntendedVelocity);
	m_KickerWheel.AsEntity1D().TimeChange(dTime_s);
}

void Nona_Robot::DriveModeManager_SetMode_Callback(DriveMode Mode) 
{
	m_KickerWheel.Stop();
	__super::DriveModeManager_SetMode_Callback(Mode);
}

  /***********************************************************************************************************/
 /*												Nona_Robot_Properties										*/
/***********************************************************************************************************/

Nona_Robot_Properties::Nona_Robot_Properties() : m_KickerWheelProps(
	"Drive",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//These should match the settings in the script
	2.450592,   //Max Speed (This is linear movement speed)
	//10, //TODO find out why I need 8 for turns
	10.0,10.0, //ACCEL, BRAKE  (These can be ignored)
	//Make these as fast as possible without damaging chain or motor
	//These must be fast enough to handle rotation angular acceleration
	4.0,4.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eSimpleMotor,
	false	//No limit ever!
	)
{
	//Always use aggressive stop for driving
	m_KickerWheelProps.RotaryProps().UseAggressiveStop=true;
}

void Nona_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("kicker");
	if (!err)
	{
		m_KickerWheelProps.LoadFromScript(script);
		script.Pop();
	}
	__super::LoadFromScript(script);
}

