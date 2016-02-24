/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#include "Config.h"
#include "pugixml.h"
#include "ConfigStructs.h"
#include "Preproc.h"
#include "Log.h"
#include "out.h"

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
		out << "XML [" << file << "] parsed without errors, config version: " << doc.child("Version").attribute("version").value() << "\n";
		out << doc.child("Comment").attribute("comment").value() << "\n\n";
		quickLoad = doc.child("QuickLoad").attribute("value").as_bool();
		loadValues(doc);
	}
	else
	{
	    out << "XML [" << file << "] parsed with errors.\n";
	    out << "Error description: " << result.description() << "\n";
	    out << "Error offset: " << result.offset << " (error at [..." << (file + result.offset) << "]\n\n";
	}
}

void Config::loadValues(xml_document &doc)
{

	for (xml_node node = doc.child("Robot").child("Victors").child("DriveTrain").first_child(); node; node = node.next_sibling())
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

	for (xml_node node = doc.child("Robot").child("Victors").child("Extra").first_child(); node; node = node.next_sibling())
		{
		    VictorItem item(new Victor(node.attribute("channel").as_int()), node.name(), node.attribute("reversed").as_bool());
		    //item.GetVictorDirect()->SetSafetyEnabled(true);
		    Victors.push_back(item);
		    cout << "Loaded Victor: " << node.name() << ":" << node.attribute("channel").as_int() << endl;
		}

	for (xml_node node = doc.child("Robot").child("Solenoids").first_child(); node; node = node.next_sibling())
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

	for (xml_node node = doc.child("Robot").child("DI").first_child(); node; node = node.next_sibling())
			{
				DIItem item;
				item.name = node.name();
				item.in = new DigitalInput(node.attribute("channel").as_int());
				DIs.push_back(item);
				cout << "Loaded DIO: " << node.name() << " : " << node.attribute("channel").as_int() << endl;
			}

	for (xml_node node = doc.child("Robot").child("DO").first_child(); node; node = node.next_sibling())
				{
					DOItem item;
					item.name = node.name();
					item.out = new DigitalOutput(node.attribute("channel").as_int());
					DOs.push_back(item);
					cout << "Loaded DIO: " << node.name() << " : " << node.attribute("channel").as_int() << endl;
				}


	for (xml_node node = doc.child("Robot").child("Encoders").first_child(); node; node = node.next_sibling())
			{
				EncoderItem item;
				item.name = node.name();
				item.encoder = new Encoder(node.attribute("aChannel").as_int(), node.attribute("bChannel").as_int());
				Encoders.push_back(item);
				cout << "Loaded Encoder: " << node.name() << " : " << node.attribute("aChannel").as_int() << ", " << node.attribute("bChannel").as_int() << endl;
			}

	for (xml_node node = doc.child("Robot").child("Relays").first_child(); node; node = node.next_sibling())
			{
				RelayItem item;
				item.name = node.name();
				item.relay = new Relay(node.attribute("relay").as_int());
				Relays.push_back(item);
				cout << "Loaded Relay: " << node.name() << " : " << node.attribute("relay").as_int() << endl;
			}




#ifdef DRIVE_CONFIG

	_DriverConfig.type = (DriveType)doc.child("Controls").child("Driver").child("driveType").attribute("type").as_int();

	_DriverConfig.controllerSlot = doc.child("Controls").child("Driver").child("slot").attribute("controllerSlot").as_int();

	_DriverConfig.driveFit = (DriveFit)doc.child("Controls").child("Driver").child("drive").attribute("driveFit").as_int();
	_DriverConfig.polynomialFitPower = doc.child("Controls").child("Driver").child("drive").attribute("power").as_int();

	_DriverConfig.accelerationFactor = doc.child("Controls").child("Driver").child("acceleration").attribute("accelerationFactor").as_double();

	_DriverConfig.leftPowerMultiplier = doc.child("Controls").child("Driver").child("leftPower").attribute("powerMultiplier").as_double();
	_DriverConfig.rightPowerMultiplier = doc.child("Controls").child("Driver").child("rightPower").attribute("powerMultiplier").as_double();

	/* Tank configuration*/
	_DriverConfig.leftAxis = doc.child("Controls").child("Driver").child("left_drive").attribute("axis").as_int();
	_DriverConfig.leftPolarity = doc.child("Controls").child("Driver").child("left_drive").attribute("polarity").as_int();
	_DriverConfig.leftDeadZone = doc.child("Controls").child("Driver").child("left_drive").attribute("deadZone").as_double();

	_DriverConfig.rightAxis = doc.child("Controls").child("Driver").child("right_drive").attribute("axis").as_int();
	_DriverConfig.rightPolarity = doc.child("Controls").child("Driver").child("right_drive").attribute("polarity").as_int();
	_DriverConfig.rightDeadZone = doc.child("Controls").child("Driver").child("right_drive").attribute("deadZone").as_double();

	/* Arcade configuration*/
	_DriverConfig.arcade_driveAxis = doc.child("Controls").child("Driver").child("drivex").attribute("axis").as_int();
	_DriverConfig.arcade_drivePolarity = doc.child("Controls").child("Driver").child("drivex").attribute("polarity").as_int();
	_DriverConfig.arcade_driveDeadZone = doc.child("Controls").child("Driver").child("drivex").attribute("deadZone").as_double();
	_DriverConfig.arcade_turningAxis = doc.child("Controls").child("Driver").child("turning").attribute("axis").as_int();
	_DriverConfig.arcade_turningPolarity = doc.child("Controls").child("Driver").child("turning").attribute("polarity").as_int();
	_DriverConfig.arcade_turningDeadZone = doc.child("Controls").child("Driver").child("turning").attribute("deadZone").as_double();

	_DriverConfig.kicker_LeftDeadZone = doc.child("Controls").child("Driver").child("left_kicker").attribute("deadZone").as_double();
	_DriverConfig.kicker_RightDeadZone = doc.child("Controls").child("Driver").child("right_kicker").attribute("deadZone").as_double();
	_DriverConfig.kicker_LeftPolarity = doc.child("Controls").child("Driver").child("left_kicker").attribute("polarity").as_int();
	_DriverConfig.kicker_RightPolarity = doc.child("Controls").child("Driver").child("right_kicker").attribute("polarity").as_int();
	_DriverConfig.kicker_left = doc.child("Controls").child("Driver").child("left_kicker").attribute("axis").as_int();
	_DriverConfig.kicker_right = doc.child("Controls").child("Driver").child("right_kicker").attribute("axis").as_int();

	_DriverConfig.shift_Low = doc.child("Controls").child("Driver").child("shift_High").attribute("button").as_int();
	_DriverConfig.shift_Hight = doc.child("Controls").child("Driver").child("shift_Low").attribute("button").as_int();

	_DriverConfig.reverse_a = doc.child("Controls").child("Driver").child("reverse").attribute("button_a").as_int();
	_DriverConfig.reverse_b = doc.child("Controls").child("Driver").child("reverse").attribute("button_b").as_int();
#endif


#ifdef OPERATOR_CONFIG
_OperatorConfig.controllerSlot = doc.child("Controls").child("Operator").child("slot").attribute("controllerSlot").as_int();
_OperatorConfig.climberUpButton = doc.child("Controls").child("Operator").child("climber").attribute("upButton").as_int();
_OperatorConfig.climberPolarity = doc.child("Controls").child("Operator").child("climber").attribute("polarity").as_int();
_OperatorConfig.climberDownButton = doc.child("Controls").child("Operator").child("climber").attribute("downButton").as_int();
_OperatorConfig.climberPowerMultiplier = doc.child("Controls").child("Operator").child("climber").attribute("powerMultiplier").as_double();

_OperatorConfig.intakeInButton = doc.child("Controls").child("Operator").child("intake").attribute("inButton").as_int();
_OperatorConfig.intakeOutButton = doc.child("Controls").child("Operator").child("intake").attribute("outButton").as_int();
_OperatorConfig.intakePolarity = doc.child("Controls").child("Operator").child("intake").attribute("polarity").as_int();
_OperatorConfig.intakePowerMultiplier = doc.child("Controls").child("Operator").child("intake").attribute("powerMultiplier").as_double();

_OperatorConfig.shooterAxis = doc.child("Controls").child("Operator").child("shooter").attribute("axis").as_int();
_OperatorConfig.shooterPolarity = doc.child("Controls").child("Operator").child("shooter").attribute("polarity").as_int();
_OperatorConfig.shooterPowerMultiplier = doc.child("Controls").child("Operator").child("shooter").attribute("powerMultiplier").as_double();

_OperatorConfig.indexerAxis = doc.child("Controls").child("Operator").child("indexer").attribute("axis").as_int();
_OperatorConfig.indexerPolarity = doc.child("Controls").child("Operator").child("indexer").attribute("polarity").as_int();
_OperatorConfig.indexerPowerMultiplier = doc.child("Controls").child("Operator").child("indexer").attribute("powerMultiplier").as_double();

_OperatorConfig.shooterShift = doc.child("Controls").child("Operator").child("shooter_shift").attribute("button").as_int();
_OperatorConfig.climberShift = doc.child("Controls").child("Operator").child("climber_shift").attribute("button").as_int();
_OperatorConfig.intakeShift = doc.child("Controls").child("Operator").child("intake_shift").attribute("button").as_int();

#endif
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


