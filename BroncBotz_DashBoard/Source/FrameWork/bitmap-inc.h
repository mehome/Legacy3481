#pragma once

// Constructor
template< typename pixel_type > __forceinline
bitmap<pixel_type>::bitmap( const int xres, const int yres, const int align, const int stride )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	// Allocate
	resize( xres, yres, align, stride );
}

template< typename pixel_type > __forceinline
bitmap<pixel_type>::bitmap( pixel_type* pImage, const int xres, const int yres, const int stride )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	// Allocate
	reference( pImage, xres, yres, stride );
}

template< typename pixel_type > __forceinline
bitmap<pixel_type>::bitmap( const pixel_type* pImage, const int xres, const int yres, const int stride )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	// Allocate
	reference( pImage, xres, yres, stride );
}

template< typename pixel_type > __forceinline
bitmap<pixel_type>::bitmap( void )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{
}

template< typename pixel_type > __forceinline
bitmap<pixel_type>::bitmap( const bitmap& from )
	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )
{	resize( from.xres(), from.yres() );
	operator= ( from );
}

// Do we follow a particular alignment (must be power of two)
template< typename pixel_type > __forceinline
bool bitmap<pixel_type>::is_aligned( const int alignment ) const
{	const size_t addr_align   = (size_t)( (*this)() ) & (alignment-1);
	const size_t stride_align = abs( stride_in_bytes() ) & (alignment-1);
	return ( addr_align | stride_align ) ? false : true;
}

template< typename pixel_type > __forceinline
void bitmap<pixel_type>::swap_with( bitmap& other )
{	std::swap( m_pData, other.m_pData );
	std::swap( m_xres, other.m_xres );
	std::swap( m_yres, other.m_yres );
	std::swap( m_stride_in_bytes, other.m_stride_in_bytes );
	std::swap( m_owned, other.m_owned );
}

template< typename pixel_type > __forceinline
void bitmap<pixel_type>::swap_to( bitmap& other )
{	swap_with( other );
}

// Destructor
template< typename pixel_type > __forceinline
bitmap<pixel_type>::~bitmap( void )
{	// Free
	clear();
}

// Is this buffer empty
template< typename pixel_type > __forceinline
const bool bitmap<pixel_type>::empty( void ) const
{
	return ( xres() | yres() ) ? false : true;
}

template< typename pixel_type > __forceinline
void bitmap<pixel_type>::operator=( const bitmap& from )
{	// Resize
	if ( !m_pData ) resize( from.xres(), from.yres() ); 
	assert( is_same_size_as( from ) ); 

	if ( ( stride_in_bytes() >= xres_in_bytes() ) && ( stride_in_bytes() == from.stride_in_bytes() ) )
	{	// A single, large copy is possible.
		memcpy( (*this)(), from(), stride_in_bytes()*yres() );
	}
	else
	{	// A line by line copy is going to be needed.
		for( int y=0; y<yres(); y++ ) 
			memcpy( (*this)( y ), from( y ), xres_in_bytes() );
	}
}

template< typename pixel_type > __forceinline
void bitmap<pixel_type>::operator=( const pixel_type& val )
{	for( int y=0; y<yres(); y++ ) 
	{	pixel_type *p_line = (*this)( y ); 
		for( int x=0; x<xres(); x++ ) 
			p_line[ x ] = val; 
	} 
}

// Resize
template< typename pixel_type >
bool bitmap<pixel_type>::resize_in_bytes( const int xres, const int yres, const int align, const int stride_in_bytes )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// Get the real alignment
	const int align_to_use = ( __alignof(pixel_type) > align ) ? __alignof(pixel_type) : align;

	// Check whether a change is needed
	if ( ( m_xres == xres ) && ( m_yres == yres ) && 
		 ( (((size_t)m_pData)&(align_to_use-1)) == 0 ) && 
		 ( (stride_in_bytes==0) || (m_stride_in_bytes==stride_in_bytes) ) && 
		 ( m_owned ) )
		return true;

	// First clear
	clear();

	// No memory to allocate ?
	if ( (!xres) || (!yres) ) return true;

	// This is a precondition
	assert( stride_in_bytes >= 0 );
#ifdef max // GAH, windows.h sucks in C++ conformance
	m_stride_in_bytes = max( (int)( xres*sizeof(pixel_type) ), (int)stride_in_bytes );
#else
	m_stride_in_bytes = std::max( (int)( xres*sizeof(pixel_type) ), (int)stride_in_bytes );
#endif

	// Round to the stride size
	m_stride_in_bytes = ( ( m_stride_in_bytes + align_to_use - 1 ) / align_to_use ) * align_to_use;

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

// Resize
template< typename pixel_type >
bool bitmap<pixel_type>::resize( const int xres, const int yres, const int align, const int stride )
{	return resize_in_bytes( xres, yres, align, stride*sizeof( pixel_type ) );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference( pixel_type* pImage, const int xres, const int yres, const int stride )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = ( stride ? stride : xres ) * sizeof( pixel_type );
	m_owned = false;
}

template< typename pixel_type >
void bitmap<pixel_type>::reference( const pixel_type* pImage, const int xres, const int yres, const int stride )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = (pixel_type*)pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = ( stride ? stride : xres ) * sizeof( pixel_type );
	m_owned = false;
}


// Utility functions
template< typename pixel_type >
void bitmap<pixel_type>::reference_even_lines(	     bitmap& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_even_lines( const bitmap& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_odd_lines(	    bitmap& from )
{	reference_in_bytes( from( 1 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_odd_lines( const bitmap& from )
{	reference_in_bytes( from( 1 ), from.xres(), from.yres()/2, from.stride_in_bytes()*2 );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_top_half(	   bitmap& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_top_half( const bitmap& from )
{	reference_in_bytes( from( 0 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_bottom_half(	      bitmap& from )
{	reference_in_bytes( from( from.yres()/2 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_bottom_half( const bitmap& from )
{	reference_in_bytes( from( from.yres()/2 ), from.xres(), from.yres()/2, from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_in_bytes( pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = stride_in_bytes ? stride_in_bytes : ( xres * sizeof( pixel_type ) );
	m_owned = false;
}

template< typename pixel_type >
void bitmap<pixel_type>::reference_in_bytes( const pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes )
{	// Obvious sanity check
	assert( ( xres >= 0 ) && ( yres >= 0 ) );
	
	// First clear
	clear();

	// Now allocate
	m_pData = (pixel_type*)pImage;

	// Store the rest
	m_xres = xres;
	m_yres = yres;
	m_stride_in_bytes = stride_in_bytes ? stride_in_bytes : ( xres * sizeof( pixel_type ) );
	m_owned = false;
}

template< typename pixel_type >
void bitmap<pixel_type>::reference( const bitmap &from )
{	reference_in_bytes( from(), from.xres(), from.yres(), from.stride_in_bytes() );
}

template< typename pixel_type >
void bitmap<pixel_type>::reference( bitmap &from )
{	reference_in_bytes( from(), from.xres(), from.yres(), from.stride_in_bytes() );
}

// Clear
template< typename pixel_type >
void bitmap<pixel_type>::clear( void )
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
int bitmap<pixel_type>::xres( void ) const
{	return m_xres;
}

template< typename pixel_type > __forceinline
int bitmap<pixel_type>::xres_in_bytes( void ) const
{	return m_xres*sizeof(pixel_type);
}

template< typename pixel_type > __forceinline
int bitmap<pixel_type>::yres( void ) const
{	return m_yres;
}

template< typename pixel_type > __forceinline
int bitmap<pixel_type>::stride( void ) const
{	return m_stride_in_bytes / sizeof( pixel_type );
}

template< typename pixel_type > __forceinline
int bitmap<pixel_type>::stride_in_bytes( void ) const
{	return m_stride_in_bytes;
}

// Get pixel locations
template< typename pixel_type > __forceinline
pixel_type&			bitmap<pixel_type>::operator() ( const int x, const int y )
{	// Debugging
	assert( ( x>=0 ) && ( x<m_xres ) && ( y>=0 ) && ( y<m_yres ) );
	assert( m_pData );

	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x ];
}

template< typename pixel_type > __forceinline
const pixel_type&	bitmap<pixel_type>::operator() ( const int x, const int y ) const
{	// Debugging
	assert( ( x>=0 ) && ( x<m_xres ) && ( y>=0 ) && ( y<m_yres ) );
	assert( m_pData );

	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x ];
}

// Get the line pointers
template< typename pixel_type > __forceinline
pixel_type*			bitmap<pixel_type>::operator() ( const int y )
{	// Debugging
	assert( ( y>=0 ) && ( y<m_yres ) );
	assert( m_pData );

	//assert( !m_const );
	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) );
}

template< typename pixel_type > __forceinline
const pixel_type*	bitmap<pixel_type>::operator() ( const int y ) const
{	// Debugging
	assert( ( y>=0 ) && ( y<m_yres ) );
	assert( m_pData );

	//assert( m_pData );
	return ( (pixel_type*)( (BYTE*)m_pData + y*m_stride_in_bytes ) );
}

// Get the image pointers
template< typename pixel_type > __forceinline
pixel_type*			bitmap<pixel_type>::operator() ( void )
{	// Debugging
	return m_pData;
}

template< typename pixel_type > __forceinline
const pixel_type*	bitmap<pixel_type>::operator() ( void ) const
{	// Debugging
	return m_pData;
}

// Get the total data size
template< typename pixel_type > __forceinline
const size_t bitmap<pixel_type>::size( void ) const
{	return xres()*yres()*sizeof(pixel_type);
}