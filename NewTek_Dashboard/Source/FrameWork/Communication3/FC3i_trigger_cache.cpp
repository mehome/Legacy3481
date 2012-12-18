#include "StdAfx.h"
#include "FrameWork.Communication3.h"

// Use the namespace
using namespace FrameWork::Communication3::implementation;

// The shared data segment
#pragma bss_seg( ".fc3" )

// This is the current, cross process block that is currently being used to	
// allocate from. When we run out of blocks, this is incremented.
DWORD trigger_cache::g_next_event_id;

#pragma bss_seg()

// Get the global cache
struct global_trigger_cache : public trigger_cache {} g_trigger_cache;
trigger_cache& trigger_cache::get_cache( void )
{	return g_trigger_cache;
}

// Constructor
trigger_cache::trigger_cache( void )
	: m_time_stamp( -1 )
{
}

// Destructor
trigger_cache::~trigger_cache( void )
{	// We are going to dispose all of the cache items
	m_cache_lock.write_lock();

	// Cycle over all of the currently accessed 
	for( int i=0; i<(int)m_events.size(); i++ )
		m_events[ i ].m_p_trigger_event->release();

	// Unlock the list
	m_cache_lock.write_unlock();
}

// This will get a event of known id
bool trigger_cache::new_trigger( trigger *p_dst )
{	// We are always going to create a new event here.
	// Note that we cannot have an ID of 0
	DWORD new_id = (DWORD)::_InterlockedIncrement( (LONG*)&g_next_event_id );
	while( !new_id ) new_id = (DWORD)::_InterlockedIncrement( (LONG*)&g_next_event_id );

	// Create the event ID
	trigger_event* p_new_event = new trigger_event( new_id );
	if ( p_new_event->error() ) 
	{	p_new_event->release();
		return NULL;
	}

	// We do need to add it to the cache
	write_auto_lock	lock( m_cache_lock );

	// Add it to the list, we add the time-stamp tot he number created
	p_new_event->addref();
	const block_desc new_item = { ++m_time_stamp, p_new_event };
	m_events.push_back( new_item );

	// Clean the list
	clean();

	// Return the results
	return p_dst->setup( p_new_event );
}

// This will get a new event
bool trigger_cache::ref_trigger( trigger *p_dst, const DWORD event_id )
{	{	// The first step is to look across the list that we currently have
		read_auto_lock	lock( m_cache_lock );

		// We try to find this item on the list		
		iterator i = std::reverse_find( m_events.begin(), m_events.end(), event_id );

		// If it was found
		if ( i != m_events.end() )
		{	// We add a reference count, and time-stamp.
			i->m_time_stamp = m_time_stamp;
			i->m_p_trigger_event->addref();
			return p_dst->setup( i->m_p_trigger_event );
		}
	}

		// Now write lock
		write_auto_lock	lock( m_cache_lock );

		// We check again, whether the item got added in the time between the read and write (unlikely!)	
		iterator i = std::reverse_find( m_events.begin(), m_events.end(), event_id );

		// If the item was not found, we will need to create it
		if ( i != m_events.end() )
		{	// We add a reference count, and time-stamp.
			i->m_time_stamp = m_time_stamp;
			i->m_p_trigger_event->addref();
			return p_dst->setup( i->m_p_trigger_event );
		}

		// Create the event ID
		trigger_event *p_ret = new trigger_event( event_id );

		// Check whether there was an error
		if ( p_ret->error() ) { p_ret->release(); return NULL; }
		
		// Add it to the list, we add the time-stamp tot he number created
		p_ret->addref();
		const block_desc new_item = { ++m_time_stamp, p_ret };
		m_events.push_back( new_item );

		// Gah, we clean up the list
		clean();
		
		// Return the restulr
		return p_dst->setup( p_ret );
}

void trigger_cache::clean( void )
{	// We are going to cycle across the list
	int dst = 0;
	for( int i = 0; i<(int)m_events.size(); i++ )
	{	if ( // And it still not referenced outside of me
			 ( m_events[ i ].m_p_trigger_event->refcount() == 1 ) &&
			 // The item has not been recovered from the cache in the time that the this number of items where added
			 ( (long)( m_time_stamp - m_events[ i ].m_time_stamp ) >= (long)FrameWork::Communication3::config::trigger_cache_history )
		   )
		{	// Release the item
			m_events[ i ].m_p_trigger_event->release();
		}
		else
		{	// Move new items in
			m_events[ dst++ ] = m_events[ i ];
		}

	} // for( iterator i = m_events.begin(); i != m_events.end(); )

	// Shorten the list as required
	m_events.resize( dst );
}