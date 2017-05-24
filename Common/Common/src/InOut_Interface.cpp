
#include "WPILib.h"

#include "Base/src/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/src/Vec2d.h"
#include "Base/src/Misc.h"
#include "Base/src/Event.h"
#include "Base/src/EventMap.h"
#include "Base/src/Script.h"
#include "Base/src/Script.h"

#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"
//#include "Vehicle_Drive.h"
//#include "PIDController.h"
//#include "AI_Base_Controller.h"
//#include "Robot_Control_Interface.h"

#include "Base/src/Joystick.h"
#include "Base/src/JoystickBinder.h"
//#include "UI_Controller.h"
//#include "PIDController.h"

#include "InOut_Interface.h"
#include "Debug.h"
#include "Robot_Control_Common.h"

using namespace Framework::Base;
#undef __DisableTankDrive__

  /***********************************************************************************************************************************/
 /*																Encoder2															*/
/***********************************************************************************************************************************/


Encoder2::Encoder2(UINT8 ModuleNumber,UINT32 aChannel, UINT32 bChannel, bool reverseDirection, EncodingType encodingType) : 
	Encoder(aChannel,bChannel,reverseDirection,encodingType),m_LastDistance(0.0)
{
}

void Encoder2::Reset2()
{
	m_LastDistance=0.0;
	Reset();
}

double Encoder2::GetRate2(double dTime_s)
{
	//Using distance will yield the same rate as GetRate, without precision loss to GetRaw()
	const double CurrentDistance=GetDistance();
	const double delta=CurrentDistance - m_LastDistance;
	m_LastDistance=CurrentDistance;
	return delta/dTime_s;
}

  /***********************************************************************************************************************************/
 /*														Driver_Station_Joystick														*/
/***********************************************************************************************************************************/

size_t Driver_Station_Joystick::GetNoJoysticksFound() 
{
	return m_NoJoysticks;
}

unsigned long GetCapabilitiesFlags(const LUA_Controls_Properties &control_props,size_t slot_index)
{
	typedef Framework::Base::IJoystick::JoystickInfo JoystickInfo;
	typedef LUA_Controls_Properties::DriverStation_Slot_List Driver_Station_SlotList;
	const Driver_Station_SlotList &list=control_props.GetDriverStation_SlotList();
	const LUA_Controls_Properties::Controls_List &controls=control_props.Get_Controls();
	typedef LUA_Controls_Properties::Controls_List::const_iterator Controls_List_iter;

	std::string ControlName=list[slot_index];

	unsigned long ret;
	size_t AxisCount=6;

	//Now for the fun part... search through the control list structure to find its match to obtain the axis count
	Controls_List_iter posn=find(controls.begin(),controls.end(),ControlName);  //gotta love c++ ;)
	if (posn!= controls.end())
		AxisCount=(*posn).AxisCount;

	#if 1
	//Testing
	std::string VarName="AxisCount_";
	VarName+=ControlName;
	SmartDashboard::PutNumber(VarName,AxisCount);
	#endif

	switch (AxisCount)
	{
		case 1:
			ret=JoystickInfo::fX_Axis;
			break;
		case 2:
			ret=JoystickInfo::fX_Axis|JoystickInfo::fY_Axis;
			break;
		case 3:
			ret=JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis;
			break;
		case 4:
			ret=
				JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
				JoystickInfo::fX_Rot;
			break;
		case 5:
			ret=
				JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
				JoystickInfo::fX_Rot|JoystickInfo::fY_Rot;
			break;
		default:
			ret=
				JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
				JoystickInfo::fX_Rot|JoystickInfo::fY_Rot|JoystickInfo::fZ_Rot;
			break;
	}
	return ret;
}

void Driver_Station_Joystick::SetSlotList(const LUA_Controls_Properties &control_props)
{
	const Driver_Station_SlotList &list=control_props.GetDriverStation_SlotList();
	//if this is empty then we'll leave it as it was
	if (list.size()==0)
		return;

	m_SlotList=list;
	m_NoJoysticks=list.size();
	m_JoyInfo.clear();  //we'll repopulate with the slot names

	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName=list[0];
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=GetCapabilitiesFlags(control_props,0);
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=strcmp(common.ProductName.c_str(),"none")!=0;  //make use of this bool instead of doing a strcmp each iteration
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	for (size_t i=1;i<list.size();i++)
	{
		common.ProductName=list[i];
		common.JoyCapFlags=GetCapabilitiesFlags(control_props,i);
		common.bPresent=strcmp(common.ProductName.c_str(),"none")!=0;  //make use of this bool instead of doing a strcmp each iteration
		m_JoyInfo.push_back(common);
	}
}

bool Driver_Station_Joystick::read_joystick (size_t nr, JoyState &Info)
{
	//First weed out numbers not in range
	int Number=(int)nr;
	Number-=m_StartingPort;
	bool ret=false;
	int AxisOffset=0;  //for cRIO compatibility
	const Framework::Base::IJoystick::JoystickInfo &JoyInfo=m_JoyInfo[nr];
	const unsigned long JoyCapFlags=JoyInfo.JoyCapFlags;
	#ifdef __USE_LEGACY_WPI_LIBRARIES__
	nr++;  //DOH the number selection is cardinal!  :(
	AxisOffset=1;
	#endif
	if ((Number>=0) && (Number<m_NoJoysticks) && (JoyInfo.bPresent))
	{
		memset(&Info,0,sizeof(JoyState));  //zero the memory


		//The axis selection is also ordinal
		if ((JoyCapFlags & JoystickInfo::fX_Axis)!=0)
			Info.lX=m_ds->GetStickAxis(nr,0+AxisOffset);
		if ((JoyCapFlags & JoystickInfo::fY_Axis)!=0)
			Info.lY=m_ds->GetStickAxis(nr,1+AxisOffset);
		if ((JoyCapFlags & JoystickInfo::fZ_Axis)!=0)
			Info.lZ=m_ds->GetStickAxis(nr,2+AxisOffset);
		if ((JoyCapFlags & JoystickInfo::fX_Rot)!=0)
			Info.lRx=m_ds->GetStickAxis(nr,3+AxisOffset);
		if ((JoyCapFlags & JoystickInfo::fY_Rot)!=0)
			Info.lRy=m_ds->GetStickAxis(nr,4+AxisOffset);
		if ((JoyCapFlags & JoystickInfo::fZ_Rot)!=0)
			Info.lRz=m_ds->GetStickAxis(nr,5+AxisOffset);
		//Testing---
		#if 0
		if (nr==AxisOffset)   //using AxisOffset is just a clever way of checking the first Joystick for both cRIO and roboRIO
		{
			//Putting the if statement in this test... gives the test as it is written above... with a clean smart dashboard
			//The ones that are not capable will not be written to and should either be blank or not populated
			if ((JoyCapFlags & JoystickInfo::fX_Axis)!=0)
				SmartDashboard::PutNumber("axis0",Info.lX);
			if ((JoyCapFlags & JoystickInfo::fY_Axis)!=0)
				SmartDashboard::PutNumber("axis1",Info.lY);
			if ((JoyCapFlags & JoystickInfo::fZ_Axis)!=0)
				SmartDashboard::PutNumber("axis2",Info.lZ);
			if ((JoyCapFlags & JoystickInfo::fX_Rot)!=0)
				SmartDashboard::PutNumber("axis3",Info.lRx);
			if ((JoyCapFlags & JoystickInfo::fY_Rot)!=0)
				SmartDashboard::PutNumber("axis4",Info.lRy);
			if ((JoyCapFlags & JoystickInfo::fZ_Rot)!=0)
				SmartDashboard::PutNumber("axis5",Info.lRz);
		}
		#endif
		Info.ButtonBank[0]=m_ds->GetStickButtons(nr);
		ret=true;
	}
	return ret;
}

Driver_Station_Joystick::Driver_Station_Joystick(int NoJoysticks,int StartingPort) : m_NoJoysticks(NoJoysticks), m_StartingPort(StartingPort)
{
	#ifdef USE_LEGACY_WPI_LIBRARIES
	m_ds = DriverStation::GetInstance();
	#else
	m_ds = &DriverStation::GetInstance();
	#endif
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName="joystick_1";
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot|JoystickInfo::fZ_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=true;
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	common.ProductName="joystick_2";
	m_JoyInfo.push_back(common);
	common.ProductName="joystick_3";
	m_JoyInfo.push_back(common);
	common.ProductName="joystick_4";
	m_JoyInfo.push_back(common);
}

Driver_Station_Joystick::~Driver_Station_Joystick()
{
}






