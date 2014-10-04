#include "WPILib.h"

#include "Base/src/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/src/Vec2d.h"
#include "Base/src/Misc.h"
#include "Base/src/Event.h"
#include "Base/src/EventMap.h"
#include "Base/src/Script.h"

//#include "Entity_Properties.h"
//#include "Physics_1D.h"
//#include "Physics_2D.h"
//#include "Entity2D.h"
//#include "Goal.h"
//#include "Ship_1D.h"
//#include "Ship.h"
//#include "AI_Base_Controller.h"
//#include "Vehicle_Drive.h"
//#include "PIDController.h"
//#include "Poly.h"
//#include "Robot_Control_Interface.h"
//#include "Rotary_System.h"

#include "Base/src/Joystick.h"
#include "Base/src/JoystickBinder.h"
#include "InOut_Interface.h"
#include "Debug.h"

#include "Robot_Control_Common.h"
#include "SmartDashboard/SmartDashboard.h"


  /***********************************************************************************************************************************/
 /*													Control_Assignment_Properties													*/
/***********************************************************************************************************************************/

static void LoadControlElement_1C_Internal(Scripting::Script& script,Control_Assignment_Properties::Controls_1C &Output)
{
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf(Buffer,"id_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Control_Element_1C newElement;
			{
				double fTest;
				err = script.GetField("channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.Channel=(size_t)fTest-1;  //make ordinal
				err = script.GetField("name",&newElement.name,NULL,NULL);
				assert(!err);
				err = script.GetField("module",NULL,NULL,&fTest);
				newElement.Module=(err)?0:(size_t)fTest;
				//assert(newElement.Module!=0);  //note: All module parameters are all ordinal for roboRIO
			}
			Output.push_back(newElement);
			script.Pop();
			index++;
		}
	} while (!fieldtable_err);

}

static void LoadControlElement_2C_Internal(Scripting::Script& script,Control_Assignment_Properties::Controls_2C &Output)
{
	typedef Control_Assignment_Properties::Control_Element_2C Control_Element_2C;
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf(Buffer,"id_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Control_Element_2C newElement;
			{
				double fTest;
				err = script.GetField("forward_channel",NULL,NULL,&fTest);
				if (err)
					err=script.GetField("a_channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.ForwardChannel=(size_t)fTest-1;  //make ordinal
				err = script.GetField("reverse_channel",NULL,NULL,&fTest);
				if (err)
					err=script.GetField("b_channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.ReverseChannel=(size_t)fTest-1;  //make ordinal
				err = script.GetField("name",&newElement.name,NULL,NULL);
				assert(!err);
				err = script.GetField("module",NULL,NULL,&fTest);
				newElement.Module=(err)?0:(size_t)fTest;
				//assert(newElement.Module!=0);  //note: All module parameters are all ordinal for roboRIO
			}
			Output.push_back(newElement);
			script.Pop();
			index++;
		}
	} while (!fieldtable_err);
}

void Control_Assignment_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("control_assignments");
	if (!err) 
	{
		err = script.GetFieldTable("victor");
		if (!err)
		{
			LoadControlElement_1C_Internal(script,m_Victors);
			script.Pop();
		}
		err = script.GetFieldTable("relay");
		if (!err)
		{
			LoadControlElement_1C_Internal(script,m_Relays);
			script.Pop();
		}
		err = script.GetFieldTable("digital_input");
		if (!err)
		{
			LoadControlElement_1C_Internal(script,m_Digital_Inputs);
			script.Pop();
		}
		err = script.GetFieldTable("double_solenoid");
		if (!err)
		{
			LoadControlElement_2C_Internal(script,m_Double_Solenoids);
			script.Pop();
		}
		err = script.GetFieldTable("digital_input_encoder");
		if (!err)
		{
			LoadControlElement_2C_Internal(script,m_Encoders);
			script.Pop();
		}
		err = script.GetFieldTable("compressor");
		if (!err)
		{
			double fTest;
			err = script.GetField("relay",NULL,NULL,&fTest);
			assert(!err);
			m_Compressor_Relay=(size_t)fTest;
			err = script.GetField("limit",NULL,NULL,&fTest);
			assert(!err);
			m_Compressor_Limit=(size_t)fTest;
			script.Pop();
		}
		else
			m_Compressor_Relay=8,m_Compressor_Limit=14;

		script.Pop();
	}
}


  /***********************************************************************************************************************************/
 /*														RobotControlCommon															*/
/***********************************************************************************************************************************/

RobotControlCommon::~RobotControlCommon()
{

}

template <class T>
__inline void Initialize_1C_LUT(const Control_Assignment_Properties::Controls_1C &control_props,std::vector<T *> &constrols,
								RobotControlCommon::Controls_LUT &control_LUT,RobotControlCommon *instance,size_t (RobotControlCommon::*delegate)(const char *name) const)
{
	typedef Control_Assignment_Properties::Controls_1C Controls_1C;
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	for (size_t i=0;i<control_props.size();i++)
	{
		const Control_Element_1C &element=control_props[i];
		//ensure this name exists in the list
		size_t enumIndex=(instance->*delegate)(element.name.c_str());
		//The name may not exist in this list (it may be a name specific to the robot)... in which case there is no work to do
		if (enumIndex==(size_t)-1)
			continue;
		//create the new Control
		#ifdef Robot_TesterCode
		T *NewElement=new T(element.Module,element.Channel,element.name.c_str());  //adding name for UI
		#else
		T *NewElement=new T(element.Channel);
		#endif
		const size_t LUT_index=constrols.size(); //get size before we add it in
		//const size_t PopulationIndex=constrols.size();  //get the ordinal value before we add it
		constrols.push_back(NewElement);  //add it to our list of victors
		//Now to work out the new LUT
		//our LUT is the EnumIndex position set to the value of i... make sure we have the slots created
		assert(enumIndex<10);  //sanity check we have a limit to how many victors we have
		while(control_LUT.size()<=enumIndex)
			control_LUT.push_back(-1);  //fill with -1 as a way to indicate nothing is located for that slot
		control_LUT[enumIndex]=LUT_index;
	}
}

template <class T>
__inline void Initialize_2C_LUT(const Control_Assignment_Properties::Controls_2C &control_props,std::vector<T *> &constrols,
								RobotControlCommon::Controls_LUT &control_LUT,RobotControlCommon *instance,size_t (RobotControlCommon::*delegate)(const char *name) const)
{
	typedef Control_Assignment_Properties::Controls_2C Controls_2C;
	typedef Control_Assignment_Properties::Control_Element_2C Control_Element_2C;
	for (size_t i=0;i<control_props.size();i++)
	{
		const Control_Element_2C &element=control_props[i];
		//ensure this name exists in the list
		size_t enumIndex=(instance->*delegate)(element.name.c_str());
		//The name may not exist in this list (it may be a name specific to the robot)... in which case there is no work to do
		if (enumIndex==(size_t)-1)
			continue;
		//create the new Control
		#ifdef Robot_TesterCode
		T *NewElement=new T(element.Module,element.ForwardChannel,element.ReverseChannel,element.name.c_str());
		#else
		T *NewElement=new T(element.Module,element.ForwardChannel,element.ReverseChannel);
		#endif
		const size_t LUT_index=constrols.size(); //get size before we add it in
		//const size_t PopulationIndex=constrols.size();  //get the ordinal value before we add it
		constrols.push_back(NewElement);  //add it to our list of victors
		//Now to work out the new LUT
		//our LUT is the EnumIndex position set to the value of i... make sure we have the slots created
		assert(enumIndex<10);  //sanity check we have a limit to how many victors we have
		while(control_LUT.size()<=enumIndex)
			control_LUT.push_back(-1);  //fill with -1 as a way to indicate nothing is located for that slot
		control_LUT[enumIndex]=LUT_index;
	}
}


void RobotControlCommon::RobotControlCommon_Initialize(const Control_Assignment_Properties &props)
{
	m_Props=props;
	typedef Control_Assignment_Properties::Controls_1C Controls_1C;
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	typedef Control_Assignment_Properties::Controls_2C Controls_2C;
	typedef Control_Assignment_Properties::Control_Element_2C Control_Element_2C;
	//create control elements and their LUT's
	//victors
	Initialize_1C_LUT<Victor>(props.GetVictors(),m_Victors,m_VictorLUT,this,&RobotControlCommon::RobotControlCommon_Get_Victor_EnumValue);
	//relays
	Initialize_1C_LUT<Relay>(props.GetRelays(),m_Relays,m_RelayLUT,this,&RobotControlCommon::RobotControlCommon_Get_Victor_EnumValue);
	//double solenoids
	Initialize_2C_LUT<DoubleSolenoid>(props.GetDoubleSolenoids(),m_DoubleSolenoids,m_DoubleSolenoidLUT,this,&RobotControlCommon::RobotControlCommon_Get_DoubleSolenoid_EnumValue);
	//digital inputs
	Initialize_1C_LUT<DigitalInput>(props.GetDigitalInputs(),m_DigitalInputs,m_DigitalInputLUT,this,&RobotControlCommon::RobotControlCommon_Get_DigitalInput_EnumValue);
	//encoders
	Initialize_2C_LUT<Encoder2>(props.GetEncoders(),m_Encoders,m_EncoderLUT,this,&RobotControlCommon::RobotControlCommon_Get_Victor_EnumValue);
}


void RobotControlCommon::TranslateToRelay(size_t index,double Voltage)
{
	IF_LUT(m_RelayLUT)
	{
		Relay::Value value=Relay::kOff;  //*NEVER* want both on!
		const double Threshold=0.08;  //This value is based on dead voltage for arm... feel free to adjust, but keep high enough to avoid noise

		if (Voltage>Threshold)
			value=Relay::kForward;
		else if (Voltage<-Threshold)
			value=Relay::kReverse;

		m_Relays[m_RelayLUT[index]]->Set(value);
	}
}

#ifdef Robot_TesterCode
  /***********************************************************************************************************************************/
 /*																Encoder2															*/
/***********************************************************************************************************************************/

Encoder2::Encoder2(uint8_t ModuleNumber,UINT32 aChannel, UINT32 bChannel,const char *name) : 
	Control_2C_Element_UI(ModuleNumber,aChannel,bChannel,name), m_LastDistance(0.0), m_Distance(0.0),m_LastTime(0.010), m_ValueScalar(1.0)
{
	m_Name+="_encoder";
}

void Encoder2::TimeChange(double dTime_s,double adjustment_delta)
{
	m_LastTime=dTime_s;
	m_Distance+=adjustment_delta;
	display_number(m_Distance);
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
	return (delta/dTime_s) * m_ValueScalar;
}

void Encoder2::Start() {}
int32_t Encoder2::Get() {return 0;}
int32_t Encoder2::GetRaw() {return 0;}
void Encoder2::Reset() 
{
	m_Distance=0;
}
void Encoder2::Stop() {}
double Encoder2::GetDistance() {return m_Distance;}
double Encoder2::GetRate() {return GetRate2(m_LastTime);}
void Encoder2::SetMinRate(double minRate) {}
void Encoder2::SetDistancePerPulse(double distancePerPulse) {}
void Encoder2::SetReverseDirection(bool reverseDirection) 
{
	m_ValueScalar?1.0:-1.0;
}


  /***********************************************************************************************************************************/
 /*																RobotDrive															*/
/***********************************************************************************************************************************/

const int32_t kMaxNumberOfMotors=4;

void RobotDrive::InitRobotDrive() {
	m_frontLeftMotor = NULL;
	m_frontRightMotor = NULL;
	m_rearRightMotor = NULL;
	m_rearLeftMotor = NULL;
	m_sensitivity = 0.5;
	m_maxOutput = 1.0;
	m_LeftOutput=0.0,m_RightOutput=0.0;
}

RobotDrive::RobotDrive(Victor *frontLeftMotor, Victor *rearLeftMotor,
						Victor *frontRightMotor, Victor *rearRightMotor)
{
	InitRobotDrive();
	if (frontLeftMotor == NULL || rearLeftMotor == NULL || frontRightMotor == NULL || rearRightMotor == NULL)
	{
		//wpi_setWPIError(NullParameter);
		assert(false);
		return;
	}
	m_frontLeftMotor = frontLeftMotor;
	m_rearLeftMotor = rearLeftMotor;
	m_frontRightMotor = frontRightMotor;
	m_rearRightMotor = rearRightMotor;
	for (int32_t i=0; i < kMaxNumberOfMotors; i++)
	{
		m_invertedMotors[i] = 1;
	}
	m_deleteSpeedControllers = false;
}

RobotDrive::RobotDrive(Victor &frontLeftMotor, Victor &rearLeftMotor,
						Victor &frontRightMotor, Victor &rearRightMotor)
{
	InitRobotDrive();
	m_frontLeftMotor = &frontLeftMotor;
	m_rearLeftMotor = &rearLeftMotor;
	m_frontRightMotor = &frontRightMotor;
	m_rearRightMotor = &rearRightMotor;
	for (int32_t i=0; i < kMaxNumberOfMotors; i++)
	{
		m_invertedMotors[i] = 1;
	}
	m_deleteSpeedControllers = false;
}

RobotDrive::~RobotDrive()
{
	if (m_deleteSpeedControllers)
	{
		delete m_frontLeftMotor;
		delete m_rearLeftMotor;
		delete m_frontRightMotor;
		delete m_rearRightMotor;
	}
}

void RobotDrive::SetLeftRightMotorOutputs(float leftOutput, float rightOutput)
{
	//this is added for convenience in simulation
	m_LeftOutput=leftOutput,m_RightOutput=rightOutput;

	assert(m_rearLeftMotor != NULL && m_rearRightMotor != NULL);

	uint8_t syncGroup = 0x80;

	if (m_frontLeftMotor != NULL)
		m_frontLeftMotor->Set(Limit(leftOutput) * m_invertedMotors[kFrontLeftMotor] * m_maxOutput, syncGroup);
	m_rearLeftMotor->Set(Limit(leftOutput) * m_invertedMotors[kRearLeftMotor] * m_maxOutput, syncGroup);

	if (m_frontRightMotor != NULL)
		m_frontRightMotor->Set(-Limit(rightOutput) * m_invertedMotors[kFrontRightMotor] * m_maxOutput, syncGroup);
	m_rearRightMotor->Set(-Limit(rightOutput) * m_invertedMotors[kRearRightMotor] * m_maxOutput, syncGroup);

	//CANJaguar::UpdateSyncGroup(syncGroup);  ah ha... sync group only works with CAN / Jaguar
}

float RobotDrive::Limit(float num)
{
	if (num > 1.0)
	{
		return 1.0;
	}
	if (num < -1.0)
	{
		return -1.0;
	}
	return num;
}

void RobotDrive::Normalize(double *wheelSpeeds)
{
	double maxMagnitude = fabs(wheelSpeeds[0]);
	int32_t i;
	for (i=1; i<kMaxNumberOfMotors; i++)
	{
		double temp = fabs(wheelSpeeds[i]);
		if (maxMagnitude < temp) maxMagnitude = temp;
	}
	if (maxMagnitude > 1.0)
	{
		for (i=0; i<kMaxNumberOfMotors; i++)
		{
			wheelSpeeds[i] = wheelSpeeds[i] / maxMagnitude;
		}
	}
}

void RobotDrive::RotateVector(double &x, double &y, double angle)
{
	double cosA = cos(angle * (3.14159 / 180.0));
	double sinA = sin(angle * (3.14159 / 180.0));
	double xOut = x * cosA - y * sinA;
	double yOut = x * sinA + y * cosA;
	x = xOut;
	y = yOut;
}

void RobotDrive::SetInvertedMotor(MotorType motor, bool isInverted)
{
	if (motor < 0 || motor > 3)
	{
		//wpi_setWPIError(InvalidMotorIndex);
		assert(false);
		return;
	}
	m_invertedMotors[motor] = isInverted ? -1 : 1;
}

void RobotDrive::SetExpiration(float timeout){}

float RobotDrive::GetExpiration()
{
	return 0.0;
}

bool RobotDrive::IsAlive()
{
	return true;
}

bool RobotDrive::IsSafetyEnabled()
{
	return true;
}

void RobotDrive::SetSafetyEnabled(bool enabled) {}

void RobotDrive::GetDescription(char *desc)
{
	sprintf(desc, "RobotDrive");
}

void RobotDrive::StopMotor()
{
	if (m_frontLeftMotor != NULL) m_frontLeftMotor->Disable();
	if (m_frontRightMotor != NULL) m_frontRightMotor->Disable();
	if (m_rearLeftMotor != NULL) m_rearLeftMotor->Disable();
	if (m_rearRightMotor != NULL) m_rearRightMotor->Disable();
}



  /***********************************************************************************************************************************/
 /*														Control_1C_Element_UI														*/
/***********************************************************************************************************************************/

Control_1C_Element_UI::Control_1C_Element_UI(uint8_t moduleNumber, uint32_t channel,const char *name,double DefaultNumber) : m_DefaultNumber(DefaultNumber),
	m_PutNumber_Used(false),m_PutBoolUsed(false)
{
	m_Name=name;
	char Buffer[4];
	m_Name+="_";
	itoa(channel,Buffer,10);
	m_Name+=Buffer;
	m_Name+="_";
	itoa(moduleNumber,Buffer,10);
	m_Name+=Buffer;
}

void Control_1C_Element_UI::display_number(double value)
{
	SmartDashboard::PutNumber(m_Name,value);
}

void Control_1C_Element_UI::display_bool(bool value)
{
	SmartDashboard::PutBoolean(m_Name,value);
}

//Note: For the get implementation, I restrict use of the bool used members to these functions as a first run
//It's just makes there use-case more restricted

bool Control_1C_Element_UI::get_bool() const
{
	if (!m_PutBoolUsed)
	{
		SmartDashboard::PutBoolean(m_Name,false);
		m_PutBoolUsed=true;
	}
	return SmartDashboard::GetBoolean(m_Name);
}

double Control_1C_Element_UI::get_number() const
{
	if (!m_PutNumber_Used)
	{
		SmartDashboard::PutNumber(m_Name,m_DefaultNumber);
		m_PutNumber_Used=true;
	}
	return (double)SmartDashboard::GetNumber(m_Name);
}

  /***********************************************************************************************************************************/
 /*														Control_2C_Element_UI														*/
/***********************************************************************************************************************************/

Control_2C_Element_UI::Control_2C_Element_UI(uint8_t moduleNumber, uint32_t forward_channel, uint32_t reverse_channel,const char *name)
{
	m_Name=name;
	char Buffer[4];
	m_Name+="_";
	itoa(forward_channel,Buffer,10);
	m_Name+=Buffer;
	m_Name+="_";
	itoa(reverse_channel,Buffer,10);
	m_Name+=Buffer;
	m_Name+="_";
	itoa(moduleNumber,Buffer,10);
	m_Name+=Buffer;
}

void Control_2C_Element_UI::display_bool(bool value)
{
	SmartDashboard::PutBoolean(m_Name,value);
}

void Control_2C_Element_UI::display_number(double value)
{
	SmartDashboard::PutNumber(m_Name,value);
}

bool Control_2C_Element_UI::get_bool() const
{
	return SmartDashboard::GetBoolean(m_Name);
}

double Control_2C_Element_UI::get_number() const
{
	return SmartDashboard::GetNumber(m_Name);
}
#endif

