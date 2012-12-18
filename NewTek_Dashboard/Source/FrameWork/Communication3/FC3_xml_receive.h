#pragma once

struct FRAMEWORKCOMMUNICATION3_API receive : private FrameWork::Communication3::implementation::receive::client,
											 public FrameWork::Communication3::implementation::receive
{			// This is how you receive items
			struct FRAMEWORKCOMMUNICATION3_API client
			{	// Process the message here.
				// The virtual function names reflect the format delivered so multiple ingeritance from multiple client types is possible.
				// If this asserts, then you have probably got a race condition in setting up the vtable of this function. Ask Andrew
				// if this confuses you.
				virtual void deliver_xml( const FrameWork::Communication3::xml::message* p_xml_msg ) { assert( false ); }
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