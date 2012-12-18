#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::audio;

// Constructor
message::message( const int no_samples, const int no_channels, const int extra_data_size )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( message_size( no_samples, no_channels, extra_data_size ) ),
		m_p_header( NULL )
{	// Set the type
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Set the size
		type() = message_type_audio;

		// Setup the memory
		setup_memory( no_samples, no_channels, extra_data_size );
	}
}

// Internal use only :)
message::message( const DWORD block_id, const DWORD addr )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( block_id, addr ),
		m_p_header( NULL )
{	// Check the size and the type
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// If the type is correct, set it up.
		if ( type() == message_type_audio )
			setup_memory();
	}
}

message::~message( void )
{
}

// Reference counting
const long message::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long message::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

const long message::refcount( void ) const
{	return m_ref;
}

// Is there an error in this message, most likely caused by a failed allocation or transmission
bool message::error( void ) const
{	return m_p_header ? false : true;
}

// Get the size of a message
const int message::message_size( const int no_samples, const int no_channels, const int extra_data_size )
{	// Has a header
	int size = header_size;

	// Now the body of the data
	size += no_samples * no_channels * sizeof( float ) + extra_data_size;

	// Finished
	return size;
}

// Setup all pointers
void message::setup_memory( const int no_samples, const int no_channels, const int extra_data_size )
{	// Set the header pointer
	assert( !m_p_header );
	m_p_header = (header*)ptr();
	assert( m_p_header );

	// Setup the header
	m_p_header->m_sample_rate = 96000;
	m_p_header->m_max_no_samples = no_samples;
	m_p_header->m_no_samples = no_samples;
	m_p_header->m_no_channels = no_channels;
	m_p_header->m_time_stamp = 0;
	m_p_header->m_extra_data_size = extra_data_size;

	// Setup the rest
	setup_memory();
}

void message::setup_memory( void )
{	// Set the header pointer
	m_p_header = (header*)ptr();
	assert( m_p_header );

	// Store the size
	const int no_samples = m_p_header->m_no_samples;
	const int max_no_samples = m_p_header->m_max_no_samples;
	const int no_channels = m_p_header->m_no_channels;

	// We get the current offset
	BYTE*	p_ptr = ptr( header_size );

	// Reference the data
	m_audio.reference_in_bytes( (float*)p_ptr, no_channels, no_samples, max_no_samples * sizeof( float ) );
	p_ptr += max_no_samples * no_channels * sizeof( float );

	// Store the extra data pointer
	m_p_extra_data = m_p_header->m_extra_data_size ? (void*)p_ptr : NULL;

	// Increment the pointer for the assert below
	p_ptr += m_p_header->m_extra_data_size;

	// Debugging
	assert( p_ptr == ptr() + size() );
}

// Get the number of audio samples and channels.
const int message::no_samples( void ) const
{	assert( !error() );
	return m_p_header->m_no_samples;
}

const int message::no_channels( void ) const
{	assert( !error() );
	return m_p_header->m_no_channels;
}

// Get and set the audio sample rate
const int message::sample_rate( void ) const
{	assert( !error() );
	return m_p_header->m_sample_rate;
}

int& message::sample_rate( void )
{	assert( !error() );
	return m_p_header->m_sample_rate;
}

// Get access to the buffers
FrameWork::Audio2::buffer_f32&	message::audio( void )
{	return m_audio;
}

const FrameWork::Audio2::buffer_f32&	message::audio( void ) const
{	return m_audio;
}

// Use this function very very carefully.
bool message::change_number_of_audio_samples( const int new_no_audio_samples )
{	assert( !error() );

	// This is scary
	if ( ( new_no_audio_samples < 0 ) || ( new_no_audio_samples > m_p_header->m_max_no_samples ) ) 
	{	assert( false );
		return false;
	}

	// Set the data
	m_p_header->m_no_samples = new_no_audio_samples;
	setup_memory();

	// Success
	return true;
}

// Get and set the time-stamp of the field.
// This is stored in 10ns intervals.
const __int64  message::time_stamp( void ) const
{	assert( !error() );
	return m_p_header->m_time_stamp;
}

__int64& message::time_stamp( void )
{	assert( !error() );
	return m_p_header->m_time_stamp;
}

const void* message::extra_data( void ) const
{	return m_p_extra_data;
}

void* message::extra_data( void )
{	return m_p_extra_data;
}

const int message::extra_data_size( void ) const
{	return m_p_header->m_extra_data_size;	
}

// This ensures that people can delete the read_with_info structs returned above correctly
static FC3::implementation::memory_pool< sizeof( message ) > g_mem_alloc;

void* message::operator new ( const size_t size )
{	assert( sizeof( message ) == size );
	return g_mem_alloc.malloc();
}

void  message::operator delete ( void* ptr )
{	return g_mem_alloc.free( ptr );
}

void* message::operator new [] ( const size_t size )
{	return ::malloc( size );
}

void  message::operator delete [] ( void* ptr )
{	::free( ptr );
}

// Get the duration in 10ns intervals
const __int64 message::duration( const __int64 time_base ) const
{	// Get the properties of this frame
	const __int64 no_samples  = this->no_samples();
	const __int64 sample_rate = abs( this->sample_rate() );
	
	// Return the correctly rounded result
	return ( time_base*no_samples + sample_rate/2 ) / sample_rate;
}

const double message::duration_in_seconds( void ) const
{	return (double)this->no_samples() / (double)abs( this->sample_rate() );
}