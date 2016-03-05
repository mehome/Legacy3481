#include "StdAfx.h"
#include "FrameWork.h"

using namespace FrameWork::Bitmaps;

//**********************************************************************************************************
template< typename draw_type >
struct rasterize_line
{			// constructor
			rasterize_line( draw_type& draw, const int posnA[], const int posnB[] );

private:	// Draw along the X direction
			void draw_x( const int posnA[], const int posnB[] );
			void draw_y( const int posnA[], const int posnB[] );
	
			// Draw implementation
			draw_type&	m_draw;
};

// constructor
template< typename draw_type >
rasterize_line<draw_type>::rasterize_line( draw_type& draw, const int posnA[], const int posnB[] )
	:	m_draw( draw )
{	// Draw along X or Y directions
	const int dx = posnB[0] - posnA[0];
	const int dy = posnB[1] - posnA[1];

	// X or Y direction
	if (abs(dx)>abs(dy))
	{	if (dx>0)	draw_x( posnA, posnB );
		else		draw_x( posnB, posnA );
	}
	else
	{	if (dy>0)	draw_y( posnA, posnB );
		else		
		{	if ( !dx && !dy ) return;
			draw_y( posnB, posnA );
		}
	}
}

// Draw along the X direction
template< typename draw_type >
void rasterize_line<draw_type>::draw_x( const int posnA[], const int posnB[] )
{	// Get the resolution
	const int xres = m_draw.xres();
	const int yres = m_draw.yres();

	// Get the gradient
	const int dx = posnB[0] - posnA[0];
	const int dy = posnB[1] - posnA[1];

	// Get the range
		  int x   = std::max( posnA[0], 0 );
	const int x_e = std::min( posnB[0], xres-1 );

	// Get the gradient
	const int dy16 = (dy<<16)/dx;
		  int y16  = (posnA[1]<<16) + dy16*( x - posnA[0] );

	// Scan along the line
	for( ; x<=x_e; x++, y16+=dy16 )
	{	const int y = y16>>16;
		if ( ( y>=0 ) && ( y<yres ) ) m_draw( x, y );
	}
}

template< typename draw_type >
void rasterize_line<draw_type>::draw_y( const int posnA[], const int posnB[] )
{	// Get the resolution
	const int xres = m_draw.xres();
	const int yres = m_draw.yres();

	// Get the gradient
	const int dx = posnB[0] - posnA[0];
	const int dy = posnB[1] - posnA[1];

	// Get the range
		  int y   = std::max( posnA[1], 0 );
	const int y_e = std::min( posnB[1], yres-1 );

	// Get the gradient
	const int dx16 = (dx<<16)/dy;
		  int x16  = (posnA[0]<<16) + dx16*( y - posnA[1] );

	// Scan along the line
	for( ; y<=y_e; y++, x16+=dx16 )
	{	const int x = x16>>16;
		if ( ( x>=0 ) && ( x<xres ) ) m_draw( x, y );
	}
}


//**********************************************************************************************************
void FBMP::rasterize::line( bitmap_ycbcr_u8& dst, const int posn0[], const int posn1[], const pixel_ycbcr_u8 col )
{	struct draw_impl
	{	// Get the resolution
		const int xres( void ) 
		{	return m_img.xres(); 
		}

		const int yres( void ) 
		{	return m_img.yres(); 
		}

		void operator() ( const int x, const int y )
		{	pixel_ycbcr_u8* p_img = &m_img( x&(~1), y );
			p_img->m_cr = m_col.m_cr;
			p_img->m_cb = m_col.m_cb;
			if ( x&1 )	p_img->m_y1 = m_col.m_y0;
			else		p_img->m_y0 = m_col.m_y0;
		}

		// The image to draw with
		bitmap_ycbcr_u8& m_img;
		pixel_ycbcr_u8	 m_col;

	}	draw_lines = { dst, col };

	// Draw the lines using templates
	rasterize_line<draw_impl>( draw_lines, posn0, posn1 );
}

//**********************************************************************************************************
void FBMP::rasterize::line( bitmap_bgra_u8& dst, const int posn0[], const int posn1[], const pixel_bgra_u8 col )
{	struct draw_impl
	{	// Get the resolution
		const int xres( void ) 
		{	return m_img.xres(); 
		}

		const int yres( void ) 
		{	return m_img.yres(); 
		}

		void operator() ( const int x, const int y )
		{	m_img( x, y ) = m_col;
		}

		// The image to draw with
		bitmap_bgra_u8& m_img;
		pixel_bgra_u8 m_col;

	}	draw_lines = { dst, col };

	// Draw the lines using templates
	rasterize_line<draw_impl>( draw_lines, posn0, posn1 );
}

void FBMP::rasterize::line( FBMP::bitmap_y_u8& dst_y, FBMP::bitmap_cbcr_u8& dst_cbcr, const int posn0[], const int posn1[], const pixel_ycbcr_u8 col )
{	struct draw_impl
	{	// Get the resolution
		const int xres( void ) 
		{	return img_y.xres(); 
		}

		const int yres( void ) 
		{	return img_y.yres(); 
		}

		void operator() ( const int x, const int y )
		{	img_y( x, y ).m_y = m_col.m_y0;
			img_cbcr( x/2, y/2 ).m_cb = m_col.m_cb;
			img_cbcr( x/2, y/2 ).m_cr = m_col.m_cr;
		}

		// The image to draw with
		FBMP::bitmap_y_u8& img_y;
		FBMP::bitmap_cbcr_u8& img_cbcr;
		pixel_ycbcr_u8 m_col;

	}	draw_lines = { dst_y, dst_cbcr, col };

	// Draw the lines using templates
	rasterize_line<draw_impl>( draw_lines, posn0, posn1 );
}

void FBMP::rasterize::line( FBMP::bitmap_y_u8& dst_y, FBMP::bitmap_cb_u8& dst_cb, FBMP::bitmap_cr_u8& dst_cr, const int posn0[], const int posn1[], const pixel_ycbcr_u8 col )
{	struct draw_impl
	{	// Get the resolution
		const int xres( void ) 
		{	return img_y.xres(); 
		}

		const int yres( void ) 
		{	return img_y.yres(); 
		}

		void operator() ( const int x, const int y )
		{	img_y( x, y ).m_y = m_col.m_y0;
			img_cr( x/2, y/2 ).m_cr = m_col.m_cr;
			img_cb( x/2, y/2 ).m_cb = m_col.m_cb;
		}

		// The image to draw with
		FBMP::bitmap_y_u8&	img_y;
		FBMP::bitmap_cb_u8& img_cb;
		FBMP::bitmap_cr_u8& img_cr;
		pixel_ycbcr_u8		m_col;

	}	draw_lines = { dst_y, dst_cb, dst_cr, col };

	// Draw the lines using templates
	rasterize_line<draw_impl>( draw_lines, posn0, posn1 );
}

// Draw a line between two points: YCbCrA
void FBMP::rasterize::line( bitmap_ycbcra_u8& dst, const int posn0[], const int posn1[], const pixel_ycbcra_u8 col )
{	struct draw_impl
	{	// Get the resolution
		const int xres( void ) 
		{	return m_img.xres(); 
		}

		const int yres( void ) 
		{	return m_img.yres(); 
		}

		void operator() ( const int x, const int y )
		{	m_img( x, y ) = m_col;
		}

		// The image to draw with
		FBMP::bitmap_ycbcra_u8& m_img;
		pixel_ycbcra_u8	m_col;

	}	draw_lines = { dst, col };

	// Draw the lines using templates
	rasterize_line<draw_impl>( draw_lines, posn0, posn1 );
}

// Draw a line between two points: YCbCrA
void FBMP::rasterize::line( bitmap_y_u8& dst, const int posn0[], const int posn1[], const pixel_y_u8 col )
{	struct draw_impl
	{	// Get the resolution
		const int xres( void ) 
		{	return m_img.xres(); 
		}

		const int yres( void ) 
		{	return m_img.yres(); 
		}

		void operator() ( const int x, const int y )
		{	m_img( x, y ) = m_col;
		}

		// The image to draw with
		FBMP::bitmap_y_u8& m_img;
		pixel_y_u8	m_col;

	}	draw_lines = { dst, col };

	// Draw the lines using templates
	rasterize_line<draw_impl>( draw_lines, posn0, posn1 );
}