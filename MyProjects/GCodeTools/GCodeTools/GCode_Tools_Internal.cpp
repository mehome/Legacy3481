//This file contains everything needed to establish a connection

#include "pch.h"
#include "GCodeTools.h"
namespace DirectSound
{
	namespace Output
	{
#include "DS_Output.h"
	}
}
#include "NotePlayer.h"

std::vector<std::string>& split(const std::string& s,
	char delim,
	std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}


class GCodeTools_Internal
{
private:
	DirectSound::Output::DirectSound_Initializer m_ds_init;
	DirectSound::Output::DS_Output m_DS;
	NotePlayer m_NotePlayer;
public:
	GCodeTools_Internal()
	{
	}
	void connect()
	{
		printf("Connecting");
	}
	void TestSound_Start() { m_DS.StartStreaming(); }
	void TestSound_Stop() { m_DS.StopStreaming(); }

	bool LoadSequence_CT(const char *filename) { return m_NotePlayer.LoadSequence_CT(filename); }
};

  /*******************************************************************************************************/
 /*													GCodeTools											*/
/*******************************************************************************************************/

void GCodeTools::GCodeTools_init(void)
{
	m_p_GCodeTools = std::make_shared<GCodeTools_Internal>();
}

void GCodeTools::GCodeTools_connect()
{
	m_p_GCodeTools->connect();
}

void GCodeTools::TestSound_Start() 
{
	m_p_GCodeTools->TestSound_Start();
}
void GCodeTools::TestSound_Stop() 
{
	m_p_GCodeTools->TestSound_Stop();
}
bool GCodeTools::LoadSequence_CT(const char *filename)
{
	return m_p_GCodeTools->LoadSequence_CT(filename);
}