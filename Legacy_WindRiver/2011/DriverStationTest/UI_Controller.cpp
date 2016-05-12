#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"

#undef __WindRiverJoysticks__
#undef __UsingXTerminator__
#define __UsingWPTH_UI__ //The WPLib Testing Harness UI (where the second joystick is on the UI itself)
#undef __EnableTestKeys__

using namespace Framework::Base;
using namespace Framework::UI;

  /***************************************************************************************************************/
 /*												UI_Controller													*/
/***************************************************************************************************************/

void UI_Controller::Init_AutoPilotControls()
{
}


//! TODO: Use the script to grab the head position to provide the HUD
UI_Controller::UI_Controller(JoyStick_Binder &joy,AI_Base_Controller *base_controller) : 
	/*m_HUD_UI(new HUD_PDCB(osg::Vec3(0.0, 4.0, 0.5))), */
	m_Base(NULL),m_SlideButtonToggle(false),m_isControlled(false),m_CruiseSpeed(0.0),m_autoPilot(true),m_enableAutoLevelWhenPiloting(false),
	/*m_hud_connected(false),*/
	m_Test1(false),m_Test2(false),m_Ship_UseHeadingSpeed(true)
{
	ResetPos();
	Set_AI_Base_Controller(base_controller); //set up ship (even if we don't have one)
	m_LastSliderTime[0]=m_LastSliderTime[1]=0.0;

	//TODO hard code the events to the correct mappings
	//I'll need to add parm for instance name to support multiple joysticks
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Joystick_SetCurrentSpeed_2",true,1.0,0.1,false,"Joystick_1");
	//These are not assigned by default but can configured to use via xml preferences
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",true,1.0,0.1,true,"Joystick_1");
	//joy.AddJoy_Button_Default(6,"Slide",false);
	//joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Rot,"Analog_StrafeRight");

	#ifdef __UsingXTerminator__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Rot,"Arm_SetCurrentVelocity",false,1.0,0.04,true,"Joystick_1");
	joy.AddJoy_Button_Default(6,"Arm_SetPos0feet",false);
	joy.AddJoy_Button_Default(5,"Arm_SetPos3feet",false);
	joy.AddJoy_Button_Default(4,"Arm_SetPos6feet",false);
	joy.AddJoy_Button_Default(8,"Arm_SetPos9feet",false);
	#endif
	#ifdef __UsingWPTH_UI__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Arm_SetCurrentVelocity",false,1.0,0.04,true,"Joystick_2");
	//joy.AddJoy_Button_Default(0,"Arm_SetPos0feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default(0,"Arm_Claw",true,false,"Joystick_2");
	//Not sure why the simulator skipped 1
	joy.AddJoy_Button_Default(2,"Robot_OpenDoor",true,false,"Joystick_2");
	joy.AddJoy_Button_Default(3,"Arm_SetPos0feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default(4,"Arm_SetPos9feet",false,false,"Joystick_2");
	#endif
	#ifdef __WindRiverJoysticks__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Arm_SetCurrentVelocity",false,1.0,0.04,true,"Joystick_2");
	joy.AddJoy_Button_Default( 5,"Arm_SetPos0feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 6,"Arm_SetPos3feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default(10,"Arm_SetPos6feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 9,"Arm_SetPos9feet",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 0,"Arm_Claw",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 7,"Robot_OpenDoor",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 8,"Robot_ReleaseLazySusan",true,false,"Joystick_2");
	#endif
	Init_AutoPilotControls();
}

void UI_Controller::Flush_AI_BaseResources()
{
	m_ship=NULL; //we don't own this
}

UI_Controller::~UI_Controller()
{
	Flush_AI_BaseResources();
}

void UI_Controller::Set_AI_Base_Controller(AI_Base_Controller *controller)
{
	//destroy all resources associated with the previous ship
	if (m_Base)
	{
		Framework::Base::EventMap* em = m_ship->GetEventMap();
		em->EventOnOff_Map["RequestAfterburner"].Remove(*this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Remove(*this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Remove(*this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Remove(*this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Remove(*this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Remove(*this, &UI_Controller::Turn_L);
		em->Event_Map["UserResetPos"].Remove(*this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Remove(*this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Remove(*this, &UI_Controller::ToggleSlide);
		em->EventOnOff_Map["StrafeLeft"].Remove(*this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Remove(*this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["SPAWN"].Remove(*this, &UI_Controller::OnSpawn);
		//em->Event_Map["UseMouse"].Remove(*this, &UI_Controller::UseMouse);
		em->EventOnOff_Map["Test1"].Remove(*this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Remove(*this, &UI_Controller::Test2);
		//em->Event_Map["ShowHUD"].Remove(*m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);
		em->EventValue_Map["BLACKOUT"].Remove(*this, &UI_Controller::BlackoutHandler);
		em->EventValue_Map["Analog_Turn"].Remove(*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_Slider_Accel"].Remove(*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed);
		em->EventValue_Map["Joystick_SetCurrentSpeed_2"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed_2);
		m_ship->BindAdditionalEventControls(false);
		Flush_AI_BaseResources();
	}
	m_Base=controller;
	if (m_Base)
	{
		m_ship=&m_Base->m_ship;
		Framework::Base::EventMap* em = m_ship->GetEventMap();

		em->EventOnOff_Map["RequestAfterburner"].Subscribe(ehl, *this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Subscribe(ehl, *this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Subscribe(ehl, *this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Subscribe(ehl, *this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Subscribe(ehl, *this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Subscribe(ehl, *this, &UI_Controller::Turn_L);
		em->Event_Map["UserResetPos"].Subscribe(ehl, *this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Subscribe(ehl, *this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Subscribe(ehl, *this, &UI_Controller::ToggleSlide);
		em->EventOnOff_Map["StrafeLeft"].Subscribe(ehl, *this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Subscribe(ehl, *this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Subscribe(ehl, *this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["SPAWN"].Subscribe(ehl, *this, &UI_Controller::OnSpawn);

		//em->Event_Map["UseMouse"].Subscribe(ehl, *this, &UI_Controller::UseMouse);
		em->EventOnOff_Map["Test1"].Subscribe(ehl, *this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Subscribe(ehl, *this, &UI_Controller::Test2);

		// And a button for the Speed PDCB
		//em->Event_Map["ShowHUD"].Subscribe(m_HUD_UI->ehl, *m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);

		// Listen for blackout
		em->EventValue_Map["BLACKOUT"].Subscribe(ehl, *this, &UI_Controller::BlackoutHandler);

		em->EventValue_Map["Analog_Turn"].Subscribe(ehl,*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_StrafeRight"].Subscribe(ehl,*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_Slider_Accel"].Subscribe(ehl,*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed);
		em->EventValue_Map["Joystick_SetCurrentSpeed_2"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed_2);

		// Tell the HUD the name of this ship
		//m_HUD_UI->m_addnText = m_ship->GetName();

		m_ship->BindAdditionalEventControls(true);
	}
}

void UI_Controller::Test1(bool on)
{
	m_Test1=on;
}
void UI_Controller::Test2(bool on)
{
	m_Test2=on;
}

void UI_Controller::Ship_AfterBurner_Thrust(bool on)	
{		
	// Touching the Afterburner always places us back in SImFLight mode
	m_ship->SetSimFlightMode(true);

	// Set the current requested speed for the ship based on whether we are turning afterburner on or off
	if (on)
		m_ship->FireAfterburner();
	else
	{
		// Set the requested speed to our current speed or at the max engaged speed (Afterburner Brake)
		//Vec2d LocalVelocity(m_ship->GetAtt_quat().conj() * m_ship->GetPhysics().GetLinearVelocity());
		Vec2d LocalVelocity=GlobalToLocal(m_ship->GetAtt_r(),m_ship->GetPhysics().GetLinearVelocity());
		double currSpeed = LocalVelocity[1];
		m_ship->SetRequestedVelocity(MIN(currSpeed, m_ship->GetEngaged_Max_Speed()));
	}
}

void UI_Controller::Ship_Thrust(bool on)
{	
	if (on)
		m_Ship_Keyboard_currAccel[1] = m_ship->GetAccelSpeed();
	else
		m_Ship_Keyboard_currAccel[1] = 0.0;
}

void UI_Controller::Ship_Brake(bool on)
{	
	if (on)
		m_Ship_Keyboard_currAccel[1] = -m_ship->GetBrakeSpeed();
	else
		m_Ship_Keyboard_currAccel[1] = 0.0;
}

void UI_Controller::Ship_Thrust(double Intensity)
{	
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[1] = m_ship->GetAccelSpeed()*Intensity;
}

void UI_Controller::Ship_Brake(double Intensity)
{	
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[1] = -m_ship->GetBrakeSpeed()*Intensity;
}

void UI_Controller::Ship_StrafeLeft(double Intensity)	
{		
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[0]=  -m_ship->GetStrafeSpeed()*Intensity;	
}
void UI_Controller::Ship_StrafeRight(double Intensity)	
{
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[0]=  m_ship->GetStrafeSpeed()*Intensity;	
}

void UI_Controller::Ship_Turn(double dir,bool UseHeadingSpeed) 
{
	if (fabs(dir)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_UseHeadingSpeed=UseHeadingSpeed;
	m_Ship_JoyMouse_rotAcc_rad_s=(UseHeadingSpeed?dir*m_ship->GetHeadingSpeed():dir)*m_ship->GetCameraRestraintScaler();
}

void UI_Controller::Ship_Turn(Directions dir)
{
	m_Ship_Keyboard_rotAcc_rad_s=(double)dir*m_ship->GetHeadingSpeed()*m_ship->GetCameraRestraintScaler();
	m_Ship_UseHeadingSpeed=true;
}


void UI_Controller::CancelAllControls()
{
	// Turns off all controls that might be on
	AfterBurner_Thrust(false);
	Thrust(false);
	Brake(false);
	Turn_L(false);
	Turn_R(false);
	StrafeLeft(false);
	StrafeRight(false);
	Test1(false);
	Test2(false);
	TryFireMainWeapon(false);
}

void UI_Controller::TryToggleAutoPilot()
{
	SetAutoPilot(!GetAutoPilot());
}

bool UI_Controller::SetAutoPilot(bool autoPilot)
{
	// Note that Autopilot MUST be on if the ship is not being controlled
	m_autoPilot = autoPilot || !m_isControlled || !m_Base->GetCanUserPilot();

	// When in autopilot, always use mouse POV
	//ToggleMousePOV(m_autoPilot);

	// If we are in auto-pilot, we MUST set simulated flight mode if we have an auto-pilot route
	if (m_autoPilot)
	{
		bool hasAutoPilotRoute = m_Base->HasAutoPilotRoute();
		m_ship->SetStabilizeRotation(hasAutoPilotRoute);
		m_ship->SetSimFlightMode(hasAutoPilotRoute);
	}
	else
	{
		m_ship->SetStabilizeRotation(true);
		m_ship->SetSimFlightMode(true);
	}

	// When turning on or OFF the auto pilot, stop firing and other actions
	m_ship->CancelAllControls();

	return m_autoPilot;
}

void UI_Controller::UserResetPos()
{
	if (AreControlsDisabled())
		return;
	m_ship->ResetPos();
}

void UI_Controller::ResetPos()
{
	m_Ship_Keyboard_rotAcc_rad_s =	m_Ship_JoyMouse_rotAcc_rad_s = 0.0;
	m_Ship_Keyboard_currAccel = m_Ship_JoyMouse_currAccel =	Vec2d(0,0);

	//m_HUD_UI->Reset();
	//TODO see where ResetPos is called within the game, as m_Base does not have a check
	if (m_Base)
		m_Base->ResetPos();
}

void UI_Controller::BlackoutHandler(double bl)
{
	// Turn STRAFE on the ship to pull out of the blackout
	if ((bl != 0.0) && (!m_autoPilot))
		m_ship->SetSimFlightMode(false);
}

void UI_Controller::Mouse_Turn(double dir)
{
	Ship_Turn(dir,false);
}

//Note: XTerminator filter = 0.01, and is squared have been transferred to default settings
void UI_Controller::JoyStick_Ship_Turn(double dir)
{
	if (AreControlsDisabled())
		return;

	Ship_Turn(dir);
}

void UI_Controller::Slider_Accel(double Intensity)
{
	if (AreControlsDisabled())
		return;

	if (Intensity>0.0)
		Ship_Thrust(Intensity);
	else if (Intensity<0.0)
		Ship_Brake(-Intensity);
}

void UI_Controller::Joystick_SetCurrentSpeed(double Speed)
{
	if (m_Ship_Keyboard_currAccel[1]==0.0)
	{
		if (m_ship->GetAlterTrajectory())
		{
			if ((fabs(Speed-m_LastSliderTime[1])>0.05)||(Speed==0))
			{
				double SpeedToUse=m_ship->GetIsAfterBurnerOn()?m_ship->GetMaxSpeed():m_ship->GetEngaged_Max_Speed();
				//This works but I really did not like the feel of it
				double SpeedCalibrated=((Speed/2.0)+0.5)*SpeedToUse;
				m_LastSliderTime[1]=Speed;
				if (SpeedCalibrated!=m_CruiseSpeed)
				{
					m_ship->SetRequestedVelocity(SpeedCalibrated);
					m_CruiseSpeed=SpeedCalibrated;
				}
			}
		}
		else
			Ship_Thrust(0.0);
	}
}

void UI_Controller::Joystick_SetCurrentSpeed_2(double Speed)
{
	if (m_Ship_Keyboard_currAccel[1]==0.0)
	{
		if (m_ship->GetAlterTrajectory())
		{
			if ((fabs(Speed-m_LastSliderTime[1])>0.05)||(Speed==0))
			{
				double SpeedToUse=m_ship->GetIsAfterBurnerOn()?m_ship->GetMaxSpeed():m_ship->GetEngaged_Max_Speed();
				//This works but I really did not like the feel of it
				double SpeedCalibrated=Speed*SpeedToUse;
				m_LastSliderTime[1]=Speed;
				if (SpeedCalibrated!=m_CruiseSpeed)
				{
					m_ship->SetRequestedVelocity(SpeedCalibrated);
					m_CruiseSpeed=SpeedCalibrated;
				}
			}
		}
		else
			Ship_Thrust(0.0);
	}
}


bool UI_Controller::AreControlsDisabled()
{
	//return (m_autoPilot || !m_ship->IsShowing());
	return (m_autoPilot );
}

void UI_Controller::UpdateController(double dTime_s)
{
	if (AreControlsDisabled())
	{
		if (m_Base->m_Goal)
			m_Base->m_Goal->Process(dTime_s);
	}

	if (m_isControlled)
	{
		// Update Mouse Controller (This is ONLY allowed to update the POV in auto pilot)
		//m_mouseDriver->DriveShip();

		//Now for the ship
		if (!AreControlsDisabled())
		{
			// Normally we pass the the ship the addition of the keyboard and mouse accel
			Vec2d shipAccel = m_Ship_Keyboard_currAccel+m_Ship_JoyMouse_currAccel;

			// apply various input sources to current acceleration
			m_ship->SetCurrentLinearAcceleration(shipAccel); 
			
			//flush the JoyMouse current acceleration vec2 since it works on an additive nature
			m_Ship_JoyMouse_currAccel=Vec2d(0.0,0.0);
		
		
			//add all the various input types to the main rotation velocity
			double rotAcc=(m_Ship_Keyboard_rotAcc_rad_s+m_Ship_JoyMouse_rotAcc_rad_s);
			//We may have same strange undesired flicker effect if the mouse and keyboard turns are used simultaneously! So if the keyboard is used, then
			//the mouse will get ignored
			bool LockShipHeadingToOrientation=m_Ship_UseHeadingSpeed;

			if (m_Ship_Keyboard_rotAcc_rad_s!=0) 
			{
				LockShipHeadingToOrientation=true;
				rotAcc=m_Ship_Keyboard_rotAcc_rad_s;
				m_Ship_UseHeadingSpeed=true;
			}

			m_ship->SetCurrentAngularAcceleration(rotAcc,LockShipHeadingToOrientation);

			//flush the JoyMouse rotation acceleration since it works on an additive nature
			m_Ship_JoyMouse_rotAcc_rad_s=0.0;
			m_Ship_JoyMouse_currAccel=Vec2d(0.0,0.0);
		}
	}
}

// enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
const char* ThrustStateNames[] = {
	"Ship.AfterBurner_Brake", "Ship.Brake", "Ship.Coast", "Ship.Thrust", "Ship.AfterBurner", "Ship.NotVisible"
};

void UI_Controller::UpdateUI(double dTime_s)
{
	//No work to do if there is no ship to control
	if (!m_Base)
		return;
	// Positions:
	//[0]  +Right -Left
	//[1]  +Forward -Reverse
	//[2]  +Up -Down
	{
		#if 0
		Vec2d pos=m_ship->GetPos_m();
		DOUT1("x=%f y=%f r=%f",pos[0],pos[1],m_ship->GetAtt_r()*180.0/M_PI);
		DOUT3("Speed=%f mode=%s",m_ship->GetPhysics().GetLinearVelocity().length(),m_ship->GetAlterTrajectory()?"Sim":"Slide");
		#endif
		//DebugOut_PDCB::TEXT2 = BuildString("%s", ThrustStateNames[m_ship->GetThrustState()]);
	}
}

//////////////////////////////////////////////////////////////////////////

void UI_Controller::HookUpUI(bool ui)
{
	m_isControlled = ui;
	//GG_Framework::UI::MainWindow& mainWin = *GG_Framework::UI::MainWindow::GetMainWindow();
	if (m_isControlled)
	{
		//Note: This is handled explicitly in the main; however, I may want to add a member here... probably not since I do not wish to deviate from
		//the original source
		//mainWin.GetJoystick().SetControlledEventMap(m_ship->GetEventMap());
	}

	if (m_ship)
	{
		// Set Auto Pilot to true if there is control
		SetAutoPilot(!m_isControlled || !m_Base->GetCanUserPilot());
	}
}
//////////////////////////////////////////////////////////////////////////

void UI_Controller::OnSpawn(bool on)
{
	// Set Auto Pilot to true if there is control.
	// This is commented out for now so we can leave a ship in auto-pilot and it will return to auto-pilot after spawn
	// SetAutoPilot(!m_isControlled || !m_Base->GetCanUserPilot());
}

