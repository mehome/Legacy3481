#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FC3i::message
{			// Constructor
			message( const int no_samples, const int no_channels, const int extra_data_size_max = 0 );

			// Destructor
			~message( void );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;	

			// Get and set the time-stamp of the field.
			// This is stored in 10ns intervals.
			const __int64  time_stamp( void ) const;
				  __int64& time_stamp( void );

			// Get the number of audio samples and channels.
			const int no_samples( void ) const;
			const int no_channels( void ) const;

			// Get and set the audio sample rate
			const int  sample_rate( void ) const;
				  int& sample_rate( void );

			// Get access to the buffers
				  FrameWork::Audio2::buffer_f32&	audio( void );		
			const FrameWork::Audio2::buffer_f32&	audio( void ) const;

			// Use this function very very carefully.
			// The method "reduce_number_of_audio_samples" has been changed to "change_number_of_audio_samples" 
			// that allows the number of audio samples to be any number up to the original buffer allocation size. 
			// This will return a boolean that represents success in doing this. As before be careful with this. 
			// Looking at the old implementation, I am frankly not sure this has ever actually worked correctly; 
			// if using this remember that if you are going to reduce the number of audio samples, the "stride" 
			// between the channels does not change only the length of the buffer.
			bool change_number_of_audio_samples( const int new_no_audio_samples );

			// Is there an error in this message, most likely caused by a failed allocation or transmission
			bool error( void ) const;

			// This will ficegive you access to the extra data that can be allocated along with a frame.
			// Obviously this requires the sender and receiver to have a common understanding of what is
			// actually stored in the data itself.
			const void* extra_data( void ) const;
				  void* extra_data( void );

			// This allows you to reduce the extra data size. Treat this with caution.
			void set_extra_data_size( const int new_data_size );

			// Get the current extra data size
			const int extra_data_size( void ) const;

			// Get access to the unique code that identifies the "extrea" data attached to this frame.
			const DWORD	 extra_data_fourCC( void ) const;
				  DWORD	&extra_data_fourCC( void );

			// This ensures that people can pass messages between DLLs
			static void* operator new ( const size_t size );
			static void  operator delete ( void* ptr );
			static void* operator new [] ( const size_t size );
			static void  operator delete [] ( void* ptr );

			// Get the duration in 10ns intervals
			const __int64 duration( const __int64 time_base = 10000000LL ) const;
			const double  duration_in_seconds( void ) const;

protected:	// Internal use only :)
			message( const DWORD block_id, const DWORD addr );
	
private:	// This is the data header
			struct header
			{	// The sample rate
				int	m_sample_rate;

				// The sample counts
				int m_max_no_samples;
				int	m_no_samples;

				// The number of channels
				int m_no_channels;

				// The time-stamp
				__int64 m_time_stamp;

				// The size of the extra data
				int		m_extra_data_size;
				int		m_extra_data_size_max;
				DWORD	m_extra_data_fourcc;

			} *m_p_header;

			static const int header_size = fc3_size_align( sizeof( header ) );

			// This is the extra data pointer
			void*	m_p_extra_data;

			// The audio data
			FrameWork::Audio2::buffer_f32 m_audio;
			
			// Everything is reference counted
			mutable volatile long	m_ref;

			// Get the size of a message
			static const int message_size( const int no_samples, const int no_channels, const int extra_data_size );

			// Setup all pointers
			void setup_memory( const int no_samples, const int no_channels, const int extra_data_size );
			void setup_memory( void );

			// A friend
			friend receive;
			friend pull;
			friend FC3::audio_video::receive;
			friend FC3::audio_video_xml::receive;
			friend FC3i::message_slot;
};