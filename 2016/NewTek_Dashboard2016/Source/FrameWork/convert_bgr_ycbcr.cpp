#include "StdAfx.h"
#include <intrin.h>

#include "FrameWork.h"

using namespace FrameWork::Bitmaps;

struct bgr_to_uyvy
{		// Constructor
		bgr_to_uyvy( void );

		// The lookup tables
		BYTE	m_lut[ 256*8 * 3 ];
};

bgr_to_uyvy::bgr_to_uyvy( void )
{	// The global constants
	const int k_r_y = (int)( ( 0.257f*256.0f )*32.0f + 0.5f );
	const int k_r_u = (int)( (-0.148f*256.0f )*32.0f + 0.5f );
	const int k_r_v = (int)( ( 0.439f*256.0f )*32.0f + 0.5f );
	const int k_g_y = (int)( ( 0.504f*256.0f )*32.0f + 0.5f );
	const int k_g_u = (int)( (-0.291f*256.0f )*32.0f + 0.5f );
	const int k_g_v = (int)( (-0.368f*256.0f )*32.0f + 0.5f );
	const int k_b_y = (int)( ( 0.098f*256.0f )*32.0f + 0.5f );
	const int k_b_u = (int)( ( 0.439f*256.0f )*32.0f + 0.5f );
	const int k_b_v = (int)( (-0.071f*256.0f )*32.0f + 0.5f );

#define		store_r( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) ), x )
#define		store_g( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 2048 ), x )
#define		store_b( idx, x )		_mm_storel_epi64( (__m128i*)( m_lut + ( (idx) * 8 ) + 4096 ), x )

	for( int i=0; i<256; i++ )
	{	store_r( i, _mm_setr_epi16( ( ( ( k_r_u * i ) >> 8 ) + 128*32 + 16 + 1 )/2,	// U
									  ( ( k_r_y * i ) >> 8 ) + 16 *32 + 16,			// Y
									( ( ( k_r_v * i ) >> 8 ) + 128*32 + 16 + 1 )/2,	// V
									  ( ( k_r_y * i ) >> 8 ) + 16 *32 + 16,			// Y
									0, 0, 0, 0 ) );									// Nada

		store_g( i, _mm_setr_epi16( ( ( ( k_g_u * i ) >> 8 ) + 1 )/2,				// U
									  ( ( k_g_y * i ) >> 8 ),						// Y
									( ( ( k_g_v * i ) >> 8 ) + 1 )/2,				// V
									  ( ( k_g_y * i ) >> 8 ),						// Y
									0, 0, 0, 0 ) );									// Nada

		store_b( i, _mm_setr_epi16( ( ( ( k_b_u * i ) >> 8 ) + 1 )/2,				// U
									  ( ( k_b_y * i ) >> 8 ),						// Y
									( ( ( k_b_v * i ) >> 8 ) + 1 )/2,				// V
									  ( ( k_b_y * i ) >> 8 ),						// Y
									0, 0, 0, 0 ) );									// Nada
	}

#undef		store_r
#undef		store_g
#undef		store_b
}

static __declspec( align(16) )	bgr_to_uyvy	g_bgr_to_uyvy;

const __m128i	const_uyv0_0000 = _mm_set_epi32( 0x00000000, 0x00000000, 0x0000ffff, 0xffffffff );
const __m128i	const_u0vy_0000 = _mm_set_epi32( 0x00000000, 0x00000000, 0xffffffff, 0x0000ffff );

template< const int pixel_width >
__forceinline __m128i bgr_to_uyvy_x8( const BYTE *p_src )
{	
#define		load_r( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 ) )
#define		load_g( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 2048 ) )
#define		load_b( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 4096 ) )
#define		addr_r( i )		p_src[ ( i*pixel_width + 2 ) ]
#define		addr_g( i )		p_src[ ( i*pixel_width + 1 ) ]
#define		addr_b( i )		p_src[ ( i*pixel_width + 0 ) ]

	// BGRx .... .... .... .... .... .... ....
	const __m128i	uyvy_0  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(0) ), _mm_add_epi16( load_g( addr_g(0) ), load_r( addr_r(0) ) ) ), const_uyv0_0000 );
	// .... BGRx .... .... .... .... .... ....
	const __m128i	uyvy_1  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(1) ), _mm_add_epi16( load_g( addr_g(1) ), load_r( addr_r(1) ) ) ), const_u0vy_0000 );
	// BGRx BGRx .... .... .... .... .... ....
	const __m128i	uyvy_01 = _mm_add_epi16( uyvy_0, uyvy_1 );
	
	// .... .... BGRx .... .... .... .... .... 
	const __m128i	uyvy_2  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(2) ), _mm_add_epi16( load_g( addr_g(2) ), load_r( addr_r(2) ) ) ), const_uyv0_0000 );
	// .... .... .... BGRx .... .... .... .... 
	const __m128i	uyvy_3  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(3) ), _mm_add_epi16( load_g( addr_g(3) ), load_r( addr_r(3) ) ) ), const_u0vy_0000 );
	// BGRx BGRx .... .... .... .... .... ....
	const __m128i	uyvy_23 = _mm_add_epi16( uyvy_2, uyvy_3 );

	// BGRx BGRx BGRx BGRx .... .... .... ....
	const __m128i	uyvy_0123 = _mm_srai_epi16( _mm_unpacklo_epi64( uyvy_01, uyvy_23 ), 5 );

	// .... .... .... .... BGRx .... .... .... 
	const __m128i	uyvy_4  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(4) ), _mm_add_epi16( load_g( addr_g(4) ), load_r( addr_r(4) ) ) ), const_uyv0_0000 );
	// .... .... .... .... .... BGRx .... .... 
	const __m128i	uyvy_5  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(5) ), _mm_add_epi16( load_g( addr_g(5) ), load_r( addr_r(5) ) ) ), const_u0vy_0000 );
	// .... .... .... .... BGRx BGRx .... .... 
	const __m128i	uyvy_45 = _mm_add_epi16( uyvy_4, uyvy_5 );
	
	// .... .... .... ....  .... .... BGRx .... 
	const __m128i	uyvy_6  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(6) ), _mm_add_epi16( load_g( addr_g(6) ), load_r( addr_r(6) ) ) ), const_uyv0_0000 );
	// .... .... .... ....  .... .... .... BGRx 
	const __m128i	uyvy_7  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(7) ), _mm_add_epi16( load_g( addr_g(7) ), load_r( addr_r(7) ) ) ), const_u0vy_0000 );
	// .... .... .... .... BGRx BGRx .... .... 
	const __m128i	uyvy_67 = _mm_add_epi16( uyvy_6, uyvy_7 );

	// .... .... .... .... BGRx BGRx BGRx BGRx 
	const __m128i	uyvy_4567 = _mm_srai_epi16( _mm_unpacklo_epi64( uyvy_45, uyvy_67 ), 5 );

#undef		load_r	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_g	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_b	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
#undef		addr_r	// ( i*pixel_wide + 2 )
#undef		addr_g	// ( i*pixel_wide + 1 )
#undef		addr_b	// ( i*pixel_wide + 0 )

	// Final results
	// UYVY UYVY UYVY UYVY
	return _mm_packus_epi16( uyvy_0123, uyvy_4567 );
}

template< const int pixel_width >
__forceinline __m128i bgr_to_y_x8( const BYTE *p_src )
{	
#define		load_r( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 ) )
#define		load_g( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 2048 ) )
#define		load_b( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 4096 ) )
#define		addr_r( i )		p_src[ ( i*pixel_width + 2 ) ]
#define		addr_g( i )		p_src[ ( i*pixel_width + 1 ) ]
#define		addr_b( i )		p_src[ ( i*pixel_width + 0 ) ]

	// BGRx .... .... .... .... .... .... ....
	const __m128i	uyvy_0  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(0) ), _mm_add_epi16( load_g( addr_g(0) ), load_r( addr_r(0) ) ) ), const_uyv0_0000 );
	// .... BGRx .... .... .... .... .... ....
	const __m128i	uyvy_1  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(1) ), _mm_add_epi16( load_g( addr_g(1) ), load_r( addr_r(1) ) ) ), const_u0vy_0000 );
	// BGRx BGRx .... .... .... .... .... ....
	const __m128i	uyvy_01 = _mm_add_epi16( uyvy_0, uyvy_1 );
	
	// .... .... BGRx .... .... .... .... .... 
	const __m128i	uyvy_2  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(2) ), _mm_add_epi16( load_g( addr_g(2) ), load_r( addr_r(2) ) ) ), const_uyv0_0000 );
	// .... .... .... BGRx .... .... .... .... 
	const __m128i	uyvy_3  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(3) ), _mm_add_epi16( load_g( addr_g(3) ), load_r( addr_r(3) ) ) ), const_u0vy_0000 );
	// BGRx BGRx .... .... .... .... .... ....
	const __m128i	uyvy_23 = _mm_add_epi16( uyvy_2, uyvy_3 );

	// BGRx BGRx BGRx BGRx .... .... .... ....
	const __m128i	uyvy_0123 = _mm_srai_epi16( _mm_unpacklo_epi64( uyvy_01, uyvy_23 ), 5 );

	// .... .... .... .... BGRx .... .... .... 
	const __m128i	uyvy_4  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(4) ), _mm_add_epi16( load_g( addr_g(4) ), load_r( addr_r(4) ) ) ), const_uyv0_0000 );
	// .... .... .... .... .... BGRx .... .... 
	const __m128i	uyvy_5  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(5) ), _mm_add_epi16( load_g( addr_g(5) ), load_r( addr_r(5) ) ) ), const_u0vy_0000 );
	// .... .... .... .... BGRx BGRx .... .... 
	const __m128i	uyvy_45 = _mm_add_epi16( uyvy_4, uyvy_5 );
	
	// .... .... .... ....  .... .... BGRx .... 
	const __m128i	uyvy_6  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(6) ), _mm_add_epi16( load_g( addr_g(6) ), load_r( addr_r(6) ) ) ), const_uyv0_0000 );
	// .... .... .... ....  .... .... .... BGRx 
	const __m128i	uyvy_7  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(7) ), _mm_add_epi16( load_g( addr_g(7) ), load_r( addr_r(7) ) ) ), const_u0vy_0000 );
	// .... .... .... .... BGRx BGRx .... .... 
	const __m128i	uyvy_67 = _mm_add_epi16( uyvy_6, uyvy_7 );

	// .... .... .... .... BGRx BGRx BGRx BGRx 
	const __m128i	uyvy_4567 = _mm_srai_epi16( _mm_unpacklo_epi64( uyvy_45, uyvy_67 ), 5 );

#undef		load_r	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_g	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_b	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
#undef		addr_r	// ( i*pixel_wide + 2 )
#undef		addr_g	// ( i*pixel_wide + 1 )
#undef		addr_b	// ( i*pixel_wide + 0 )

	// Convert to UYVY
	const __m128i	uyvy_01234567 = _mm_packus_epi16( uyvy_0123, uyvy_4567 );

	// Convert to YY
	const __m128i	y_0123456 = _mm_srli_epi16( uyvy_01234567, 8 );

	// Final results
	// BGRx BGRx BGRx BGRx BGRx BGRx BGRx BGRx 
	return _mm_packus_epi16( y_0123456, _mm_setzero_si128() );
}

template< const int pixel_width >
__forceinline __m128i bgr_to_uyvy_x2( const BYTE *p_src )
{	
#define		load_r( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 ) )
#define		load_g( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 2048 ) )
#define		load_b( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 4096 ) )
#define		addr_r( i )		p_src[ ( i*pixel_width + 2 ) ]
#define		addr_g( i )		p_src[ ( i*pixel_width + 1 ) ]
#define		addr_b( i )		p_src[ ( i*pixel_width + 0 ) ]

	// BGRx .... .... .... .... .... .... ....
	const __m128i	uyvy_0  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(0) ), _mm_add_epi16( load_g( addr_g(0) ), load_r( addr_r(0) ) ) ), const_uyv0_0000 );
	// .... BGRx .... .... .... .... .... ....
	const __m128i	uyvy_1  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(1) ), _mm_add_epi16( load_g( addr_g(1) ), load_r( addr_r(1) ) ) ), const_u0vy_0000 );
	// BGRx BGRx .... .... .... .... .... ....
	const __m128i	uyvy_01 = _mm_srai_epi16( _mm_add_epi16( uyvy_0, uyvy_1 ), 5 );

#undef		load_r	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_g	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_b	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
#undef		addr_r	// ( i*pixel_wide + 2 )
#undef		addr_g	// ( i*pixel_wide + 1 )
#undef		addr_b	// ( i*pixel_wide + 0 )

	// Final results
	// BGRx BGRx BGRx BGRx BGRx BGRx BGRx BGRx 
	return _mm_packus_epi16( uyvy_01, uyvy_01 );
}

template< const int pixel_width >
__forceinline __m128i bgr_to_uyvy_x1( const BYTE *p_src )
{	
#define		load_r( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 ) )
#define		load_g( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 2048 ) )
#define		load_b( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 4096 ) )
#define		addr_r( i )		p_src[ ( i*pixel_width + 2 ) ]
#define		addr_g( i )		p_src[ ( i*pixel_width + 1 ) ]
#define		addr_b( i )		p_src[ ( i*pixel_width + 0 ) ]

	// BGRx .... .... .... .... .... .... ....
	const __m128i	uyvy    = _mm_add_epi16( load_b( addr_b(0) ), _mm_add_epi16( load_g( addr_g(0) ), load_r( addr_r(0) ) ) );
	const __m128i	uyvy_0  = _mm_and_si128( uyvy, const_uyv0_0000 );
	// .... BGRx .... .... .... .... .... ....
	const __m128i	uyvy_1  = _mm_and_si128( uyvy, const_u0vy_0000 );
	// BGRx BGRx .... .... .... .... .... ....
	const __m128i	uyvy_01 = _mm_srai_epi16( _mm_add_epi16( uyvy_0, uyvy_1 ), 5 );

#undef		load_r	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_g	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_b	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
#undef		addr_r	// ( i*pixel_wide + 2 )
#undef		addr_g	// ( i*pixel_wide + 1 )
#undef		addr_b	// ( i*pixel_wide + 0 )

	// Final results
	// BGRx BGRx BGRx BGRx BGRx BGRx BGRx BGRx 
	return _mm_packus_epi16( uyvy_01, uyvy_01 );
}

template< const int pixel_width >
__forceinline BYTE bgr_to_y_x1( const BYTE *p_src )
{	
#define		load_r( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 ) )
#define		load_g( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 2048 ) )
#define		load_b( i )		_mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut + (i)*8 + 4096 ) )
#define		addr_r( i )		p_src[ ( i*pixel_width + 2 ) ]
#define		addr_g( i )		p_src[ ( i*pixel_width + 1 ) ]
#define		addr_b( i )		p_src[ ( i*pixel_width + 0 ) ]

	// BGRx .... .... .... .... .... .... ....
	const __m128i	uyvy_0  = _mm_and_si128( _mm_add_epi16( load_b( addr_b(0) ), _mm_add_epi16( load_g( addr_g(0) ), load_r( addr_r(0) ) ) ), const_uyv0_0000 );
	// BGRx .... .... .... .... .... .... ....
	const __m128i	uyvy_01 = _mm_srai_epi16( _mm_add_epi16( uyvy_0, _mm_setzero_si128() ), 5 );

#undef		load_r	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_r + i*8 ) )
#undef		load_g	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_g + i*8 ) )
#undef		load_b	// _mm_loadl_epi64( (__m128i*)( g_bgr_to_uyvy.m_lut_b + i*8 ) )
#undef		addr_r	// ( i*pixel_wide + 2 )
#undef		addr_g	// ( i*pixel_wide + 1 )
#undef		addr_b	// ( i*pixel_wide + 0 )

	// Final results
	// BGRx BGRx BGRx BGRx BGRx BGRx BGRx BGRx 
	const __m128i	ret = _mm_packus_epi16( uyvy_01, uyvy_01 );
	return ret.m128i_u8[ 1 ];
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_bgr_u8* __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width )
{	// The size
	const int pixel_size = 3;	

	// Is aligned
	const bool	aligned = !( 15 & (size_t)p_dst );

	// The source width
	const pixel_bgr_u8* __restrict p_src_e = p_src + width;

	//*** 8 pixel blocks
	if ( aligned )
	{	for( ; p_src < p_src_e-7; p_src+=8, p_dst+=4 )
		{	// Color components
			_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
			_mm_store_si128( (__m128i*)p_dst, bgr_to_uyvy_x8<pixel_size>( (BYTE*)p_src ) );
		}
	}
	else
	{	for( ; p_src < p_src_e-7; p_src+=8, p_dst+=4 )
		{	// Color components
			_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
			_mm_storeu_si128( (__m128i*)p_dst, bgr_to_uyvy_x8<pixel_size>( (BYTE*)p_src ) );
		}
	}
		
	//*** 2 pixel block
	for( ; p_src < p_src_e-1; p_src+=2, p_dst++ )
		// Color components
		*(DWORD*)p_dst = _mm_cvtsi128_si32( bgr_to_uyvy_x2<pixel_size>( (BYTE*)p_src ) );

	//*** 1 pixel block
	if ( p_src < p_src_e )
		*(DWORD*)p_dst = _mm_cvtsi128_si32( bgr_to_uyvy_x1<pixel_size>( (BYTE*)p_src ) );
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_bgra_u8* __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width )
{	// The size
	const int pixel_size = 4;	

	// Is aligned
	const bool	aligned = !( 15 & (size_t)p_dst );

	// The source width
	const pixel_bgra_u8* __restrict p_src_e = p_src + width;

	//*** 8 pixel blocks
	if ( aligned )
	{	for( ; p_src < p_src_e-7; p_src+=8, p_dst+=4 )
		{	// Color components
			_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
			_mm_store_si128( (__m128i*)p_dst, bgr_to_uyvy_x8<pixel_size>( (BYTE*)p_src ) );
		}
	}
	else
	{	for( ; p_src < p_src_e-7; p_src+=8, p_dst+=4 )
		{	// Color components
			_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
			_mm_storeu_si128( (__m128i*)p_dst, bgr_to_uyvy_x8<pixel_size>( (BYTE*)p_src ) );
		}
	}
		
	//*** 2 pixel block
	for( ; p_src < p_src_e-1; p_src+=2, p_dst++ )
		// Color components
		*(DWORD*)p_dst = _mm_cvtsi128_si32( bgr_to_uyvy_x2<pixel_size>( (BYTE*)p_src ) );

	//*** 1 pixel block
	if ( p_src < p_src_e )
		*(DWORD*)p_dst = _mm_cvtsi128_si32( bgr_to_uyvy_x1<pixel_size>( (BYTE*)p_src ) );
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_y_u8* __restrict p_src, pixel_ycbcr_u8*  __restrict p_dst, const int width )
{	// Get the line pointers	
	const pixel_y_u8 *p_src_e = p_src + width;

	const __m128i _128_ = _mm_set1_epi32( 0x80808080 );

	//*** 8 pixel blocks
	if ( !( 15 & (size_t)p_src ) )
	{	for( ; p_src < p_src_e-15; p_src+=16, p_dst+=8 )
		{	// Color components
			_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );

			// Load in 16 luminances
			const __m128i	src_y = _mm_load_si128( (__m128i*)p_src );

			// And interleave with 128's and store 32 bytes worth
			_mm_store_si128( 0 + (__m128i*)p_dst, _mm_unpacklo_epi8( _128_, src_y ) );
			_mm_store_si128( 1 + (__m128i*)p_dst, _mm_unpackhi_epi8( _128_, src_y ) );
		}
	}
	else
	{	for( ; p_src < p_src_e-15; p_src+=16, p_dst+=8 )
		{	// Color components
			_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );

			// Load in 16 luminances
			const __m128i	src_y = _mm_loadu_si128( (__m128i*)p_src );

			// And interleave with 128's and store 32 bytes worth
			_mm_storeu_si128( 0 + (__m128i*)p_dst, _mm_unpacklo_epi8( _128_, src_y ) );
			_mm_storeu_si128( 1 + (__m128i*)p_dst, _mm_unpackhi_epi8( _128_, src_y ) );
		}
	}
	
	//*** 1 pixel block
	for( ; p_src < p_src_e; p_src+=2, p_dst++ )
	{	// Color components			
		p_dst->m_cr = p_dst->m_cb = 128;
		p_dst->m_y0 = p_src[0].m_y;
		p_dst->m_y1 = p_src[1].m_y;
	}
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_bgr_u8* __restrict p_src, pixel_y_u8* __restrict p_dst, const int width )
{	// The size
	const int pixel_size = 3;

	// The source width
	const pixel_bgr_u8* __restrict p_src_e = p_src + width;

	//*** 8 pixel blocks
	for( ; p_src < p_src_e-7; p_src+=8, p_dst+=8 )
	{	// Color components
		_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
		_mm_storel_epi64( (__m128i*)p_dst, bgr_to_y_x8<pixel_size>( (BYTE*)p_src ) );
	}	
		
	//*** 1 pixel block
	for( ; p_src < p_src_e; p_src++, p_dst++ )
		// Color components
		*(BYTE*)p_dst = bgr_to_y_x1<pixel_size>( (BYTE*)p_src );
}

template<> 
void FrameWork::Bitmaps::convert_line( const pixel_bgra_u8* __restrict p_src, pixel_y_u8* __restrict p_dst, const int width )
{	// The size
	const int pixel_size = 4;

	// The source width
	const pixel_bgra_u8* __restrict p_src_e = p_src + width;

	//*** 8 pixel blocks
	for( ; p_src < p_src_e-7; p_src+=8, p_dst+=8 )
	{	// Color components
		_mm_prefetch( 512 + (char*)p_src, _MM_HINT_NTA );
		_mm_storel_epi64( (__m128i*)p_dst, bgr_to_y_x8<pixel_size>( (BYTE*)p_src ) );
	}	
		
	//*** 1 pixel block
	for( ; p_src < p_src_e; p_src++, p_dst++ )
		// Color components
		*(BYTE*)p_dst = bgr_to_y_x1<pixel_size>( (BYTE*)p_src );
}