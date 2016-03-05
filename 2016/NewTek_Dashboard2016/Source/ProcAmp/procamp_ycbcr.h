#pragma once

struct ycbcr
{		// Local type defines
		typedef FBMP::bitmap_ycbcr_u8	bitmap_ycbcr_u8;
		typedef FBMP::pixel_ycbcr_u8	pixel_ycbcr_u8;

		// This is the definition of the color correction matrix.
		// This is a 4x3 matrix that works as follows
		//	[ x x x x ]	[ y  ]
		//	[ x x x x ]	[ cb ]
		//  [ x x x x ]	[ cr ]
		//				[ 1  ]
		typedef float	color_matrix[ 3 ][ 4 ];

		// Constructor
		ycbcr( const bitmap_ycbcr_u8& src, bitmap_ycbcr_u8& dst, const color_matrix& matr );
};