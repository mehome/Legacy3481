#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::video;

// Constructur for compressed data formats
message::message( const e_data_format format, const int xres, const int yres, const int compressed_data_size, const int extra_data_size_max )
	:	m_ref( 1 ), m_p_header( NULL ), m_p_compressed_data( NULL ),
		FC3i::message( message_size( format, xres, yres, compressed_data_size, extra_data_size_max ) )

{	// Set the type
	if ( !FC3i::message::error() )
	{	// Set the size
		type() = message_type_video;

		// Setup the memory
		setup_memory( format, xres, yres, compressed_data_size, extra_data_size_max );
	}	
}

// Constructur for compressed data formats
message::message( const e_data_format format, const int xres, const int yres )
	:	m_ref( 1 ), m_p_header( NULL ), m_p_compressed_data( NULL ),
		FC3i::message( message_size( format, xres, yres, 0, 0 ) )

{	// Set the type
	if ( !FC3i::message::error() )
	{	// Set the size
		type() = message_type_video;

		// Setup the memory
		setup_memory( format, xres, yres, 0, 0 );
	}	
}

// Internal use only :)
message::message( const DWORD block_id, const DWORD addr )
	:	m_ref( 1 ), m_p_header( NULL ), m_p_compressed_data( NULL ),
		FC3i::message( block_id, addr )

{	// Check the size and the type
	if ( !FC3i::message::error() )
	{	// If the type is correct, set it up.
		if ( type() == message_type_video )
			setup_memory();
	}
}

message::~message( void )
{
}

// Is there an error in this message, most likely caused by a failed allocation or transmission
bool message::error( void ) const
{	return m_p_header ? false : true;
}

// Reference counting
const long message::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long message::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

const long message::refcount( void ) const
{	return m_ref;
}

// Get the size of a message
const int message::message_size( const e_data_format format, const int xres, const int yres, const int compressed_data_size, const int extra_size )
{	// Has a header
	int size = header_size;

#define	is_even( x ) ( ((x)&1)==0 )

	// Now add in the image size
	switch( format )
	{	case data_format_ycbcr_422_u8:						// This is UYVY
				assert( is_even(xres) );
				assert( !compressed_data_size );
				size += xres * yres * 2;					// YCbCr
				break;

		case data_format_ycbcr_a_4224_u8:					// This is what we call UYVA which is UYVY + Alpha
				assert( is_even(xres) );
				assert( !compressed_data_size );
				size += xres * yres * 2;					// YCbCr
				size += xres * yres;						// Alpha
				break;

		case data_format_y_cb_cr_420_u8:					// This is YV12
		case data_format_y_cbcr_420_u8:						// This is NV12
				assert( is_even(xres) && is_even(yres) );
				assert( !compressed_data_size );
				size += xres * yres;						// Y
				size += ( xres / 2 ) * ( yres / 2 ) * 2;	// Cb, Cr
				break;		

		case data_format_ycbcra_4444_u8:					// This is YCbCrA
				assert( !compressed_data_size );
				size += xres * yres * 4;					// YCbCrA
				break;

		case data_format_bgra_4444_u8:						// This is regular BGRA
				assert( !compressed_data_size );
				size += xres * yres * 4;					// BGRA
				break;

		// Compressed data formats do not have a fixed data size
		case data_format_m2v1_422:
		case data_format_m2v1_420:
		case data_format_shq_4444:
		case data_format_shq_4444_ex:
		case data_format_shq_444:
		case data_format_shq_4224:
		case data_format_shq_4224_ex:
		case data_format_shq_422:
		case data_format_shq_4204:
		case data_format_shq_420:
				assert( compressed_data_size );
				size += fc3_size_align( compressed_data_size );
				break;

		default:assert( false );
				break;
	}

#undef is_even

	// Add in the extra data size
	size += extra_size;

	// Return the size;
	return size;
}



// Recover the resolution of the current image
const int message::xres( void ) const
{	assert( !error() );
	return m_p_header->m_xres;
}

const int message::yres( void ) const
{	assert( !error() );
	return m_p_header->m_yres;
}

// What planes are in the image
const bool message::has_ycbcr( void ) const
{	return m_ycbcr() ? true : false;
}

const bool message::has_ycbcra( void ) const
{	return m_ycbcra() ? true : false;
}

const bool message::has_y( void ) const
{	return m_y() ? true : false;
}

const bool message::has_cb( void ) const
{	return m_cb() ? true : false;
}

const bool message::has_cr( void ) const
{	return m_cr() ? true : false;
}

const bool message::has_bgra( void ) const
{	return m_bgra() ? true : false;
}

const bool message::has_alpha( void ) const
{	return m_alpha() ? true : false;
}

const bool message::has_compressed( void ) const
{	return m_p_compressed_data ? true : false;
}

const bool message::is_compressed( void ) const
{	return m_p_compressed_data ? true : false;
}

const bool message::is_uncompressed( void ) const
{	return m_p_compressed_data ? false : true;
}

// does this frame have a dirty sub rectangle region?
const bool message::has_sub_rect( void ) const
{	// Check the sub-rectangle
	assert( !error() );
	return ( ( m_p_header->m_sub_rect.left == 0 ) && ( m_p_header->m_sub_rect.top == 0 ) &&
			 ( m_p_header->m_sub_rect.right == xres() ) && ( m_p_header->m_sub_rect.bottom == yres() ) ) ? false : true;
}

// Get and set the sub rectangle region
const RECT& message::sub_rect( void ) const
{	assert( !error() );
	return m_p_header->m_sub_rect;
}

RECT& message::sub_rect( void )
{	assert( !error() );
	return m_p_header->m_sub_rect;
}

// Get and set the field type
const message::e_data_format& message::data_format( void ) const
{	assert( !error() );
	return m_p_header->m_data_format;
}

// Get and set the field type
const message::e_field_type  message::field_type( void ) const
{	assert( !error() );
	return m_p_header->m_field_type;
}

message::e_field_type& message::field_type( void )
{	assert( !error() );
	return m_p_header->m_field_type;
}

#define case_not_possible	default: __assume( false ); assert( false ); return false;

// Some utility functions that make detecting fields simpler.
const bool message::has_field_0( void ) const
{	assert( !error() );
	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return false;	// A single progressive frame
		case field_type_single_field_0:		return true;	// A single field, number 0
		case field_type_single_field_1:		return false;	// A single field, number 1
		case field_type_both_interleaved:	return true;	// Both fields, interleaved together
		case_not_possible									// Not supported
	}
}

const bool message::has_field_1( void ) const
{	assert( !error() );
	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return false;	// A single progressive frame
		case field_type_single_field_0:		return false;	// A single field, number 0
		case field_type_single_field_1:		return true;	// A single field, number 1
		case field_type_both_interleaved:	return true;	// Both fields, interleaved together
		case_not_possible									// Not supported
	}
}

const bool message::has_both_fields( void ) const
{	assert( !error() );
	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return false;	// A single progressive frame
		case field_type_single_field_0:		return false;	// A single field, number 0
		case field_type_single_field_1:		return false;	// A single field, number 1
		case field_type_both_interleaved:	return true;	// Both fields, interleaved together
		case_not_possible									// Not supported
	}
}

const bool message::has_one_field( void ) const
{	assert( !error() );
	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return false;	// A single progressive frame
		case field_type_single_field_0:		return true;	// A single field, number 0
		case field_type_single_field_1:		return true;	// A single field, number 1
		case field_type_both_interleaved:	return false;	// Both fields, interleaved together
		case_not_possible									// Not supported
	}
}

const bool message::is_fielded( void ) const
{	assert( !error() );
	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return false;	// A single progressive frame
		case field_type_single_field_0:		return true;	// A single field, number 0
		case field_type_single_field_1:		return true;	// A single field, number 1
		case field_type_both_interleaved:	return true;	// Both fields, interleaved together
		case_not_possible									// Not supported
	}
}

const bool message::is_progressive( void ) const
{	assert( !error() );
	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return true;	// A single progressive frame
		case field_type_single_field_0:		return false;	// A single field, number 0
		case field_type_single_field_1:		return false;	// A single field, number 1
		case field_type_both_interleaved:	return false;	// Both fields, interleaved together
		case_not_possible									// Not supported
	}
}

const bool message::has_field( const int i ) const
{	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return false;
		case field_type_single_field_0:		return ( i == 0 );
		case field_type_single_field_1:		return ( i == 1 );
		case field_type_both_interleaved:	return ( i == 0 ) || ( i == 1 );
		case_not_possible					// Not supported
	}
}

const bool message::has_field( const e_field_type field ) const
{	switch( m_p_header->m_field_type )
	{	case field_type_progressive:		return ( field == field_type_progressive );
		case field_type_single_field_0:		return ( field == field_type_single_field_0 );
		case field_type_single_field_1:		return ( field == field_type_single_field_1 );
		case field_type_both_interleaved:	return ( field == field_type_single_field_0 ) || ( field == field_type_single_field_1 ) || ( field == field_type_both_interleaved );
		case_not_possible					// Not supported
	}
}

// Get and set the time-stamp of the field.
// This is stored in 10ns intervals.
const __int64 message::time_stamp( void ) const
{	assert( !error() );
	return m_p_header->m_time_stamp;
}

__int64& message::time_stamp( void )
{	assert( !error() );
	return m_p_header->m_time_stamp;
}

const __int64 message::time_code( void ) const
{	assert( !error() );
	return m_p_header->m_time_code;
}

__int64& message::time_code( void )
{	assert( !error() );
	return m_p_header->m_time_code;
}

// Get and set the aspect ratio of the frame
const float message::aspect_ratio( void ) const
{	assert( !error() );
	return m_p_header->m_aspect_ratio;
}

float& message::aspect_ratio( void )
{	assert( !error() );
	return m_p_header->m_aspect_ratio;
}

const message::frame_rate_type&	message::frame_rate( void ) const
{	assert( !error() );
	return m_p_header->m_frame_rate;
}

message::frame_rate_type& message::frame_rate( void )
{	assert( !error() );
	return m_p_header->m_frame_rate;
}

// Get access to the buffers
FrameWork::Bitmaps::bitmap_ycbcr_u8& message::ycbcr( void )
{	assert( m_ycbcr() );
	return m_ycbcr;
}

const FrameWork::Bitmaps::bitmap_ycbcr_u8& message::ycbcr( void ) const
{	assert( m_ycbcr() );
	return m_ycbcr;
}

FrameWork::Bitmaps::bitmap_y_u8& message::y( void )
{	assert( m_y() );
	return m_y;
}

const FrameWork::Bitmaps::bitmap_y_u8& message::y( void ) const
{	assert( m_y() );
	return m_y;
}

FrameWork::Bitmaps::bitmap_a_u8& message::alpha( void )
{	assert( m_alpha() );
	return m_alpha;
}

const FrameWork::Bitmaps::bitmap_a_u8& message::alpha( void ) const
{	assert( m_alpha() );
	return m_alpha;
}

FrameWork::Bitmaps::bitmap_cb_u8& message::cb( void )
{	assert( m_cb() );
	return m_cb;
}

const FrameWork::Bitmaps::bitmap_cb_u8&	message::cb( void ) const
{	assert( m_cb() );
	return m_cb;
}

FrameWork::Bitmaps::bitmap_cr_u8& message::cr( void )
{	assert( m_cr() );
	return m_cr;
}

const FrameWork::Bitmaps::bitmap_cr_u8& message::cr( void ) const
{	assert( m_cr() );
	return m_cr;
}

FrameWork::Bitmaps::bitmap_cbcr_u8& message::cbcr( void )
{	assert( m_cbcr() );
	return m_cbcr;
}

const FrameWork::Bitmaps::bitmap_cbcr_u8& message::cbcr( void ) const
{	assert( m_cbcr() );
	return m_cbcr;
}

FrameWork::Bitmaps::bitmap_bgra_u8&	message::bgra( void )
{	assert( m_bgra() );
	return m_bgra;
}

const FrameWork::Bitmaps::bitmap_bgra_u8& message::bgra( void ) const
{	assert( m_bgra() );
	return m_bgra;
}

FrameWork::Bitmaps::bitmap_ycbcra_u8& message::ycbcra( void )
{	assert( m_ycbcra() );
	return m_ycbcra;
}

const FrameWork::Bitmaps::bitmap_ycbcra_u8&	message::ycbcra( void ) const
{	assert( m_ycbcra() );
	return m_ycbcra;
}

// Setup all pointers
void message::setup_memory( const e_data_format format, const int xres, const int yres, const int compressed_data_size, const int extra_size )
{	// Set the header pointer
	assert( !m_p_header );
	m_p_header = (header*)ptr();
	assert( m_p_header );

	// Setup the header
	m_p_header->m_field_type = field_type_progressive;
	m_p_header->m_data_format = format;
	m_p_header->m_xres = xres;
	m_p_header->m_yres = yres;
	m_p_header->m_compressed_data_size = compressed_data_size;
	m_p_header->m_aspect_ratio = 16.0f / 9.0f;
	m_p_header->m_frame_rate = frame_rate_type_29_97;
	m_p_header->m_time_stamp = 0;
	m_p_header->m_sub_rect.left = 0;
	m_p_header->m_sub_rect.top = 0;
	m_p_header->m_sub_rect.right = xres;
	m_p_header->m_sub_rect.bottom = yres;
	m_p_header->m_extra_data_size	  = extra_size;
	m_p_header->m_extra_data_size_max = extra_size;
	m_p_header->m_extra_data_fourcc = 0;
	m_p_header->m_time_code = 0;	

	// Setup the rest
	setup_memory();
}

void message::setup_memory( void )
{	// Set the header pointer
	m_p_header = (header*)ptr();

	// Store the size
	const int xres = m_p_header->m_xres;
	const int yres = m_p_header->m_yres;	

	// We get the current offset
	BYTE*	p_ptr = ptr( header_size );

#define	is_even( x ) ( ((x)&1)==0 )

	// Now setup the memory pointers
	// Now add in the image size
	switch( m_p_header->m_data_format )
	{	case data_format_ycbcr_422_u8:						
					// This is UYVY
					assert( is_even(xres) );
					assert( !m_p_header->m_compressed_data_size );

					// YCbCr
					m_ycbcr.reference_in_bytes( (FrameWork::Bitmaps::pixel_ycbcr_u8*)p_ptr, xres, yres, xres*2 );
					p_ptr += xres*yres*2;

					// Finished
					break;

		case data_format_ycbcr_a_4224_u8:					
					// This is what we call UYVA which is UYVY + Alpha
					assert( is_even(xres) );
					assert( !m_p_header->m_compressed_data_size );

					// YCbCr
					m_ycbcr.reference_in_bytes( (FrameWork::Bitmaps::pixel_ycbcr_u8*)p_ptr, xres, yres, xres*2 );
					p_ptr += xres*yres*2;

					// Alpha
					m_alpha.reference_in_bytes( (FrameWork::Bitmaps::pixel_a_u8*)p_ptr, xres, yres, xres );
					p_ptr += xres*yres;

					// Finished
					break;

		case data_format_y_cbcr_420_u8:					
					// This is NV12
					assert( is_even(xres) && is_even(yres) );
					assert( !m_p_header->m_compressed_data_size );

					// Y
					m_y.reference_in_bytes( (FrameWork::Bitmaps::pixel_y_u8*)p_ptr, xres, yres, xres );
					p_ptr += xres*yres;

					// VU
					m_cbcr.reference_in_bytes( (FrameWork::Bitmaps::pixel_cbcr_u8*)p_ptr, xres/2, yres/2, (xres/2)*2 );
					p_ptr += (xres/2)*(yres/2)*2;					

					// Finished
					break;

		case data_format_y_cb_cr_420_u8:					
					// This is YV12
					assert( is_even(xres) && is_even(yres) );
					assert( !m_p_header->m_compressed_data_size );

					// Y
					m_y.reference_in_bytes( (FrameWork::Bitmaps::pixel_y_u8*)p_ptr, xres, yres, xres );
					p_ptr += xres*yres;

					// V 
					m_cr.reference_in_bytes( (FrameWork::Bitmaps::pixel_cr_u8*)p_ptr, xres/2, yres/2, xres/2 );
					p_ptr += (xres/2)*(yres/2);

					// U
					m_cb.reference_in_bytes( (FrameWork::Bitmaps::pixel_cb_u8*)p_ptr, xres/2, yres/2, xres/2 );
					p_ptr += (xres/2)*(yres/2);

					// Finished
					break;

		case data_format_ycbcra_4444_u8:					
					// This is YCbCrA
					assert( !m_p_header->m_compressed_data_size );
					m_ycbcra.reference_in_bytes( (FrameWork::Bitmaps::pixel_ycbcra_u8*)p_ptr, xres, yres, xres*4 );
					p_ptr += xres*yres*4;
					break;

		case data_format_bgra_4444_u8:
					// This is regular BGRA
					assert( !m_p_header->m_compressed_data_size );
					m_bgra.reference_in_bytes( (FrameWork::Bitmaps::pixel_bgra_u8*)p_ptr, xres, yres, xres*4 );
					p_ptr += xres*yres*4;
					break;

		case data_format_m2v1_422:
		case data_format_m2v1_420:
		case data_format_shq_4444_ex:
		case data_format_shq_4444:
		case data_format_shq_444:
		case data_format_shq_4224_ex:
		case data_format_shq_4224:
		case data_format_shq_422:
		case data_format_shq_4204:
		case data_format_shq_420:
					assert( m_p_header->m_compressed_data_size );
					m_p_compressed_data = (BYTE*)p_ptr;
					p_ptr += fc3_size_align( m_p_header->m_compressed_data_size );
					break;

		default:	// Something bad
					assert( false );
					__assume( false );
					break;
	}

	// Store the extra data pointer
	m_p_extra_data = m_p_header->m_extra_data_size ? (void*)p_ptr : NULL;

	// Increment the pointer for the assert below
	p_ptr += m_p_header->m_extra_data_size_max;

	// Debugging
	assert( p_ptr == ptr() + size() );

#undef	is_even
}

const DWORD	message::extra_data_fourCC( void ) const
{	assert( !error() );
	return m_p_header->m_extra_data_fourcc;
}

DWORD &message::extra_data_fourCC( void )
{	assert( !error() );
	return m_p_header->m_extra_data_fourcc;
}

const void* message::p_extra_data( void ) const
{	return m_p_extra_data;
}

void* message::p_extra_data( void )
{	return m_p_extra_data;
}

void message::set_extra_data_size( const int new_data_size )
{	assert( !error() );
	assert( new_data_size <= m_p_header->m_extra_data_size_max );
	m_p_header->m_extra_data_size = new_data_size;
}

const int message::extra_data_size( void ) const
{	assert( !error() );
	return m_p_header->m_extra_data_size;	
}

// This ensures that people can delete the read_with_info structs returned above correctly
static FC3i::memory_pool< sizeof( message ) > g_mem_alloc;

void* message::operator new ( const size_t size )
{	assert( sizeof( message ) == size );
	return g_mem_alloc.malloc();
}

void  message::operator delete ( void* ptr )
{	return g_mem_alloc.free( ptr );
}

void* message::operator new [] ( const size_t size )
{	return ::malloc( size );
}

void  message::operator delete [] ( void* ptr )
{	::free( ptr );
}

// Get the duration of this frame in 10ns intervals
const __int64 message::duration( const __int64 time_base ) const
{	// Get the frame-rate
	const __int64 n = abs( frame_rate().n() ) * ( has_one_field() ? 2 : 1 );
	const __int64 d = abs( frame_rate().d() );
	
	// Return the length, correctly rounded
	return ( time_base*d + n/2 ) / n;
}

const double message::duration_in_seconds( void ) const
{	// Get the frame-rate
	const __int64 n = abs( frame_rate().n() ) * ( has_one_field() ? 2 : 1 );
	const __int64 d = abs( frame_rate().d() );

	// Return the result
	return (double)d / (double)n;
}

BYTE* message::p_compressed_data( void )
{	return m_p_compressed_data;
}

const BYTE* message::p_compressed_data( void ) const
{	return m_p_compressed_data;
}

const int message::compressed_data_size( void ) const
{	assert( !error() );
	return m_p_header->m_compressed_data_size;
}