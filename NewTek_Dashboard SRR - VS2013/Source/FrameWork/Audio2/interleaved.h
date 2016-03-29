#pragma once

template< typename sample_type >
void swap_channels( buffer<sample_type> &dst )
{	// Store these locally
	const int no_samples  = dst.no_samples();
	const int no_channels = dst.no_channels();

	// Cycle over all channels
	for( int i=0; i<no_channels; i+=2 )
	{	// Get the two channel pointers
		sample_type* p_src_0 = dst( i     );
		sample_type* p_src_1 = dst( i + 1 );

		// Cycle over all the samples in each channel
		for( int j=0; j<no_samples; j++ )
			std::swap( p_src_0[ j ], p_src_1[ j ] );
	}
}

template< typename sample_type >
void convert_to_interleaved( const buffer<sample_type> &from, sample_type *p_dst )
{	// Store these locally
	const int no_samples  = from.no_samples();
	const int no_channels = from.no_channels();

	for( int i=0; i<no_channels; i++ )
	{	sample_type *p_dst_scan   = p_dst + i;
		const sample_type *p_src_scan   = from( i );
		const sample_type *p_src_scan_e = p_src_scan + no_samples;

		while( p_src_scan < p_src_scan_e - 3 )
		{	p_dst_scan[ 0*no_channels ] = p_src_scan[ 0 ];
			p_dst_scan[ 1*no_channels ] = p_src_scan[ 1 ];
			p_dst_scan[ 2*no_channels ] = p_src_scan[ 2 ];
			p_dst_scan[ 3*no_channels ] = p_src_scan[ 3 ];
			p_dst_scan += 4*no_channels;
			p_src_scan += 4;
		}

		while( p_src_scan < p_src_scan_e )
		{	p_dst_scan[ 0*no_channels ] = p_src_scan[ 0 ];
			p_dst_scan += no_channels;
			p_src_scan += 1;
		}
	}
}

template< typename sample_type >
void convert_from_interleaved( const sample_type *p_src, buffer<sample_type> &to )
{	// Store these locally
	const int no_samples  = to.no_samples();
	const int no_channels = to.no_channels();

	for( int i=0; i<no_channels; i++ )
	{	const sample_type *p_src_scan   = p_src + i;
		sample_type *p_dst_scan   = to( i );
		sample_type *p_dst_scan_e = p_dst_scan + no_samples;

		while( p_dst_scan < p_dst_scan_e - 3 )
		{	p_dst_scan[ 0 ] = p_src_scan[ 0*no_channels ];
			p_dst_scan[ 1 ] = p_src_scan[ 1*no_channels ];
			p_dst_scan[ 2 ] = p_src_scan[ 2*no_channels ];
			p_dst_scan[ 3 ] = p_src_scan[ 3*no_channels ];
			p_src_scan += 4*no_channels;
			p_dst_scan += 4;
		}

		while( p_dst_scan < p_dst_scan_e )
		{	p_dst_scan[ 0 ] = p_src_scan[ 0*no_channels ];
			p_src_scan += no_channels;
			p_dst_scan += 1;
		}
	}
}
