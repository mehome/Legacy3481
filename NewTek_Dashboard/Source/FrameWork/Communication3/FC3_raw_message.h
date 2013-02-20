#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FrameWork::Communication3::implementation::message
{			// Constructor
			message( const DWORD size_in_bytes );
			message( const BYTE* p_data, const DWORD size_in_bytes );

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

			// Cast this to a string
			operator const char* ( void ) const;
			operator	   char* ( void );

			const char* data( void ) const;
				  char* data( void );

			// Get the data size of this element in bytes
			const DWORD size( void ) const;

			// This ensures that people can pass messages between DLLs
			static void* operator new ( const size_t size );
			static void  operator delete ( void* ptr );
			static void* operator new [] ( const size_t size );
			static void  operator delete [] ( void* ptr );

protected:	// Internal use only :)
			message( const DWORD block_id, const DWORD addr );
	
private:	// Everything is reference counted
			mutable volatile long	m_ref;

			// A friend
			friend receive;
			friend pull;
			friend FrameWork::Communication3::audio_video_xml::receive;
};