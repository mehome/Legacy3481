#include "pch.h"
#include "Time_Type.h"
#include "NotePlayer.h"

//For now using a similar type of data structure as that used in OctaMed
struct Song
{
	struct Note
	{
		double FreqInHertz; //zero indicates rest
		double Duration;
	};
	struct Track
	{
		std::string VoiceName;  //a way to identify this track
		std::vector<Note> Notes;  //a list of notes for one track
	};
	struct Block
	{
		size_t Block_Number;
		std::vector<Track> Tracks; //parallel tracks for a block
	};
	using Sequence = std::vector<Block>;  //for completion, probably more than we need though
	Sequence Music;  //A container to store the song
};

class NotePlayer_Internal
{
private:
public:
	NotePlayer_Internal()
	{

	}
	bool LoadSequence(const char *filename)
	{}
	void Play()
	{}
	void Stop()
	{}
	void Seek(size_t position)
	{}
	bool ExportGCode(const char *filename)
	{}
};


  /***********************************************************************/
 /*								NotePlayer								*/			
/***********************************************************************/

NotePlayer::NotePlayer()
{
	m_Player = std::make_shared<NotePlayer_Internal>();
}
