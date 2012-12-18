#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FrameWork::Communication3::implementation::message
{			// The frame format to use
			typedef enum e_data_format
			{	// For a description of the different YCbCr formats, please visit
				//	http://www.fourcc.org/yuv.php
				data_format_none            = -1,	// There is no video frame
				data_format_ycbcr_422_u8    = 0,	// This is UYVY
				data_format_ycbcr_a_4224_u8 = 1,	// This is what we call UYVA which is UYVY + Alpha
				data_format_y_cb_cr_420_u8  = 2,	// This is YV12
				data_format_y_cbcr_420_u8   = 5,	// This is NV12
				data_format_ycbcra_4444_u8  = 3,	// This is YCbCrA
				data_format_bgra_4444_u8    = 4,	// This is regular BGRA
			};

			// The field types
			typedef enum e_field_type
			{	field_type_progressive      = 0,	// A single progressive frame
				field_type_single_field_0   = 1,	// A single field, number 0
				field_type_single_field_1   = 2,	// A single field, number 1
				field_type_both_interleaved = 3,	// Both fields, interleaved together
				field_type_both_stacked     = 4		// Both fields, field 0 stacked above field 1
			};

			// This defines how time-stamps are defined.
			struct FRAMEWORKCOMMUNICATION3_API frame_rate_type;

			// Defaults
			static const frame_rate_type	frame_rate_type_60;
			static const frame_rate_type	frame_rate_type_59_94;
			static const frame_rate_type	frame_rate_type_30;
			static const frame_rate_type	frame_rate_type_29_97;
			static const frame_rate_type	frame_rate_type_25;
			static const frame_rate_type	frame_rate_type_24;
			static const frame_rate_type	frame_rate_type_23_97;

			// To keep this class readable.
			#include "FC3_video_frame_rate.h"			
			
			// Constructor
			message( const e_data_format format, const int xres, const int yres, const int extra_data_size = 0 );

			// Destructor
			~message( void );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;

			// Recover the resolution of the current image
			const int xres( void ) const;
			const int yres( void ) const;

			// What planes are in the image
			const bool has_ycbcr( void ) const;
			const bool has_ycbcra( void ) const;
			const bool has_y( void ) const;
			const bool has_cb( void ) const;
			const bool has_cr( void ) const;
			const bool has_bgra( void ) const;
			const bool has_alpha( void ) const;

			// does this frame have a dirty sub rectangle region?
			const bool has_sub_rect( void ) const;

			// Get and set the sub rectangle region
			const RECT& sub_rect( void ) const;
				  RECT& sub_rect( void );

			// Get and set the field type
			const e_data_format& data_format( void ) const;

			// Get and set the field type
			const e_field_type  field_type( void ) const;
				  e_field_type& field_type( void );

			// Some utility functions that make detecting fields simpler.
			const bool has_field_0( void ) const;
			const bool has_field_1( void ) const;
			const bool has_both_fields( void ) const;
			const bool has_one_field( void ) const;
			const bool is_fielded( void ) const;
			const bool is_progressive( void ) const;

			const bool has_field( const int i ) const;
			const bool has_field( const e_field_type field ) const;

			// Get and set the time-stamp of the field.
			// This is stored in 10ns intervals.
			const __int64  time_stamp( void ) const;
				  __int64& time_stamp( void );

			// Get and set the current time-code of a field
			const __int64	time_code( void ) const;
				  __int64&	time_code( void );

			// Get and set the aspect ratio of the frame
			const float  aspect_ratio( void ) const;
				  float& aspect_ratio( void );

			// Get and set the aspect ratio of the frame
			const frame_rate_type&	frame_rate( void ) const;
				  frame_rate_type&	frame_rate( void );

			// Get access to the buffers
				  FrameWork::Bitmaps::bitmap_ycbcr_u8& ycbcr( void );
			const FrameWork::Bitmaps::bitmap_ycbcr_u8& ycbcr( void ) const;

				  FrameWork::Bitmaps::bitmap_y_u8& y( void );
			const FrameWork::Bitmaps::bitmap_y_u8& y( void ) const;

				  FrameWork::Bitmaps::bitmap_a_u8& alpha( void );
			const FrameWork::Bitmaps::bitmap_a_u8& alpha( void ) const;

				  FrameWork::Bitmaps::bitmap_cb_u8& cb( void );
			const FrameWork::Bitmaps::bitmap_cb_u8&	cb( void ) const;

				  FrameWork::Bitmaps::bitmap_cr_u8& cr( void );
			const FrameWork::Bitmaps::bitmap_cr_u8& cr( void ) const;

				  FrameWork::Bitmaps::bitmap_cbcr_u8& cbcr( void );
			const FrameWork::Bitmaps::bitmap_cbcr_u8& cbcr( void ) const;

				  FrameWork::Bitmaps::bitmap_bgra_u8& bgra( void );
			const FrameWork::Bitmaps::bitmap_bgra_u8& bgra( void ) const;

				  FrameWork::Bitmaps::bitmap_ycbcra_u8&	ycbcra( void );
			const FrameWork::Bitmaps::bitmap_ycbcra_u8&	ycbcra( void ) const;

			// This will ficegive you access to the extra data that can be allocated along with a frame.
			// Obviously this requires the sender and receiver to have a common understanding of what is
			// actually stored in the data itself.
			const void* extra_data( void ) const;
				  void* extra_data( void );

			const int	extra_data_size( void ) const;

			// Is there an error in this message, most likely caused by a failed allocation or transmission
			bool error( void ) const;

			// This ensures that people can pass messages between DLLs
			static void* operator new ( const size_t size );
			static void  operator delete ( void* ptr );
			static void* operator new [] ( const size_t size );
			static void  operator delete [] ( void* ptr );

			// Get the duration of this frame in 10ns intervals
			const __int64 duration( const __int64 time_base = 10000000LL ) const;
			const double duration_in_seconds( void ) const;

protected:	// Internal use only :)
			message( const DWORD block_id, const DWORD addr );
	
private:	// This is the data header
			struct header
			{	// What field type is this ?
				e_field_type	m_field_type;

				// That format is the data in
				e_data_format	m_data_format;

				// The resolution
				int				m_xres, m_yres;

				// The current frame-rate
				frame_rate_type	m_frame_rate;

				// The aspect ratio
				float			m_aspect_ratio;

				// The time-stamp
				__int64			m_time_stamp;

				// The default sub-rectangle
				RECT			m_sub_rect;

				// The extra data size
				int				m_extra_data_size;

				// The time-code
				__int64			m_time_code;

			} *m_p_header;			

			static const int header_size = fc3_size_align( sizeof( header ) );

			// This is the extra data pointer
			void*	m_p_extra_data;

			// The video data 
			FrameWork::Bitmaps::bitmap_ycbcr_u8		m_ycbcr;
			FrameWork::Bitmaps::bitmap_a_u8			m_alpha;
			FrameWork::Bitmaps::bitmap_y_u8			m_y;						
			FrameWork::Bitmaps::bitmap_cb_u8		m_cb;
			FrameWork::Bitmaps::bitmap_cr_u8		m_cr;
			FrameWork::Bitmaps::bitmap_cbcr_u8		m_cbcr;
			FrameWork::Bitmaps::bitmap_bgra_u8		m_bgra;
			FrameWork::Bitmaps::bitmap_ycbcra_u8	m_ycbcra;
	
			// Everything is reference counted
			mutable volatile long	m_ref;

			// Get the size of a message
			static const int message_size( const e_data_format format, const int xres, const int yres, const int extra_size );

			// Setup all pointers
			void setup_memory( const e_data_format format, const int xres, const int yres, const int extra_size );
			void setup_memory( void );

			// A friend
			friend receive;
			friend pull;
			friend FrameWork::Communication3::audio_video::receive;
			friend FrameWork::Communication3::audio_video_xml::receive;
};