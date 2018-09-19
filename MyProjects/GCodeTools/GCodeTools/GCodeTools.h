#pragma once

class GCodeTools_Internal; //forward declare... 

class GCodeTools
{
public:
	void GCodeTools_init(void);  // allows the declaration to remain here
	void GCodeTools_connect(); 
	//using direct sound to simulate motor frequency
	void TestSound_Start();
	void TestSound_Stop();
	bool LoadSequence_CT(const char *filename);
	void PlayBlock(size_t block_number);
	void PlaySong(double position);
	void Stop_NotePlayer();
	bool ExportGCode(const char *filename);
private:
	std::shared_ptr<GCodeTools_Internal> m_p_GCodeTools; //a pimpl idiom (using shared_ptr allows declaration to be hidden from destructor)
};

