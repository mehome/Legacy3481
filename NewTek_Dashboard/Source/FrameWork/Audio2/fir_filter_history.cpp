#include "stdafx.h"
#include "FrameWork.Audio2.h"

// Namespace stuff
using namespace FAUD::fir;

// Constructor
history::history( filter& filter_to_use )
	:	m_filter_to_use( filter_to_use ),
		m_filter_size( filter_to_use.size() ),
		m_no_channels( 0 )
{
}

// Destructor
history::~history( void )
{	// Free all allocated memory
	for( int i=0; i<(int)m_history.size(); i++ )
		::_aligned_free( m_history[ i ] );
}

// Change the number of channels
void history::change_no_channels( const int no_channels )
{	// No change !
	if ( no_channels == m_no_channels ) return;

	// Expand the number of channels
	if ( no_channels > m_no_channels )
	{	// Resize the buffer
		m_history.resize( no_channels );

		// Allocate new buffers
		for( int i=m_no_channels; i<no_channels; i++ )
		{	m_history[ i ] = (float*)::_aligned_malloc( sizeof(float)*m_filter_size*2, 16 );		
			::memset( m_history[ i ], 0, sizeof(float)*m_filter_size );
		}
	}
	// Contract the number of channels
	else
	{	// We clear out the now unused channels
		for( int i=no_channels; i<m_no_channels; i++ )
			::memset( m_history[ i ], 0, sizeof(float)*m_filter_size );
	}

	// Store the new number of channels
	m_no_channels = no_channels;
}

// Apply a FIR filter to a buffer
void history::operator() ( const FAUD::buffer_f32& src, FAUD::buffer_f32& dst )
{	// First check the number of samples is all correct
	assert( src.is_same_size_as( dst ) );

	// Check the number of channels is correct
	change_no_channels( src.no_channels() );

	// The number of samples
	const int no_samples = src.no_samples();
	const int filter_size_in_bytes = sizeof( float ) * m_filter_size;

	// We cycle over all channels
	for( int i=0; i<m_no_channels; i++ )
	{	// The destination buffer position
		const float* p_src = src( i );
			  float* p_dst = dst( i );		
		
		// Step 1. Handle previous history section
			// Step 1a. Copy the beginning buffers into the history section
			::memcpy( m_history[ i ] + m_filter_size, p_src, filter_size_in_bytes );

			// Step 1b. Use this history section to filter the past buffer
			p_dst += m_filter_to_use( m_history[ i ], p_dst, m_filter_size*2 );

		// Step 2. Resample the rest of the buffer
			p_src += m_filter_to_use( p_src, p_dst, no_samples );

		// Step 3. Keep the end of the buffer as history
			::memcpy( m_history[ i ], p_src, filter_size_in_bytes );
	}
}