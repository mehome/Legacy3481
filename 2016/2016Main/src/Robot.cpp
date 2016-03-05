/*

Copyright (c) 2016, Ryan S. Cooper, Sophie He, Dylan Watson, BroncBotz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of BroncBotz nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

 */

#include <WPILib.h>
#include <fstream>

#include "Log.h"
#include "Auton.h"
#include "Config.h"
#include "Sensing.h"
#include "Pneumatics.h"
#include "Initializers.h"
#include "SystemsCollection.h"

using namespace std;
using namespace Systems;
using namespace Configuration;


#define VERSION 27 //!< Defines the program version for the entire program.
#define REVISION "D" //!< Defines the revision of this version of the program.

/*! BroncBotz class is the entry point of the program (where WPILib sends it's calls for
 *  robot control, operation, auton, test etc. */
class BroncBotz: public SampleRobot
{
	//Task *mpc; //!< Pointer to where we will allocate the mpc task.
	Task *drive; //!< Pointer to where we will allocate the drive task.
	//Task *beacon; //!< Pointer to where we will allocate the beacon task.
	Task *sensing; //!< Pointer to where we will allocate the sensing task.
	Task *operation; //!< Pointer to where we will allocate the operation task.

	Config *config; //!< Pointer to where we will allocate the configuration object.
	const char *configFile = "/robot_roborio.xml";  //!< Location of the XML config file.

public:
	BroncBotz()
		{
			Initialize();
		}

		/*! \brief Initialize.
	 	 *         Prepares all functions and resources.
	 	 *
	 	 *  Here is where all tasks, configuration, and allocation of resources occurs.
	 	 *  All systems are inizialized and the //beacon system is started.
	 	 */
		void Initialize()
		{

			Log::Instance().DestroyLog();
			Log::Instance().SetLog_SetLongLog("/log.txt", "/long_log.txt");

			Log::Instance().Append("Program Version: " + VERSION );
			cout << "Program Version: " << VERSION << " Revision: " << REVISION << endl;

			ifstream infile(configFile);
			if(infile.good())
				cout << "Config file found at " << configFile << endl;
			else
				cout << "Could not find the config file at " << configFile << endl;


			Log::Instance().Append("Configuration loaded: " + infile.good());

			config = Config::Instance();
			config->Load(configFile);
			config->BuildControlSchema();
		}

		void Autonomous()
		{
			/*if(config->QuickLoad())
			{
				delete config;
				config = Config::Instance();
				config->Load(configFile);
			}*/
			Auton auton;
			auton.Start();
		}

		/*! \brief Operation.
		 *         Starts the operation portion of the game.
		 *
		 *  Operator control starts the loops of all systems that are required during the
		 *  robot operation period.
		 */
		void OperatorControl()
		{
			/*if(config->QuickLoad())
			{
				delete config;
				config = Config::Instance();
				config->Load(configFile);
			}*/

			//mpc = new Task("MPCService", (FUNCPTR)InitializeMPC);
			sensing = new Task("Sensing", (FUNCPTR)InitializeSensors);
			operation = new Task("Operation", (FUNCPTR)InitializeOperation);
			drive = new Task("Drive", (FUNCPTR)InitializeDrive);

			//SystemsCollection::Instance().MPC.RegisterClient(&SystemsCollection::Instance().operator_);

			while (IsOperatorControl() && IsEnabled())
					{
						Wait(0.005);
					}

			//clean-up
			delete sensing;
			delete operation;
			delete drive;
			//assert(0);//fixes the twitching bug, something is wrong in memory somewhere, so we are restarting the program.
			SystemsCollection::Instance().Reset();
		}

};

START_ROBOT_CLASS(BroncBotz);
