#pragma once

class DS_Output_Internal;
struct audio_frame
{
	float *data;
};


/// Direct Sound output
class  DS_Output
{
	public:
		DS_Output();
		~DS_Output();

		//TODO see if you want to throw... this depends on how recoverable it is 
		/// \param AudioOutput  Throws exception if out of bounds
		//void SetAudioOutputChannel(size_t AudioOutput);
		//size_t GetAudioOutputChannel() const;

		/// \param AudioOutput  Throws exception if out of bounds
		//void AddOutputChannel(size_t AudioOutput);
		/// You may add additional destinations to send your frames.  Let me know if we need to add a remove method.
		//virtual void add_thru_dest(const wchar_t *audio_destination_name);

		virtual void StartStreaming(void);
		virtual void StopStreaming(void);

		//void SetUseDisplayedCallbacks(bool UseDisplayedCallbacks);
		//WAVEFORMATEX *GetWaveFormatToUse(void);

	protected:

		bool m_IsStreaming=false;
	private:
		//the file server, we use scoped pointer to delay instantiation until m_DS_Output_Internal has been initialized
		//SCOPED_PTR<FrameWork::Communication3::audio::receive> m_Server_Audio;

		//Cached since this is only managed on construction
		//size_t m_QueueDepthLevel;
		//std::wstring m_AudioName;
		//bool m_ServerStarted; //Clearly mark when the server has been started

		size_t m_AudioOutput;
		//size_t m_CurrentFieldIndex_audio; // defined instead in DS_Output_Internal
		std::shared_ptr<DS_Output_Internal>  m_DS; //encapsulate SDK specifics from public
		//HD::Base::AudioScope m_AudioScope;

		//struct DestThru
		//{
		//	std::wstring Audio;
		//};

		//std::vector<DestThru> m_Dest_Thru;

		//double m_AudioSampleRate;
		//std::queue <const AudioMsg *> m_IncomingBufferQueueFrame_audio;
		//FrameWork::Threads::critical_section m_BlockIncomingBufferQueue_audio;
};

class DirectSound_Globals;

class  DirectSound_Initializer
{
public:
	DirectSound_Initializer( const std::vector<std::wstring>* p_device_list = NULL ) {m_DirectSoundInitStatus=StartUp_DirectSound(p_device_list);}
	~DirectSound_Initializer(){ ShutDown_DirectSound(); }
	operator bool (void) const {return m_DirectSoundInitStatus;}
	bool GetDirectSoundInitStatus() {return m_DirectSoundInitStatus;}
protected:
private:
	bool StartUp_DirectSound( const std::vector<std::wstring>* p_device_list );
	void ShutDown_DirectSound();
	bool m_DirectSoundInitStatus;
	static long s_ref;
};
