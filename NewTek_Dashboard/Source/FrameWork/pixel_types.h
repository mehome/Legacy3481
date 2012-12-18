#pragma once

struct pixel_a_u8		{ typedef unsigned char value_type;		value_type m_a; };
struct pixel_bgra_u8	{ typedef unsigned char value_type;		union { struct { value_type m_b, m_g, m_r, m_a; }; DWORD m_b_g_r_a; }; };
struct pixel_rgba_u8	{ typedef unsigned char value_type;		union { struct { value_type m_r, m_g, m_b, m_a; }; DWORD m_r_g_b_a; }; };
struct pixel_bgr_u8		{ typedef unsigned char value_type;		value_type m_b, m_g, m_r; };
struct pixel_y_u8		{ typedef unsigned char value_type;		value_type m_y; };
struct pixel_ycbcra_u8	{ typedef unsigned char value_type;		value_type m_y, m_cb, m_cr, m_a; };
struct pixel_crcbya_u8	{ typedef unsigned char value_type;		value_type m_cr, m_cb, m_y, m_a; };		// Analogous to BGRA in YCbCr space
struct pixel_crcby_u8	{ typedef unsigned char value_type;		value_type m_cr, m_cb, m_y; };			// Analogous to BGRA in YCbCr space
struct pixel_ya_u8		{ typedef unsigned char value_type;		value_type m_y; value_type m_a; };
struct pixel_cb_u8		{ typedef unsigned char value_type;		value_type m_cb; };
struct pixel_cr_u8		{ typedef unsigned char value_type;		value_type m_cr; };
struct pixel_ycbcr_u8	{ typedef unsigned char value_type;		union { struct { value_type m_cb, m_y0, m_cr, m_y1; }; DWORD m_cb_y0_cr_y1; }; };
struct pixel_cbcr_u8	{ typedef unsigned char value_type;		union { struct { value_type m_cb, m_cr; }; WORD m_cb_cr; }; };

struct pixel_a_s8		{ typedef signed char value_type;		value_type m_a; };
struct pixel_bgra_s8	{ typedef signed char value_type;		value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_s8	{ typedef signed char value_type;		value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_s8		{ typedef signed char value_type;		value_type m_b, m_g, m_r; };
struct pixel_y_s8		{ typedef signed char value_type;		value_type m_y; };
struct pixel_ycbcra_s8	{ typedef signed char value_type;		value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_s8		{ typedef signed char value_type;		value_type m_y; value_type m_a; };
struct pixel_cb_s8		{ typedef signed char value_type;		value_type m_cb; };
struct pixel_cr_s8		{ typedef signed char value_type;		value_type m_cr; };
struct pixel_ycbcr_s8	{ typedef signed char value_type;		value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_s8	{ typedef signed char value_type;		value_type m_cb, m_cr; };

struct pixel_a_u16		{ typedef unsigned short value_type;	value_type m_a; };
struct pixel_bgra_u16	{ typedef unsigned short value_type;	value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_u16	{ typedef unsigned short value_type;	value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_u16	{ typedef unsigned short value_type;	value_type m_b, m_g, m_r; };
struct pixel_y_u16		{ typedef unsigned short value_type;	value_type m_y; };
struct pixel_ycbcra_u16	{ typedef unsigned short value_type;	value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_u16		{ typedef unsigned short value_type;	value_type m_y; value_type m_a; };
struct pixel_cb_u16		{ typedef unsigned short value_type;	value_type m_cb; };
struct pixel_cr_u16		{ typedef unsigned short value_type;	value_type m_cr; };
struct pixel_ycbcr_u16	{ typedef unsigned short value_type;	value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_u16	{ typedef unsigned short value_type;	value_type m_cb, m_cr; };

struct pixel_a_s16		{ typedef signed short value_type;		value_type m_a; };
struct pixel_bgra_s16	{ typedef signed short value_type;		value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_s16	{ typedef signed short value_type;		value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_s16	{ typedef signed short value_type;		value_type m_b, m_g, m_r; };
struct pixel_y_s16		{ typedef signed short value_type;		value_type m_y; };
struct pixel_ycbcra_s16	{ typedef signed short value_type;		value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_s16		{ typedef signed short value_type;		value_type m_y; value_type m_a; };
struct pixel_cb_s16		{ typedef signed short value_type;		value_type m_cb; };
struct pixel_cr_s16		{ typedef signed short value_type;		value_type m_cr; };
struct pixel_ycbcr_s16	{ typedef signed short value_type;		value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_s16	{ typedef signed short value_type;		value_type m_cb, m_cr; };

struct pixel_a_s32		{ typedef signed int value_type;		value_type m_a; };
struct pixel_bgra_s32	{ typedef signed int value_type;		value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_s32	{ typedef signed int value_type;		value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_s32	{ typedef signed int value_type;		value_type m_b, m_g, m_r; };
struct pixel_y_s32		{ typedef signed int value_type;		value_type m_y; };
struct pixel_ycbcra_s32	{ typedef signed int value_type;		value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_s32		{ typedef signed int value_type;		value_type m_y; value_type m_a; };
struct pixel_cb_s32		{ typedef signed int value_type;		value_type m_cb; };
struct pixel_cr_s32		{ typedef signed int value_type;		value_type m_cr; };
struct pixel_ycbcr_s32	{ typedef signed int value_type;		value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_s32	{ typedef signed int value_type;		value_type m_cb, m_cr; };

struct pixel_a_u32		{ typedef unsigned int value_type;		value_type m_a; };
struct pixel_bgra_u32	{ typedef unsigned int value_type;		value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_u32	{ typedef unsigned int value_type;		value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_u32	{ typedef unsigned int value_type;		value_type m_b, m_g, m_r; };
struct pixel_y_u32		{ typedef unsigned int value_type;		value_type m_y; };
struct pixel_ycbcra_u32	{ typedef unsigned int value_type;		value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_u32		{ typedef unsigned int value_type;		value_type m_y; value_type m_a; };
struct pixel_cb_u32		{ typedef unsigned int value_type;		value_type m_cb; };
struct pixel_cr_u32		{ typedef unsigned int value_type;		value_type m_cr; };
struct pixel_ycbcr_u32	{ typedef unsigned int value_type;		value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_u32	{ typedef unsigned int value_type;		value_type m_cb, m_cr; };

struct pixel_a_f32		{ typedef float value_type;				value_type m_a; };
struct pixel_bgra_f32	{ typedef float value_type;				value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_f32	{ typedef float value_type;				value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_f32	{ typedef float value_type;				value_type m_b, m_g, m_r; };
struct pixel_rgb_f32	{ typedef float value_type;				value_type m_r, m_g, m_b; };
struct pixel_y_f32		{ typedef float value_type;				value_type m_y; };
struct pixel_ycbcra_f32	{ typedef float value_type;				value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_f32		{ typedef float value_type;				value_type m_y; value_type m_a; };
struct pixel_cb_f32		{ typedef float value_type;				value_type m_cb; };
struct pixel_cr_f32		{ typedef float value_type;				value_type m_cr; };
struct pixel_ycbcr_f32	{ typedef float value_type;				value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_f32	{ typedef float value_type;				value_type m_cb, m_cr; };

struct pixel_a_f64		{ typedef double value_type;			value_type m_a; };
struct pixel_bgra_f64	{ typedef double value_type;			value_type m_b, m_g, m_r, m_a; };
struct pixel_rgba_f64	{ typedef double value_type;			value_type m_r, m_g, m_b, m_a; };
struct pixel_bgr_f64	{ typedef double value_type;			value_type m_b, m_g, m_r; };
struct pixel_y_f64		{ typedef double value_type;			value_type m_y; };
struct pixel_ycbcra_f64	{ typedef double value_type;			value_type m_y, m_cb, m_cr, m_a; };
struct pixel_ya_f64		{ typedef double value_type;			value_type m_y; value_type m_a; };
struct pixel_cb_f64		{ typedef double value_type;			value_type m_cb; };
struct pixel_cr_f64		{ typedef double value_type;			value_type m_cr; };
struct pixel_ycbcr_f64	{ typedef double value_type;			value_type m_cb, m_y0, m_cr, m_y1; };
struct pixel_cbcr_f64	{ typedef double value_type;			value_type m_cb, m_cr; };