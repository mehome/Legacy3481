#pragma once

struct FRAMEWORKCOMMUNICATION3_API receive : private FrameWork::Communication3::implementation::receive::client,
											 public FrameWork::Communication3::implementation::receive
{			// This is how you receive items
			struct FRAMEWORKCOMMUNICATION3_API client : // Overload the functions used here
														public FrameWork::Communication3::video::receive::client,
														// Overload the functions used here
														public FrameWork::Communication3::audio::receive::client,
														// Overload the functions used here
														public FrameWork::Communication3::xml::receive::client
			{	
			};

			// Constructor
			receive( const wchar_t name[], client *p_dst, const bool flush_queue = false );

			// Destructor
			~receive( void );

private:	// The client implementation
			virtual void deliver( const DWORD block_id, const DWORD addr );

			// The current destination
			client *m_p_destination;
};