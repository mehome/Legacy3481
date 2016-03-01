#pragma once

struct FRAMEWORKCOMMUNICATION3_API message : public FC3i::message
{			// Constructor
			message( const std::pair< DWORD, DWORD > sizes );

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

			// Display this on output
			const wchar_t* category( void ) const;
				  wchar_t* category( void );

			const wchar_t* content( void ) const;
				  wchar_t* content( void );

			// This ensures that people can pass messages between DLLs
			static void* operator new ( const size_t size );
			static void  operator delete ( void* ptr );
			static void* operator new [] ( const size_t size );
			static void  operator delete [] ( void* ptr );

protected:	// Internal use only :)
			message( const DWORD block_id, const DWORD addr );
	
private:	// Everything is reference counted
			mutable volatile long	m_ref;

			// This is the data header
			struct header
			{	// The sizes
				DWORD	m_category_size;
				DWORD	m_message_size;

			} *m_p_header;

			// The data elements
			wchar_t *m_p_category, *m_p_message;

			static const int header_size = fc3_size_align( sizeof( header ) );

			// A friend
			friend receive;
			friend pull;
			friend FC3i::message_slot;
};