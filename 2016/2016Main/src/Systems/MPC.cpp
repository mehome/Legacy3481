/*
 * MPC.cpp
 *
 *  Created on: 3 Jan 2016
 *      Author: cooper.ryan
 */

#include "MPC.h"
#include "WPILib.h"

using namespace std;

void MPCService::Initialize()
{
	analysisLoop();
}

void MPCService::analysisLoop()
{
	for(;;)
	{
		for(unsigned int i=0; i<clients.size(); i++)
		{
			MPCClient *client = clients[i];

			if(client->SingleEncoder)
			{
				//double rate = client->SingleTarget->GetRate();
				//SmartDashboard::PutNumber("Rate", rate);
			}
			else
			{

			}

			if(client->checkMe)
			{
				if(client->SingleEncoder)
				{

				}
				else
				{

				}
			}
		}

		Wait(.000001);//update every microsecond
	}
}
