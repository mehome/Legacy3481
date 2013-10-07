#include "StdAfx.h"
#include "FrameWork.Communication3.h"

namespace FC3i  = FC3i;
namespace FC3ir = FC3i::remote;

using namespace FC3ir;

struct global_client_cache : public client_cache {};
static global_client_cache g_global_client_cache;

client_cache& client_cache::get_cache( void )
{	return g_global_client_cache;
}

// Constructor
client_cache::client_cache( void )
{
}

// Destructor
client_cache::~client_cache( void )
{	// Destroy all clients
	while( !m_p_clients.empty() )
	{	delete m_p_clients.back();
		m_p_clients.pop_back();
	}
}

// Send a message to a client
const bool client_cache::send( const wchar_t *p_client_name, const int port_no, const wchar_t* p_dst_name, const void* p_data, const DWORD data_size )
{	while( true )
	{	// We lock for reading
		{	read_auto_lock	rd( m_clients_lock );

			// Cycle over all items
			for( std::vector< client* >::iterator i = m_p_clients.begin(); i != m_p_clients.end(); i++ )
			if ( ( !::wcsicmp( p_client_name, (*i)->server_name() ) ) && ( port_no == (*i)->port_no() ) )
			{	// Sockets try to reconnect
				// If this item has an error, we need to skip
				//if ( (*i)->error() ) break;
				
				// Send using this item
				return (*i)->send( p_dst_name, p_data, data_size );
			}
		}

		// We lock for writing
		{	write_auto_lock	wt( m_clients_lock );

			// Cycle over all items
			bool found = false;
			for( std::vector< client* >::iterator i = m_p_clients.begin(); i != m_p_clients.end(); i++ )
			if ( ( !::wcsicmp( p_client_name, (*i)->server_name() ) ) && ( port_no == (*i)->port_no() ) ) { found = true; break; }						

			// Create the item if its needed
			if ( !found )
			{	// We create a new item
				client* p_new_client = new client( p_client_name, port_no );		
			
				// Add this item to the list and send
				m_p_clients.push_back( p_new_client );		
			}

			// We now send the message by looping back through with only a ready lock
			// ...
		}
	}
}