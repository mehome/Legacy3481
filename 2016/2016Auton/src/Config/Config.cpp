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

#ifdef ROBORIO
#define DRIVE_CONFIG
#define OPERATOR_CONFIG
#endif

using namespace std;
using namespace pugi;
using namespace Configuration;

bool Config::instanceFlag = false;
Config* Config::single = NULL;

#ifdef ROBORIO
void Config::Load(const char* file)
{
	xml_document doc;
	xml_parse_result result = doc.load_file(file);

	if (result)
	{
		out << "XML [" << file << "] parsed without errors, config version: " << doc.child("Version").attribute("version").value() << "\n";
		out << doc.child("Comment").attribute("comment").value() << "\n\n";
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
	    VictorItem item;
	    item.name = node.name();
	    item.victor = new Victor(node.attribute("channel").as_int());
	    Victors.push_back(item);
	    cout << "Loaded CIM: " << node.name() << ":" << node.attribute("channel").as_int() << endl;
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
		    VictorItem item;
		    item.name = node.name();
		    item.victor = new Victor(node.attribute("channel").as_int());
		    Victors.push_back(item);
		    cout << "Loaded Victor: " << node.name() << ":" << node.attribute("channel").as_int() << endl;
		}

	for (xml_node node = doc.child("Robot").child("Solenoids").first_child(); node; node = node.next_sibling())
		{
			SolenoidItem item;
			item.name = node.name();
			item.solenoid = new DoubleSolenoid(node.attribute("positiveFlow").as_int(), node.attribute("negativeFlow").as_int());
			Solenoids.push_back(item);
			cout << "Loaded Solenoid: " << node.name() << " : " << node.attribute("positiveFlow").as_int() << ", " << node.attribute("negativeFlow").as_int() << endl;
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

	_DriverConfig.breakMode = doc.child("Controls").child("Driver").child("break_mode").attribute("breakMode").as_bool();
	_DriverConfig.breakTime = doc.child("Controls").child("Driver").child("break_mode").attribute("breakTime").as_double();
	_DriverConfig.breakIntensity = doc.child("Controls").child("Driver").child("break_mode").attribute("breakIntensity").as_double();

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
#endif


#ifdef OPERATOR_CONFIG
_OperatorConfig.controllerSlot = doc.child("Controls").child("Operator").child("slot").attribute("controllerSlot").as_int();
_OperatorConfig.armAxis = doc.child("Controls").child("Operator").child("arm").attribute("axis").as_int();
_OperatorConfig.armPolarity = doc.child("Controls").child("Operator").child("arm").attribute("polarity").as_int();
_OperatorConfig.armDeadZone = doc.child("Controls").child("Operator").child("arm").attribute("deadZone").as_double();
_OperatorConfig.armPwerMultiplier = doc.child("Controls").child("Operator").child("arm").attribute("powerMultiplier").as_double();
_OperatorConfig.manipulatorButton = doc.child("Controls").child("Operator").child("manipulator").attribute("toggle").as_int();
_OperatorConfig.wingsButton = doc.child("Controls").child("Operator").child("wings").attribute("toggle").as_int();
_OperatorConfig.redModeButton = doc.child("Controls").child("Operator").child("redMode").attribute("toggle").as_int();
_OperatorConfig.blueModeButton = doc.child("Controls").child("Operator").child("blueMode").attribute("toggle").as_int();
#endif
}
#endif

#ifdef CRIO
void Config::Load()
{
	loadValues();
}

void Config::loadValues()
{

	//load the drive train victors
for(int i=1;i<4;i++)
{
		VictorItem item;
		ostringstream stm;
		stm << i;
		item.name = "leftDrive_"+stm.str();
	    item.victor = new Victor(i);
		Victors.push_back(item);
		cout << "Loaded CIM: " << item.name << ":" << i << endl;
}
for(int i=3;i<6;i++)
{
		VictorItem item;
		ostringstream stm;
		stm << (i-2);
		item.name = "rightDrive_"+stm.str();
	    item.victor = new Victor(i);
		Victors.push_back(item);
		cout << "Loaded CIM: " << item.name << ":" << i << endl;
}

	//determin the type of drive train we are using (6 CIM is the legal limit as of current for FRC)
	switch (Victors.size()) {
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



		DIItem dioitem;
		dioitem.name = "compressorSwitch";
		dioitem.in = new DigitalInput(2);
		DIs.push_back(dioitem);

	//load solenoids
		SolenoidItem solenoiditem;
		solenoiditem.name = "gearShift";
		solenoiditem.solenoid = new DoubleSolenoid(1, 2);
		Solenoids.push_back(solenoiditem);


	//load relays
		RelayItem item;
		item.name = "Compressor";
		item.relay = new Relay(1);
		Relays.push_back(item);
}
#endif

Victor *Config::GetVictor(CommonName name)
{
	for(int i=0; i<(int)Victors.size();i++)
		if(Victors[i].name==(string)name)
			return Victors[i].victor;
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

DoubleSolenoid *Config::GetSolenoid(CommonName name)
{
	for(int i=0; i<(int)Solenoids.size();i++)
		if(Solenoids[i].name==(string)name)
			return Solenoids[i].solenoid;
	return NULL;

}

bool Config::IsLoaded() { return loaded; }



//Get the singleton instance of this class
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


