#pragma once

template< typename base_filter_type >
struct base_filter
{			// Constructor
			base_filter( void );

private:	// Internal methods
			void change_no_channels( const int no_channels );
	
			// We need to maintain a list of the filters being used for all channel
			std::vector< base_filter_type >	m_filters;
};


template< typename base_filter_type >
base_filter< base_filter_type >::base_filter( void )
	:	// There must be at least one channel to store the filter settings in.
		m_filters( 1 )
{
}

// Internal methods
template< typename base_filter_type >
void base_filter< base_filter_type >::change_no_channels( const int no_channels )
{	// Sanity check
	assert( no_channels > 0 );

	// The number of channels is increasing
	if ( no_channels > (int)m_filters.size() )
	{	// The old number of channels
		const int old_no_channels = (int)m_filters.size();

		// Just discard the channels
		m_filters.resize( no_channels );

		// Copy the channel settings
		for( int i=old_no_channels; i<no_channels; i++ )
			m_filters[ i ] = m_filters[ 0 ];
	}

	// The number of channels is decreasing
	else if ( no_channels < (int)m_filters.size() )
	{	// Just discard the channels
		m_filters.resize( no_channels );
	}
}