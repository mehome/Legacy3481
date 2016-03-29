// FrameWork.Audio2.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "FrameWork.Audio2.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif


#define is_aligned( a ) ( ! ( 15 & (size_t)a ) )

template<> 
void FrameWork::Audio2::scale( float* __restrict p_buffer, const int width, double scale_factor )
{	// Nothing to do ?
	if ( scale_factor == 1.0 ) return;
	
	// The line position
	int i = 0;

	const float		scale_x1 = (float)scale_factor;
	const __m128	scale_x4 = _mm_set1_ps( scale_x1 );

	if ( is_aligned( p_buffer ) )
	{	
		for( ; i<width-15; i+=16 )
		{	_mm_prefetch( 512 + (char*)( p_buffer + i ), _MM_HINT_NTA );
			const __m128	x0 = _mm_mul_ps( scale_x4, _mm_load_ps( p_buffer + i +  0 ) );
			const __m128	x1 = _mm_mul_ps( scale_x4, _mm_load_ps( p_buffer + i +  4 ) );
			const __m128	x2 = _mm_mul_ps( scale_x4, _mm_load_ps( p_buffer + i +  8 ) );
			const __m128	x3 = _mm_mul_ps( scale_x4, _mm_load_ps( p_buffer + i + 12 ) );

			_mm_store_ps( p_buffer + i +  0, x0 );
			_mm_store_ps( p_buffer + i +  4, x1 );
			_mm_store_ps( p_buffer + i +  8, x2 );
			_mm_store_ps( p_buffer + i + 12, x3 );
		}
	}
	else
	{	for( ; i<width-15; i+=16 )
			{	_mm_prefetch( 512 + (char*)( p_buffer + i ), _MM_HINT_NTA );
				const __m128	x0 = _mm_mul_ps( scale_x4, _mm_loadu_ps( p_buffer + i +  0 ) );
				const __m128	x1 = _mm_mul_ps( scale_x4, _mm_loadu_ps( p_buffer + i +  4 ) );
				const __m128	x2 = _mm_mul_ps( scale_x4, _mm_loadu_ps( p_buffer + i +  8 ) );
				const __m128	x3 = _mm_mul_ps( scale_x4, _mm_loadu_ps( p_buffer + i + 12 ) );

				_mm_storeu_ps (p_buffer + i +  0,x0);
				_mm_storeu_ps (p_buffer + i +  4,x1);
				_mm_storeu_ps (p_buffer + i +  8,x2);
				_mm_storeu_ps (p_buffer + i + 12,x3);
			}
	}

	// Remaining single items
	for( ; i<width; i++ )
		p_buffer[ i ] *= (float)scale_factor ;
}