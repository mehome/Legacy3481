#pragma once

template<typename pixel_type> struct bitmap_422;

// Comments :
//	1)	Alignment is the maximum of the requested alignment, and the requested alignement of pixel_type. i.e. 
//		in most cases (even __m128i or composite classes of them) alignment can safely be ignored.

template< typename pixel_type >
struct bitmap
{		// Type definitions
		typedef typename pixel_type pixel_type;
	
		// Constructor
		__forceinline bitmap( const int xres, const int yres, const int align=16, const int stride=0 );
		__forceinline bitmap(       pixel_type* pImage, const int xres, const int yres, const int stride=0 );
		__forceinline bitmap( const pixel_type* pImage, const int xres, const int yres, const int stride=0 );
		__forceinline bitmap( void );

		template< typename pixel_type_from >
		__forceinline explicit bitmap( const bitmap<pixel_type_from>& from )		: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false )	  { resize( from.xres(), from.yres() ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		template< typename pixel_type_from >
		__forceinline explicit bitmap( const bitmap_422<pixel_type_from>& from )	: m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride_in_bytes( 0 ), m_owned( false ) { resize( from.xres(), from.yres() ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		// Specialized version
		__forceinline explicit bitmap( const bitmap& from );

		// Destructor
		__forceinline ~bitmap( void );

		// Resize
		bool resize( const int xres, const int yres, const int align=16, const int stride=0 ); 
		bool resize_in_bytes( const int xres, const int yres, const int align=16, const int stride=0 ); 

		// Reference an external image
		void reference(		  pixel_type* pImage, const int xres, const int yres, const int stride=0 );
		void reference(	const pixel_type* pImage, const int xres, const int yres, const int stride=0 );		
		void reference(		  bitmap& from );
		void reference( const bitmap& from );

		// Utility functions
		void reference_even_lines(	     bitmap& from );
		void reference_even_lines( const bitmap& from );

		void reference_odd_lines(	    bitmap& from );
		void reference_odd_lines( const bitmap& from );

		void reference_top_half(	   bitmap& from );
		void reference_top_half( const bitmap& from );

		void reference_bottom_half(	      bitmap& from );
		void reference_bottom_half( const bitmap& from );

		// Support for byte aligned stride
		void reference_in_bytes(		  pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes=0 );
		void reference_in_bytes(	const pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes=0 );

		// Clear
		void clear( void );

		// Is this bitmap empty
		const bool empty( void ) const;

		// Get the resolution
		__forceinline int xres( void ) const;
		__forceinline int xres_in_bytes( void ) const;
		__forceinline int yres( void ) const;

		// Are we the same size as another bitmap
		template< typename other_bitmap >
		__forceinline bool is_same_size_as( const other_bitmap &with ) const { return ((xres()==with.xres())&&(yres()==with.yres())); }

		// Do we follow a particular alignment (must be power of two)
		bool __forceinline is_aligned( const int alignment = 16 ) const;

		// Is the memory owned ?
		const bool is_owned( void ) const { return m_owned; }

		// Is this interleaved
		const bool is_interleaved( void ) const { return m_IsInterleaved; }
		void set_interleaved( bool IsInterleaved ) { m_IsInterleaved=IsInterleaved; }

		// Get the stride
		__forceinline int stride( void ) const;
		__forceinline int stride_in_bytes( void ) const;		

		// Get the total data size
		const size_t size( void ) const;

		// Copy from
		template< typename pixel_type_from >
		void operator=( const bitmap<pixel_type_from>& from )		{ if ( !m_pData ) resize( from.xres(), from.yres() ); assert( is_same_size_as( from ) ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		template< typename pixel_type_from >
		void operator=( const bitmap_422<pixel_type_from>& from )	{ if ( !m_pData ) resize( from.xres(), from.yres() ); assert( is_same_size_as( from ) ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		void operator=( const bitmap& from );

		// Set
		void operator=( const pixel_type& x );

		// Swap with
		__forceinline void swap_with( bitmap& other );
		__forceinline void swap_to( bitmap& other );

		// Get pixel locations
		__forceinline pixel_type&		operator() ( const int x, const int y );
		__forceinline const pixel_type&	operator() ( const int x, const int y ) const;

		// Get the line pointers
		__forceinline pixel_type*		operator() ( const int y );
		__forceinline const pixel_type*	operator() ( const int y ) const;

		// Get the image pointers
		__forceinline pixel_type*		operator() ( void );
		__forceinline const pixel_type*	operator() ( void ) const;

private:// Data
		pixel_type *m_pData;

		// The resolution
		int m_xres, m_yres, m_stride_in_bytes;
		// For now we'll tag these frames, but if we integrate fc3 and pass message video I'll take this back out
		//  [12/11/2012 James]
		bool m_IsInterleaved; 

		// Is the memory owned ?
		bool m_owned;
};


//struct pixel_bgr_u8		{ typedef unsigned char value_type;		value_type m_b, m_g, m_r; };
//typedef bitmap<pixel_bgr_u8>			bitmap_bgr_u8;