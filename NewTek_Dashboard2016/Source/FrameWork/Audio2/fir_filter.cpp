#include "stdafx.h"
#include "FrameWork.Audio2.h"

// Internal implementation
#include "fir_filter_impl.h"

// Namespace stuff
using namespace FAUD::fir;

// A constant
const double PI = 3.1415926535897932384626433832795028841971693993751058209749445;

// Constructor
filter::filter( const low_pass& from, const float gain )
	: m_p_coefficients( NULL )
{	// Get the order, without being smaller than two
	const int N = std::max( from.m_N, 2 );

	// Allocate a vector for the results
	constraint_list_type	H( N );

	// Get the descriptors
	const double Wc = from.m_Fc * 2 * PI / from.m_Fs;

	// delta_w is the difference in w between two consecutive freq. samples
	const double delta_w = PI / ( N - 1 );

	// Allocate all entries
	for( int i=0; i<N; i++ )
	{	// The frequency
		H[i].first = (double)i*delta_w;

		// In the pass-band
			 if ( H[i].first <= Wc-2*delta_w ) H[i].second = gain;
		// In the stop-band
        else if ( H[i].first >= Wc+2*delta_w)  H[i].second = 0;
		// In the transition band (near Wc)
        // Use a raised cosine to calculate values 
        else H[i].second = ( 1 + cos( PI*( H[i].first - ( Wc - 2*delta_w ) ) / ( 4*delta_w ) ) )*gain/2;
	}

	// Compute the coefficients
	compute_coefficients( H );
}

filter::filter( const high_pass& from, const float gain )
	: m_p_coefficients( NULL )
{	// Get the order, without being smaller than two
	const int N = std::max( from.m_N, 2 );

	// Allocate a vector for the results
	constraint_list_type	H( N );

	// Get the descriptors
	const double Wc = from.m_Fc * 2 * PI / from.m_Fs;

	// delta_w is the difference in w between two consecutive freq. samples
	const double delta_w = PI / ( N - 1 );

	// Allocate all entries
	for( int i=0; i<N; i++ )
	{	// The frequency
		H[i].first = (double)i*delta_w;

		// In the stop-band
			 if ( H[i].first <= Wc-2*delta_w ) H[i].second = 0;
		// In the pass-band
        else if ( H[i].first >= Wc+2*delta_w)  H[i].second = gain;
		// In the transition band (near Wc)
        // Use a raised cosine to calculate values 
        else H[i].second = ( 1 - cos( PI*( H[i].first - ( Wc - 2*delta_w ) ) / ( 4*delta_w ) ) )*gain/2;
	}

	// Compute the coefficients
	compute_coefficients( H );
}

filter::filter( const band_pass& from, const float gain )
	: m_p_coefficients( NULL )
{	// Get the order, without being smaller than two
	const int N = std::max( from.m_N, 2 );

	// Allocate a vector for the results
	constraint_list_type	H( N );

	// Get the descriptors
	const double Wl = from.m_Fl*2*PI / from.m_Fs;
    const double Wh = from.m_Fh*2*PI / from.m_Fs;

	// delta_w is the difference in w between two consecutive freq. samples
	const double delta_w = PI / ( N - 1 );

	// Allocate all entries
	for( int i=0; i<N; i++ )
	{	// The frequency
		H[i].first = (double)i*delta_w;

		// In one of the stop-bands
			 if ( ( H[i].first <= Wl - 2*delta_w ) || ( H[i].first >= Wh + 2*delta_w ) )
				H[i].second = 0;
		// In the pass-band
        else if ( ( H[i].first >= Wl + 2*delta_w ) && ( H[i].first <= Wh - 2*delta_w ) )
				H[i].second = gain;
		// In the transition near Wl
        else if ( H[i].first < Wl + 2*delta_w )
				H[i].second = ( 1 - cos( PI*( H[i].first - ( Wl - 2*delta_w ) ) / ( 4*delta_w ) ) )*gain/2;
		// In the transition near Wh
        else	H[i].second = ( 1 + cos( PI*( H[i].first - ( Wh - 2*delta_w ) ) / ( 4*delta_w ) ) )*gain/2;
	}

	// Compute the coefficients
	compute_coefficients( H );
}

filter::filter( const custom_pass& from, const float gain )
	: m_p_coefficients( NULL )
{	// Get the order, without being smaller than two
	const int N = std::max( from.m_N, 2 );

	// Allocate a vector for the results
	constraint_list_type	H( N );

	// delta_w is the difference in w between two consecutive freq. samples
	const double delta_w = PI / ( N - 1 );

	// Allocate all entries
	for( int i=0; i<N; i++ )
	{	// The frequency
		H[i].first = (double)i*delta_w;

		// Call the function
		H[i].second = from.gain( H[i].first * from.m_Fs );
	}

	// Compute the coefficients
	compute_coefficients( H );
}

// Destructor
filter::~filter( void )
{	::_aligned_free( m_p_coefficients );
}

// This will apply a FIR filter to a raw block of data
const int filter::operator() ( const float* p_src, float *p_dst, const int no_samples ) const
{	// Get the alignment
	const bool src_aligned = ( 15 & (size_t)p_src ) ? false : true;
	const bool dst_aligned = ( 15 & (size_t)p_dst ) ? false : true;
	
	// Call the appropriate routine
	if ( src_aligned )	
			if ( dst_aligned )
					FIR_apply< 1, 1 >( p_src, m_p_coefficients, p_dst, no_samples, m_filter_size );
			else	FIR_apply< 1, 0 >( p_src, m_p_coefficients, p_dst, no_samples, m_filter_size );

	else	if ( dst_aligned )
					FIR_apply< 0, 1 >( p_src, m_p_coefficients, p_dst, no_samples, m_filter_size );
			else	FIR_apply< 0, 0 >( p_src, m_p_coefficients, p_dst, no_samples, m_filter_size );

	// Return the number of samples writtern
	return no_samples - m_filter_size;
}

// This uses matrices to solve the equations
void filter::compute_coefficients( const constraint_list_type& H )
{	// Get the problem size
	const int N = (int)H.size();

	// We allocate a matrix ...
	double**	ppA = new double* [ N ];
	for( int i=0; i<N; i++ ) 
		ppA[ i ] = new double [ N+1 ];

	// Create the matrix to use
	for( int i=0; i<N; i++ )
	{	ppA[i][0] = 1;
		for( int k=1; k<N; k++ )
			ppA[i][k] = 2*cos( k*H[i].first );
		ppA[i][N] = H[i].second;
	}

#if 0
	for( int i=0; i<N; i++ )
	{	printf( "[" );
		for( int j=0; j<N; j++ )
			printf("%1.3f,", ppA[i][j] );
		printf( "],\n" );
	}

	printf( "\n\n[" );
	for( int j=0; j<N; j++ )
		printf("%1.3f,", ppA[j][N] );
	printf( "]" );
#endif

	// Perform Forward elimination
	for( int i=0; i<N; i++ ) 
	{	// Locate the pivot row
		int max = i;
		for( int j=i+1; j<N; j++ )
		if ( abs( ppA[j][i] ) > abs( ppA[max][i] ) )
				max = j;

		// Check the matrix does not look singular
		assert( abs( ppA[i][i] ) > 1.0e-10 );
		
		// Swap the tows
		for( int j=0; j<N+1; j++ ) 
			std::swap( ppA[max][j], ppA[i][j] );
		
		// Eliminate this row		
		for( int j=N; j>=i; j-- )
		{	const double coeff = ppA[i][j] / ppA[i][i];
			for( int k=i+1; k<N; k++ )
				ppA[k][j] -= ppA[k][i]*coeff;
		}
	}

	// Back substitution
	for( int i=N-1; i>=0; i-- ) 
	{	ppA[i][N] = ppA[i][N] / ppA[i][i];
		ppA[i][i] = 1;

		// Cycle across the tow
		for( int j=i-1; j>=0; j-- ) 
		{	ppA[j][N] -= ppA[j][i] * ppA[i][N];
			ppA[j][i] = 0;
		}
	}

	// We now build up the matrix of what we are going to use
	_aligned_free( m_p_coefficients );

	// Get the filter sizes
	m_real_filter_size = N*2 - 1;
	m_filter_size	   = ( m_real_filter_size + 3 ) & (~3);	

	// Allocate it
	m_p_coefficients = (float*)::_aligned_malloc( sizeof( float ) * m_filter_size, 16 );

	// Unused coefficients
	for( int i=m_real_filter_size; i<m_filter_size; i++ )
		m_p_coefficients[ i ] = 0;

	// Now copy across the coefficients
	m_p_coefficients[N-1] = (float)ppA[0][N];
	for( int i=1; i<N; i++ )
	{	m_p_coefficients[N-1-i] = 
		m_p_coefficients[N-1+i] = (float)ppA[i][N];
	}

	// Free the matrix
	for( int i=0; i<N; i++ ) delete [] ppA[ i ];
	delete [] ppA;
}

// Get the size of the current filter
const int filter::size( void ) const
{	return m_filter_size;
}