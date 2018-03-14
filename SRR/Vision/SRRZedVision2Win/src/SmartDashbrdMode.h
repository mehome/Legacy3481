#pragma once

//Helper class that will manipulate cam#_op_mode variable through SmartDashboard
class SmartDashboard_ModeManager
{
private:
	int &m_CurrentSelection;  //reference to actual variable to manipulate
	std::string Variable_Name;
	int m_LastMode; //used as a valve for flood control
	size_t m_IterationCount = 0;  //used to cut down on probing iterations as changes do not need to happen immediately
public:
	SmartDashboard_ModeManager(int &CurrentSelection, const char *SmartName, int DefaultMode) : m_CurrentSelection(CurrentSelection),
		Variable_Name(SmartName), m_LastMode(DefaultMode)
	{
		try
		{
			SmartDashboard::GetNumber(SmartName);
		}
		catch (...)
		{
			const double default_value = (double)DefaultMode;  //too bad they don't support integers yet
			SmartDashboard::PutNumber(SmartName, default_value);
		}
	}
	//luckily this should have the same changes between either camera
	void SmartDashboardChanged(bool DestroyMasked)
	{
		switch (m_CurrentSelection)
		{
		case 0: printf("mode NONE\n"); break;
		case 1: printf("mode Find Hook\n"); break;
		case 2: printf("mode Find Rock\n"); break;
		case 3: printf("mode Find Beacon\n"); break;
		case 4: printf("mode passthrough\n"); break;
		}
		if (DestroyMasked)
			cv::destroyWindow("Masked");
	}
	void operator()()
	{
		const size_t IntervalCount = 60;   //give some time between reads
										   //Will be managing 3 variables... the current smart value, current selection, and last selection
										   //if there is a conflict between the current selection and current smart value where neither are the last... the smart dashboard has priority
		int current_smart = m_LastMode;
		if (m_IterationCount++ > IntervalCount)
		{
			//probe a new update
			current_smart = (int)SmartDashboard::GetNumber(Variable_Name.c_str());
			m_IterationCount = 0;
		}
		if ((current_smart != m_CurrentSelection) && (current_smart != m_LastMode))
		{
			m_CurrentSelection = current_smart;
			const bool DestroyMasked = (m_LastMode == 2);  //only if we were finding the rock last time
			assert((DestroyMasked && m_CurrentSelection != 2) || (!DestroyMasked));  //sanity check
			m_LastMode = m_CurrentSelection;
			SmartDashboardChanged(DestroyMasked);  //show change
		}
		else if (m_CurrentSelection != m_LastMode) //e.t. changed from keyboard
		{
			SmartDashboard::PutNumber(Variable_Name.c_str(), (double)m_CurrentSelection);
			m_LastMode = m_CurrentSelection;
		}
	}
};

