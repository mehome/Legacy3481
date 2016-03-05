#pragma once

template< typename precision_type, const bool inverse = false >
struct fft
{	fft( const precision_type *p_src_r, const precision_type *p_src_i, 
			   precision_type *p_dst_r,		  precision_type *p_dst_i,
		 const int n )
	{	// Calculate m=log_2(n)
		int m = 0;
		for( int p=1; p<n; p*=2, m++ );

		// Get the normalization factor
		const precision_type norm = 1 / (precision_type)( inverse ? 1 : n );

		// Bit reversal
		p_dst_r[ n-1 ] = p_src_r[ n-1 ] * norm;
		p_dst_i[ n-1 ] = p_src_i[ n-1 ] * norm;

		int j = 0;
		for( int i=0, j=0; i<n-1; i++ )
		{	// Copy the values
			p_dst_r[ i ] = p_src_r[ j ] * norm;
			p_dst_i[ i ] = p_src_i[ j ] * norm;

			// Increment J
			int k = n / 2;
			for( ; k<=j; j-=k, k/=2 );
			j += k;
		}

		// Calculate the FFT
		precision_type ca = -1, sa = 0;
	  
		int l1 = 1, l2 = 1;
		for( int l=0; l<m; l++ )
		{	l1 = l2;
			l2 *= 2;
	    
			precision_type u1 = 1, u2 = 0;
	    
			for(int j = 0; j < l1; j++)
			{	for(int i = j; i < n; i += l2)
				{	const int i1 = i + l1;
					const precision_type t1 = u1*p_dst_r[ i1 ] - u2*p_dst_i[ i1 ];
					const precision_type t2 = u1*p_dst_i[ i1 ] + u2*p_dst_r[ i1 ];
					
					p_dst_r[ i1 ] = p_dst_r[ i ] - t1;
					p_dst_i[ i1 ] = p_dst_i[ i ] - t2;
					p_dst_r[ i ] += t1;
					p_dst_i[ i ] += t2;
				}

				const precision_type z =  u1*ca - u2*sa;
				u2 = u1 * sa + u2 * ca;
				u1 = z;
			}

			sa = sqrt( ( 1 - ca ) / 2 ) * ( inverse ? +1 : -1 );
			ca = sqrt( ( 1 + ca ) / 2 );
		}
	}

};

template< typename precision_type >
struct inverse_fft
{	inverse_fft( const precision_type *p_src_r, const precision_type *p_src_i, 
					   precision_type *p_dst_r,		  precision_type *p_dst_i,
				 const int n )
	{	fft< precision_type, true >( p_src_r, p_src_i, p_dst_r, p_dst_i, n );
	}
};