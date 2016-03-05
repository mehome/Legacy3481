resampler::resampler( const float src_rate, const float dst_rate )
	:	m_output_posn( 0.0f ), m_sample_index( 0 ), m_output_delta( 1.0f ), m_scaling( 1.0f )
{	// We start out with a history of silence
	::memset( m_last_samples, 0, sizeof(m_last_samples) );

	// Setup the sample rates
	set_sample_rates( src_rate, dst_rate );
}

resampler::resampler( void )
	:	m_output_posn( 0.0f ), m_sample_index( 0 ), m_output_delta( 1.0f ), m_scaling( 1.0f )
{	// We start out with a history of silence
	::memset( m_last_samples, 0, sizeof(m_last_samples) );
}

// Apply a scaling factor
void resampler::set_scaling_factor( const float factor )
{	m_scaling = factor;
}

// The the sample rates
const bool resampler::set_sample_rates( const float src_rate, const float dst_rate )
{	// A quick check for no change of any kind
	const float new_output_delta = src_rate / dst_rate;
	if ( new_output_delta == m_output_delta ) return false;
	
	// We transform the sample position
	m_output_posn /= m_output_delta;
	
	// We update the output delta
	m_output_delta = new_output_delta;

	// Transform it back into the correct position and ensure that we are bound in the correct range
	m_output_posn *= new_output_delta;
	m_output_posn = m_output_posn - floor( m_output_posn );

	// Not changed
	return true;
}

// This is a complex function. It is used when you need to update the position og a 
// resampler to match another. For instance, if you went to two channels, then back to
// four, one needs to reset the second two.
void resampler::reset( void )
{	// Reset all data
	m_sample_index = 0;
	::memset( m_last_samples, 0, sizeof( m_last_samples ) );
	m_output_posn  = 0.0f;
	m_output_delta = 1.0f;
}

void resampler::reset( const resampler &match_timing_to )
{	// Reset	
	::memset( m_last_samples, 0, sizeof( m_last_samples ) );
	m_sample_index = match_timing_to.m_sample_index;
	m_output_posn  = match_timing_to.m_output_posn;
	m_output_delta = match_timing_to.m_output_delta;
}

#if 0

// This will perform a re-sampling operation
const resampler::e_reason resampler::operator() ( src_data_desc &src, dst_data_desc &dst, const DWORD alignment_distance )
{	// Check that there is some work, any work to do
	if ( src.second == src.first ) return e_source_empty;
	if ( dst.second == dst.first ) return e_destination_full;

	// Debugging
	assert( m_output_posn >= 0.0f );

	// Get the output delta value
	float output_delta = m_output_delta;

	// We now try to align the delta with a sample boundary at some distance
	// in the future. 
	if ( alignment_distance != (DWORD)-1 )
		// Now update the output delta estimate
		output_delta = ( floor( 0.5f + m_output_posn + (float)alignment_distance*output_delta ) - m_output_posn ) / (float)alignment_distance;

	// If we are very close to a source sample and the rate is such that we 
	// end the buffer very close to a sample then we can take an optimal path
	const int   no_samples   = (int)std::min( src.second - src.first, dst.second - dst.first );
	const float no_samples_f = (float)no_samples;

	// The offset from the first and last samples
	const float start_sample_offset = m_output_posn;
	const float end_sample_offset   = m_output_posn + no_samples_f*( output_delta-1.0f );

	// The error allowed
	const float eps = 0.01f;
	if ( // Very close to the start sample
		 ( fabs( start_sample_offset ) < eps ) && 
		 // Very close to the end sample, but enough that it got used that we advanced to the next sample
		 ( end_sample_offset >= 0.0f ) && ( end_sample_offset <= eps ) )
	{	// Copy
		for( int i=0; i<no_samples; i++ )
			dst.first[ i ] = src.first[ i ] * m_scaling;

		// Offset the positions
		src.first += no_samples;
		dst.first += no_samples;

		// Update the position exactly
		m_output_posn = end_sample_offset;

		// Finished
		if ( src.second == src.first ) return e_source_empty;
		return e_destination_full;
	}
	else
	{	// Down-sampling.
		while( true )
		{	// While the current position is out of range
			while( m_output_posn >= 1.0f )
			{	// Step a new sample in
				m_output_posn -= 1.0f;
			
				// Move this sample in
				m_last_samples[ ( m_sample_index++ ) & 3 ] = *( src.first++ );

				// Finished
				if ( src.second == src.first ) return e_source_empty;
			}
			
			// Get the short history that we use
			const float y0 = m_last_samples[ ( m_sample_index/*-4*/) & 3 ];
			const float y1 = m_last_samples[ ( m_sample_index - 3 ) & 3 ];
			const float y2 = m_last_samples[ ( m_sample_index - 2 ) & 3 ];
			const float y3 = m_last_samples[ ( m_sample_index - 1 ) & 3 ];
 
			// Get the constants
			const float a = ( 3.0f*( y1 - y2 ) - y0 + y3 )*0.5f;
			const float b = 2.0f*y2 + y0 - ( 5.0f*y1 + y3 )*0.5f;
			const float c = ( y2 - y0 )*0.5f;

			// Perform cubic interpolation to get these samples
			//assert( m_output_posn>=0 && m_output_posn<=1 );
			*( dst.first++ ) = ( ( ( a*m_output_posn + b )*m_output_posn + c )*m_output_posn + y1 ) * m_scaling;

			// Step the sample
			m_output_posn += output_delta;			
			
			// Check whether the output buffer is now full
			if ( dst.second == dst.first ) return e_destination_full;			
		}
	}
}

#endif

int resampler::get_max_buffer_length_in_samples( const int input_no_samples ) const
{	// This should be right
	return 2 + (int)ceil( (float)input_no_samples / m_output_delta );
}