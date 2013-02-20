#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FrameWork::Communication3::implementation::message
{			// Constructor
			message( const wchar_t xml_data[], const DWORD extra_data_size = 0 );
			message( const char xml_data[], const DWORD extra_data_size = 0 );
			message( const DWORD size_in_bytes );
			message( const FrameWork::xml::tree  &xml_tree, const DWORD extra_data_size = 0 );
			message( const FrameWork::xml::node2 &xml_tree, const DWORD extra_data_size = 0 );

			// Destructor
			~message( void );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;

			// Is there an error in this message, most likely caused by a failed allocation or transmission
			bool error( void ) const;

			// Parse this message
			const bool parse( FrameWork::xml::node *p_node ) const;

			// Cast this to a string
			operator const char* ( void ) const;
			operator	   char* ( void );

			operator const wchar_t* ( void ) const;
			operator	   wchar_t* ( void );

			// Get the data size of this element in bytes
			const DWORD size( void ) const;

			// This ensures that people can pass messages between DLLs
			static void* operator new ( const size_t size );
			static void  operator delete ( void* ptr );
			static void* operator new [] ( const size_t size );
			static void  operator delete [] ( void* ptr );

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

protected:	// Internal use only :)
			message( const DWORD block_id, const DWORD addr );
	
private:	// Everything is reference counted
			mutable volatile long	m_ref;

			// The header
			struct	header
			{	// The extra data size
				int				m_extra_data_size;
				int				m_extra_data_size_max;
				DWORD			m_extra_data_fourcc;				

			}		*m_p_header;			

			// For debugging purposes
			union
			{	wchar_t	*m_p_data_W;
				char	*m_p_data_A;
			};

			BYTE	*m_p_extra_data;

			// A friend
			friend receive;
			friend pull;
			friend FrameWork::Communication3::audio_video_xml::receive;
};