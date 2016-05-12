#include "Base_Includes.h"
#include "misc.h"
#include "Event.h"
#include "EventMap.h"
#include "Joystick.h"
#include "JoystickBinder.h"

using namespace Framework::UI;
using namespace Framework::Base;
using namespace Framework;

const double DOUBLE_CLICK_TIME = 0.25;

  /***************************************************************************************************************/
 /*											JoyStick_Binder														*/
/***************************************************************************************************************/

JoyStick_Binder::JoyStick_Binder(IJoystick &joystick) :  m_Joystick(joystick),m_controlledEventMap(NULL), m_eventTime(0.0)
{
	memset(m_lastReleaseTime,0,sizeof(m_lastReleaseTime));
}

void JoyStick_Binder::SetControlledEventMap(Framework::Base::EventMap* em)
{
	if (m_controlledEventMap != em)
		m_controlledEventMap = em;
}

void JoyStick_Binder::AddJoy_Analog_Binding(JoyAxis_enum WhichAxis,const char eventName[],bool IsFlipped,double Multiplier,
															  double FilterRange,bool isSquared,const char ProductName[])
{
	
	Analog_EventEntry key(WhichAxis,ProductName,IsFlipped,Multiplier,FilterRange,isSquared);
	std::vector<std::string> *eventNames = m_JoyAnalogBindings[key];
	if (!eventNames)
	{
		eventNames = new std::vector<std::string>;
		eventNames->push_back(eventName);
		m_JoyAnalogBindings[key] = eventNames;
	}
	else
	{
		bool exists = false;
		std::vector<std::string>::iterator pos;
		for (pos = eventNames->begin(); pos != eventNames->end() && !exists; ++pos)
			exists = (eventName == *pos);
		if (!exists)
			eventNames->push_back(eventName);
	}

	std::vector<Analog_EventEntry> *keys = m_AssignedJoyAnalogs[eventName];
	if (!keys)
	{
		keys = new std::vector<Analog_EventEntry>;
		keys->push_back(key);
		m_AssignedJoyAnalogs[eventName] = keys;
	}
	else
	{
		bool exists = false;
		std::vector<Analog_EventEntry>::iterator pos;
		//Check for duplicate entries of the same key (This may be a typical case)
		for (pos = keys->begin(); pos != keys->end() && !exists; ++pos)
			exists = (key == *pos);
		if (!exists)
			keys->push_back(key);
	}
}

void JoyStick_Binder::AddJoy_Button_Binding(size_t WhichButton,const char eventName[],bool useOnOff,bool dbl_click,const char ProductName[])
{
	Button_EventEntry key(WhichButton,ProductName,useOnOff,dbl_click);
	std::vector<std::string>* eventNames = m_JoyButtonBindings[key];
	if (!eventNames)
	{
		eventNames = new std::vector<std::string>;
		eventNames->push_back(eventName);
		m_JoyButtonBindings[key] = eventNames;
	}
	else
	{
		bool exists = false;
		std::vector<std::string>::iterator pos;
		for (pos = eventNames->begin(); pos != eventNames->end() && !exists; ++pos)
			exists = (eventName == *pos);
		if (!exists)
			eventNames->push_back(eventName);
	}

	std::vector<Button_EventEntry> *keys = m_AssignedJoyButtons[eventName];
	if (!keys)
	{
		keys = new std::vector<Button_EventEntry>;
		keys->push_back(key);
		m_AssignedJoyButtons[eventName] = keys;
	}
	else
	{
		bool exists = false;
		std::vector<Button_EventEntry>::iterator pos;
		//Check for duplicate entries of the same key (This may be a typical case)
		for (pos = keys->begin(); pos != keys->end() && !exists; ++pos)
			exists = (key == *pos);
		if (!exists)
			keys->push_back(key);
	}
}


void JoyStick_Binder::AddJoy_Analog_Default(JoyAxis_enum WhichAxis,const char eventName[],bool IsFlipped,double Multiplier,
											double FilterRange,bool isSquared,const char ProductName[])
{
	//removed intercept since we are not using a config manager
	AddJoy_Analog_Binding(WhichAxis,eventName,IsFlipped,Multiplier,FilterRange,isSquared,ProductName);
}

void JoyStick_Binder::AddJoy_Button_Default(size_t WhichButton,const char eventName[],bool useOnOff,bool dbl_click,const char ProductName[])
{
	//removed intercept since we are not using a config manager
	AddJoy_Button_Binding(WhichButton,eventName,useOnOff,dbl_click,ProductName);
}

bool JoyStick_Binder::IsDoubleClicked(size_t i)
{
	return (m_eventTime-m_lastReleaseTime[i] < DOUBLE_CLICK_TIME);
}

void JoyStick_Binder::UpdateJoyStick(double dTick_s)
{
	if (!m_controlledEventMap)
		return;

	m_eventTime += dTick_s;

	for (size_t JoyNum=0;JoyNum<m_Joystick.GetNoJoysticksFound();JoyNum++)
	{
		//Poll this Joystick
		Base::IJoystick::JoyState joyinfo;
		const Base::IJoystick::JoystickInfo &joyinfo2=m_Joystick.GetJoyInfo(JoyNum);
		if (m_Joystick.read_joystick(JoyNum,joyinfo))
		{
			//Ensure we have a flood control entry now
			while (m_FloodControl.size()<=JoyNum)
				m_FloodControl.push_back(joyinfo);

			Base::IJoystick::JoyState &floodcontrol=m_FloodControl[JoyNum];
			//here's a quick test to see if we are working
			//printf("\r no=%d xaxis=%f yaxis=%f  button=0x%x             ",joyinfo.JoystickNumber,joyinfo.lX,joyinfo.lY,joyinfo.ButtonBank[0]);

			//Now to iterate each slider and button to fire events for them
			//first the sliders
			for (size_t i=0;i<eNoJoyAxis_Entries;i++)
			{
				double Value,OldValue;
				//This assumes that JoyAxis_enum and JoyCapFlag_enum have integrity... a bit fragile assumption but effective for performance
				bool IsSupported=((1<<i)&joyinfo2.JoyCapFlags)!=0;
				//Only fire events for axis that the joystick supports
				if (IsSupported)
				{
					switch(i)
					{
					case eX_Axis:	Value=joyinfo.lX,OldValue=floodcontrol.lX;	break;
					case eY_Axis:	Value=joyinfo.lY,OldValue=floodcontrol.lY;	break;
					case eZ_Axis:	Value=joyinfo.lZ,OldValue=floodcontrol.lZ;	break;
					case eX_Rot:	Value=joyinfo.lRx,OldValue=floodcontrol.lRx;	break;
					case eY_Rot:	Value=joyinfo.lRy,OldValue=floodcontrol.lRy;	break;
					case eZ_Rot:	Value=joyinfo.lRz,OldValue=floodcontrol.lRz;	break;
					case eSlider0:	Value=joyinfo.rgSlider[0],OldValue=floodcontrol.rgSlider[0];	break;
					case eSlider1:	Value=joyinfo.rgSlider[1],OldValue=floodcontrol.rgSlider[1];	break;
					case ePOV_0:	Value=joyinfo.rgPOV[0],OldValue=floodcontrol.rgPOV[0];		break;
					case ePOV_1:	Value=joyinfo.rgPOV[1],OldValue=floodcontrol.rgPOV[1];		break;
					case ePOV_2:	Value=joyinfo.rgPOV[2],OldValue=floodcontrol.rgPOV[2];		break;
					case ePOV_3:	Value=joyinfo.rgPOV[3],OldValue=floodcontrol.rgPOV[3];		break;
					};

					//Note: at this time I cannot perform flood control for axis controls because of the nature of how
					//the client code needs a constant update on the position (as it will zero out afterward)
					//For now we'll abide by these requirements, but I may want to evaluate that and see if we can 
					//support flood control for this
					//We need not worry about a false positive we can rest assured they will be equal if they are
					//if (Value!=OldValue)
					{
						//I'm writing this out long hand so that I can access my original key (grabbing its contents)
						//std::vector<std::string> *AnalogEvents=GetBindingsForJoyAnalog((JoyAxis_enum)i);

						JoyAnalogBindings::iterator iter;
						{
							//First try to find the product specific binding
							Analog_EventEntry key((JoyAxis_enum)i,joyinfo2.ProductName.c_str());
							iter=m_JoyAnalogBindings.find(key);
						}

						//If the product is not found (typical) try to find any
						if (iter==m_JoyAnalogBindings.end())
							iter=m_JoyAnalogBindings.find((JoyAxis_enum)i);

						//If we found any events for this axis
						if (iter!=m_JoyAnalogBindings.end())
						{
							std::vector<std::string> *AnalogEvents=(*iter).second;
							const Analog_EventEntry &key=(*iter).first;

							if (AnalogEvents)
							{
								//Now to use the attributes to tweak the value
								//First evaluate dead zone range... if out of range subtract out the offset for no loss in precision
								//The /(1.0-filter range) will restore the full range
								
								double Temp=fabs(Value); //take out the sign... put it back in the end
								Temp=(Temp>=key.FilterRange) ? Temp-key.FilterRange:0.0; 

								Temp=key.Multiplier*(Temp/(1.0-key.FilterRange)); //apply scale first then 
								if (key.isSquared) Temp*=Temp;  //square it if it is squared

								//Now to restore the sign
								Value=(Value<0.0)?-Temp:Temp;
	
								std::vector<std::string>::iterator pos;
								for (pos = AnalogEvents->begin(); pos != AnalogEvents->end(); ++pos)
									m_controlledEventMap->EventValue_Map[*pos].Fire(key.IsFlipped?-Value:Value);
							}
						}
					}
				}
			}
			//Now the buttons
			for (size_t i=0;i<32;i++)
			{
				//Flood control check
				if ((joyinfo.ButtonBank[0] ^ floodcontrol.ButtonBank[0]) & (1<<i))
				{
					bool IsPressed=((1<<i) & joyinfo.ButtonBank[0])!=0;

					//I'm writing this out long hand so that I can access my original key (grabbing its contents)
					//std::vector<std::string> *ButtonEvents=GetBindingsForJoyButton(i);

					JoyButtonBindings::iterator iter;
					if (!IsPressed) m_lastReleaseTime[i]=m_eventTime;
					{
						//Since it is more expensive to find elements than to test dbl click we'll go ahead and test every case
						//The worst case scenario is nudging for keys that have no double click support which costs an additional
						//2 finds that fail.  Most of the time nudges are rare, and a double click initiated has no penalty
						if (IsPressed && IsDoubleClicked(i))
						{
							//First try to find the product specific binding
							Button_EventEntry key(i,joyinfo2.ProductName.c_str(),true,true);
							iter=m_JoyButtonBindings.find(key);
							//If the product is not found (typical) try to find any
							if (iter==m_JoyButtonBindings.end())
								iter=m_JoyButtonBindings.find(Button_EventEntry(i,"any",true,true));
							//See if there are any double clicked events found
							if (iter!=m_JoyButtonBindings.end())
								m_UseDoubleClickBindings[i]=iter;
						}
					}

					//TODO see if there is a more proper std method to test for NULL (e.g. empty, which doesn't work here)
					if (!m_UseDoubleClickBindings[i]._Mynode())
					{
						//First try to find the product specific binding
						Button_EventEntry key(i,joyinfo2.ProductName.c_str());
						iter=m_JoyButtonBindings.find(key);

						//If the product is not found (typical) try to find any
						if (iter==m_JoyButtonBindings.end())
							iter=m_JoyButtonBindings.find(i);
					}
					else
						iter=m_UseDoubleClickBindings[i];  //for release case (arguably would check for release here)

					//If we found any events for this button
					if (iter!=m_JoyButtonBindings.end())
					{
						std::vector<std::string> *ButtonEvents=(*iter).second;
						const Button_EventEntry &key=(*iter).first;

						if (!IsPressed)
							m_UseDoubleClickBindings[i]=JoyButtonBindings::iterator();  //ensure double click setting is reset for release

						if (ButtonEvents)
						{
							std::vector<std::string>::iterator pos;
							//fire the correct event based on the useOnOff attribute
							if (key.useOnOff)
							{
								for (pos = ButtonEvents->begin(); pos != ButtonEvents->end(); ++pos)
									m_controlledEventMap->EventOnOff_Map[*pos].Fire(IsPressed);
							}
							else
							{
								//Send on the down key only
								if (IsPressed)
								{
									for (pos = ButtonEvents->begin(); pos != ButtonEvents->end(); ++pos)
										m_controlledEventMap->Event_Map[*pos].Fire();
								}
							}
						}
					}
				}
			}
			//Update flood control
			floodcontrol=joyinfo;
		}
	}
}


