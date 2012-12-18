#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FrameWork::Communication3::implementation::message
{			// Constructor
			message( const wchar_t xml_data[] );
			message( const char xml_data[] );
			message( const DWORD size_in_bytes );
			message( const FrameWork::xml::tree  &xml_tree );
			message( const FrameWork::xml::node2 &xml_tree );

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

			// This ensures that people can pass messages between DLLs
			static void* operator new ( const size_t size );
			static void  operator delete ( void* ptr );
			static void* operator new [] ( const size_t size );
			static void  operator delete [] ( void* ptr );

protected:	// Internal use only :)
			message( const DWORD block_id, const DWORD addr );
	
private:	// Everything is reference counted
			mutable volatile long	m_ref;

			// For debugging purposes
			union
			{	const wchar_t	*m_p_debugW;
				const char		*m_p_debugA;
			};

			// A friend
			friend receive;
			friend pull;
			friend FrameWork::Communication3::audio_video_xml::receive;
};