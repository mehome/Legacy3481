#pragma once

class UI_Controller
{
	public:
		enum Directions
		{
			Dir_None = 0,
			//Reversed from game since the 2D world uses the topview axis
			Dir_Left = -1,
			Dir_Right = 1,
		};

		UI_Controller(Framework::UI::JoyStick_Binder &joy,AI_Base_Controller *base_controller=NULL);
		virtual ~UI_Controller();

		///This is the most important method, as we must have a ship to control
		void Set_AI_Base_Controller(AI_Base_Controller *controller);
		void TryFireMainWeapon(bool on)
		{
			if (AreControlsDisabled() && on) return;
			//m_ship->GetEventMap()->EventOnOff_Map["Ship.TryFireMainWeapon"].Fire(on);
		}

		void AfterBurner_Thrust(bool on){if (AreControlsDisabled() && on) return; Ship_AfterBurner_Thrust(on);}
		void Thrust(bool on){if (AreControlsDisabled() && on) return; Ship_Thrust(on);}
		void Slider_Accel(double Intensity);
		void Brake(bool on){if (AreControlsDisabled() && on) return; Ship_Brake(on);}
		void Stop() {if (AreControlsDisabled()) return; m_ship->Stop();}
		void MatchSpeed(double speed) {if (AreControlsDisabled()) return; m_ship->SetRequestedVelocity(speed);}
		void Turn_R(bool on){if (AreControlsDisabled() && on) return; Ship_Turn(on?Dir_Right:Dir_None);}
		void Turn_L(bool on){if (AreControlsDisabled() && on) return; Ship_Turn(on?Dir_Left:Dir_None);}
		virtual void ResetPos();
		void UserResetPos();
		void ToggleSlide() {if (AreControlsDisabled()) return; m_ship->SetSimFlightMode(!m_ship->GetAlterTrajectory());}
		void ToggleAutoLevel();
		void StrafeLeft(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeLeft(on);}
		void StrafeLeft(double dir) {if (!AreControlsDisabled()) Ship_StrafeLeft(dir); }
		void StrafeRight(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeRight(on);}
		void StrafeRight(double dir) {if (!AreControlsDisabled()) Ship_StrafeRight(dir); }

		void TryToggleAutoPilot();

		// This may not always happen.  Some ships can ONLY be in auto pilot
		bool SetAutoPilot(bool autoPilot);
		bool GetAutoPilot(){return m_autoPilot;}

		void OnSpawn(bool on);

		void UseMouse();
		void Test1(bool on);
		void Test2(bool on);

		//osg::Geometry* MakeVelLine(osg::Vec2 vel);
		void HookUpUI(bool ui);
		bool GetControlled(){return m_isControlled;}

		///This is called from the ship's time change (first before the ship does its physics)
		virtual void UpdateController(double dTime_s);
		void UpdateUI(double dTime_s);

		virtual void CancelAllControls();

	protected:
		void BlackoutHandler(double bl);

		///All non-right button mouse movements will come here to be dispatched to the proper place
		void Mouse_Turn(double dir);

		/// \param dir When UseHeadingSpeed=true this is a velocity scaler applied to the scripted turning speed.
		/// When UseHeadingSpeed=false the dir parameter becomes the actual velocity in radians per second.
		/// \param UseHeadingSpeed Typically the Joystick and Keyboard will set UseHeadingSpeed to true, and mouse will be false
		/// when this is true it also implicitly applies the ships torque restraints to the intended speed.
		/// \todo implement these to work with intended orientation quaternion.
		void Ship_Turn(double dir,bool UseHeadingSpeed=true);

		void JoyStick_Ship_Turn(double dir);

		void Ship_Turn(Directions dir);

		void Ship_AfterBurner_Thrust(bool on);
		void Ship_Thrust(bool on);
		void Ship_Brake(bool on);

		void Ship_Thrust(double Intensity);
		void Ship_Brake(double Intensity);

		void Joystick_SetCurrentSpeed(double Speed);
		void Joystick_SetCurrentSpeed_2(double Speed);

		void Ship_StrafeLeft(bool on)	{		m_Ship_Keyboard_currAccel[0]= on? -m_ship->GetStrafeSpeed() : 0.0;	}
		void Ship_StrafeRight(bool on)	{		m_Ship_Keyboard_currAccel[0]= on? m_ship->GetStrafeSpeed() : 0.0;	}

		void Ship_StrafeLeft(double Intensity);
		void Ship_StrafeRight(double Intensity);

		IEvent::HandlerList ehl;
	private:
		void Flush_AI_BaseResources(); //i.e. chase plane and mouse driver
		void Init_AutoPilotControls();
		AI_Base_Controller *m_Base;
		Ship_2D *m_ship; //there is an overwhelming use of the ship so we'll cache a pointer of it here

		double m_LastSliderTime[2]; //Keep track of the slider to help it stay smooth;
		bool m_SlideButtonToggle;
		bool m_isControlled;

		///This is used exclusively for keyboard turn methods
		double m_Ship_Keyboard_rotAcc_rad_s;
		///This one is used exclusively for the Joystick and Mouse turn methods
		double m_Ship_JoyMouse_rotAcc_rad_s;
		Framework::Base::Vec2d m_Ship_Keyboard_currAccel,m_Ship_JoyMouse_currAccel;
		double m_CruiseSpeed; ///< This is used with the Joystick control to only apply speed changes when a change occurs

		//I have to monitor when it is down then up
		bool m_FireButton;

		// Are we flying in auto-pilot?
		bool m_autoPilot;
		bool m_enableAutoLevelWhenPiloting;

		// Are we disabling UI controls?
		bool AreControlsDisabled();

		bool m_Test1,m_Test2; //Testing
		bool m_Ship_UseHeadingSpeed;
};

