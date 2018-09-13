#pragma once

class GCodeTools_Internal; //forward declare... 

class GCodeTools
{
public:
	void GCodeTools_init(void);  // allows the declaration to remain here
	void GCodeTools_connect(); 
private:
	std::shared_ptr<GCodeTools_Internal> m_p_GCodeTools; //a pimpl idiom (using shared_ptr allows declaration to be hidden from destructor)
};

