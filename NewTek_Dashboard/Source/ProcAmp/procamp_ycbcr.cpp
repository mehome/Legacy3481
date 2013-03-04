#include "StdAfx.h"
#include "ProcAmp.h"

#include <intrin.h>

using namespace PFX::procamp;

// Apply the procamp
template< const bool src_align, const bool dst_align >
void apply_procamp( const FBMP::bitmap_ycbcr_u8& src, FBMP::bitmap_ycbcr_u8& dst, const ycbcr::color_matrix& matr )
{	// Debugging
	assert( src.is_same_size_as( dst ) );
	
	// We first need to build the LUTs
	__declspec( align( 16 ) ) 
	BYTE m_lut[ 256*8 * 3 + /* fix compiler bug */16 ];

#define		store_y( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) ), (x) )
#define		store_cb( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 2048 ), (x) )
#define		store_cr( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 4096 ), (x) )

#define		load_y( i )				_mm_loadl_epi64( (__m128i*)( m_lut + (i)*8 ) )
#define		load_cb( i )			_mm_loadl_epi64( (__m128i*)( m_lut + (i)*8 + 2048 ) )
#define		load_cr( i )			_mm_loadl_epi64( (__m128i*)( m_lut + (i)*8 + 4096 ) )

	// Build up the matrices	
	const __m128	kA  = _mm_set1_ps( 32.0f*255.0f );	// 5 bits of precision
		  __m128	y   = _mm_mul_ps( kA, _mm_set_ps( /* cb */ matr[ 0 ][ 3 ], /* y */ matr[ 2 ][ 3 ], /* cr */ matr[ 0 ][ 3 ], /* y */ matr[ 1 ][ 3 ] ) );
		  __m128	cb  = _mm_setzero_ps();
		  __m128	cr  = _mm_setzero_ps();
	const __m128	kB  = _mm_set1_ps( 32.0f );	// 5 bits of precision
	const __m128	dy  = _mm_mul_ps( kB, _mm_set_ps( /* cb */ matr[ 0 ][ 0 ], /* y */ matr[ 2 ][ 0 ], /* cr */ matr[ 0 ][ 0 ], /* y */ matr[ 1 ][ 0 ] ) );
	const __m128	dcb = _mm_mul_ps( kB, _mm_set_ps( /* cb */ matr[ 0 ][ 1 ], /* y */ matr[ 2 ][ 1 ], /* cr */ matr[ 0 ][ 1 ], /* y */ matr[ 1 ][ 1 ] ) );
	const __m128	dcr = _mm_mul_ps( kB, _mm_set_ps( /* cb */ matr[ 0 ][ 2 ], /* y */ matr[ 2 ][ 2 ], /* cr */ matr[ 0 ][ 2 ], /* y */ matr[ 1 ][ 2 ] ) );

	// Build the look-up table
	for( int i=0; i<256; i++ )
	{	// Update the Y LUT
		store_y( i, _mm_packs_epi32( _mm_cvtps_epi32( y ), _mm_setzero_si128() ) ); 
		y = _mm_add_ps( y, dy );
		
		// Update the CB LUT
		store_cb( i, _mm_packs_epi32( _mm_cvtps_epi32( cb ), _mm_setzero_si128() ) );
		cb = _mm_add_ps( cb, dcb );

		// Update the CR LUT
		store_cr( i, _mm_packs_epi32( _mm_cvtps_epi32( cr ), _mm_setzero_si128() ) );
		cr = _mm_add_ps( cr, dcr );
	}

	// Get the resolution
	const int xres = src.xres();
	const int yres = src.yres();

	// A constant to mask luminances
	const __m128i mask_y = _mm_setr_epi32( -1, 0, -1, -1 );

	// Cycle down the image
	for( int y=0; y<yres; y++ )
	{	// Get the line pointers
		const BYTE* p_src   = (BYTE*)src( y );
		const BYTE* p_src_e = p_src + xres*2;
			  BYTE* p_dst   = (BYTE*)dst( y );

		// Cycle across the line 8 pixels at a time
		while( p_src + 15 < p_src_e )
		{	// Prefetch
			_mm_prefetch( 256 + (char*)p_src, _MM_HINT_NTA );
			
			// Color correct the first pixel
			const DWORD src_0 = *( 0 + (DWORD*)p_src );
			const __m128i ycbcr_0 = _mm_adds_epi16( _mm_adds_epi16( load_cb( (BYTE)(src_0) ), load_cr( (BYTE)(src_0>>16) ) ),
													_mm_adds_epi16( _mm_and_si128( mask_y, load_y( (BYTE)(src_0>>8) ) ), _mm_andnot_si128( mask_y, load_y( (BYTE)(src_0>>24) ) ) ) );

			// Color correct the second pixel
			const DWORD src_1 = *( 1 + (DWORD*)p_src );
			const __m128i ycbcr_1 = _mm_adds_epi16( _mm_adds_epi16( load_cb( (BYTE)(src_1) ), load_cr( (BYTE)(src_1>>16) ) ),
													_mm_adds_epi16( _mm_and_si128( mask_y, load_y( (BYTE)(src_1>>8) ) ), _mm_andnot_si128( mask_y, load_y( (BYTE)(src_1>>24) ) ) ) );

			// Combine the first two pixels together
			const __m128i ycbcr_01 = _mm_srai_epi16( _mm_unpacklo_epi64( ycbcr_0, ycbcr_1 ), 5 );

			// Color correct the third pixel
			const DWORD src_2 = *( 2 + (DWORD*)p_src );
			const __m128i ycbcr_2 = _mm_adds_epi16( _mm_adds_epi16( load_cb( (BYTE)(src_2) ), load_cr( (BYTE)(src_2>>16) ) ),
													_mm_adds_epi16( _mm_and_si128( mask_y, load_y( (BYTE)(src_2>>8) ) ), _mm_andnot_si128( mask_y, load_y( (BYTE)(src_2>>24) ) ) ) );

			// Color correct the forth pixel
			const DWORD src_3 = *( 3 + (DWORD*)p_src );
			const __m128i ycbcr_3 = _mm_adds_epi16( _mm_adds_epi16( load_cb( (BYTE)(src_3) ), load_cr( (BYTE)(src_3>>16) ) ),
													_mm_adds_epi16( _mm_and_si128( mask_y, load_y( (BYTE)(src_3>>8) ) ), _mm_andnot_si128( mask_y, load_y( (BYTE)(src_3>>24) ) ) ) );

			// Combine the second two pixels together
			const __m128i ycbcr_23 = _mm_srai_epi16( _mm_unpacklo_epi64( ycbcr_2, ycbcr_3 ), 5 );
								 
			// Generate all four pixels
			const __m128i ycbcr_0123 = _mm_packus_epi16( ycbcr_01, ycbcr_23 );

			// Store the results
			if ( dst_align )	_mm_store_si128 ( (__m128i*)p_dst, ycbcr_0123 );
			else				_mm_storeu_si128( (__m128i*)p_dst, ycbcr_0123 );

			// Increment the pointers
			p_src += 16;
			p_dst += 16;
		}

		// Cycle across the line 2 pixels at a time
		while( p_src + 3 < p_src_e )
		{	// Load in 2 source pixels
			const int src_0 = *(int*)p_src;

			// Color correct the first pixel
			const __m128i ycbcr_0 = _mm_adds_epi16( _mm_adds_epi16( load_cb( (src_0&255) ), load_cr( ((src_0>>16)&255) ) ),
													_mm_adds_epi16( _mm_and_si128   ( mask_y, load_y( ((src_0>>8)&255) ) ), _mm_andnot_si128( mask_y, load_y( ((src_0>>24)&255) ) ) ) );

			// Generate all four pixels
			const __m128i ycbcr_0123 = _mm_packus_epi16( _mm_srai_epi16( ycbcr_0, 5 ), _mm_setzero_si128() );

			// Store the results
			*((int*)p_dst) = _mm_cvtsi128_si32( ycbcr_0123 );

			// Increment the pointers
			p_src += 4;
			p_dst += 4;
		}
	}


#undef		store_y		// ( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) ), x )
#undef		store_cb	// ( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 2048 ), x )
#undef		store_cr	// ( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 4096 ), x )
}

// Constructor
ycbcr::ycbcr( const bitmap_ycbcr_u8& src, bitmap_ycbcr_u8& dst, const color_matrix& matr )
{	// Debugging
	assert( src.is_same_size_as( dst ) );
	
	// Check alignments
	if ( src.is_aligned( 16 ) ) if ( dst.is_aligned( 16 ) )		apply_procamp< 1, 1 >( src, dst, matr );
								else							apply_procamp< 1, 0 >( src, dst, matr );
	else						if ( dst.is_aligned( 16 ) )		apply_procamp< 0, 1 >( src, dst, matr );
								else							apply_procamp< 0, 0 >( src, dst, matr );
}