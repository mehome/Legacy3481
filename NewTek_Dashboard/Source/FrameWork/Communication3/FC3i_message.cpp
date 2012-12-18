#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructors
message::message( const DWORD size )
	:	m_msg_source( true ), m_p_block( NULL ), m_p_header( NULL ), m_p_data( NULL )
{	// Try to get the frame
	memory_cache::get_cache().new_message( this, size );
}

message::message( const DWORD block_id, const DWORD addr ) 
	:	m_msg_source( false ), m_p_block( NULL ), m_p_header( NULL ), m_p_data( NULL )
{	// Try to get the frame
	memory_cache::get_cache().ref_message( this, block_id, addr );
}

// Try sending a message
bool message::send( const wchar_t destination[] ) const
{	// Debugging
	assert( !error() );
	if ( error() ) return false;

	// Is this a remote send ?
	// This code is safe without checking the length (!)
	if ( ( destination[ 0 ] == '\\' ) && ( destination[ 1 ] == '\\' ) )
	{	// Get the server name
		const wchar_t *p_server_start = destination + 2;
		const wchar_t *p_server_end	  = ::wcschr( p_server_start, L'\\' );
		if ( !p_server_end )
		{	// Incorrectly formed server name
			assert( false );
			return false;
		}		


		// Store the server name
		const int len = (int)(p_server_end - p_server_start);
		wchar_t *p_server_name = (wchar_t*)_alloca( ( len + 1 ) * sizeof( wchar_t ) );
		::wcsncpy( p_server_name, p_server_start, len );
		p_server_name[ len ] = 0;

		// Was there a port number ?
		wchar_t *p_port_no = ::wcschr( p_server_name, L':' );
		int port_no = FrameWork::Communication3::config::remote_port_number;
		if ( p_port_no )
		{	// Just in case
			if ( p_server_name == p_port_no )
			{	// Incorrectly formed server name
				assert( false );
				return false;
			}
			
			// Terminate the server name string
			*p_port_no = 0;

			// Get the true port no
			if ( ( p_port_no[1] >= L'0' ) && ( p_port_no[1] <= L'9' ) )
				port_no = ::_wtoi( p_port_no + 1 );
		}

		// Store the real destination name
		const wchar_t *p_dst_name = p_server_end + 1;

		// Now try sending the message
		return remote::client_cache::get_cache().send( p_server_name, port_no, p_dst_name, m_p_header, allocation_size( size() ) );
	}
	else
	{	// Get the server to send this message too.
		server *p_server = server_cache::get_cache().get_server( destination );
		if ( !p_server ) return false;	

		// Pre-emptively increment the send count
		::_InterlockedIncrement( (long*)&m_p_header->m_no_outstanding_sends );

		// Pre-emptively increment the reference count
		::_InterlockedIncrement( (long*)&m_p_header->m_ref );

		// Now try sending the message
		const bool success = p_server->send_message( m_p_block->block_id(), m_p_block->addr( (BYTE*)m_p_header ) );

		// If it was not succesful, we undo the send count
		if ( !success ) 
		{	// Undo the send count
			::_InterlockedDecrement( (long*)&m_p_header->m_no_outstanding_sends );

			// Undo the reference count
			::_InterlockedDecrement( (long*)&m_p_header->m_ref );
		}
		else
		{	
	#ifndef	FC3_VERSION_3_5
			// Time-stamp the memory block so that we know it should remain active for some time
			m_p_block->time_stamp();
	#endif	FC3_VERSION_3_5
		}

		// Release the server lock
		p_server->release();	

		// Return the results
		return success;
	}
}

// Setup
bool message::setup( memory_block* p_block, BYTE *p_ptr, const DWORD size )
{	// Debugging
	assert( !m_p_block );
	assert( !m_p_header );
	assert( !m_p_data );
	
	// If the message looks legitimate
	if ( ( p_block ) && ( p_ptr ) ) 
	{	// Store the block we are part of
		m_p_block = p_block;
	
		// The data pointer
		m_p_header = (header*)p_ptr;
		m_p_data   = p_ptr + header_size;

		// Setup the header
		m_p_header->m_type = message_type_raw;
		m_p_header->m_size = size;

		// One reference now
		m_p_header->m_ref = 1;

		// Cleanp the other settings
		m_p_header->m_no_outstanding_sends = 0;
		m_p_header->m_trigger_outstanding_sends = 0;
		m_p_header->m_no_displayed = 0;
		m_p_header->m_trigger_displayed = 0;
		m_p_header->m_trigger_granularity = (DWORD)-1;
		m_p_header->m_from_network = 0;

		// Success, we cannot go wrong
		return true;
	}

	// Not allocated
	return false;
}

// Set this message as being from the network
void message::set_from_network( const DWORD size )
{	assert( m_p_header );

	// One reference now (we do not want the references from the other source to apply)
	if ( m_p_header )
	{	// Reset all displayed settings
		m_p_header->m_size = size;
		m_p_header->m_ref = 1;
		m_p_header->m_no_outstanding_sends = 0;
		m_p_header->m_trigger_outstanding_sends = 0;
		m_p_header->m_no_displayed = 0;
		m_p_header->m_trigger_displayed = 0;
		m_p_header->m_trigger_granularity = (DWORD)-1;
		m_p_header->m_from_network = 1;
	}
}

// Setup
bool message::setup( memory_block* p_block, BYTE *p_ptr )
{	// Debugging
	assert( !m_p_block );
	assert( !m_p_header );
	assert( !m_p_data );
	
	// If the message looks legitimate
	if ( ( p_block ) && ( p_ptr ) ) 
	{	// Store the block we are part of
		m_p_block = p_block;
		
		// The data pointer
		m_p_header = (header*)p_ptr;
		m_p_data   = p_ptr + header_size;

		// Check we succeeded
		if ( type() != message_type_error ) return true;

		// Release the block and reset
		m_p_block->release();
		m_p_block  = NULL;
		m_p_header = NULL;
		m_p_data   = NULL;
	}

	// Error
	return false;
}

void message::simulate_send( void )
{	// Increment the send count
	assert( !error() );
	::_InterlockedIncrement( (long*)&m_p_header->m_no_outstanding_sends );
	::_InterlockedIncrement( (long*)&m_p_header->m_ref );
}
	
// Destructor
message::~message( void )
{	// Unwind if there is no error
	if ( !error() )
	{	// Handle outstanding sends
		if ( !m_msg_source )
		{	// One less outstanding send, the data MUST have been sent here
			const long no_sends = ::_InterlockedDecrement( (long*)&m_p_header->m_no_outstanding_sends );

			// If there are none left, and a trigger, then apply it
			if ( ( !no_sends ) && ( m_p_header->m_trigger_outstanding_sends ) )
				// Get the trigger and set it
				trigger( m_p_header->m_trigger_outstanding_sends ).set( true );
		}
		
		// Free eveything
		if ( m_p_block ) 
		{	// Reference count the frame
			if ( ! ::_InterlockedDecrement( (long*)&m_p_header->m_ref ) )
				// Try to recycle the block to avoid to many memory maps being created
				m_p_block->free( (BYTE*)m_p_header );
			
			// Reference count eh memory map
			m_p_block->release();
		}
	}
}

// Get allocation size
const DWORD message::allocation_size( const DWORD size )
{	return fc3_size_align( size + header_size );
}

// Get and set the message type
message::message_type& message::type( void )
{	assert( !error() );
	return m_p_header->m_type;
}

const message::message_type  message::type( void ) const
{	assert( !error() );
	return m_p_header->m_type;
}
	
// Get the data size of this message
const DWORD message::size( void ) const
{	assert( !error() );
	return m_p_header ? m_p_header->m_size : 0;
}

const bool message::was_sent_from_remote_source( void ) const
{	assert( !error() );
	return m_p_header ? ( m_p_header->m_from_network ? true : false ) : false;
}

// Is there an error
const bool message::error( void ) const
{	return m_p_header ? false : true;
}

// Set the display trigger
void message::set_displayed_trigger( trigger& from, const DWORD display_granularity ) const
{	// Set the displayed trigger
	assert( !error() );
	from.set( false );
	m_p_header->m_trigger_displayed = from.trigger_id();
	m_p_header->m_trigger_granularity = std::max( (DWORD)1, display_granularity );
}

// Is there a displayed trigger
const bool message::has_displayed_trigger( void ) const
{	return m_p_header->m_trigger_displayed ? true : false;
}

void message::reset_displayed_trigger( void )
{	// Set the displayed trigger
	assert( !error() );
	::_InterlockedExchange( (long*)&m_p_header->m_trigger_displayed, 0 );
}

// Set the outstanding sents trigger
void message::set_outstanding_sends_trigger( trigger& from )
{	// Set the outstanding sends trigger
	assert( !error() );
	from.set( false );
	m_p_header->m_trigger_outstanding_sends = from.trigger_id();
}

// Is there an outstanding sends trigger
const bool message::has_outstanding_sends_trigger( void ) const
{	return m_p_header->m_trigger_outstanding_sends ? true : false;
}

void message::reset_outstanding_sends_trigger( void )
{	// Set the outstanding sends trigger
	assert( !error() );
	::_InterlockedExchange( (long*)&m_p_header->m_trigger_outstanding_sends, 0 );
}

// Mark this frame displayed
void message::displayed( void ) const
{	// Increment the displayed count
	assert( !error() );
	const DWORD displayed_count = (DWORD)::_InterlockedIncrement( (long*)&m_p_header->m_no_displayed ) - 1;

	// If this was the first time the frame was displayed, we need to trigger the displayed
	// event, if relevant.
	if (  // Is there a trigger
		  ( m_p_header->m_trigger_displayed ) &&
		  // Determines how often to trigget the displayed message
		  ( !( displayed_count % m_p_header->m_trigger_granularity ) ) )
				// Get the trigger and set it
				trigger( m_p_header->m_trigger_displayed ).set( true );
}

// Reset the displayed count
void message::reset_displayed_count( void )
{	// Pretty simply
	assert( !error() );
	::_InterlockedExchange( (long*)&m_p_header->m_no_displayed, 0 );
}

// Get the displayed count
const long message::displayed_count( void ) const
{	assert( !error() );
	return (long)m_p_header->m_no_displayed;
}

// Get the number of outstanding sends
const long message::no_outstanding_sends( void ) const
{	assert( !error() );
	return (long)m_p_header->m_no_outstanding_sends;
}

// Get the data pointed to by this message
const BYTE* message::operator() ( const DWORD i ) const
{	assert( !error() );
	assert( i < m_p_header->m_size );
	return m_p_data + i;
}

BYTE* message::operator() ( const DWORD i )
{	assert( !error() );
	assert( i < m_p_header->m_size );
	return m_p_data + i;
}

const BYTE* message::ptr( const DWORD i ) const
{	assert( !error() );
	assert( i < m_p_header->m_size );
	return m_p_data + i;
}

BYTE* message::ptr( const DWORD i )
{	assert( !error() );
	assert( i < m_p_header->m_size );
	return m_p_data + i;
}

const BYTE* message::ptr_raw( void ) const
{	assert( !error() );
	return (BYTE*)m_p_header;
}

BYTE* message::ptr_raw( void )
{	assert( !error() );
	return (BYTE*)m_p_header;
}