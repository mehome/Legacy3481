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


		virtual void StartStreaming(void);
		virtual void StopStreaming(void);


	protected:

		bool m_IsStreaming=false;
	private:

		size_t m_AudioOutput;
		std::shared_ptr<DS_Output_Internal>  m_DS; //encapsulate SDK specifics from public
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
