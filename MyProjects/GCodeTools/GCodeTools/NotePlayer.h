#pragma once
namespace DirectSound
{
	namespace Output
	{

#include "DS_Output.h"
	}
}


//Ideally this noteplayer is written to export a song into GCode to be played on a CNC machine, but can also be previewed in DSound... given the ultimate goal
//it is greatly simplified to the bare necessities of note playing, and could be built upon later.


class NotePlayer_Internal;
class NotePlayer
{
public:
	NotePlayer();
	void Link_DSound(std::shared_ptr<DirectSound::Output::DS_Output> instance);

	//compact text format NULL will use default song, returns true if successful
	bool LoadSequence_CT(const char *filename);
	void PlayBlock(size_t block_number);
	void Stop();
	void SeekBlock(double position);  // in seconds
	bool ExportGCode(const char *filename);
private:
	std::shared_ptr<NotePlayer_Internal>  m_Player; //encapsulate SDK specifics from public
};