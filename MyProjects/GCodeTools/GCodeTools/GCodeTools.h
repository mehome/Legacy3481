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
	//Note Player Commands
	bool LoadSequence_CT(const char *filename);
	void PlayBlock(size_t block_number);
	void PlaySong(double position);
	void Stop_NotePlayer();
	void Pause_NotePlayer(bool IsPaused);
	void ReveseChannels(bool IsReversed);
	bool ExportGCode(const char *filename);
	void SetBounds(double x, double y, double z);  //specify different size dimensions of when to flip
	//Tab helper commands
	bool LoadToolJob(const char *filename);
	void SetWorkingFile(const char *filename);  //can be NULL for console dump
	void SetGlobalTabSize(double height, double width);
	//where tab is inserted after line number and offset is distance away from the line's coordinates
	bool AddTab(size_t line_number, double offset); 
	bool AddTab(double height, double width,size_t line_number, double offset);
	bool RemoveTab(size_t line_number);
	bool LoadProject(const char *filename);
	bool SaveProject(const char *filename);
	void Test();
private:
	std::shared_ptr<GCodeTools_Internal> m_p_GCodeTools; //a pimpl idiom (using shared_ptr allows declaration to be hidden from destructor)
};

