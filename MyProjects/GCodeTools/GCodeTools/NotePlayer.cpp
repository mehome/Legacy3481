//TODO
//get multiple blocks playing and exporting
//get file reading and writing to work
//support tg command for beats per minute
//seek (low priority), playing from position

#include "pch.h"
#include "Time_Type.h"
#include "NotePlayer.h"
#include "SineWaveGenerator.h"
#include"VectorMath.h"
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
		using const_track_iter = std::map<double, Note>::const_iterator;
		double current_time=0.0;  //maintain last time for this track (for append, and for duration count)
	};
	struct Block
	{
		std::map<size_t, Track> tracks;  //we'll just map out the voices as well
		using block_iter = std::map<size_t, Track>::iterator;
		using const_block_iter = std::map<size_t, Track>::const_iterator;
		double block_duration=0.0;
	};
	//Actually there would be another layer where the sequence represented block numbers, but I'm not going that far yet
	//for now its one block list that goes from start to finish... no repeats, as long as the song technique is method driven to advance
	//it should be seamless to add later
	using Sequence = std::map<size_t, Block>;

	//double BeatPerMinute = 120;  //a master clock for playback... default to an easy divisible number for debugging
	double BeatPerMinute = 80; //TODO make this programable
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
	static __inline bool NoteInRange(double a_begin, double a_end, double b_begin, double b_end)
	{
		bool ret = false;
		//this is similar logic to Rick's timeline region code
		//establish where B is... in relation to A range... either before in or after
		if (b_begin > a_begin)
		{
			//not before
			ret = b_begin < a_end;  //it is in some where after A start
		}
		else if (b_begin < a_begin)
		{
			//before... check B end if it overlaps
			ret = (b_end > a_begin);  //has to be greater... if it's equal its not really in range
		}
		else
		{
			assert(a_begin == b_begin);   //This better be true... we may have to deal with precision issues later
			ret = true;
		}
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
		bool m_AutoAdvanceBlock = false;  //this is really the difference between playing the song and a block
		const Song &m_Song; //<-- read only!
		//Not sure yet if I need to cache the track positions like this, but I will need to get the actual block
		struct Song_Cache
		{
			size_t m_LastBlock_Number=-1;
			Song::Sequence::const_iterator m_CurrentBlockNumber;  //used by play song to quickly advance to the next block
			const Song::Block *m_LastBlock_ptr=nullptr;
			struct Track_Cache
			{
				Track_Cache(const Song::Track *_track_ptr,size_t _track_number) : track_ptr(_track_ptr), track_number(_track_number),
					m_NoteIndex(_track_ptr->notes.begin())
				{
				}
				const Song::Track *track_ptr=nullptr;  //the actual track
				const size_t track_number;  //keep track number when advancing to next track
				//double m_LastPositionTime=0.0;  //last time of this note  <---we may be able to work without this
				Song::Track::const_track_iter m_NoteIndex;  //keep index on note
			};
			std::vector<Track_Cache> m_tracks;
			void flush()
			{
				m_tracks.clear();
				m_LastBlock_ptr = nullptr;
				m_LastBlock_Number = -1;
			}
		} m_cache;

		const Song::Block *FindBlock()
		{
			using Sequence = Song::Sequence;
			using Block = Song::Block;
			const Sequence &music = m_Song.Music;
			m_cache.m_CurrentBlockNumber = music.find(m_block_number);
			const Sequence::const_iterator &iter = m_cache.m_CurrentBlockNumber;
			if (iter != music.end())
			{
				return &iter->second;
			}
			else
			{
				//No block exists... was it loaded
				//assert(false);
				Block *block = nullptr;
				return block;  //not going to make this recoverable
			}
		}

		void client_fillbuffer(size_t no_channels, short *dst_buffer, size_t no_samples)
		{
			assert(no_channels == 2);  //always work with 2 channels
			const double sample_rate = 48000.0; //TODO: I shouldn't hard code the samplerate
			const double duration= no_samples / sample_rate;
			//printf("time=%.2f %d\n",m_current_block_time,no_samples);
			//Use cache to determine which note to play
			using Block = Song::Block;
			using const_block_iter = Block::const_block_iter;
			const Block *block = m_cache.m_LastBlock_ptr;
			if (m_cache.m_LastBlock_Number != m_block_number)
				block = FindBlock();
			if (block)
			{
				//Now we have a block... get the tracks
				if (m_cache.m_tracks.empty())
				{
					//grab all the tracks
					for (auto &i : block->tracks)
						m_cache.m_tracks.push_back(Song_Cache::Track_Cache(&i.second,i.first));
				}
				double end_time = m_current_block_time + duration;  //handy to have this for range
				size_t voice = 0;  //keep track of the track count to work out how to add on the sine wave
				short *dst_buffer_end = dst_buffer + (no_samples * no_channels);  //must never pass this point
				bool AdvancedTracks = false;  //if any track advances this will be true
				for (auto &i : m_cache.m_tracks)
				{
					short *dst_buffer_index = dst_buffer;  //reset the buffer as we are on the next channel
					double current_time_index = m_current_block_time;  //reset this time as well
					const size_t channel = voice & 1;  //all odd's on one side, all evens on the other
					const bool add_samples = voice > 1;  //all voice past the first two need to be added 
					voice++;  //done reading voice... increment for next iteration
					Song_Cache::Track_Cache *track = &i;  //grab the track in case it advances to next block
				PlayTrack:
					while ((dst_buffer_index < dst_buffer_end)&&(track->m_NoteIndex!=track->track_ptr->notes.end()))
					{
						//Now we have a track determine our note index and time
						const double note_start = track->m_NoteIndex->first;
						const double note_duration = track->m_NoteIndex->second.Duration;
						const double note_end = note_start + note_duration;
						//TODO find way to pick note if it isn't in range
						if (NoteInRange(note_start, note_end, current_time_index, end_time))
						{
							if (note_start > current_time_index)
							{
								assert(note_start != 0.0); //this should not be possible!
								//grab previous note
								Song::Track::const_track_iter prev_note = track->m_NoteIndex;
								prev_note--;
								if (prev_note->first > current_time_index)
								{
									assert(false);
									//todo else.. seeking using find assign to prev_note
								}
								size_t pre_no_samples = (size_t)((note_start - current_time_index) * sample_rate);
								//sanity check... todo this may be possible if we have spaces, but since we have rests, it shouldn't be an issue
								assert(pre_no_samples <= (size_t)(prev_note->second.Duration * sample_rate));
								//populate this range with this note's frequency... todo may need to do multiple notes here
								sine_wave.frequency(channel, prev_note->second.FreqInHertz);
								sine_wave.gen_sw_short(channel, dst_buffer_index + channel, pre_no_samples);
								//advance the dest buffer index
								dst_buffer_index += (pre_no_samples* no_channels);
								current_time_index = note_start;
							}
							//Now the current time and the note time are either aligned, or the note already has played from a previous fill buffer
							double adjusted_duration = note_end - current_time_index;
							size_t adj_no_samples = (size_t)(adjusted_duration * sample_rate);  //start with the maximum amount of samples to populate
							const size_t samples_remain = (dst_buffer_end - dst_buffer_index) / no_channels;
							bool advance_note = true;
							if (adj_no_samples > samples_remain)
							{
								adj_no_samples = samples_remain;
								adjusted_duration = adj_no_samples / sample_rate;
								advance_note = false;
							}
							sine_wave.frequency(channel, track->m_NoteIndex->second.FreqInHertz);
							sine_wave.gen_sw_short(channel, dst_buffer_index + channel, adj_no_samples);
							//advance the dest buffer index
							dst_buffer_index += (adj_no_samples* no_channels);
							current_time_index += adjusted_duration;
							//advance to the next note:
							if (advance_note)
								track->m_NoteIndex++;
						}
					}
					if (dst_buffer_index < dst_buffer_end)
					{
						//This bool is redundant because I have a goto for success but I can make an assertion to make it clear
						bool AbleToAdvance = m_AutoAdvanceBlock;
						if (m_AutoAdvanceBlock)
						{
							AbleToAdvance = false;
							Song::Sequence::const_iterator current_block = m_cache.m_CurrentBlockNumber;
							//make sure we are not already at the end
							if (current_block != m_Song.Music.end())
							{
								current_block++; //we are advanced... now to reset the block time
								if (current_block != m_Song.Music.end())
								{
									AbleToAdvance = true;  //short lived but here for completion
									AdvancedTracks = true;
									current_time_index = 0.0;
									end_time = m_current_block_time + duration;
									//advance the track
									i.track_ptr = &(current_block->second.tracks.find(i.track_number)->second);
									i.m_NoteIndex = i.track_ptr->notes.begin();
									goto PlayTrack;
								}
							}
						}
						assert(!AbleToAdvance);  //if it has made it this far it's because it can't advance
						if (!AbleToAdvance)
						{
							//this will happen when we reach the end of the block... for now we simply fill with silence, but we may loop as well
							const size_t samples_remain = (dst_buffer_end - dst_buffer_index) / no_channels;
							sine_wave.frequency(channel, 0.0);
							sine_wave.gen_sw_short(channel, dst_buffer_index + channel, samples_remain);
							//nothing to advance here
						}
					}
				}
				if (AdvancedTracks)
				{
					//advance the block... all voices should be in sync... as this will be triggered if any have advanced
					m_cache.m_CurrentBlockNumber++;
					m_cache.m_LastBlock_ptr = &m_cache.m_CurrentBlockNumber->second;
					block = m_cache.m_LastBlock_ptr;
					m_current_block_time = 0.0;
					m_block_number++;
					m_cache.m_LastBlock_Number = m_block_number;
				}
			}
			//advance time
			m_current_block_time += duration;
		}
	public:
		WavePlayer(const Song &song) : m_Song(song)
		{}
		void Link_DSound(std::shared_ptr<DirectSound::Output::DS_Output> &instance)
		{
			m_DSound = instance;
			//I'll keep both techniques for reference, but lambda technique is more efficent
			#if 1
			//Use a lambda technique to provide a standard function to callback to a method
			auto Callback = [this](size_t no_channels,short *dst_buffer,size_t no_samples) 	
				{ 	this->client_fillbuffer(no_channels,dst_buffer,no_samples); 
				};
			m_DSound->SetCallbackFillBuffer(Callback);
			#else
			//Using std::bind also works making it a bit more streamlined
			m_DSound->SetCallbackFillBuffer(std::bind(&WavePlayer::client_fillbuffer,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
			#endif	
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
		void PlayBlock(size_t block_number)
		{
			StopStreaming();  //stop it if it was playing
			m_cache.flush();
			//setup the play cursor
			m_block_number = block_number;
			m_current_block_time = 0.0;
			m_AutoAdvanceBlock = false;
			StartStreaming();
		}
		void PlaySong(double position)
		{
			StopStreaming();
			m_cache.flush();
			//TODO set position here for now always at the beginning
			m_current_block_time = 0.0;
			m_block_number = 0;
			m_AutoAdvanceBlock = true;
			StartStreaming();
		}
	} m_WavePlayer;
	class GCode_Writer
	{
	private:
		const Song &m_Song; //<-- read only!
		std::string m_BlockWrite;
		struct Interleaved_element
		{
			Interleaved_element() : x(0.0),y(0.0),z(0.0),duration(0.0) {}
			Interleaved_element(double _x, double _y, double _z) : x(_x), y(_y), z(_z), duration(0.0) {}
			Interleaved_element(const Vec3d &vec3) : x(vec3.x()), y(vec3.y()), z(vec3.z()) {}
			double x, y, z; //frequency in hertz
			double duration; //in seconds
		};
		std::vector<Interleaved_element> m_block_list;
		//this will step through the first 3 tracks and segment out each interleaved entry
		bool populate_block_list(size_t block_number)
		{
			using Sequence = Song::Sequence;
			using Block = Song::Block;
			using Track = Song::Track;
			const Sequence &music = m_Song.Music;
			Sequence::const_iterator seq_iter = music.find(block_number);
			bool result = false;

			if (seq_iter != music.end())
			{
				const Block &block = seq_iter->second;
				Block::const_block_iter block1_iter = block.tracks.find(0);
				assert(block1_iter != block.tracks.end()); //we must have at least one voice!
				const Track &track1 = block1_iter->second;
				Track::const_track_iter voice1 = block1_iter->second.notes.begin();
				Block::const_block_iter block_iter = block.tracks.find(1);
				//See if we have a track 2
				const Track *track2 = (block_iter != block.tracks.end()) ? &block_iter->second: nullptr;
				//We set the iterator to the beginning of its track if it exists to the end of track1 otherwise
				Track::const_track_iter voice2 = (block_iter != block.tracks.end())? 
					track2->notes.begin() :	block1_iter->second.notes.end();
				block_iter = block.tracks.find(2);
				//the same for voice3
				const Track *track3 = (block_iter != block.tracks.end()) ? &block_iter->second : nullptr;
				Track::const_track_iter voice3 = (block_iter != block.tracks.end()) ?
					track3->notes.begin() : block1_iter->second.notes.end();
				double current_time_index = 0.0;
				//iterate through all the notes for all 3 voices
				while ((voice1!=track1.notes.end()) &&
					(!track2 || voice2!=track2->notes.end()) &&
					(!track3 || voice3!=track3->notes.end()))
				{
					//sanity check... we should never prematurely advance past the timer... this holds true as long as we do not have
					//any extra spacing between notes and/or rests
					assert((voice1->first <= current_time_index) &&
						(!track2 || voice2->first <= current_time_index) &&
						(!track3 || voice3->first <= current_time_index));
					Interleaved_element element;  //go ahead and populate this entry
					element.x = voice1->second.FreqInHertz;
					element.y = track2 ? voice2->second.FreqInHertz : 0.0;
					element.z = track3 ? voice3->second.FreqInHertz : 0.0;
					//negotiate a duration which is a min operation of each
					double NoteOffset = current_time_index - voice1->first;
					const double voice1_duration = voice1->second.Duration - ((NoteOffset>0.0) ? NoteOffset : 0.0);
					double duration = voice1_duration;
					if (track2)
					{
						NoteOffset = current_time_index - voice2->first;
						const double voice2_duration = voice2->second.Duration - ((NoteOffset>0.0) ? NoteOffset : 0.0);
						duration = min(duration, voice2_duration);
					}
					if (track3)
					{
						NoteOffset = current_time_index - voice3->first;
						const double voice3_duration = voice3->second.Duration - ((NoteOffset>0.0) ? NoteOffset : 0.0);
						duration = min(duration, voice3_duration);
					}
					element.duration = duration;
					m_block_list.push_back(element);
					//advance the time
					current_time_index += duration;
					//test each voice advance only if the end note < or = the current time
					if (voice1->first + voice1->second.Duration <= current_time_index)
						voice1++;
					if ((track2)&&(voice2->first + voice2->second.Duration <= current_time_index))
						voice2++;
					if ((track3) && (voice3->first + voice3->second.Duration <= current_time_index))
						voice3++;
				}
				//all three voices should have reached the end
				result = (voice1 == track1.notes.end() &&
					(!track2 || voice2 == track2->notes.end()) &&
					(!track3 || voice3 == track3->notes.end()));
			}
			return result;
		}
		//This will return positive deltas of each vector along with the feed speed
		//the return can use the same struct replacing duration for the feed speed
		Interleaved_element GetPositionDeltas(const Interleaved_element &source)
		{
			const double scale_tune = 0.059; //this came from feed of 25.97 producing 440.5 tone
			Vec3d vector(source.x, source.y, source.z);
			vector *= scale_tune;
			//I have a vector with correct direction, but will need to work out the magnitude
			double magnitude = vector.length(); //this becomes the feedrate:
			//one way to check the math
			//we know that F6.5 is 77.7 when x and y are at a 45 degree angle
			//when 6.5 is 110 when only one dimension is used
			//this is because sin(45) = rouphly .70 * 110 = 77.7
			//when using the magnitude the feedrate boosts from 6.5 to 9.192388155425117
			//which gives a rate of 155.80 (instead of 110... using same 0.059 scale tune)
			//now when testing sin(45) * 155.8 brings it back down to 110

			//currently when the magnitude is equal to the feedrate the time equals one minute
			vector *= 1.0 / 60.0;
			//now time is one second... same units as the duration
			vector *= source.duration;
			//Now we scale vector to equal the duration
			Interleaved_element ret(vector);
			ret.duration = magnitude;
			return ret;
		}
		double m_last_x = 0.0;
		double m_last_y = 0.0;
		double m_last_z = 0.0;
	public:
		GCode_Writer(const Song&song) : m_Song(song)
		{}
		const char *WriteBlock(size_t block_number)
		{
			//get the interleaved list
			populate_block_list(block_number);
			//iterate through each element
			for (auto &i : m_block_list)
			{
				Interleaved_element vector_feed=GetPositionDeltas(i);
				//printf("x%.3f y%.3f z%.1f f%.1f\n",vector_feed.x,vector_feed.y,vector_feed.z,vector_feed.duration);
				//TODO determine direction
				//use absolute positioning so we can monitor the edge cases
				m_last_x += vector_feed.x;
				m_last_y += vector_feed.y;
				m_last_z += vector_feed.z;
				char Buffer[70];  //we have 70 on CNC machine, but really I never plan to go this far
				sprintf(Buffer,"X%.4f Y%.4f Z%.4f F%.1f\n", m_last_x, m_last_y, m_last_z, vector_feed.duration);
				m_BlockWrite += Buffer;
			}
			return m_BlockWrite.c_str();
		}
	} m_GCode_Writer;
public:
	NotePlayer_Internal() : m_WavePlayer(m_Song),m_GCode_Writer(m_Song)
	{
	}
	void Link_DSound (std::shared_ptr<DirectSound::Output::DS_Output> &instance)
	{
		m_WavePlayer.Link_DSound(instance);
	}
	bool LoadSequence_CT(const char *filename)
	{
		bool ret = false;
		if (!filename || filename[0]==0)
		{
			//simple test song
			const bool test2voices = true;
			const bool test2blocks = true;
			const char *const ctm = "v1o4sreao5co4bebo5diceo4#go5e";
			ret =PopulateBlock_CT(ctm,0);
			
			if (test2voices && ret)
			{
				//const char *const ctm_v2 = "v2o4sreao5co4bebo5diceo4#go5e";
				const char *const ctm_v2 = "v2o2iao3a#grsaeao4co3bebo4d";
				ret = PopulateBlock_CT(ctm_v2, 0);
			}
			if (test2blocks && ret)
			{
				const char *const ctm = "v1o4saeao5co4bebo5dico4aqr";
				ret = PopulateBlock_CT(ctm, 1);
				if (test2voices && ret)
				{
					const char *const ctm_v2 = "v2o4ico3a#gesaeao4co3bebo4d";
					ret = PopulateBlock_CT(ctm_v2, 1);
				}
			}
		}
		//TODO load a file and populate it per line per block
		return ret;
	}
	void PlayBlock(size_t block_number)
	{
		m_WavePlayer.PlayBlock(block_number);
	}
	void PlaySong(double position)
	{
		m_WavePlayer.PlaySong(position);
	}
	void Stop()
	{
		m_WavePlayer.StopStreaming();
	}
	void SeekBlock(double position)
	{
	}
	bool ExportGCode(const char *filename)
	{
		//for now do one block
		const char *block=m_GCode_Writer.WriteBlock(0);
		if (!filename)
			printf("test->%s\n",block);
		return (block != nullptr);
	}
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

void NotePlayer::Link_DSound(std::shared_ptr<DirectSound::Output::DS_Output> instance) 
{
	m_Player->Link_DSound(instance);
}

void NotePlayer::PlayBlock(size_t block_number)
{
	m_Player->PlayBlock(block_number);
}

void NotePlayer::PlaySong(double position)
{
	m_Player->PlaySong(position);
}

void NotePlayer::Stop()
{
	m_Player->Stop();
}

void NotePlayer::SeekBlock(double position)
{
}

bool NotePlayer::ExportGCode(const char *filename)
{
	return m_Player->ExportGCode(filename);
}
