#pragma once

// This will create a server and return true if it was possible.
// There can only be one server on any machine across all processes, 32 and 64 bit
struct FRAMEWORKCOMMUNICATION3_API server
{			// Constructor
			server( void );

			// Destructor
			~server( void );

			// Tells you whether a server was created or not
			const bool error( void ) const;

private:	// The handle
			HANDLE m_hNamed;

			// A pointer
			FrameWork::Communication3::implementation::remote::server	*m_p_server;
};