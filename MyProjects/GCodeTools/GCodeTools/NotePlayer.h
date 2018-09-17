#pragma once

//Ideally this noteplayer is written to export a song into GCode to be played on a CNC machine, but can also be previewed in DSound... given the ultimate goal
//it is greatly simplified to the bare necessities of note playing, and could be built upon later.


class NotePlayer_Internal;
class NotePlayer
{
public:
	NotePlayer();
	bool LoadSequence(const char *filename);
	void Play();
	void Stop();
	void Seek(size_t position);
	bool ExportGCode(const char *filename);
private:
	std::shared_ptr<NotePlayer_Internal>  m_Player; //encapsulate SDK specifics from public
};