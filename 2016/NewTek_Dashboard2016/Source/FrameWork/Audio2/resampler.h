#pragma once

struct FRAMEWORKAUDIO2_API resampler
{			// Constructor
			resampler( void );
			resampler( const float src_rate, const float dst_rate );

			// The the sample rates
			const bool set_sample_rates( const float src_rate, const float dst_rate );

			// Apply a scaling factor
			void set_scaling_factor( const float factor = 1.0f );

			// This will return the maximum output buffer size such that a particular input
			// buffer will always return e_source_empty. The number of output samples might
			// not b e this exact number however depending on unpredictable machine rounding
			// effects.
			int get_max_buffer_length_in_samples( const int input_no_samples ) const;

			// This will perform a re-sampling operation
			enum e_reason { e_source_empty, e_destination_full };
			typedef std::pair<       float*, const float* > dst_data_desc;
			typedef std::pair< const float*, const float* > src_data_desc;

private:	// Conversions
			template< typename T >  struct scale			{	/* Deliberately not implemented */ };
			template<>				struct scale<float>		{	__forceinline static const float val( void ) { return 1.0f; } };
			template<>				struct scale<double>	{	__forceinline static const float val( void ) { return 1.0f; } };
			template<>				struct scale<short>		{	__forceinline static const float val( void ) { return (float)SHRT_MAX; } };
			template<>				struct scale<int>		{	__forceinline static const float val( void ) { return (float)INT_MAX;  } };

			__forceinline static void write( float*  p_dst, const float x ) { *p_dst = x; }
			__forceinline static void write( double* p_dst, const float x ) { *p_dst = x; }
			__forceinline static void write( short*  p_dst, const float x ) { *p_dst = (int)std::max( (float)SHRT_MIN, std::min( (float)SHRT_MAX, x ) ); }
			__forceinline static void write( int*    p_dst, const float x ) { *p_dst = (int)std::max( (float)INT_MIN,  std::min( (float)INT_MAX,  x ) ); }

public:		template< typename dst_precision >
			const e_reason operator()( // This is a pair representing the current position, and the output
										// position. This is returned to show you how far through either the
										// source or destination buffer got used. 
										std::pair< dst_precision*, const dst_precision* > &dst,
										// The source and destination strides
										const int dst_stride = 1
									  )
			{	// Debugging
				assert( (dst_stride>0) );

				// Quite simple
				for( ; dst.first < dst.second; dst.first+=dst_stride )
					*dst.first = 0;

				// Always fill the destination completely
				return e_destination_full;
			}
			
			template< typename src_precision, typename dst_precision >
			const e_reason operator() ( // This is a pair representing the current position, and the output
										// position. This is returned to show you how far through either the
										// source or destination buffer got used. 
										std::pair< const src_precision*, const src_precision* > &src, 
										std::pair<       dst_precision*, const dst_precision* > &dst,
										// This represents the distance at which we would ideally like the
										// input and output samples to align exactly. This is used when the
										// frame-rates are very close to unity to align samples back exactly.
										// -1 means do not mess with this
										const DWORD alignment_distance = (DWORD)-1,
										// The source and destination strides
										const int src_stride = 1, 
										const int dst_stride = 1
									  )
			{	// Debugging
				assert( (src_stride>0) && (dst_stride>0) );				

				// Check that there is some work, any work to do
				if ( src.first >= src.second ) return e_source_empty;
				if ( dst.first >= dst.second ) return e_destination_full;

				// Debugging
				assert( m_output_posn >= 0.0f );

				// Get the scale
				const float scaling = m_scaling * scale<dst_precision>::val() / scale<src_precision>::val();

				// Get the output delta value
				float output_delta = m_output_delta;

				// We now try to align the delta with a sample boundary at some distance
				// in the future. 
				if ( alignment_distance != (DWORD)-1 )
					// Now update the output delta estimate
					output_delta = ( floor( 0.5f + m_output_posn + (float)alignment_distance*output_delta ) - m_output_posn ) / (float)alignment_distance;

				// If we are very close to a source sample and the rate is such that we 
				// end the buffer very close to a sample then we can take an optimal path
				const int   no_samples   = (int)std::min( ( src.second - src.first )/src_stride, ( dst.second - dst.first )/dst_stride );
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
						write( dst.first + i*dst_stride, (float)( src.first[ i*src_stride ] ) * scaling );

					// Offset the positions
					src.first += no_samples*src_stride;
					dst.first += no_samples*dst_stride;

					// Update the position exactly
					m_output_posn = end_sample_offset;

					// Finished
					if ( src.first >= src.second ) 
						return e_source_empty;

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
							m_last_samples[ ( m_sample_index++ ) & 3 ] = *src.first;
							src.first += src_stride;

							// Finished
							if ( src.first >= src.second ) 
								return e_source_empty;
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
						write( dst.first, ( ( ( a*m_output_posn + b )*m_output_posn + c )*m_output_posn + y1 ) * scaling );
						dst.first += dst_stride;

						// Step the sample
						m_output_posn += output_delta;			
						
						// Check whether the output buffer is now full
						if ( dst.first >= dst.second  ) 
							return e_destination_full;			
					}
				}
			}

			// This is a complex function. It is used when you need to update the position og a 
			// resampler to match another. For instance, if you went to two channels, then back to
			// four, one needs to reset the second two.
			void reset( void );
			void reset( const resampler &match_timing_to );

private:	// These represent the last two samples
			int		m_sample_index;
			float	m_last_samples[ 4 ];

			// This is the current sub-sample position
			float	m_output_posn;
			float	m_output_delta;

			// The scaling factor
			float	m_scaling;
};