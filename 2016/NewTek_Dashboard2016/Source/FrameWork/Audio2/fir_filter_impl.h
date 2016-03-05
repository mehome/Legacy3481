__forceinline __m128i to_m128i( const __m128  x ) { return *(__m128i*)&x; }
__forceinline __m128  to_m128 ( const __m128i x ) { return *(__m128* )&x; }

// Apply an 8 element FIR filter.
template< const bool src_aligned, const bool dst_aligned, const bool add_to_destination >
void FIR_16_element( const float* p_src, const float* p_krn, float* p_dst, const int no_samples )
{	// Store the kernel
	const __m128 krn_0 = _mm_load_ps( p_krn + 0  );
	const __m128 krn_1 = _mm_load_ps( p_krn + 4  );
	const __m128 krn_2 = _mm_load_ps( p_krn + 8  );
	const __m128 krn_3 = _mm_load_ps( p_krn + 12 );

	// Load the first 8 values from memory
	__m128 src_0 = _mm_load_ps( p_src + 0  );
	__m128 src_1 = _mm_load_ps( p_src + 4  );
	__m128 src_2 = _mm_load_ps( p_src + 8  );
	__m128 src_3 = _mm_load_ps( p_src + 12 );

	// Now cycle over the results
	p_src += 16;
	for( int i=0; i<no_samples-19; i+=4 )
	{	// Load the next 4 floats
		const __m128	src_4 = src_aligned ? _mm_load_ps( p_src + i ) : _mm_loadu_ps( p_src + i );

		// The first float of output
		const __m128	d0 = _mm_add_ps( _mm_add_ps( _mm_mul_ps( krn_0, src_0 ), _mm_mul_ps( krn_1, src_1 ) ),
										 _mm_add_ps( _mm_mul_ps( krn_2, src_2 ), _mm_mul_ps( krn_3, src_3 ) ) );

		// The second float of output
		__m128			s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 4 ) );
		__m128			s1 = to_m128( _mm_alignr_epi8( to_m128i( src_2 ), to_m128i( src_1 ), 4 ) );
		__m128			s2 = to_m128( _mm_alignr_epi8( to_m128i( src_3 ), to_m128i( src_2 ), 4 ) );
		__m128			s3 = to_m128( _mm_alignr_epi8( to_m128i( src_4 ), to_m128i( src_3 ), 4 ) );

		const __m128	d1 = _mm_add_ps( _mm_add_ps( _mm_mul_ps( krn_0, s0 ), _mm_mul_ps( krn_1, s1 ) ),
										 _mm_add_ps( _mm_mul_ps( krn_2, s2 ), _mm_mul_ps( krn_3, s3 ) ) );

		// Compile these down to save register
		const __m128	d01 = _mm_hadd_ps( d0, d1 );

		// The third float of output
						s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 8 ) );
						s1 = to_m128( _mm_alignr_epi8( to_m128i( src_2 ), to_m128i( src_1 ), 8 ) );
						s2 = to_m128( _mm_alignr_epi8( to_m128i( src_3 ), to_m128i( src_2 ), 8 ) );
						s3 = to_m128( _mm_alignr_epi8( to_m128i( src_4 ), to_m128i( src_3 ), 8 ) );

		const __m128	d2 = _mm_add_ps( _mm_add_ps( _mm_mul_ps( krn_0, s0 ), _mm_mul_ps( krn_1, s1 ) ),
										 _mm_add_ps( _mm_mul_ps( krn_2, s2 ), _mm_mul_ps( krn_3, s3 ) ) );

		// The fourth float of output
						s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 12 ) );
						s1 = to_m128( _mm_alignr_epi8( to_m128i( src_2 ), to_m128i( src_1 ), 12 ) );
						s2 = to_m128( _mm_alignr_epi8( to_m128i( src_3 ), to_m128i( src_2 ), 12 ) );
						s3 = to_m128( _mm_alignr_epi8( to_m128i( src_4 ), to_m128i( src_3 ), 12 ) );
		const __m128	d3 = _mm_add_ps( _mm_add_ps( _mm_mul_ps( krn_0, s0 ), _mm_mul_ps( krn_1, s1 ) ),
										 _mm_add_ps( _mm_mul_ps( krn_2, s2 ), _mm_mul_ps( krn_3, s3 ) ) );

		// Compile these down to save registers
		const __m128	d23 = _mm_hadd_ps( d2, d3 );

		// Store the results
		__m128			d0123 = _mm_hadd_ps( d01,d23 );

		// Generate the results
		if ( add_to_destination )
						d0123 = _mm_add_ps( d0123, dst_aligned ? _mm_load_ps( p_dst + i ) : _mm_loadu_ps( p_dst + i ) );
		
		// Store
		if ( dst_aligned )	_mm_store_ps ( p_dst + i, d0123 );
		else				_mm_storeu_ps( p_dst + i, d0123 );		

		// Cycle the values
		src_0 = src_1;
		src_1 = src_0;
	}
}

// Apply an 8 element FIR filter.
template< const bool src_aligned, const bool dst_aligned, const bool add_to_destination >
void FIR_8_element( const float* p_src, const float* p_krn, float* p_dst, const int no_samples )
{	// Store the kernel
	const __m128	krn_0 = _mm_load_ps( p_krn + 0 );
	const __m128	krn_1 = _mm_load_ps( p_krn + 4 );

	// Load the first 8 values from memory
	__m128	src_0 = _mm_load_ps( p_src + 0 );
	__m128	src_1 = _mm_load_ps( p_src + 4 );

	// Now cycle over the results
	p_src += 8;
	for( int i=0; i<no_samples-11; i+=4 )
	{	// Load the next 4 floats
		const __m128	src_2 = src_aligned ? _mm_load_ps( p_src + i ) : _mm_loadu_ps( p_src + i );

		// The first float of output
		const __m128	d0 = _mm_add_ps( _mm_mul_ps( krn_0, src_0 ), _mm_mul_ps( krn_1, src_1 ) );

		// The second float of output
		__m128			s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 4 ) );
		__m128			s1 = to_m128( _mm_alignr_epi8( to_m128i( src_2 ), to_m128i( src_1 ), 4 ) );
		const __m128	d1 = _mm_add_ps( _mm_mul_ps( krn_0, s0 ), _mm_mul_ps( krn_1, s1 ) );

		// Compile these down to save register
		const __m128	d01 = _mm_hadd_ps( d0, d1 );

		// The third float of output
						s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 8 ) );
						s1 = to_m128( _mm_alignr_epi8( to_m128i( src_2 ), to_m128i( src_1 ), 8 ) );
		const __m128	d2 = _mm_add_ps( _mm_mul_ps( krn_0, s0 ), _mm_mul_ps( krn_1, s1 ) );

		// The fourth float of output
						s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 12 ) );
						s1 = to_m128( _mm_alignr_epi8( to_m128i( src_2 ), to_m128i( src_1 ), 12 ) );
		const __m128	d3 = _mm_add_ps( _mm_mul_ps( krn_0, s0 ), _mm_mul_ps( krn_1, s1 ) );

		// Compile these down to save registers
		const __m128	d23 = _mm_hadd_ps( d2, d3 );

		// Store the results
		__m128			d0123 = _mm_hadd_ps( d01,d23 );

		// Generate the results
		if ( add_to_destination )
						d0123 = _mm_add_ps( d0123, dst_aligned ? _mm_load_ps( p_dst + i ) : _mm_loadu_ps( p_dst + i ) );
		
		// Store
		if ( dst_aligned )	_mm_store_ps ( p_dst + i, d0123 );
		else				_mm_storeu_ps( p_dst + i, d0123 );		

		// Cycle the values
		src_0 = src_1;
		src_1 = src_0;
	}
}

// Apply a 4 element FIR filter.
template< const bool src_aligned, const bool dst_aligned, const bool add_to_destination >
void FIR_4_element( const float* p_src, const float* p_krn, float* p_dst, const int no_samples )
{	// Store the kernel
	const __m128	krn_0 = _mm_load_ps( p_krn + 0 );

	// Load the first 8 values from memory
	__m128			src_0 = _mm_load_ps( p_src + 0 );

	// Now cycle over the results
	p_src += 4;
	for( int i=0; i<no_samples-7; i+=4 )
	{	// Load the next 4 floats
		const __m128	src_1 = src_aligned ? _mm_load_ps( p_src + i ) : _mm_loadu_ps( p_src + i );

		// The first float of output
		const __m128	d0 = _mm_mul_ps( krn_0, src_0 );

		// The second float of output
		__m128			s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 4 ) );
		const __m128	d1 = _mm_mul_ps( krn_0, s0 );

		// Compile these down to save register
		const __m128	d01 = _mm_hadd_ps( d0, d1 );

		// The third float of output
						s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 8 ) );
		const __m128	d2 = _mm_mul_ps( krn_0, s0 );

		// The fourth float of output
						s0 = to_m128( _mm_alignr_epi8( to_m128i( src_1 ), to_m128i( src_0 ), 12 ) );
		const __m128	d3 = _mm_mul_ps( krn_0, s0 );

		// Compile these down to save registers
		const __m128	d23 = _mm_hadd_ps( d2, d3 );

		// Store the results
		__m128			d0123 = _mm_hadd_ps( d01,d23 );

		// Generate the results
		if ( add_to_destination )
						d0123 = _mm_add_ps( d0123, dst_aligned ? _mm_load_ps( p_dst + i ) : _mm_loadu_ps( p_dst + i ) );
		
		// Store
		if ( dst_aligned )	_mm_store_ps ( p_dst + i, d0123 );
		else				_mm_storeu_ps( p_dst + i, d0123 );		

		// Cycle the values
		src_0 = src_1;
	}
}

template< const bool src_aligned, const bool dst_aligned >
void FIR_apply( const float* p_src, const float* p_krn, float* p_dst, const int no_samples, const int kernel_size )
{	// The number of samples
	int		N	  = no_samples;
	int		sz	  = kernel_size;
	bool	first = true;

	// 16 element FIR
	while( sz > 15 )
	{	// Apply an 8 element FIR filter
		if ( first ) FIR_16_element< src_aligned, dst_aligned, 0 >( p_src, p_krn, p_dst, no_samples );
		else		 FIR_16_element< src_aligned, dst_aligned, 1 >( p_src, p_krn, p_dst, no_samples );
		first = false;

		// Offset the pointers now
		p_krn += 16;
		p_src += 16;
		sz -= 16;
	}
		
	// 8 element FIR
	while( sz > 7 )
	{	// Apply an 8 element FIR filter
		if ( first ) FIR_8_element< src_aligned, dst_aligned, 0 >( p_src, p_krn, p_dst, no_samples );
		else		 FIR_8_element< src_aligned, dst_aligned, 1 >( p_src, p_krn, p_dst, no_samples );
		first = false;

		// Offset the pointers now
		p_krn += 8;
		p_src += 8;
		sz -= 8;
	}
	
	// 4 element FIR
	while( sz > 3 )
	{	// Apply an 8 element FIR filter
		if ( first ) FIR_4_element< src_aligned, dst_aligned, 0 >( p_src, p_krn, p_dst, no_samples );
		else		 FIR_4_element< src_aligned, dst_aligned, 1 >( p_src, p_krn, p_dst, no_samples );
		first = false;

		// Offset the pointers now
		p_krn += 4;
		p_src += 4;
		sz -= 4;
	}
}