#pragma once

namespace rasterize
{	// Draw a line between two points: YCbCr
	void line( bitmap_ycbcr_u8& dst, const int posn0[], const int posn1[], const pixel_ycbcr_u8 col );

	// Draw a line between two points: BGRA
	void line( bitmap_bgra_u8&  dst, const int posn0[], const int posn1[], const pixel_bgra_u8  col );

	// Draw a line between two points: Luminance
	void line( bitmap_y_u8&  dst, const int posn0[], const int posn1[], const pixel_y_u8  col );

	// Draw a line between two points: NV12 (!)
	void line( bitmap_y_u8& dst_y, bitmap_cbcr_u8& dst_cbcr, const int posn0[], const int posn1[], const pixel_ycbcr_u8 col );

	// Draw a line between two points: YV12
	void line( bitmap_y_u8& dst_y, bitmap_cb_u8& dst_cb, bitmap_cr_u8& dst_cr, const int posn0[], const int posn1[], const pixel_ycbcr_u8 col );

	// Draw a line between two points: YCbCrA
	void line( bitmap_ycbcra_u8& dst, const int posn0[], const int posn1[], const pixel_ycbcra_u8 col );
};