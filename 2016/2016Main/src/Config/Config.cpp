/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoftware.co
\*********************************************************************/

#include "Config.h"
#include "pugixml.h"
#include "Preproc.h"
#include "ConfigEnums.h"
#include "AxisControl.h"
#include "ControlItem.h"
#include "ControlName.h"
#include "ConfigStructs.h"
#include "ButtonControl.h"
#include "ToggleControl.h"

#define ROBOT_CONFIG //!< Defines whether or not to load the parameters configuration from the XML, or to use default settings.
#define DRIVE_CONFIG //!< Defines whether or not to load the driver configuration from the XML, or to use default settings.
#define OPERATOR_CONFIG //!< Defines whether or not to load the operator configuration from the XML, or to use default settings.


using namespace std;
using namespace pugi;
using namespace Configuration;

bool Config::instanceFlag = false; //!< Flag used to determine if an instance of the singleton already exists.
Config* Config::single = NULL; //!< Pointer to the instance of the singleton.

void Config::Load(const char* file)
{
	xml_document doc; //!< xml_document so store configuration file data.
	xml_parse_result result = doc.load_file(file); //!< Loads and checks the integrity of the configuration file.

	if (result)
	{
		cout << "XML [" << file << "] parsed without errors, config version: " << doc.child("RobotConfig").child("Version").attribute("version").value() << "\n";
		cout << doc.child("RobotConfig").child("Comment").attribute("comment").value() << "\n\n";
		quickLoad = doc.child("RobotConfig").child("QuickLoad").attribute("value").as_bool();
		autonEnabled = doc.child("RobotConfig").child("EnableAuton").attribute("value").as_bool();
		loadValues(doc);
	}
	else
	{
	    cout << "XML [" << file << "] parsed with errors.\n";
	    cout << "Error description: " << result.description() << "\n";
	    cout << "Error offset: " << result.offset << " (error at [..." << (file + result.offset) << "]\n\n";
	}
}

void Config::loadValues(xml_document &doc)
{

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("Victors").child("DriveTrain").first_child(); node; node = node.next_sibling())
	{
	    VictorItem item(new Victor(node.attribute("channel").as_int()), node.name(), node.attribute("reversed").as_bool());
	    //item.GetVictorDirect()->SetSafetyEnabled(true);
	    Victors.push_back(item);
	    cout << "Loaded CIM: " << node.name() << ":" << node.attribute("channel").as_int() << " isReversed:" << node.attribute("reversed").as_bool() << endl;
	}

	switch (Victors.size())
	{
		case 4:
			_DrivePower = DrivePower::fourCIM;
			cout << "Looks like we have ourselves a " << _DrivePower << " CIM drive train" << endl;
			break;
		case 5:
			_DrivePower = DrivePower::fiveCIM;
			cout << "Looks like we have ourselves a " << _DrivePower << " CIM drive train" << endl << "I am going to assume one of these is a kicker wheel." << endl;
			break;
		case 6:
			_DrivePower = DrivePower::sixCIM;
			cout << "Looks like we have ourselves a " << _DrivePower << " CIM drive train" << endl;
			break;
		default:
			cout << "I was not programmed to use a " << Victors.size() << " CIM drive train!" << endl;
			break;
		}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("Victors").child("Extra").first_child(); node; node = node.next_sibling())
		{
		    VictorItem item(new Victor(node.attribute("channel").as_int()), node.name(), node.attribute("reversed").as_bool());
		    //item.GetVictorDirect()->SetSafetyEnabled(true);
		    Victors.push_back(item);
		    cout << "Loaded Victor: " << node.name() << ":" << node.attribute("channel").as_int() << endl;
		}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("Solenoids").first_child(); node; node = node.next_sibling())
		{
			DoubleSolenoid::Value val;
		    string default_ = node.attribute("default").value();

		    if(default_=="forward")
		    	val=DoubleSolenoid::kForward;
		    else if(default_=="reverse")
		    	val=DoubleSolenoid::kReverse;
		    else
		    	val=DoubleSolenoid::kOff;

			SolenoidItem item(new DoubleSolenoid(node.attribute("forward").as_int(), node.attribute("reverse").as_int()), node.name(), val);
			Solenoids.push_back(item);
			cout << "Loaded Solenoid: " << node.name() << " : " << node.attribute("forward").as_int() << ", " << node.attribute("reverse").as_int() << ", Default: "<< node.attribute("default").value() << endl;
		}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("DI").first_child(); node; node = node.next_sibling())
			{
				DIItem item;
				item.name = node.name();
				item.in = new DigitalInput(node.attribute("channel").as_int());
				DIs.push_back(item);
				cout << "Loaded DI: " << node.name() << " : " << node.attribute("channel").as_int() << endl;
			}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("DO").first_child(); node; node = node.next_sibling())
				{
					DOItem item;
					item.name = node.name();
					item.out = new DigitalOutput(node.attribute("channel").as_int());
					DOs.push_back(item);
					cout << "Loaded DO: " << node.name() << " : " << node.attribute("channel").as_int() << endl;
				}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("AO").first_child(); node; node = node.next_sibling())
				{
					AnalogOItem item;
					item.name = node.name();
					item.out = new AnalogOutput(node.attribute("channel").as_int());
					AnalogOutputDevices.push_back(item);
					cout << "Loaded AO: " << node.name() << " : " << node.attribute("channel").as_int() << endl;
				}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("AI").first_child(); node; node = node.next_sibling())
					{
						AnalogIItem item;
						item.name = node.name();
						item.in = new AnalogInput(node.attribute("channel").as_int());
						AnalogInputDevices.push_back(item);
						cout << "Loaded AI: " << node.name() << " : " << node.attribute("channel").as_int() << endl;
					}


	for (xml_node node = doc.child("RobotConfig").child("Robot").child("Encoders").first_child(); node; node = node.next_sibling())
			{
				EncoderItem item;
				item.name = node.name();
				item.encoder = new Encoder(node.attribute("aChannel").as_int(), node.attribute("bChannel").as_int(), true);
				Encoders.push_back(item);
				cout << "Loaded Encoder: " << node.name() << " : " << node.attribute("aChannel").as_int() << ", " << node.attribute("bChannel").as_int() << endl;
			}

	for (xml_node node = doc.child("RobotConfig").child("Robot").child("Relays").first_child(); node; node = node.next_sibling())
			{
				RelayItem item;
				item.name = node.name();
				item.relay = new Relay(node.attribute("relay").as_int());
				Relays.push_back(item);
				cout << "Loaded Relay: " << node.name() << " : " << node.attribute("relay").as_int() << endl;
			}




#ifdef DRIVE_CONFIG

	_DriverConfig.type = (DriveType)doc.child("RobotConfig").child("Controls").child("Driver").child("driveType").attribute("type").as_int();

	_DriverConfig.controllerSlot = doc.child("RobotConfig").child("Controls").child("Driver").child("slot").attribute("controllerSlot").as_int();

	_DriverConfig.driveFit = (DriveFit)doc.child("RobotConfig").child("Controls").child("Driver").child("drive").attribute("driveFit").as_int();
	_DriverConfig.polynomialFitPower = doc.child("RobotConfig").child("Controls").child("Driver").child("drive").attribute("power").as_int();

	_DriverConfig.accelerationFactor = doc.child("RobotConfig").child("Controls").child("Driver").child("acceleration").attribute("accelerationFactor").as_double();

	_DriverConfig.leftPowerMultiplier = doc.child("RobotConfig").child("Controls").child("Driver").child("leftPower").attribute("powerMultiplier").as_double();
	_DriverConfig.rightPowerMultiplier = doc.child("RobotConfig").child("Controls").child("Driver").child("rightPower").attribute("powerMultiplier").as_double();

	/* Tank configuration*/
	_DriverConfig.leftAxis = doc.child("RobotConfig").child("Controls").child("Driver").child("left_drive").attribute("axis").as_int();
	_DriverConfig.leftPolarity = doc.child("RobotConfig").child("Controls").child("Driver").child("left_drive").attribute("polarity").as_int();
	_DriverConfig.leftDeadZone = doc.child("RobotConfig").child("Controls").child("Driver").child("left_drive").attribute("deadZone").as_double();

	_DriverConfig.rightAxis = doc.child("RobotConfig").child("Controls").child("Driver").child("right_drive").attribute("axis").as_int();
	_DriverConfig.rightPolarity = doc.child("RobotConfig").child("Controls").child("Driver").child("right_drive").attribute("polarity").as_int();
	_DriverConfig.rightDeadZone = doc.child("RobotConfig").child("Controls").child("Driver").child("right_drive").attribute("deadZone").as_double();

	/* Arcade configuration*/
	_DriverConfig.arcade_driveAxis = doc.child("RobotConfig").child("Controls").child("Driver").child("drivex").attribute("axis").as_int();
	_DriverConfig.arcade_drivePolarity = doc.child("RobotConfig").child("Controls").child("Driver").child("drivex").attribute("polarity").as_int();
	_DriverConfig.arcade_driveDeadZone = doc.child("RobotConfig").child("Controls").child("Driver").child("drivex").attribute("deadZone").as_double();
	_DriverConfig.arcade_turningAxis = doc.child("RobotConfig").child("Controls").child("Driver").child("turning").attribute("axis").as_int();
	_DriverConfig.arcade_turningPolarity = doc.child("RobotConfig").child("Controls").child("Driver").child("turning").attribute("polarity").as_int();
	_DriverConfig.arcade_turningDeadZone = doc.child("RobotConfig").child("Controls").child("Driver").child("turning").attribute("deadZone").as_double();

	_DriverConfig.kicker_LeftDeadZone = doc.child("RobotConfig").child("Controls").child("Driver").child("left_kicker").attribute("deadZone").as_double();
	_DriverConfig.kicker_RightDeadZone = doc.child("RobotConfig").child("Controls").child("Driver").child("right_kicker").attribute("deadZone").as_double();
	_DriverConfig.kicker_LeftPolarity = doc.child("RobotConfig").child("Controls").child("Driver").child("left_kicker").attribute("polarity").as_int();
	_DriverConfig.kicker_RightPolarity = doc.child("RobotConfig").child("Controls").child("Driver").child("right_kicker").attribute("polarity").as_int();
	_DriverConfig.kicker_left = doc.child("RobotConfig").child("Controls").child("Driver").child("left_kicker").attribute("axis").as_int();
	_DriverConfig.kicker_right = doc.child("RobotConfig").child("Controls").child("Driver").child("right_kicker").attribute("axis").as_int();

	_DriverConfig.shiftGear = doc.child("RobotConfig").child("Controls").child("Driver").child("shiftGear").attribute("button").as_int();

	_DriverConfig.reverse_a = doc.child("RobotConfig").child("Controls").child("Driver").child("reverse").attribute("button_a").as_int();
	_DriverConfig.reverse_b = doc.child("RobotConfig").child("Controls").child("Driver").child("reverse").attribute("button_b").as_int();
#endif


#ifdef OPERATOR_CONFIG
_OperatorConfig.controllerSlot = doc.child("RobotConfig").child("Controls").child("Operator").child("slot").attribute("controllerSlot").as_int();

_OperatorConfig.climberInButton = doc.child("RobotConfig").child("Controls").child("Operator").child("climber").attribute("inButton").as_int();
_OperatorConfig.climberOutButton = doc.child("RobotConfig").child("Controls").child("Operator").child("climber").attribute("outButton").as_int();
_OperatorConfig.climberDeadZone = doc.child("RobotConfig").child("Controls").child("Operator").child("climber").attribute("deadZone").as_double();
_OperatorConfig.climberPolarity = doc.child("RobotConfig").child("Controls").child("Operator").child("climber").attribute("polarity").as_int();
_OperatorConfig.climberAxis = doc.child("RobotConfig").child("Controls").child("Operator").child("climber").attribute("axis").as_int();
_OperatorConfig.climberPowerMultiplier = doc.child("RobotConfig").child("Controls").child("Operator").child("climber").attribute("powerMultiplier").as_double();

_OperatorConfig.intakeInButton = doc.child("RobotConfig").child("Controls").child("Operator").child("intake").attribute("inButton").as_int();
_OperatorConfig.intakeOutButton = doc.child("RobotConfig").child("Controls").child("Operator").child("intake").attribute("outButton").as_int();
_OperatorConfig.intakeAxis = doc.child("RobotConfig").child("Controls").child("Operator").child("intake").attribute("axis").as_int();
_OperatorConfig.intakeDeadZone = doc.child("RobotConfig").child("Controls").child("Operator").child("intake").attribute("deadZone").as_double();
_OperatorConfig.intakePolarity = doc.child("RobotConfig").child("Controls").child("Operator").child("intake").attribute("polarity").as_int();
_OperatorConfig.intakePowerMultiplier = doc.child("RobotConfig").child("Controls").child("Operator").child("intake").attribute("powerMultiplier").as_double();

_OperatorConfig.shooterInButton = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter").attribute("inButton").as_int();
_OperatorConfig.shooterOutButton = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter").attribute("outButton").as_int();
_OperatorConfig.shooterDeadZone = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter").attribute("deadZone").as_double();
_OperatorConfig.shooterAxis = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter").attribute("axis").as_int();
_OperatorConfig.shooterPolarity = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter").attribute("polarity").as_int();
_OperatorConfig.shooterPowerMultiplier = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter").attribute("powerMultiplier").as_double();

_OperatorConfig.indexerInButton = doc.child("RobotConfig").child("Controls").child("Operator").child("indexer").attribute("inButton").as_int();
_OperatorConfig.indexerOutButton = doc.child("RobotConfig").child("Controls").child("Operator").child("indexer").attribute("outButton").as_int();
_OperatorConfig.indexerAxis = doc.child("RobotConfig").child("Controls").child("Operator").child("indexer").attribute("axis").as_int();
_OperatorConfig.indexerDeadZone = doc.child("RobotConfig").child("Controls").child("Operator").child("indexer").attribute("deadZone").as_double();
_OperatorConfig.indexerPolarity = doc.child("RobotConfig").child("Controls").child("Operator").child("indexer").attribute("polarity").as_int();
_OperatorConfig.indexerPowerMultiplier = doc.child("RobotConfig").child("Controls").child("Operator").child("indexer").attribute("powerMultiplier").as_double();

_OperatorConfig.shooterShift = doc.child("RobotConfig").child("Controls").child("Operator").child("shooter_shift").attribute("button").as_int();
_OperatorConfig.climberShift = doc.child("RobotConfig").child("Controls").child("Operator").child("climber_shift").attribute("button").as_int();
_OperatorConfig.intakeShift = doc.child("RobotConfig").child("Controls").child("Operator").child("intake_shift").attribute("button").as_int();

#endif

#ifdef ROBOT_CONFIG
_RobotParameters.minBatterShootRange = doc.child("RobotConfig").child("Robot").child("Parameters").child("BatterShootRange").attribute("min").as_double()*10;
_RobotParameters.maxBatterShootRange = doc.child("RobotConfig").child("Robot").child("Parameters").child("BatterShootRange").attribute("max").as_double()*10;
#endif

}

void Config::BuildControlSchema()
{
	if(_OperatorConfig.indexerInButton != 0 && _OperatorConfig.indexerOutButton != 0)
			Controls.push_back(new ButtonControl(ControlName::indexer(), ControlItemType::buttonControl,
					_OperatorConfig.indexerInButton, _OperatorConfig.indexerOutButton, _OperatorConfig.indexerPolarity, _OperatorConfig.indexerPowerMultiplier, false));
		else if(_OperatorConfig.indexerAxis != 0)
			Controls.push_back(new AxisControl(ControlName::indexer(), ControlItemType::axisControl,
					_OperatorConfig.indexerAxis, _OperatorConfig.indexerPolarity, _OperatorConfig.indexerDeadZone, _OperatorConfig.indexerPowerMultiplier));
		else
		{
			DriverStation::ReportError("There was an error creating the indexer control item, check the config.\n");
			cout << "There was an error creating the indexer control item, check the config."<< endl;
		}

		if(_OperatorConfig.intakeInButton != 0 && _OperatorConfig.intakeOutButton != 0)
			Controls.push_back(new ButtonControl(ControlName::intake(), ControlItemType::buttonControl,
					_OperatorConfig.intakeInButton, _OperatorConfig.intakeOutButton, _OperatorConfig.intakePolarity, _OperatorConfig.intakePowerMultiplier, false));
		else if(_OperatorConfig.intakeAxis != 0)
			Controls.push_back(new AxisControl(ControlName::intake(), ControlItemType::axisControl,
					_OperatorConfig.intakeAxis, _OperatorConfig.intakePolarity, _OperatorConfig.intakeDeadZone, _OperatorConfig.intakePowerMultiplier));
		else
		{
			DriverStation::ReportError("There was an error creating the intake control item, check the config.\n");
			cout << "There was an error creating the intake control item, check the config." << endl;
		}


		if(_OperatorConfig.climberInButton != 0 && _OperatorConfig.climberOutButton != 0)
			Controls.push_back(new ButtonControl(ControlName::climber(), ControlItemType::buttonControl,
				_OperatorConfig.climberInButton, _OperatorConfig.climberOutButton, _OperatorConfig.climberPolarity, _OperatorConfig.climberPowerMultiplier, false));
		else if(_OperatorConfig.climberAxis != 0)
			Controls.push_back(new AxisControl(ControlName::climber(), ControlItemType::axisControl,
					_OperatorConfig.climberAxis, _OperatorConfig.climberPolarity, _OperatorConfig.climberDeadZone, _OperatorConfig.climberPowerMultiplier));
		else
		{
			DriverStation::ReportError("There was an error creating the climber control item, check the config.\n");
			cout << "There was an error creating the climber control item, check the config."<< endl;
		}

		if(_OperatorConfig.shooterInButton != 0 && _OperatorConfig.shooterOutButton != 0)
			Controls.push_back(new ButtonControl(ControlName::shooter(), ControlItemType::buttonControl,
				_OperatorConfig.shooterInButton, _OperatorConfig.shooterOutButton, _OperatorConfig.shooterPolarity, _OperatorConfig.shooterPowerMultiplier, false));
		else if(_OperatorConfig.shooterAxis != 0)
			Controls.push_back(new AxisControl(ControlName::shooter(), ControlItemType::axisControl,
					_OperatorConfig.shooterAxis, _OperatorConfig.shooterPolarity, _OperatorConfig.shooterDeadZone, _OperatorConfig.shooterPowerMultiplier));
		else
		{
			DriverStation::ReportError("There was an error creating the shooter control item, check the config.\n");
			cout << "There was an error creating the shooter control item, check the config." << endl;
		}

		Controls.push_back(new ToggleControl(ControlName::gearShift(), ControlItemType::buttonControl,
				_DriverConfig.shiftGear, 1, true, false));
		GetControlItem(ControlName::gearShift())->AddSolenoidItem(GetSolenoid(CommonName::gearShift()));

		Controls.push_back(new ToggleControl(ControlName::intakeShift(), ControlItemType::toggleControl,
				_OperatorConfig.intakeShift, 1, true, false));
		GetControlItem(ControlName::intakeShift())->AddSolenoidItem(GetSolenoid(CommonName::intakeShift()));

		Controls.push_back(new ToggleControl(ControlName::climberShift(), ControlItemType::toggleControl,
				_OperatorConfig.climberShift, 1, true, false));
		GetControlItem(ControlName::climberShift())->AddSolenoidItem(GetSolenoid(CommonName::climberShift()));

		Controls.push_back(new ToggleControl(ControlName::shooterShift(), ControlItemType::toggleControl,
				_OperatorConfig.shooterShift, 1, true, false));
		GetControlItem(ControlName::shooterShift())->AddSolenoidItem(GetSolenoid(CommonName::shooterShift()));

		GetControlItem(ControlName::indexer())->AddVictorItem(GetVictor(CommonName::indexerMotor()));
		GetControlItem(ControlName::intake())->AddVictorItem(GetVictor(CommonName::intakeMotor()));

		GetControlItem(ControlName::climber())->AddVictorItem(GetVictor(CommonName::climber_1()));
		GetControlItem(ControlName::climber())->AddVictorItem(GetVictor(CommonName::climber_2()));

		GetControlItem(ControlName::shooter())->AddVictorItem(GetVictor(CommonName::shooter_1()));
		GetControlItem(ControlName::shooter())->AddVictorItem(GetVictor(CommonName::shooter_2()));
}

VictorItem *Config::GetVictor(CommonName name)
{
	for(int i=0; i<(int)Victors.size();i++)
		if(Victors[i].GetName()==(string)name)
			return &Victors[i];
	return NULL;
}

Relay *Config::GetRelay(CommonName name)
{
	for(int i=0; i<(int)Relays.size();i++)
		if(Relays[i].name==(string)name)
			return Relays[i].relay;
	return NULL;
}

AnalogInput *Config::GetAnalogInputDevice(CommonName name)
{
	for(int i=0; i<(int)AnalogInputDevices.size();i++)
		if(AnalogInputDevices[i].name==(string)name)
			return AnalogInputDevices[i].in;
	return NULL;
}

AnalogOutput *Config::GetAnalogOutputDevice(CommonName name)
{
	for(int i=0; i<(int)AnalogOutputDevices.size();i++)
		if(AnalogOutputDevices[i].name==(string)name)
			return AnalogOutputDevices[i].out;
	return NULL;
}

DigitalInput *Config::GetDInput(CommonName name)
{
	for(int i=0; i<(int)DIs.size();i++)
		if(DIs[i].name==(string)name)
			return DIs[i].in;
	return NULL;
}

DigitalOutput *Config::GetDOutput(CommonName name)
{
	for(int i=0; i<(int)DOs.size();i++)
		if(DOs[i].name==(string)name)
			return DOs[i].out;
	return NULL;
}

SolenoidItem *Config::GetSolenoid(CommonName name)
{
	for(int i=0; i<(int)Solenoids.size();i++)
		if(Solenoids[i].GetName()==(string)name)
			return &Solenoids[i];
	return NULL;

}

Encoder *Config::GetEncoder(CommonName name)
{
	for(int i=0; i<(int)Encoders.size();i++)
		if(Encoders[i].name==(string)name)
			return Encoders[i].encoder;
	return NULL;

}

ControlItem *Config::GetControlItem(ControlName name)
{
	for(int i=0; i<(int)Controls.size();i++)
		if(Controls[i]->GetName()==(string)name)
			return Controls[i];
	return NULL;
}

bool Config::IsLoaded() { return loaded; }


Config* Config::Instance()
{
    if(! instanceFlag)
    {
        single = new Config();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}


