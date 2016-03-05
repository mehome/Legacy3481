#pragma once

template< typename base_filter_type >
struct base_filter
{			// Constructor
			base_filter( void );

			// Apply filtering to a buffer. This can work in or out of place
			void operator() ( const buffer_f32& src, buffer_f32& dst );
			void operator() ( buffer_f32& dst );

			// This will reset the history of the filter
			void reset( void );

protected:	// Overload this !
			// Setup a filter and return if anything changed.
			virtual void setup( base_filter_type& dst ) = 0;
	
			// We need to maintain a list of the filters being used for all channel
			base_filter_type m_filter;
};


template< typename base_filter_type >
base_filter< base_filter_type >::base_filter( void )
{
}

template< typename base_filter_type >
void base_filter< base_filter_type >::operator() ( const buffer_f32& src, buffer_f32& dst )
{	// Handle the obvious case
	assert( src.is_same_size_as( dst ) );	

	// We actually always run in place ...
	dst = src;

	// Now run in place
	operator()( dst );
}

template< typename base_filter_type >
void base_filter< base_filter_type >::operator() ( buffer_f32& dst )
{	// Set the number of channels
	setup( m_filter );

	// We cycle over all channles
	for( int i=0; i<dst.no_channels(); i++ )
		m_filter.ProcessI_mono( dst.no_samples(), dst( i ), i );
}

// This will reset the history of the filter
template< typename base_filter_type >
void base_filter< base_filter_type >::reset( void )
{	m_filter.Clear();
}