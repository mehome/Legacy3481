#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::remote;

// Constructor
server::server( void )
{	// Create an event
	::SetLastError( 0 );
	m_hNamed = ::CreateEventW( NULL, FALSE, FALSE, FC3::config::name_remote_server );
	
	// Check whether it existed
	if ( ( m_hNamed == NULL ) || ( ::GetLastError() == ERROR_ALREADY_EXISTS ) )
	{	// No server possible
		m_p_server = NULL;
	}
	else
	{	// Create the server
		m_p_server = new FrameWork::Communication3::implementation::remote::server();

		// Check for errors
		if ( m_p_server->error() )
		{	delete m_p_server;
			m_p_server = NULL;
		}
	}
}

// Destructor
server::~server( void )
{	// This is always safe even when m_p_server == NULL
	delete m_p_server;
	if ( m_hNamed ) 
		::CloseHandle( m_hNamed );
}

// Tells you whether a server was created or not
const bool server::error( void ) const
{	return m_p_server ? false : true;
}