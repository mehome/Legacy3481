#pragma once

#if		FBMP_USE_INTERNAL_MEMOP

// Local memcpy implementation
void  memcpy( void* __restrict p_dst, const void* __restrict p_src, const int size );
void  memset( void* __restrict p_dst, const unsigned char v, const int size );

// Src aligned, Dst = aligned
void  memcpy_a_a( void* __restrict p_dst, const void* __restrict p_src, const int size );
// Src aligned, Dst = unaligned
void  memcpy_a_u( void* __restrict p_dst, const void* __restrict p_src, const int size );
// Src aligned, Dst = aligned
void  memcpy_u_a( void* __restrict p_dst, const void* __restrict p_src, const int size );
// Src aligned, Dst = unaligned
void  memcpy_u_u( void* __restrict p_dst, const void* __restrict p_src, const int size );

// This uses streaming stores, and is useful for the GPU. As a general copy behavior this might sometimes be
// better, while not at other times.
void  memcpy_stream( void* __restrict p_dst, const void* __restrict p_src, const int size );

#else	FBMP_USE_INTERNAL_MEMOP

__forceinline void memcpy		( void* p_dst, const void* p_src, const int size ) { ::memcpy( p_dst, p_src, size ); }
__forceinline void memcpy_a_a	( void* p_dst, const void* p_src, const int size ) { ::memcpy( p_dst, p_src, size ); }
__forceinline void memcpy_a_u	( void* p_dst, const void* p_src, const int size ) { ::memcpy( p_dst, p_src, size ); }
__forceinline void memcpy_u_a	( void* p_dst, const void* p_src, const int size ) { ::memcpy( p_dst, p_src, size ); }
__forceinline void memcpy_u_u	( void* p_dst, const void* p_src, const int size ) { ::memcpy( p_dst, p_src, size ); }
__forceinline void memcpy_stream( void* p_dst, const void* p_src, const int size ) { ::memcpy( p_dst, p_src, size ); }

__forceinline void memset( void* __restrict p_dst, const unsigned char v, const int size ) { ::memset( p_dst, v, size ); }

#endif	FBMP_USE_INTERNAL_MEMOP

// Because the = operator will always non stream, this version will stream, with slightly fancy end of line support.
template< typename type1, typename type2 > __forceinline
void stream_copy( type1 &to, const type2 &from )
{	// Debugging
	assert( from.yres() == to.yres() );
	assert( from.xres_in_bytes() == to.xres_in_bytes() );

	// Copy in one or more pieces
	if ( ( from.stride_in_bytes() == to.stride_in_bytes() ) && ( from.xres_in_bytes() <= from.stride_in_bytes() ) )
	{	// Copy in one chunk
		memcpy_stream( to(), from(), to.stride_in_bytes() * to.yres() );
	}
	else
	{	// Cycle top to bottom
		for( int y=0; y<from.yres(); y++ )
			memcpy_stream( to( y ), from( y ), from.xres_in_bytes() );
	}
}

// Convert an image without alpha channel
template< const typename src_type, const typename dst_type >
void convert_line( const src_type* __restrict p_src, dst_type* __restrict p_dst, const int width );

// Implementation to copy a straight image
template<const typename src_type> 
__forceinline void convert_line( const src_type* __restrict p_src, src_type* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, width*sizeof(src_type) ); }								// Optimized

// 422 lines need to be sprcialized to ensure that the resolution is supported right
template<> __forceinline void convert_line( const pixel_ycbcr_u8 * __restrict p_src, pixel_ycbcr_u8 * __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_s8 * __restrict p_src, pixel_ycbcr_s8 * __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_u16* __restrict p_src, pixel_ycbcr_u16* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_s16* __restrict p_src, pixel_ycbcr_s16* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_u32* __restrict p_src, pixel_ycbcr_u32* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_s32* __restrict p_src, pixel_ycbcr_s32* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_f32* __restrict p_src, pixel_ycbcr_f32* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized
template<> __forceinline void convert_line( const pixel_ycbcr_f64* __restrict p_src, pixel_ycbcr_f64* __restrict p_dst, const int width ) { memcpy( p_dst, p_src, (width/2)*sizeof(p_src[0]) ); }	// Optimized

template<> __forceinline void convert_line( const pixel_ycbcr_u8 * __restrict p_src, pixel_a_u8 * __restrict p_dst, const int width ) { memset( p_dst, 255, width ); }								// Optimized
template<> __forceinline void convert_line( const pixel_bgr_u8*    __restrict p_src, pixel_a_u8 * __restrict p_dst, const int width ) { memset( p_dst, 255, width ); }								// Optimized
template<> __forceinline void convert_line( const pixel_y_u8*      __restrict p_src, pixel_a_u8 * __restrict p_dst, const int width ) { memset( p_dst, 255, width ); }								// Optimized

// from YUV color conversion
template<> void  convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_bgr_u8*   __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_ycbcr_u8* __restrict p_src, pixel_y_u8*     __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_y_u8*     __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width );		// Optimized

// to YUV color conversion
template<> void  convert_line( const pixel_bgra_u8*  __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_bgr_u8*   __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_y_u8*     __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_bgr_u8*   __restrict p_src, pixel_y_u8*     __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_bgra_u8*  __restrict p_src, pixel_y_u8*     __restrict p_dst, const int width );		// Optimized

// BGRA to A
template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_a_u8*  __restrict p_dst, const int width );			// Optimized
// BGRA to BGR
template<> void  convert_line( const pixel_bgra_u8*  __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_y_u8*     __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width );		// Optimized
template<> void  convert_line( const pixel_bgra_u16* __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width );		// Optimized

// Depth conversions
template<> void  convert_line( const WORD*  __restrict p_src, BYTE* __restrict p_dst, const int width );						// Optimized
template<> void  convert_line( const float*	__restrict p_src, WORD* __restrict p_dst, const int width );						// Nor Optimized
template<> void  convert_line( const float*	__restrict p_src, BYTE* __restrict p_dst, const int width );						// Optimized
template<> void  convert_line( const float*	__restrict p_src, int* __restrict p_dst, const int width );							// Optimized
template<> void  convert_line( const float*	__restrict p_src, short* __restrict p_dst, const int width );						// Optimized
template<> void  convert_line( const short*	__restrict p_src, float* __restrict p_dst, const int width );						// Optimized
template<> void  convert_line( const int*	__restrict p_src, float* __restrict p_dst, const int width );						// Optimized

template<> __forceinline void convert_line( const pixel_bgra_u16* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width ) { convert_line<WORD ,BYTE>( (WORD *)p_src, (BYTE*)p_dst, width*4 ); }
template<> __forceinline void convert_line( const pixel_bgra_f32* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width ) { convert_line<float,BYTE>( (float*)p_src, (BYTE*)p_dst, width*4 ); }
template<> __forceinline void convert_line( const pixel_bgra_f32* __restrict p_src, pixel_bgra_u16* __restrict p_dst, const int width ) { convert_line<float,WORD>( (float*)p_src, (WORD*)p_dst, width*4 ); }
template<> __forceinline void convert_line( const pixel_bgr_u16*  __restrict p_src, pixel_bgr_u8*   __restrict p_dst, const int width ) { convert_line<WORD ,BYTE>( (WORD *)p_src, (BYTE*)p_dst, width*3 ); }
template<> __forceinline void convert_line( const pixel_bgr_f32*  __restrict p_src, pixel_bgr_u8*   __restrict p_dst, const int width ) { convert_line<float,BYTE>( (float*)p_src, (BYTE*)p_dst, width*3 ); }

// BGR to BGRA
template<> void  convert_line( const pixel_bgr_u8*  __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width );		// Decently fast
template<> void  convert_line( const pixel_bgr_u16* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width );		// Not optimized
template<> void  convert_line( const pixel_bgr_f32* __restrict p_src, pixel_bgra_u8*  __restrict p_dst, const int width );		// Not optimized

// Place holders
template<> void  convert_line( const pixel_bgr_u16*  __restrict p_src, pixel_y_u8* __restrict p_dst, const int width );
template<> void  convert_line( const pixel_bgra_u16* __restrict p_src, pixel_y_u8* __restrict p_dst, const int width );
template<> void  convert_line( const pixel_bgr_f32*  __restrict p_src, pixel_y_u8* __restrict p_dst, const int width );
template<> void  convert_line( const pixel_bgra_f32* __restrict p_src, pixel_y_u8* __restrict p_dst, const int width );

template<> void  convert_line( const pixel_bgra_f32* __restrict p_src, pixel_y_f32* __restrict p_dst, const int width );		// Decently fast

// YCbCrA to BGRA
template<> void  convert_line( const pixel_ycbcra_u8* __restrict p_src, pixel_bgra_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_ycbcra_u8* __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_crcbya_u8* __restrict p_src, pixel_bgra_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_crcbya_u8* __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_crcby_u8* __restrict p_src, pixel_bgra_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_crcby_u8* __restrict p_src, pixel_bgr_u8*  __restrict p_dst, const int width );		// Not optimized, but integer.

// BGRA to YCbCrA
template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_ycbcra_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgr_u8*  __restrict p_src, pixel_ycbcra_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgr_u8*  __restrict p_src, pixel_ycbcr_u8* __restrict p_dst, const int width );		// Not optimized, but integer.

template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_crcbya_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgr_u8*  __restrict p_src, pixel_crcbya_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_crcby_u8* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgr_u8*  __restrict p_src, pixel_crcby_u8* __restrict p_dst, const int width );		// Not optimized, but integer.

// 
template<> void  convert_line( const pixel_bgr_u8*  __restrict p_src, pixel_bgr_f32* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgr_u16* __restrict p_src, pixel_bgr_f32* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_bgr_f32* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgra_u16* __restrict p_src, pixel_bgr_f32* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_bgra_f32* __restrict p_src, pixel_bgr_f32* __restrict p_dst, const int width );		// Not optimized, but integer.
template<> void  convert_line( const pixel_y_u8* __restrict p_src, pixel_bgr_f32* __restrict p_dst, const int width );			// Not optimized, but integer.

template<> void  convert_line( const pixel_bgra_u8* __restrict p_src, pixel_bgra_f32* __restrict p_dst, const int width );		// Not optimized
template<> void  convert_line( const pixel_bgr_u8* __restrict p_src, pixel_bgra_f32* __restrict p_dst, const int width );		// Not optimized
template<> void  convert_line( const pixel_bgr_u16* __restrict p_src, pixel_bgra_f32* __restrict p_dst, const int width );		// Not optimized
template<> void  convert_line( const pixel_bgra_u16* __restrict p_src, pixel_bgra_f32* __restrict p_dst, const int width );		// Not optimized
template<> void  convert_line( const pixel_bgr_f32* __restrict p_src, pixel_bgra_f32* __restrict p_dst, const int width );		// Not optimized
template<> void  convert_line( const pixel_y_u8* __restrict p_src, pixel_bgra_f32* __restrict p_dst, const int width );			// Not optimized

template<> void  convert_line( const pixel_bgra_f32* __restrict p_src, pixel_ycbcra_f32* __restrict p_dst, const int width );	// Not optimized

template<> void  convert_line( const pixel_bgra_f32* __restrict p_src, pixel_bgr_u8* __restrict p_dst, const int width );		// Not optimized