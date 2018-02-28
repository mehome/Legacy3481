// SmartDashboardTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../SmartDashboard/SmartDashboard_import.h"
#include "../SmartDashboard/tables/TableKeyNotDefinedException.h"
#pragma warning (disable: 4101)

int _tmain(int argc, _TCHAR* argv[])
{
	char key = 0;

	SmartDashboard::SetClientMode();
	SmartDashboard::SetIPAddress("127.0.0.1");
	SmartDashboard::init();

	double x_target = 0.0;
	double y_target = 0.0;

	while (true)
	{
		if (SmartDashboard::IsConnected())
		{
			try {
				x_target = SmartDashboard::GetNumber("X Position");
			}
			catch (TableKeyNotDefinedException& e)
			{
				SmartDashboard::PutNumber("X Position", 0.0);
			}

			try {
				y_target = SmartDashboard::GetNumber("Y Position");
			}
			catch (TableKeyNotDefinedException& e)
			{
				SmartDashboard::PutNumber("Y Position", 0.0);
			}

			std::cout << "X position " << x_target << "  Y position " << y_target << std::endl;
		}
	}

	SmartDashboard::shutdown();

	return 0;
}

