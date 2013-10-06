#include "stdafx.h"
#include "../FrameWork.h"

using namespace FrameWork;
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
const double PI_2 = 1.57079632679489661923;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*														LUA_Controls_Properties														*/
/***********************************************************************************************************************************/

LUA_Controls_Properties::LUA_Controls_Properties(LUA_Controls_Properties_Interface *parent) : m_pParent(parent)
{
}

LUA_Controls_Properties &LUA_Controls_Properties::operator= (const LUA_Controls_Properties &CopyFrom)
{
	m_Controls=CopyFrom.m_Controls;
	//Note: this one probably requires a static interface
	m_pParent=CopyFrom.m_pParent;
	return *this;
}

const char *LUA_Controls_Properties::ExtractControllerElementProperties(Controller_Element_Properties &Element,const char *Eventname,Scripting::Script& script)
{
	const char *err=NULL;
	err = script.GetFieldTable(Eventname);
	if (!err)
	{
		Element.Event=Eventname;
		std::string sType;
		err = script.GetField("type",&sType,NULL,NULL);
		ASSERT_MSG(!err, err);
		
		if (strcmp(sType.c_str(),"joystick_analog")==0)
		{
			Element.Type=Controller_Element_Properties::eJoystickAnalog;
			JoyAxis_enum JoyAxis;
			double dJoyAxis;
			err = script.GetField("key", NULL, NULL,&dJoyAxis);
			ASSERT_MSG(!err, err);
			//cast to int first, and then to the enumeration
			JoyAxis=(JoyAxis_enum)((int)dJoyAxis);
			bool IsFlipped;
			err = script.GetField("is_flipped", NULL, &IsFlipped,NULL);
			ASSERT_MSG(!err, err);
			double Multiplier;
			err = script.GetField("multiplier", NULL, NULL,&Multiplier);
			ASSERT_MSG(!err, err);
			double FilterRange;
			err = script.GetField("filter", NULL, NULL,&FilterRange);
			ASSERT_MSG(!err, err);
			double CurveIntensity;
			err = script.GetField("curve_intensity", NULL, NULL, &CurveIntensity);
			ASSERT_MSG(!err, err);

			Controller_Element_Properties::ElementTypeSpecific::AnalogSpecifics_rw &set=Element.Specifics.Analog;
			set.JoyAxis=JoyAxis;
			set.IsFlipped=IsFlipped;
			set.Multiplier=Multiplier;
			set.FilterRange=FilterRange;
			set.CurveIntensity=CurveIntensity;
			//joy.AddJoy_Analog_Default(JoyAxis,Eventname,IsFlipped,Multiplier,FilterRange,IsSquared,ProductName.c_str());
		}
		else if (strcmp(sType.c_str(),"joystick_culver")==0)
		{
			Element.Type=Controller_Element_Properties::eJoystickCulver;
			JoyAxis_enum JoyAxis_X,JoyAxis_Y;
			double dJoyAxis;
			err = script.GetField("key_x", NULL, NULL,&dJoyAxis);
			ASSERT_MSG(!err, err);
			//cast to int first, and then to the enumeration
			JoyAxis_X=(JoyAxis_enum)((int)dJoyAxis);

			err = script.GetField("key_y", NULL, NULL,&dJoyAxis);
			ASSERT_MSG(!err, err);
			//cast to int first, and then to the enumeration
			JoyAxis_Y=(JoyAxis_enum)((int)dJoyAxis);

			bool IsFlipped;
			err = script.GetField("is_flipped", NULL, &IsFlipped,NULL);
			ASSERT_MSG(!err, err);
			double Magnitude_Scalar_Arc;
			err = script.GetField("magnitude_scalar_arc", NULL, NULL,&Magnitude_Scalar_Arc);
			if (err)
				Magnitude_Scalar_Arc=1.0/PI_2;  //this is a great default for game controllers that have 1.0 intensity at the corners
			double Magnitude_Scalar_Base;
			err = script.GetField("magnitude_scalar_base", NULL, NULL,&Magnitude_Scalar_Base);
			if (err)
				Magnitude_Scalar_Base=1.0/PI_2;  //this is a great default for all (may need to be slightly tweaked to get perfect on some controllers)

			double Multiplier;
			err = script.GetField("multiplier", NULL, NULL,&Multiplier);
			ASSERT_MSG(!err, err);
			double FilterRange;
			err = script.GetField("filter", NULL, NULL,&FilterRange);
			ASSERT_MSG(!err, err);
			double CurveIntensity;
			err = script.GetField("curve_intensity", NULL, NULL, &CurveIntensity);
			ASSERT_MSG(!err, err);

			Controller_Element_Properties::ElementTypeSpecific::CulverSpecifics_rw &set=Element.Specifics.Culver;
			set.JoyAxis_X=JoyAxis_X,set.JoyAxis_Y=JoyAxis_Y;
			set.MagnitudeScalarArc=Magnitude_Scalar_Arc;
			set.MagnitudeScalarBase=Magnitude_Scalar_Base;
			set.IsFlipped=IsFlipped;
			set.Multiplier=Multiplier;
			set.FilterRange=FilterRange;
			set.CurveIntensity=CurveIntensity;
			//joy.AddJoy_Culver_Default(JoyAxis_X,JoyAxis_Y,Magnitude_Scalar,Eventname,IsFlipped,Multiplier,FilterRange,CurveIntensity,ProductName.c_str());
		}
		else if (strcmp(sType.c_str(),"joystick_button")==0)
		{
			Element.Type=Controller_Element_Properties::eJoystickButton;
			size_t WhichButton,WhichKey;
			double dWhichButton;
			err = script.GetField("key", NULL, NULL,&dWhichButton);
			ASSERT_MSG(!err, err);

			std::string stringWhichButton;
			err = script.GetField("keyboard",&stringWhichButton, NULL, NULL);
			if (!err)
				WhichKey=stringWhichButton.c_str()[0];
			else
				WhichKey=-1;

			//cast to int first, and then to the enumeration; The -1 allows for cardinal types (good since we can use numbers written on button)
			WhichButton=(JoyAxis_enum)((int)dWhichButton-1);
			bool useOnOff;
			err = script.GetField("on_off", NULL, &useOnOff,NULL);
			ASSERT_MSG(!err, err);
			bool dbl_click=false;
			err = script.GetField("dbl", NULL, &dbl_click,NULL); //This one can be blank
			err=NULL;  //don't return an error (assert for rest)

			Controller_Element_Properties::ElementTypeSpecific::ButtonSpecifics_rw &set=Element.Specifics.Button;
			set.WhichButton=WhichButton;
			set.WhichKey=WhichKey;
			set.useOnOff=useOnOff;
			set.dbl_click=dbl_click;
			//joy.AddJoy_Button_Default( WhichButton,Eventname,useOnOff,dbl_click,ProductName.c_str());
		}
		else if (strcmp(sType.c_str(),"keyboard")==0)
		{
			Element.Type=Controller_Element_Properties::eKeyboard;
			size_t WhichKey;
			std::string stringWhichButton;
			err = script.GetField("key",&stringWhichButton, NULL, NULL);
			ASSERT_MSG(!err, err);
			//cast to int first, and then to the enumeration; The -1 allows for cardinal types (good since we can use numbers written on button)
			WhichKey=stringWhichButton.c_str()[0];
			bool useOnOff;
			err = script.GetField("on_off", NULL, &useOnOff,NULL);
			ASSERT_MSG(!err, err);
			bool dbl_click=false;
			err = script.GetField("dbl", NULL, &dbl_click,NULL); //This one can be blank
			err=NULL;  //don't return an error (assert for rest)

			Controller_Element_Properties::ElementTypeSpecific::KeyboardSpecifics_rw &set=Element.Specifics.Keyboard;
			set.WhichKey=WhichKey;
			set.useOnOff=useOnOff;
			set.dbl_click=dbl_click;
		}
		else assert(false);
		script.Pop();
	}
	return err;
}

void LUA_Controls_Properties::LoadFromScript(Scripting::Script& script)
{
	//ensure the list is clean (incase it gets called again)
	m_Controls.clear();

	const char* err=NULL;
	//Note i is cardinal (more readable in LUA)
	size_t i=1,j=0;
	std::string Controls;
	const char * Events;
	char Buffer[4];
	while ( Controls="Joystick_",Controls+=itoa(i++,Buffer,10) ,	(err = script.GetFieldTable(Controls.c_str()))==NULL)
	{
		Control_Props control;
		//Wind River uses generic name, and AI tester uses product name
		#ifndef Robot_TesterCode
		control.Controller=Controls.c_str();
		#else
		err=script.GetField("control", &control.Controller, NULL, NULL);
		#endif
		//ensure the controller is lower case
		std::transform(control.Controller.begin(),control.Controller.end(),control.Controller.begin(),tolower);
		j=0;
		while ( Events=m_pParent->LUA_Controls_GetEvents(j++) , Events)
		{
			Controller_Element_Properties element;
			err=ExtractControllerElementProperties(element,Events,script);
			if (!err)
				control.EventList.push_back(element);
		}
		m_Controls.push_back(control);
		script.Pop();
	}
}

void LUA_Controls_Properties::BindAdditionalUIControls(bool Bind,void *joy,void *key) const
{
	typedef UI::JoyStick_Binder JoyStick_Binder;
	typedef UI::KeyboardMouse_CB Keyboard_Binder;
	JoyStick_Binder *p_joy=(JoyStick_Binder *)joy;
	Keyboard_Binder *p_key=(Keyboard_Binder *)key;
	const Controls_List &controls=Get_Controls();
	for (size_t i=0;i<controls.size();i++)
	{
		const Control_Props &control=controls[i];

		for (size_t j=0;j<control.EventList.size();j++)
		{
			const Controller_Element_Properties &element=control.EventList[j];
			switch (element.Type)
			{
			case Controller_Element_Properties::eJoystickAnalog:
				if (Bind)
				{
					const Controller_Element_Properties::ElementTypeSpecific::AnalogSpecifics_rw &analog=element.Specifics.Analog;
					//Note the cast... these are not going to change, but there is dup code to on axis enum to avoid dependency issues
					p_joy->AddJoy_Analog_Default((JoyStick_Binder::JoyAxis_enum)analog.JoyAxis,element.Event.c_str(),analog.IsFlipped,analog.Multiplier,
						analog.FilterRange,analog.CurveIntensity,control.Controller.c_str());
				}
				else
					p_joy->RemoveJoy_Analog_Binding(element.Event.c_str(),control.Controller.c_str());
				break;
			case Controller_Element_Properties::eJoystickCulver:
				if (Bind)
				{
					const Controller_Element_Properties::ElementTypeSpecific::CulverSpecifics_rw &analog=element.Specifics.Culver;
					//Note the cast... these are not going to change, but there is dup code to on axis enum to avoid dependency issues
					p_joy->AddJoy_Culver_Default((JoyStick_Binder::JoyAxis_enum)analog.JoyAxis_X,(JoyStick_Binder::JoyAxis_enum)analog.JoyAxis_Y,analog.MagnitudeScalarArc,
						analog.MagnitudeScalarBase,element.Event.c_str(),analog.IsFlipped,analog.Multiplier,analog.FilterRange,analog.CurveIntensity,control.Controller.c_str());
				}
				else
					p_joy->RemoveJoy_Analog_Binding(element.Event.c_str(),control.Controller.c_str());
				break;
			case Controller_Element_Properties::eJoystickButton:
				if (Bind)
				{
					const Controller_Element_Properties::ElementTypeSpecific::ButtonSpecifics_rw &button=element.Specifics.Button;
					p_joy->AddJoy_Button_Default(button.WhichButton,element.Event.c_str(),button.useOnOff,button.dbl_click,control.Controller.c_str());
					if ((p_key) && (button.WhichKey!=-1))
					{
						if (Bind)
							p_key->AddKeyBindingR(button.useOnOff,element.Event.c_str(),button.WhichKey);
						else
							p_key->RemoveKeyBinding(button.WhichKey,element.Event.c_str(),button.useOnOff);
					}
				}
				else
					p_joy->RemoveJoy_Button_Binding(element.Event.c_str(),control.Controller.c_str());
				break;
			case Controller_Element_Properties::eKeyboard:
				if (p_key)
				{
					const Controller_Element_Properties::ElementTypeSpecific::KeyboardSpecifics_rw &button=element.Specifics.Keyboard;
					if (Bind)
						p_key->AddKeyBindingR(button.useOnOff,element.Event.c_str(),button.WhichKey);
					else
						p_key->RemoveKeyBinding(button.WhichKey,element.Event.c_str(),button.useOnOff);
				}
				break;
			}
		}
	}
}
