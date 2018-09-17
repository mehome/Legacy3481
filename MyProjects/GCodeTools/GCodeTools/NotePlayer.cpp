#include "pch.h"
#include "Time_Type.h"
#include "NotePlayer.h"

//For now using a similar type of data structure as that used in OctaMed
struct Song
{
	struct Note
	{
		Note(double _FreqInHertz, double _Duration) : FreqInHertz(_FreqInHertz), Duration(_Duration)
		{
		}
		double FreqInHertz; //zero indicates rest
		double Duration;
	};
	using Track = std::map<double, Note>; //a list of notes for one track with PTS (in seconds) in a map form where zero point of origin is per block
	using Block= std::map<size_t,Track>;  //we'll just map out the voices as well
	using Sequence = std::map<size_t, Block>;

	double BeatPerMinute = 120;  //a master clock for playback... default to an easy divisible number for debugging
	Sequence Music;  //A container to store the song
};

class NotePlayer_Internal
{
private:
	Song m_Song;
	//--- is rest, G-4, G#4, G$4, shows note, accidental, and octave respectively, returns zero if its a rest
	__inline double GetFrequency(const char *Note)
	{
		//using equal temperment the formula is
		//https://pages.mtu.edu/~suits/NoteFreqCalcs.html
		//fn = f0 *(a)^(1/12)
		//where
		//	f0 = the frequency of one fixed note which must be defined.A common choice is setting the A above middle C(A4) at f0 = 440 Hz.
		//	n = the number of half steps away from the fixed note you are.If you are at a higher note, n is positive.If you are on a lower note, n is negative.
		//	fn = the frequency of the note n half steps away.
		//	a = (2)1 / 12 = the twelth root of 2 = the number which when multiplied by itself 12 times equals 2 = 1.059463094359...

		//So first let's define a since it is a constant
		const double a = 1.0594630943592952645618252949463;
		//To make life easy we'll simply use a fixed number of A on whichever octave we are on, and count the half number of steps from it
		size_t halfstep_count = 0;
		size_t Octave = Note[2] - '0';
		switch (Note[0])
		{
		//A already set
		//case 'A': break;
		case 'B':		halfstep_count = 2;		break;
		case 'C':		halfstep_count = 3;		Octave--;	break;
		case 'D':		halfstep_count = 5;		Octave--;	break;
		case 'E':		halfstep_count = 7;		Octave--;	break;
		case 'F':		halfstep_count = 8;		Octave--;	break;
		case 'G':		halfstep_count = 10;	Octave--;	break;
		}
		if (Note[1] == '#')
			halfstep_count++;
		else if (Note[1] == '$')
			halfstep_count--;
		if (halfstep_count == -1)
		{
			halfstep_count = 11;  //turn A$ into G#... to keep formula positive
			Octave--;
		}
		//Not going to optimize this... trusting the compiler to do so
		double frequency = 27.5;  //start with A0 fixed frequency... doubling it per octave set
		for (size_t i = 0; i < halfstep_count; i++)
			Octave *= 2;
		//using this fixed frequency multiply a for each half step
		double halfstep_powered=1.0;
		for (size_t i = 0; i < halfstep_count; i++)
			halfstep_powered *= a;
		frequency *= halfstep_powered;  //now to factor in the fixed frequency
		return frequency;
	}
	//track starting with 0... each time this is called it will append to the track so voices may be interleaved
	bool AppendTrack_CT(const char *CompactTextMusic,Song::Block &block, size_t TrackNumber,double BPM)
	{
		bool ret = false;
		const char * const CTM = CompactTextMusic; //shorthand
		//preconditions, must be valid and should start with a voice and number and have some data in it
		if ((!CTM) || strlen(CTM) < 3)
		{
			assert(false);
			return false;
		}
		using Block = Song::Block;
		using Track = Song::Track;
		Block::iterator iter = block.find(TrackNumber);
		if (iter == block.end())
		{
			block[TrackNumber] = Track();
			iter = block.find(TrackNumber);
		}
		if (iter != block.end())
		{
			Track &track = iter->second;
			//add notes here:
			double current_time = 0;  //TODO find last element's time to append
			const char *index_ptr = &CTM[0];
			size_t Octave = 4;  // a good default, but the content should override this
			char accidental = '-';  //either - #, or $
			const double BPS = 60.0 / BPM;
			const double WPS = BPS * 4.0;  //multiply it now whole note duration per second
			double DurationScaleFactor = 1.0;  //where the 8th note is 1/8 for example, default to whole note
			bool Dotted = false;
			while (*index_ptr != 0)
			{
				//expecting a note, rest or octave setting
				switch (*index_ptr)
				{
				case 'o':
				case 'O':
					index_ptr++;
					//expect a number
					if ((index_ptr[0] >= '0') && (index_ptr[0] <= '9'))
						Octave = index_ptr[0] - '0';
					else
					{
						assert(false);
						break;
					}
				case '$':
				case '#':
					accidental = *index_ptr;
					break;
				case '3':
					DurationScaleFactor =0.03125;  //Note: we'll try this for now... now sure if numbers will confuse parser
					if (Dotted)
					{
						assert(false); //not supported
						break;
					}
					break;
				case 's':
				case 'S':
					DurationScaleFactor = !Dotted ? 0.0625 : 0.09375;
					break;
				case 'i':
				case 'I':
					DurationScaleFactor = !Dotted ? 0.125 : 0.1875;
					break;
				case 'q':
				case 'Q':
					DurationScaleFactor = !Dotted ? 0.25 : 0.375;
					break;
				case 'h':
				case 'H':
					DurationScaleFactor = !Dotted ? 0.5 : 0.75;
					break;
				case 'w':
				case 'W':
					DurationScaleFactor = !Dotted? 1.0 : 1.5;
					break;
				case 'a':
				case 'A':
				case 'b':
				case 'B':
				case 'c':
				case 'C':
				case 'd':
				case 'D':
				case 'e':
				case 'E':
				case 'f':
				case 'F':
				case 'g':
				case 'G':
					//form the note:
					std::string Note;
					Note[0] = *index_ptr;
					Note[1] = accidental;
					Note[2] = (char)Octave + '0';
					double frequency = GetFrequency(Note.c_str());
					//now to compute duration
					const double duration = WPS * DurationScaleFactor;
					//populate the note
					Song::Note note(frequency, duration);
					//track[current_time] = note;
					//reset accidental for next note
					accidental = '-';
					//reset dotted
					Dotted = false;
					break;
				}
			}
		}
		return ret;
	}
	//Block starting with 0... block may be appended on multiple calls
	bool PopulateBlock_CT(const char *CompactTextMusic,size_t BlockNumber)
	{
		const char * const CTM = CompactTextMusic; //shorthand
		//preconditions, must be valid and should start with a voice and number and have some data in it
		if ((!CTM) || strlen(CTM) < 3)
		{
			assert(false);
			return false;
		}
		using Sequence = Song::Sequence;
		using Block = Song::Block;
		Sequence &music = m_Song.Music;
		Sequence::iterator iter = music.find(BlockNumber);
		if (iter == music.end())
		{
			music[BlockNumber] = Block();
			iter = music.find(BlockNumber);
		}
		if (iter != music.end())
		{
			const char *index_ptr = &CTM[0];
			while (*index_ptr != 0)
			{
				// key exists, do something with iter->second (the value)
				size_t track_no = -1;
				if (index_ptr[0] == 'v' || index_ptr[0] == 'V')
				{
					index_ptr++;
					//For now we only have up to 1-9 voices starting with 1
					if ((index_ptr[0] >= '1') && (index_ptr[0] <= '9'))
					{
						track_no = CTM[1] - '1';  //will convert cardinal string to ordinal index here
						assert(track_no != -1);
					}
					else
						assert(false);
					index_ptr++;
					//Our source will copy the voice track and present it to the track populator
					std::string TrackSegment;
					while ((*index_ptr != 'v') && (*index_ptr != 'V') && (*index_ptr != 0))
						TrackSegment += *index_ptr++;
					//Note: for now BPM is global, can make per block later
					if (!AppendTrack_CT(TrackSegment.c_str(),iter->second, track_no, m_Song.BeatPerMinute))
					{
						assert(false);
						break;
					}
				}
				else
					assert(false);  //this should not be possible!
			}
		}
		else
			assert(false);
		bool ret = false;
		return ret;
	}
public:
	NotePlayer_Internal()
	{

	}
	bool LoadSequence_CT(const char *filename)
	{
		bool ret = false;
		if (!filename || filename[0]==0)
		{
			const char *const ctm = "v1o4eao5co4bebo5diceo4#go5e";
			ret =PopulateBlock_CT(ctm,0);
		}
		return ret;
	}
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

bool NotePlayer::LoadSequence_CT(const char *filename)
{
	return m_Player->LoadSequence_CT(filename);
}