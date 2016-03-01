#pragma once

struct FRAMEWORKAUDIO2_API wav
{			// Open from a file
			wav( const wchar_t* p_fn );

			// Destructor
			~wav( void );

private:	// The buffer
			FrameWork::Audio2::buffer_f32	m_data;
};