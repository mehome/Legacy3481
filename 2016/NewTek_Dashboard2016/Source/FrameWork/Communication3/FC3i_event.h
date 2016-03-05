#pragma once

struct get_event_handle
{			template< typename event_type >
			get_event_handle( const event_type &x ) : m_handle( x.get_handle() ) {}

			operator HANDLE ( void )		{ return m_handle; }
			operator HANDLE ( void ) const	{ return m_handle; }
			
private:	// The internal data
			HANDLE	 m_handle;
};

struct event
{			// Set and reset the event
			void set( const bool flag = true );

			// Wait for the event
			const bool wait( const DWORD timeout = INFINITE ) const;

			// Was there an error
			const bool error( void ) const;			

protected:	// Constructor
			event( const wchar_t name[] );
			
			// The constructor to set up the item specifically
			event( void );
			void setup( const wchar_t name[] );

			// Destructor
			~event( void );

			// Get the handle
			HANDLE get_handle( void ) const;

private:	// The handle
			HANDLE m_handle;			

			// A friend
			friend get_event_handle;
};