#pragma once

struct FRAMEWORKCOMMUNICATION3_API receive : private FrameWork::Communication3::implementation::receive::client,
											 public FrameWork::Communication3::implementation::receive
{			// This is how you receive items
			struct FRAMEWORKCOMMUNICATION3_API client
			{	// Process the message here. If there is no audio or video
				// then one of the pointers might be NULL. 
				// The virtual function names reflect the format delivered so multiple ingeritance from multiple client types is possible.
				// If this asserts, then you have probably got a race condition in setting up the vtable of this function. Ask Andrew
				// if this confuses you.
				virtual void deliver_video_audio( const FrameWork::Communication3::video::message* p_vid_msg,
												  const FrameWork::Communication3::audio::message* p_aud_msg ) { assert( false ); }
			};
	
			// Constructor
			receive( const wchar_t name[], client *p_dst, const bool flush_queue = false );

			// Destructor
			~receive( void );

			// This allows you to get and set the ability for this server to run in low latency mode. When this is enabled then
			// you will never have FrameWork::Communication3::video::message and FrameWork::Communication3::audio::message at the same time. Instead one will always be NULL.
			// This takes out the potential for one field of latency that is caused by trying to sync the timestamps from A+V
			void set_av_sync( const bool flag );
			bool get_av_sync( void ) const;


private:	// The client implementation
			virtual void deliver( const DWORD block_id, const DWORD addr );

			// The current destination
			client *m_p_destination;

			// Do we want to sync AV together
			bool m_sync_av;

			// We need to store the last frame.
			const FrameWork::Communication3::video::message* m_p_vid_msg;
			const FrameWork::Communication3::audio::message* m_p_aud_msg;
};