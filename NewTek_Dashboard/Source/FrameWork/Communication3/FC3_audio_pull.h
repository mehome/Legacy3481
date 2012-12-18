#pragma once

struct FRAMEWORKCOMMUNICATION3_API pull : public FrameWork::Communication3::implementation::receive
{			// Constructor
			pull( const wchar_t name[], const bool flush_queue = false );

			// Destructor
			~pull( void );

			// Get a message from the queue
			// You are responsible for releasing this message. For instance
			//	pull		hello( L"blah" );
			//	message*	p_msg = hello();
			//	if ( p_msg ) p_msg->release();
			const FrameWork::Communication3::audio::message* operator() ( const DWORD timeout = INFINITE );
};