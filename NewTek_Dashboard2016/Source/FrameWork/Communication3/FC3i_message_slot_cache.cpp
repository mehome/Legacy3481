#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3i;

// Get the cache
struct global_slot_cache : public slot_cache {} g_slot_cache;
slot_cache& slot_cache::get_cache( void )
{	return g_slot_cache;
}

// Constructor
slot_cache::slot_cache( void )
	: m_time_stamp( (DWORD)-1 )
{
}

// Destructor
slot_cache::~slot_cache( void )
{	// Lock for writing
	m_cache_lock.write_lock();

	// Cycle over all of the slots
	for( int i=0; i<(int)m_slots.size(); i++ )
		m_slots[ i ].m_p_slot->release();

	// Unlock for writing
	m_cache_lock.write_unlock();
}

// This will recover a particular slot pointer
message_slot* slot_cache::get_slot( const wchar_t name[] )
{	// We cannot work across the network
	// This code is safe without checking the length (!)
	if ( ( name[ 0 ] == '\\' ) && ( name[ 1 ] == '\\' ) ) return NULL;
	
	{	// First read-lock the list
		read_auto_lock	lock( m_cache_lock );

		// Try to locate the item
		iterator i = std::reverse_find( m_slots.begin(), m_slots.end(), name );
		if ( i != m_slots.end() )
		{	// It was found, so add a reference count and reset the time
			i->m_p_slot->addref();
			i->m_time_stamp = m_time_stamp;

			// Finished
			return i->m_p_slot;
		}
	}

		// Write lock the list
		write_auto_lock	lock( m_cache_lock );

		// Try to locate the item
		iterator i = std::reverse_find( m_slots.begin(), m_slots.end(), name );
		if ( i != m_slots.end() )
		{	// It was found, so add a reference count and reset the time
			i->m_p_slot->addref();
			i->m_time_stamp = m_time_stamp;

			// Finished
			return i->m_p_slot;
		}
		
		// Create a new slot
		message_slot* p_ret = new message_slot( name );
		if ( p_ret->error() ) 
		{	p_ret->release();
			return NULL;
		}

		slot_desc new_item = { ++m_time_stamp, p_ret };
		m_slots.push_back( new_item );

		// Return the results
		p_ret->addref();
		return p_ret;
}