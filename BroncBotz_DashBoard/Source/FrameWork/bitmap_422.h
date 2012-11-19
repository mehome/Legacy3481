#pragma once

// Comments :
//	1)	Alignment is the maximum of the requested alignment, and the requested alignment of pixel_type. i.e. 
//		in most cases (even __m128i or composite classes of them) alignment can safely be ignored.

template< typename pixel_type >
struct bitmap_422
{		// Local type definition
		typedef typename pixel_type	pixel_type;
	
		// Constructor
		__forceinline bitmap_422( const int xres, const int yres, const int align=16, const int stride=0 );
		__forceinline bitmap_422(	    pixel_type* pImage, const int xres, const int yres, const int stride=0 );
		__forceinline bitmap_422( const pixel_type* pImage, const int xres, const int yres, const int stride=0 );
		__forceinline bitmap_422( void );

		//template< typename pixel_type_from >
		//__forceinline explicit bitmap_422( const bitmap<pixel_type_from>& from ) : m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride( 0 ), m_owned( false )		{ resize( from.xres(), from.yres() ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		template< typename pixel_type_from >
		__forceinline explicit bitmap_422( const bitmap_422<pixel_type_from>& from ) : m_pData( NULL ), m_xres( 0 ), m_yres( 0 ) , m_stride( 0 ), m_owned( false )	{ resize( from.xres(), from.yres() ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		// Specialized version
		__forceinline explicit bitmap_422( const bitmap_422& from );

		// Destructor
		__forceinline ~bitmap_422( void );

		// Resize
		bool resize( const int xres, const int yres, const int align=16, const int stride=0 ); 

		// Reference an external image
		void reference(		  pixel_type* pImage, const int xres, const int yres, const int stride=0 );
		void reference( const pixel_type* pImage, const int xres, const int yres, const int stride=0 );		

		// Utility functions
		void reference_even_lines(	     bitmap_422& from );
		void reference_even_lines( const bitmap_422& from );

		void reference_odd_lines(	    bitmap_422& from );
		void reference_odd_lines( const bitmap_422& from );

		void reference_top_half(	   bitmap_422& from );
		void reference_top_half( const bitmap_422& from );

		void reference_bottom_half(	      bitmap_422& from );
		void reference_bottom_half( const bitmap_422& from );

		// Reference with byte-wise stride support.
		void reference_in_bytes(	   pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes=0 );
		void reference_in_bytes( const pixel_type* pImage, const int xres, const int yres, const int stride_in_bytes=0 );

		void reference(       bitmap_422& from );
		void reference( const bitmap_422& from );

		// Clear
		void clear( void );

		// Is this bitmap empty
		const bool empty( void ) const;

		// Get the resolution
		__forceinline int xres( void ) const;
		__forceinline int xres_in_bytes( void ) const;
		__forceinline int yres( void ) const;

		// Get the bitmap size in bytes
		const size_t size( void ) const;

		// Are we the same size as another bitmap
		template< typename other_bitmap >
		__forceinline bool is_same_size_as( const other_bitmap &with ) const { return ((xres()==with.xres())&&(yres()==with.yres())); }

		// Do we follow a particular alignment (must be power of two)
		bool __forceinline is_aligned( const int alignment = 16 ) const;

		// Is the memory owned ?
		const bool is_owned( void ) const { return m_owned; }

		// Get the stride
		__forceinline int stride( void ) const;
		__forceinline int stride_in_bytes( void ) const;

		// Copy from
		template< typename pixel_type_from >
		void operator=( const bitmap<pixel_type_from>& from )		{ if ( !m_pData ) resize( from.xres(), from.yres() ); assert( is_same_size_as( from ) ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		template< typename pixel_type_from >
		void operator=( const bitmap_422<pixel_type_from>& from )	{ if ( !m_pData ) resize( from.xres(), from.yres() ); assert( is_same_size_as( from ) ); for( int y=0; y<yres(); y++ ) convert_line<pixel_type_from,pixel_type>( from( y ), (*this)( y ), xres() ); }

		void operator=( const bitmap_422& from );

		// Set
		void operator=( const pixel_type& x );

		// Swap with
		__forceinline void swap_with( bitmap_422& other );
		__forceinline void swap_to( bitmap_422& other );

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

		// Is the memory owned ?
		bool m_owned;		
};

//struct pixel_ycbcr_u8	{ typedef unsigned char value_type;		union { struct { value_type m_cb, m_y0, m_cr, m_y1; }; DWORD m_cb_y0_cr_y1; }; };
////bitmap types
//typedef bitmap_422<pixel_ycbcr_u8>		bitmap_ycbcr_u8;
