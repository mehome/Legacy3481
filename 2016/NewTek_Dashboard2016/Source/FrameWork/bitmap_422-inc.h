#pragma once

// Constructor
template< typename pixel_type > __forceinline
bitmap_422<pixel_type>::bitmap_422( const int xres, const int yres, const int align, const int stride )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	// Allocate
	resize( xres, yres, align, stride );
}

template< typename pixel_type > __forceinline
bitmap_422<pixel_type>::bitmap_422( pixel_type* pImage, const int xres, const int yres, const int stride )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	// Allocate
	reference( pImage, xres, yres, stride );
}

template< typename pixel_type > __forceinline
bitmap_422<pixel_type>::bitmap_422( const pixel_type* pImage, const int xres, const int yres, const int stride )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	// Allocate
	reference( pImage, xres, yres, stride );
}

template< typename pixel_type > __forceinline
bitmap_422<pixel_type>::bitmap_422( const bitmap_422& from )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	resize( from.xres(), from.yres() );
	operator= ( from );
}

template< typename pixel_type > __forceinline
bitmap_422<pixel_type>::bitmap_422( void )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{
}

// Is this buffer empty
template< typename pixel_type > __forceinline
const bool bitmap_422<pixel_type>::empty( void ) const
{
	return ( xres() | yres() ) ? false : true;
}

template< typename pixel_type > __forceinline
void bitmap_422<pixel_type>::operator=( const bitmap_422& from )
{	if ( !m_pData ) resize( from.xres(), from.yres() ); 
	assert( is_same_size_as( from ) ); 

	if ( ( stride_in_bytes() >= xres_in_bytes() ) && ( stride_in_bytes() == from.stride_in_bytes() ) )
	{	// A single, large copy is possible.
		memcpy( (*this)(), from(), stride_in_bytes()*yres() );
	}
	else
	{	// Line by line copy
		for( int y=0; y<yres(); y++ ) 
			memcpy( (*this)( y ), from( y ), xres_in_bytes() );
	}
}

template< typename pixel_type > __forceinline
void bitmap_422<pixel_type>::operator=( const pixel_type& val )		
{	for( int y=0; y<yres(); y++ ) 
	{	pixel_type *p_line = (*this)( y ); 
		for( int x=0; x<xres()/2; x++ ) 
			p_line[ x ] = val; 
	} 
}

template< typename pixel_type > __forceinline
void bitmap_422<pixel_type>::swap_with( bitmap_422& other )
{	std::swap( m_pData, other.m_pData );
	std::swap( m_xres, other.m_xres );
	std::swap( m_yres, other.m_yres );
	std::swap( m_stride_in_bytes, other.m_stride_in_bytes );
	std::swap( m_owned, other.m_owned );
}

template< typename pixel_type > __forceinline
void bitmap_422<pixel_type>::swap_to( bitmap_422& other )
{	swap_with( other );
}

// Destructor
template< typename pixel_type > __forceinline
bitmap_422<pixel_type>::~bitmap_422( void )
{	// Free
	clear();
}

// Do we follow a particular alignment (must be power of two)
template< typename pixel_type > __forceinline
bool bitmap_422<pixel_type>::is_aligned( const int alignment ) const
{	const size_t addr_align   = (size_t)( (*this)() ) & (alignment-1);
	const size_t stride_align = abs( stride_in_bytes() ) & (alignment-1);
	return ( addr_align | stride_align ) ? false : true;
}

// Resize
template< typename pixel_type >
bool bitmap_422<pixel_type>::resize( const int xres, const int yres, const int align, const int stride )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// Get the real alignment
	const int align_to_use = ( __alignof(pixel_type) > align ) ? __alignof(pixel_type) : align;

	// Check whether a change is needed
	if ( ( m_xres == xres ) && ( m_yres == yres ) && 
		 ( (((size_t)m_pData)&((size_t)align_to_use-1)) == 0 ) && 
		 ( (stride==0) || ((int)m_stride_in_bytes==(int)stride*(int)sizeof(pixel_type)) ) && 
		 ( m_owned ) )
		return true;

	// First clear
	clear();

	// No memory to allocate ?
	if ( (!xres) || (!yres) ) return true;

	// This is a requirement for now !
	assert( stride>=0 );
	int l_stride = (xres+1)/2;
	if ( stride > l_stride ) l_stride = stride;	

	// Get the stride in bytes
	m_stride_in_bytes = sizeof(pixel_type)*l_stride;

	// Round the stride to the alignment
	m_stride_in_bytes = ( m_stride_in_bytes + align_to_use - 1 ) & ( ~( align_to_use - 1 ) );

	// Now allocate
	m_pData = (pixel_type*)mem::alloc( m_stride_in_bytes*yres, 16/*align_to_use*/ );
	if (!m_pData) return false;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_owned = true;

	// Success
	return true;
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference( pixel_type* pImage, const int xres, const int yres, const int stride )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = ( stride ? stride : ((xres+1)/2) ) * sizeof( pixel_type );
	m_owned = false;
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference( const pixel_type* pImage, const int xres, const int yres, const int stride )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = (pixel_type*)pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = ( stride ? stride : ((xres+1)/2) ) * sizeof( pixel_type );
	m_owned = false;
}

// Utility functions
template< typename pixel_type >
void bitmap_422<pixel_type>::reference_even_lines(	     bitmap_422& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_even_lines( const bitmap_422& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_odd_lines(	    bitmap_422& from )
{	reference_in_bytes( from( 1 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_odd_lines( const bitmap_422& from )
{	reference_in_bytes( from( 1 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_top_half(	   bitmap_422& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_top_half( const bitmap_422& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_bottom_half(	      bitmap_422& from )
{	reference_in_bytes( from( from.yres()/2 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_bottom_half( const bitmap_422& from )
{	reference_in_bytes( from( from.yres()/2 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_in_bytes( pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = stride_in_bytes ? stride_in_bytes : ( ((xres+1)/2) * sizeof( pixel_type ) );
	m_owned = false;
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference_in_bytes( const pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = (pixel_type*)pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = stride_in_bytes ? stride_in_bytes : ( ((xres+1)/2) * sizeof( pixel_type ) );
	m_owned = false;
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference( bitmap_422 &from )
{
	// 
	reference( from(), from.xres(), from.yres(), from.stride() );
}

template< typename pixel_type >
void bitmap_422<pixel_type>::reference( const bitmap_422 &from )
{
	// 
	reference( from(), from.xres(), from.yres(), from.stride() );
}

// Clear
template< typename pixel_type >
void bitmap_422<pixel_type>::clear( void )
{	// Fast exit
	if (!m_pData) return;

	// Free memory
	if ( (m_owned) && (m_pData) )
		mem::free( m_pData );

	// Nothing stored
	m_xres = 0;
	m_yres = 0;
	m_stride_in_bytes = 0;
	m_owned = false;
	m_pData = NULL;
}

// Get the resolution
template< typename pixel_type > __forceinline
int bitmap_422<pixel_type>::xres( void ) const
{	return m_xres;
}

template< typename pixel_type > __forceinline
int bitmap_422<pixel_type>::xres_in_bytes( void ) const
{	return ((m_xres+1)/2)*sizeof(pixel_type);
}

template< typename pixel_type > __forceinline
int bitmap_422<pixel_type>::yres( void ) const
{	return m_yres;
}

template< typename pixel_type > __forceinline
int bitmap_422<pixel_type>::stride( void ) const
{	return m_stride_in_bytes / sizeof( pixel_type );
}

template< typename pixel_type > __forceinline
int bitmap_422<pixel_type>::stride_in_bytes( void ) const
{	return m_stride_in_bytes;
}

// Get pixel locations
template< typename pixel_type > __forceinline
pixel_type&	bitmap_422<pixel_type>::operator() ( const int x, const int y )
{	// Debugging
	assert( ( x>=0 ) && ( x<m_xres ) && ( y>=0 ) && ( y<m_yres ) );
	assert( !(x&1) );
	assert( m_pData );
	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x/2 ];
}

template< typename pixel_type > __forceinline
const pixel_type&	bitmap_422<pixel_type>::operator() ( const int x, const int y ) const
{	// Debugging
	assert( ( x>=0 ) && ( x<m_xres ) && ( y>=0 ) && ( y<m_yres ) );
	assert( !(x&1) );
	assert( m_pData );
	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x/2 ];
}

// Get the line pointers
template< typename pixel_type > __forceinline
pixel_type*			bitmap_422<pixel_type>::operator() ( const int y )
{	// Debugging
	assert( ( y>=0 ) && ( y<m_yres ) );
	assert( m_pData );
	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) );
}

template< typename pixel_type > __forceinline
const pixel_type*	bitmap_422<pixel_type>::operator() ( const int y ) const
{	// Debugging
	assert( ( y>=0 ) && ( y<m_yres ) );
	assert( m_pData );
	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) );
}

// Get the image pointers
template< typename pixel_type > __forceinline
pixel_type*			bitmap_422<pixel_type>::operator() ( void )
{	// Debugging
	return m_pData;
}

template< typename pixel_type > __forceinline
const pixel_type*	bitmap_422<pixel_type>::operator() ( void ) const
{	// Debugging
	return m_pData;
}

// Get the total data size
template< typename pixel_type > __forceinline
const size_t bitmap_422<pixel_type>::size( void ) const
{	return (xres()/2)*yres()*sizeof(pixel_type);
}