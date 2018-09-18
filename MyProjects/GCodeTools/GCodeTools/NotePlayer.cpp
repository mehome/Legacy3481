#include "pch.h"
#include "Time_Type.h"
#include "NotePlayer.h"
#include "SineWaveGenerator.h"
//For now using a similar type of data structure as that used in OctaMed
struct Song
{
	struct Note
	{
		Note(double _FreqInHertz, double _Duration) : FreqInHertz(_FreqInHertz), Duration(_Duration)
		{
		}
		Note() : FreqInHertz(1000.0),Duration(1.0)
		{}
		double FreqInHertz; //zero indicates rest
		double Duration;
	};
	struct Track
	{
		std::map<double, Note> notes; //a list of notes for one track with PTS (in seconds) in a map form where zero point of origin is per block
		double current_time=0.0;  //maintain last time for this track (for append, and for duration count)
	};
	struct Block
	{
		std::map<size_t, Track> tracks;  //we'll just map out the voices as well
		using block_iter = std::map<size_t, Track>::iterator;
		double block_duration=0.0;
	};
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
		if (Note[0] == '-')
			return 0.0;
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
		case 'b':case 'B':		halfstep_count = 2;		break;
		case 'c':case 'C':		halfstep_count = 3;		Octave--;	break;
		case 'd':case 'D':		halfstep_count = 5;		Octave--;	break;
		case 'e':case 'E':		halfstep_count = 7;		Octave--;	break;
		case 'f':case 'F':		halfstep_count = 8;		Octave--;	break;
		case 'g':case 'G':		halfstep_count = 10;	Octave--;	break;
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
		for (size_t i = 0; i < Octave; i++)
			frequency *= 2;
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
		Block::block_iter iter = block.tracks.find(TrackNumber);
		if (iter == block.tracks.end())
		{
			block.tracks[TrackNumber] = Track();
			iter = block.tracks.find(TrackNumber);
		}
		if (iter != block.tracks.end())
		{
			Track &track = iter->second;
			//add notes here:
			double current_time = track.current_time; //the easy way to find where we are
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
					break;
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
				case 'r':
				case 'R':
				{
					//form the note:
					std::string Note="---"; //default to a rest
					//if it is not a rest put in the note
					if ((*index_ptr != 'r') && (*index_ptr != 'R'))
					{
						Note[0] = *index_ptr;
						Note[1] = accidental;
						Note[2] = (char)Octave + '0';
					}
					double frequency = GetFrequency(Note.c_str());
					//now to compute duration
					const double duration = WPS * DurationScaleFactor;
					//populate the note
					Song::Note note(frequency, duration);
					track.notes[current_time] = note;
					//reset accidental for next note
					accidental = '-';
					//reset dotted
					Dotted = false;
					current_time += duration; //advance the time
					track.current_time = current_time;  //update the length of the track
					break;
				}
				default:
					assert(false);  //what do we have
				}
				index_ptr++;  //all cases should advance... we can make exceptions if necessary
			}
			ret = *index_ptr == 0;
		}
		return ret;
	}
	//Block starting with 0... block may be appended on multiple calls
	bool PopulateBlock_CT(const char *CompactTextMusic,size_t BlockNumber)
	{
		bool ret = false;
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
					Block &block = iter->second;
					ret=AppendTrack_CT(TrackSegment.c_str(), block, track_no, m_Song.BeatPerMinute);
					//augment the block duration to the track with the greatest amount of time
					//we can leave it this simple as I do not intend to make blocks shorter using this kind of parser
					if (block.tracks[track_no].current_time > block.block_duration)
						block.block_duration = block.tracks[track_no].current_time;
					if (!ret)
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
		return ret;
	}

	//This will play the song by populating callback buffers from dsound
	class WavePlayer
	{
	private:
		std::shared_ptr<DirectSound::Output::DS_Output> m_DSound;
		generator sine_wave;
		size_t m_block_number = 0;
		double m_current_block_time = 0.0;
		bool m_IsStreaming=false;

		//Not sure yet if I need to cache the track positions like this, but I will need to get the actual block
#if 0
		struct Song_Cache
		{
			struct Track
			{
				size_t note_index=0;
				double current_time = 0.0;  //how much time into this note we are
			};
			struct Block
			{
				std::map<size_t, Track> tracks;  //we'll just map out the voices as well
				using block_iter = std::map<size_t, Track>::iterator;
				//double current_time = 0.0;  not sure yet if I need this
			};
			using Sequence = std::map<size_t, Block>;

			double BeatPerMinute = 120;  //a master clock for playback... default to an easy divisible number for debugging
			Sequence Music_Cache;  //A container to store the song
		} m_cache;
		Song_Cache::Block &FindBlock()
		{
			using Sequence = Song_Cache::Sequence;
			using Block = Song_Cache::Block;
			Sequence &music = m_cache.Music_Cache;
			Sequence::iterator iter = music.find(m_block_number);
			if (iter == music.end())
			{
				music[m_block_number] = Block();
				iter = music.find(m_block_number);
			}
			if (iter != music.end())
			{
				return iter->second;
			}
			else
			{
				assert(false);
				Block *block = nullptr;
				return *block;  //not going to make this recoverable
			}
		}
#endif
		void client_fillbuffer(size_t no_channels, short *dst_buffer, size_t no_samples)
		{
			printf("time=%.2f %d\n",m_current_block_time,no_samples);
			//Use cache to determine which note to play
			//using Block = Song_Cache::Block;
			//Block block = FindBlock();
			//advance time
			m_current_block_time += no_samples / 48000.0;  //TODO: I shouldn't hard code the samplerate
		}
	public:
		void Link_DSound(std::shared_ptr<DirectSound::Output::DS_Output> &instance)
		{
			m_DSound = instance;
			//Use a lambda technique to provide a standard function to callback to a method
			std::function<void(size_t ,short *,size_t )> Callback = [this](size_t no_channels,short *dst_buffer,size_t no_samples)
			{ this->client_fillbuffer(no_channels,dst_buffer,no_samples); };
			m_DSound->SetCallbackFillBuffer(Callback);
		}
		void StartStreaming()
		{
			if (!m_IsStreaming)
			{
				m_IsStreaming = true;
				m_DSound->StartStreaming();
			}
		}
		void StopStreaming()
		{
			if (m_IsStreaming)
			{
				m_IsStreaming = false;
				m_DSound->StopStreaming();
			}
		}
		void SetBlockNumber(size_t block_no) { m_block_number = block_no; }
		void SetBlockTime(double time) { m_current_block_time = time; }
	} m_WavePlayer;
public:
	NotePlayer_Internal()
	{
	}
	void Link_DSound(std::shared_ptr<DirectSound::Output::DS_Output> &instance)
	{
		m_WavePlayer.Link_DSound(instance);
	}
	bool LoadSequence_CT(const char *filename)
	{
		bool ret = false;
		if (!filename || filename[0]==0)
		{
			const char *const ctm = "v1o4sreao5co4bebo5diceo4#go5e";
			ret =PopulateBlock_CT(ctm,0);
		}
		//TODO load a file and populate it per line per block
		return ret;
	}
	void PlayBlock(size_t block_number)
	{
		m_WavePlayer.StopStreaming();  //stop it if it was playing
		//setup the play cursor
		m_WavePlayer.SetBlockNumber(block_number);
		m_WavePlayer.SetBlockTime(0.0);
		m_WavePlayer.StartStreaming();
	}
	void Stop()
	{
		m_WavePlayer.StopStreaming();
	}
	void SeekBlock(double position)
	{
	}
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

void NotePlayer::Link_DSound(std::shared_ptr<DirectSound::Output::DS_Output> &instance) 
{
	m_Player->Link_DSound(instance);
}

void NotePlayer::PlayBlock(size_t block_number)
{
	m_Player->PlayBlock(block_number);
}

void NotePlayer::Stop()
{
	m_Player->Stop();
}

void NotePlayer::SeekBlock(double position)
{
}
