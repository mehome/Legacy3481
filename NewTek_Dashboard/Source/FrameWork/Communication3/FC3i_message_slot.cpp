#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3i;

// No debug slots yet
DWORD message_slot::g_debug_no_objects = 0;

extern void debug_output( const wchar_t *p_category , const wchar_t *p_format, ... );

// Constructor
message_slot::message_slot( const wchar_t name[] ) 
	: m_p_header( NULL ), m_name( name ), m_ref( 1 )
{	// Create the handle names
	std::wstring map_name = config::name_slot_queue;
	map_name += name;

	mapped_file::setup( map_name.c_str(), header_size, true, false );
	assert( !mapped_file::error() );

	// We now setup the pointers
	m_p_header = (header*)mapped_file::ptr();
	
	// Debugging
	if ( FC3::config::debug_object_creation )
	{	::_InterlockedIncrement( (LONG*)&g_debug_no_objects );
		debug_output( FC3::config::debug_category, L"New slot (%d) : %s\n", g_debug_no_objects, map_name.c_str() );
	}

	// One more item
	::_InterlockedIncrement( &m_p_header->m_ref_count );
}

// Destructor
message_slot::~message_slot( void )
{	// One more item
	if ( ::_InterlockedDecrement( &m_p_header->m_ref_count ) == -1 )
	{	// We actually need to free up all entries on the slot
		// remind me to do this !
		for( int i=0; i<config::slots_items_per_group; i++ )
			// Replace the existing message with NULL
			put<FC3i::message>( i, NULL );
	}
	
	// Debugging
	if ( FC3::config::debug_object_creation )
	{	debug_output( FC3::config::debug_category, L"Del slot (%d)\n", g_debug_no_objects );
		::_InterlockedDecrement( (LONG*)&g_debug_no_objects );
	}
}

// Get the name
const wchar_t* message_slot::name( void ) const
{	return m_name.c_str();
}

// Reference counting
const long message_slot::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long message_slot::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

// This will return the current reference count. Obviously there are thread safety
// issues relating to it's use.
const long message_slot::refcount( void ) const
{	// If the reference count has
	return m_ref;
}

// Was there an error
const bool message_slot::error( void ) const
{	return mapped_file::error();
}