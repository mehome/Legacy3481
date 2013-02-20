#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FrameWork::Communication3::implementation::message
{			// The frame format to use
			typedef enum e_data_format
			{	// Support the existing formats in an alterntive enumeration
				data_format_none            = -1,			// There is no video frame
				data_format_ycbcr_422_u8    =  0,			// This is UYVY
				data_format_ycbcr_a_4224_u8 =  1,			// This is what we call UYVA which is UYVY + Alpha
				data_format_y_cb_cr_420_u8  =  2,			// This is YV12
				data_format_y_cbcr_420_u8   =  5,			// This is NV12
				data_format_ycbcra_4444_u8  =  3,			// This is YCbCrA
				data_format_bgra_4444_u8    =  4,			// This is regular BGRA
				
				// MPEG-2
				data_format_m2v1_422		 = '1v2m',		// Compressed MPEG-2, High profile, 4:2:2, I-Frame only, normally interleaved data for fielded sources.
				data_format_m2v1_420		 = '2v2m',		// Compressed MPEG-2, Normal profile, 4:2:2, I-Frame only, normally interleaved data for fielded sources.

				// SpeedHQ
				data_format_shq_4444		 = '5QHS',		// SpeedHQ 5, 4:4:4:4, I-Frame only, normally interleaved data for fielded sources.
				data_format_shq_444 		 = '4QHS',		// SpeedHQ 4, 4:4:4  , I-Frame only, normally interleaved data for fielded sources.
				data_format_shq_4224		 = '3QHS',		// SpeedHQ 3, 4:2:2:4, I-Frame only, normally interleaved data for fielded sources.
				data_format_shq_422 		 = '2QHS',		// SpeedHQ 2, 4:2:2  , I-Frame only, normally interleaved data for fielded sources.
				data_format_shq_4204		 = '1QHS',		// SpeedHQ 1, 4:2:0:4, I-Frame only, normally interleaved data for fielded sources.
				data_format_shq_420 		 = '0QHS',		// SpeedHQ 0, 4:2:0  , I-Frame only, normally interleaved data for fielded sources.

				data_format_size			 = 0xFFFFFFFF	// Force a size
			};

			// The field types
			typedef enum e_field_type
			{	field_type_progressive      = 0,	// A single progressive frame
				field_type_single_field_0   = 1,	// A single field, number 0
				field_type_single_field_1   = 2,	// A single field, number 1
				field_type_both_interleaved = 3,	// Both fields, interleaved together
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

			// The creator of new messages. 
			// Please note that for compressed data formats you cannot specify compressed_data_size == 0
			message( const e_data_format format, const int xres, const int yres, const int compressed_data_size, const int extra_data_size_max );
			message( const e_data_format format, const int xres, const int yres );

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
			const bool has_compressed( void ) const;

			// Does this contain compressed video data ?
			const bool is_compressed( void ) const;
			const bool is_uncompressed( void ) const;

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

			// Get access to the compressed data frame in the format
			// specified above.
				  BYTE* p_compressed_data( void );
			const BYTE* p_compressed_data( void ) const;

			const int compressed_data_size( void ) const;

			// This will ficegive you access to the extra data that can be allocated along with a frame.
			// Obviously this requires the sender and receiver to have a common understanding of what is
			// actually stored in the data itself.
			const void* p_extra_data( void ) const;
				  void* p_extra_data( void );

			// This allows you to reduce the extra data size. Treat this with caution.
			void set_extra_data_size( const int new_data_size );

			// Get the current extra data size
			const int extra_data_size( void ) const;

			// Get access to the unique code that identifies the "extrea" data attached to this frame.
			const DWORD	 extra_data_fourCC( void ) const;
				  DWORD	&extra_data_fourCC( void );

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
				e_field_type				m_field_type;

				// That format is the data in
				e_data_format	m_data_format;

				// The resolution
				int							m_xres, m_yres;

				// The comrpessed data size if being used
				int							m_compressed_data_size;

				// The current frame-rate
				frame_rate_type				m_frame_rate;

				// The aspect ratio
				float						m_aspect_ratio;

				// The time-stamp
				__int64						m_time_stamp;

				// The default sub-rectangle
				RECT						m_sub_rect;

				// The extra data size
				int							m_extra_data_size;
				int							m_extra_data_size_max;
				DWORD						m_extra_data_fourcc;

				// The time-code
				__int64						m_time_code;

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

			// Compressed data if it is being used
			BYTE* m_p_compressed_data;
	
			// Everything is reference counted
			mutable volatile long m_ref;

			// Get the size of a message
			static const int message_size( const e_data_format format, const int xres, const int yres, const int compressed_data_size, const int extra_size );

			// Setup all pointers
			void setup_memory( const e_data_format format, const int xres, const int yres, const int compressed_data_size, const int extra_size );
			void setup_memory( void );

			// A friend
			friend receive;
			friend pull;
			friend FrameWork::Communication3::audio_video::receive;
			friend FrameWork::Communication3::audio_video_xml::receive;
};