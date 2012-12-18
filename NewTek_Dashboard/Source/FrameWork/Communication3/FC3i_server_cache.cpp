#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Get the cache
struct global_server_cache : public server_cache {} g_server_cache;
server_cache& server_cache::get_cache( void )
{	return g_server_cache;
}

// Constructor
server_cache::server_cache( void )
	: m_time_stamp( (DWORD)-1 )
{
}

// Destructor
server_cache::~server_cache( void )
{	// Lock for writing
	m_cache_lock.write_lock();

	// Cycle over all of the servers
	for( int i=0; i<(int)m_servers.size(); i++ )
		m_servers[ i ].m_p_server->release();

	// Unlock for writing
	m_cache_lock.write_unlock();
}

// This will recover a particular server pointer
server*	server_cache::get_server( const wchar_t name[] )
{	// We cannot work across the network
	// This code is safe without checking the length (!)
	if ( ( name[ 0 ] == '\\' ) && ( name[ 1 ] == '\\' ) ) return NULL;
	
	{	// First read-lock the list
		read_auto_lock	lock( m_cache_lock );

		// Try to locate the item
		iterator i = std::reverse_find( m_servers.begin(), m_servers.end(), name );
		if ( i != m_servers.end() )
		{	// It was found, so add a reference count and reset the time
			i->m_p_server->addref();
			i->m_time_stamp = m_time_stamp;

			// Finished
			return i->m_p_server;
		}
	}

		// Write lock the list
		write_auto_lock	lock( m_cache_lock );

		// Try to locate the item
		iterator i = std::reverse_find( m_servers.begin(), m_servers.end(), name );
		if ( i != m_servers.end() )
		{	// It was found, so add a reference count and reset the time
			i->m_p_server->addref();
			i->m_time_stamp = m_time_stamp;

			// Finished
			return i->m_p_server;
		}
		
		// Create a new server
		server*	p_ret = new server( name );
		if ( p_ret->error() ) 
		{	p_ret->release();
			return NULL;
		}

		server_desc new_item = { ++m_time_stamp, p_ret };
		m_servers.push_back( new_item );

		// Return the results
		p_ret->addref();
		return p_ret;
}

// This will clean up the cache of items that are no longer accessed and are outdated.
void server_cache::clean( void )
{	// We cycle over all the items
	int dst = 0;
	for( int i=0; i<(int)m_servers.size(); i++ )
	{	// If this item has only one reference count and is old enough
		if ( ( m_servers[ i ].m_p_server->refcount() == 1 ) &&
			 ( (long)( m_time_stamp - m_servers[ i ].m_time_stamp ) >= (long)FrameWork::Communication3::config::server_cache_history ) )
					// Remove the item
					m_servers[ i ].m_p_server->release();
		else		// Keep it
					m_servers[ dst++ ] = m_servers[ i ];
	}

	// Shorten the list
	m_servers.resize( dst );
}