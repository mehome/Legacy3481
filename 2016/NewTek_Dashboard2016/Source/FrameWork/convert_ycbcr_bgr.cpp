#include "StdAfx.h"
#include <intrin.h>

#include "FrameWork.h"

using namespace FrameWork::Bitmaps;

struct uyvy_to_bgr
{		// Constructor
		uyvy_to_bgr( void );

		// The lookup tables
		BYTE	m_lut[ 256*8 * 4 ];
};

uyvy_to_bgr::uyvy_to_bgr( void )
{	

#define		store_y0( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) +    0 ), x )
#define		store_y1( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 6144 ), x )
#define		store_u( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 2048 ), x )
#define		store_v( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 4096 ), x )

	for( int i=0; i<256; i++ )
	{	// B = 1.164 * Y             + 2.018 * U - 276.928
		// G = 1.164 * Y - 0.813 * V - 0.391 * U + 135.488
		// R = 1.164 * Y + 1.596 * V             - 222.912
		store_y0( i, _mm_setr_epi16( (int)floor( 0.5f + 32.0f * ( 1.164f*(float)i - 276.928f ) ),		// B
									 (int)floor( 0.5f + 32.0f * ( 1.164f*(float)i + 135.488f ) ),		// G
									 (int)floor( 0.5f + 32.0f * ( 1.164f*(float)i - 222.912f ) ),		// R
									 (int)floor( 0.5f + 32.0f * ( 255.0f ) ),							// A
									 0, 0, 0, 0 ) );

		store_y1( i, _mm_setr_epi16( (int)floor( 0.5f + 32.0f * ( 1.164f*(float)i - 276.928f ) ),		// B
									 (int)floor( 0.5f + 32.0f * ( 1.164f*(float)i + 135.488f ) ),		// G
									 (int)floor( 0.5f + 32.0f * ( 1.164f*(float)i - 222.912f ) ),		// R
									 0, 0, 0, 0, 0 ) );

		store_u( i, _mm_setr_epi16( (int)floor( 0.5f + 32.0f * (  2.018f*(float)i ) ),					// B
									(int)floor( 0.5f + 32.0f * ( -0.391f*(float)i ) ),					// G
									(int)floor( 0.5f + 32.0f * (  0.000f*(float)i ) ),					// R
									0, 0, 0, 0, 0 ) );

		store_v( i, _mm_setr_epi16( (int)floor( 0.5f + 32.0f * (  0.000f*(float)i ) ),					// B
									(int)floor( 0.5f + 32.0f * ( -0.813f*(float)i ) ),					// G
									(int)floor( 0.5f + 32.0f * (  1.596f*(float)i ) ),					// R
									0, 0, 0, 0, 0 ) );
	}

#undef		store_y0
#undef		store_y1
#undef		store_u
#undef		store_v
}

static __declspec( align(16) )	uyvy_to_bgr	g_uyvy_to_bgr;

__forceinline __m128i uyvy_to_bgra( const BYTE *p_src )
{
#define		load_y( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 ) )
#define		load_u( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 2048 ) )
#define		load_v( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 4096 ) )

	__m128i		bgra_uv  = _mm_add_epi16( load_u( p_src[ 0 ] ), load_v( p_src[ 2 ] ) );
	__m128i		bgra_y_0 = load_y( p_src[ 1 ] );
	__m128i		bgra_y_1 = load_y( p_src[ 3 ] );
	__m128i		bgra_0   = _mm_add_epi16( bgra_uv, bgra_y_0 );
	__m128i		bgra_1   = _mm_add_epi16( bgra_uv, bgra_y_1 );
	return				   _mm_srai_epi16( _mm_unpacklo_epi64( bgra_0, bgra_1 ), 5 );

#undef		load_y	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_u	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_v	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
}

__forceinline __m128i uyvy_to_bgra_x1( const BYTE *p_src )
{
#define		load_y( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 ) )
#define		load_u( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 2048 ) )
#define		load_v( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 4096 ) )

	__m128i		bgra_uv  = _mm_add_epi16( load_u( p_src[ 0 ] ), load_v( p_src[ 2 ] ) );
	__m128i		bgra_y   = load_y( p_src[ 1 ] );
	__m128i		bgra_0   = _mm_add_epi16( bgra_uv, bgra_y );
	return				   _mm_srai_epi16( bgra_0, 5 );

#undef		load_y	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_u	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_v	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
}

__forceinline __m128i uyvy_to_bgr( const BYTE *p_src )
{
#define		load_y( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 6144 ) )
#define		load_u( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 2048 ) )
#define		load_v( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 4096 ) )

	__m128i		bgra_uv  = _mm_add_epi16( load_u( p_src[ 0 ] ), load_v( p_src[ 2 ] ) );
	__m128i		bgra_y_0 = load_y( p_src[ 1 ] );
	__m128i		bgra_y_1 = load_y( p_src[ 3 ] );
	__m128i		bgra_0   = _mm_add_epi16( bgra_uv, bgra_y_0 );
	__m128i		bgra_1   = _mm_slli_si128( _mm_add_epi16( bgra_uv, bgra_y_1 ), 6 );
	return				   _mm_srai_epi16( _mm_or_si128( bgra_0, bgra_1 ), 5 );

#undef		load_y	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_u	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_v	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
}

__forceinline __m128i uyvy_to_bgr_x1( const BYTE *p_src )
{
	return uyvy_to_bgra_x1( p_src );
}

#define		load_y( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 ) )
#define		load_u( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 2048 ) )
#define		load_v( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 4096 ) )

static const __m128i	y_to_bgra_const = _mm_add_epi16( load_u( 128 ), load_v( 128 ) );

__forceinline __m128i y_to_bgra( const BYTE *p_src )
{
	__m128i		bgra_y_0 = load_y( p_src[ 0 ] );
	__m128i		bgra_y_1 = load_y( p_src[ 1 ] );
	__m128i		bgra_0   = _mm_add_epi16( y_to_bgra_const, bgra_y_0 );
	__m128i		bgra_1   = _mm_add_epi16( y_to_bgra_const, bgra_y_1 );
	return				   _mm_srai_epi16( _mm_unpacklo_epi64( bgra_0, bgra_1 ), 5 );

#undef		load_y	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_u	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_v	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
}

__forceinline __m128i y_to_bgra_x1( const BYTE *p_src )
{
#define		load_y( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 ) )
#define		load_u( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 2048 ) )
#define		load_v( i )		_mm_loadl_epi64( (__m128i*)( g_uyvy_to_bgr.m_lut + (i)*8 + 4096 ) )

	__m128i		bgra_y_0 = load_y( p_src[ 0 ] );
	__m128i		bgra_0   = _mm_add_epi16( y_to_bgra_const, bgra_y_0 );
	return				   _mm_srai_epi16( _mm_unpacklo_epi64( bgra_0, _mm_setzero_si128() ), 5 );

#undef		load_y	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_u	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_v	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width )
{	// Aligned store
	pixel_bgra_u8*  __restrict p_dst_e = p_dst + width;
		
	if ( 15 & (size_t)p_dst )
	{	// Blocks of 4 pixels
		while( p_dst < p_dst_e - 3 )
		{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
			const __m128i	bgra_01   = uyvy_to_bgra( (BYTE*)( p_src + 0 ) );
			const __m128i	bgra_23   = uyvy_to_bgra( (BYTE*)( p_src + 1 ) );
			const __m128i	bgra_0123 = _mm_packus_epi16( bgra_01, bgra_23 );
			_mm_storeu_si128( (__m128i*)p_dst, bgra_0123 );
			p_src += 2;
			p_dst += 4;
		}
	}
	else
	{	// Blocks of 4 pixels
		while( p_dst < p_dst_e - 3 )
		{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
			const __m128i	bgra_01   = uyvy_to_bgra( (BYTE*)( p_src + 0 ) );
			const __m128i	bgra_23   = uyvy_to_bgra( (BYTE*)( p_src + 1 ) );
			const __m128i	bgra_0123 = _mm_packus_epi16( bgra_01, bgra_23 );
			_mm_store_si128( (__m128i*)p_dst, bgra_0123 );
			p_src += 2;
			p_dst += 4;
		}
	}

	// Blocks of 2 pixels
	if ( p_dst < p_dst_e - 1 )
	{	const __m128i	bgra_01 = _mm_packus_epi16( uyvy_to_bgra( (BYTE*)( p_src + 0 ) ), _mm_setzero_si128() );
		_mm_storel_epi64( (__m128i*)p_dst, bgra_01 );
		p_src += 1;
		p_dst += 2;
	}

	// Single pixel block
	if ( p_dst < p_dst_e )
	{	const __m128i	bgra_0 = _mm_packus_epi16( uyvy_to_bgra_x1( (BYTE*)( p_src + 0 ) ), _mm_setzero_si128() );
		*(DWORD*)( p_dst ) = _mm_cvtsi128_si32( bgra_0 );
	}
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_y_u8* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width )
{	if ( 15 & (size_t)p_dst )
	{	// Unaligned store
		int i = 0;

		// Blocks of 4 pixels
		for( ; i<width-3; i+=4 )
		{	_mm_prefetch( 512 + (char*)( p_src + i + 0 ), _MM_HINT_NTA );
			const __m128i	bgra_01   = y_to_bgra( (BYTE*)( p_src + i + 0 ) );
			const __m128i	bgra_23   = y_to_bgra( (BYTE*)( p_src + i + 2 ) );
			const __m128i	bgra_0123 = _mm_packus_epi16( bgra_01, bgra_23 );
			_mm_storeu_si128( (__m128i*)( p_dst + i ), bgra_0123 );
		}

		// Blocks of 2 pixels
		for( ; i<width; i++ )
		{	const __m128i	bgra_01 = y_to_bgra( (BYTE*)( p_src + i + 0 ) );
			*(DWORD*)( p_dst + i ) = _mm_cvtsi128_si32( bgra_01 );
		}
	}
	else
	{	// Aligned store
		int i = 0;

		// Blocks of 4 pixels
		for( ; i<width-3; i+=4 )
		{	_mm_prefetch( 512 + (char*)( p_src + i + 0 ), _MM_HINT_NTA );
			const __m128i	bgra_01   = y_to_bgra( (BYTE*)( p_src + i + 0 ) );
			const __m128i	bgra_23   = y_to_bgra( (BYTE*)( p_src + i + 2 ) );
			const __m128i	bgra_0123 = _mm_packus_epi16( bgra_01, bgra_23 );
			_mm_store_si128( (__m128i*)( p_dst + i ), bgra_0123 );
		}

		// Blocks of 2 pixels
		for( ; i<width; i++ )
		{	const __m128i	bgra_01 = y_to_bgra( (BYTE*)( p_src + i + 0 ) );
			*(DWORD*)( p_dst + i ) = _mm_cvtsi128_si32( bgra_01 );
		}
	}
}


template<> 
void FrameWork::Bitmaps::convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width )
{	// Aligned store
	pixel_bgr_u8*  __restrict p_dst_e = p_dst + width;
	
	// Blocks of 2 pixels
	while( p_dst < p_dst_e-1 )
	{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
		const __m128i	bgra_u16 = uyvy_to_bgr( (BYTE*)p_src );
		const __m128i	bgra_u8  = _mm_packus_epi16( bgra_u16, bgra_u16 );
		*(DWORD*)( (BYTE*)p_dst     ) = _mm_cvtsi128_si32( bgra_u8 );							// bgrb
		*(WORD*) ( (BYTE*)p_dst + 4 ) = _mm_cvtsi128_si32( _mm_srli_si128( bgra_u8, 4 ) );	// gr__
		p_dst += 2;
		p_src++;
	}

	// Blocks of 1 pixel
	if ( p_dst < p_dst_e )
	{	const __m128i	bgra_u16 = uyvy_to_bgr_x1( (BYTE*)p_src );
		const __m128i	bgra_u8  = _mm_packus_epi16( bgra_u16, bgra_u16 );
		DWORD bgr_si32 = _mm_cvtsi128_si32( bgra_u8 );							// bgrx
		*(WORD*)( (BYTE*)p_dst + 0 ) = (WORD)( bgr_si32 );
		*(BYTE*)( (BYTE*)p_dst + 2 ) = (BYTE)( bgr_si32 >> 16 );
	}
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_y_u8* __restrict p_dst, const int width )
{
	pixel_y_u8* p_dst_e = p_dst + width;

	if ( 15 & (size_t)p_src )
	{	if ( 15 & (size_t)p_dst )
		{	// Blocks of 16 pixels
			for( ; p_dst < p_dst_e-15; p_dst+=16, p_src+=8 )
			{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
				const __m128i	uyvy_0  = _mm_srli_epi16( _mm_loadu_si128( (__m128i*)p_src ), 8 );
				const __m128i	uyvy_1  = _mm_srli_epi16( _mm_loadu_si128( (__m128i*)p_src ), 8 );
				const __m128i	yyyy_01 = _mm_packus_epi16( uyvy_0, uyvy_1 );
				_mm_storeu_si128( (__m128i*)p_dst, yyyy_01 );
			}
		}
		else
		{	// Blocks of 16 pixels
			for( ; p_dst < p_dst_e-15; p_dst+=16, p_src+=8 )
			{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
				const __m128i	uyvy_0  = _mm_srli_epi16( _mm_loadu_si128( (__m128i*)p_src ), 8 );
				const __m128i	uyvy_1  = _mm_srli_epi16( _mm_loadu_si128( (__m128i*)p_src ), 8 );
				const __m128i	yyyy_01 = _mm_packus_epi16( uyvy_0, uyvy_1 );
				_mm_store_si128( (__m128i*)p_dst, yyyy_01 );
			}
		}	
	}
	else
	{	if ( 15 & (size_t)p_dst )
		{	// Blocks of 16 pixels
			for( ; p_dst < p_dst_e-15; p_dst+=16, p_src+=8 )
			{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
				const __m128i	uyvy_0  = _mm_srli_epi16( _mm_load_si128( (__m128i*)p_src ), 8 );
				const __m128i	uyvy_1  = _mm_srli_epi16( _mm_load_si128( (__m128i*)p_src ), 8 );
				const __m128i	yyyy_01 = _mm_packus_epi16( uyvy_0, uyvy_1 );
				_mm_storeu_si128( (__m128i*)p_dst, yyyy_01 );
			}
		}
		else
		{	// Blocks of 16 pixels
			for( ; p_dst < p_dst_e-15; p_dst+=16, p_src+=8 )
			{	_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
				const __m128i	uyvy_0  = _mm_srli_epi16( _mm_load_si128( (__m128i*)p_src ), 8 );
				const __m128i	uyvy_1  = _mm_srli_epi16( _mm_load_si128( (__m128i*)p_src ), 8 );
				const __m128i	yyyy_01 = _mm_packus_epi16( uyvy_0, uyvy_1 );
				_mm_store_si128( (__m128i*)p_dst, yyyy_01 );
			}
		}	
	}
	
	// Blocks of 2 pixels
	for( ; p_dst < p_dst_e-1; p_dst+=2, p_src++ )
		p_dst[ 0 ].m_y = p_src[ 0 ].m_y0, p_dst[ 0 ].m_y = p_src[ 0 ].m_y1;
}