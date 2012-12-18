#pragma once

struct FRAMEWORKCOMMUNICATION3_API trigger
{			// Constructor
			trigger( void );

			// Destructor
			~trigger( void );			

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;			

			// Wait for the event. It is highly unrecommented to use an infinite time-out to
			// ensure total reliability. Remember that the module that you are waiting on might
			// crash which we never want to lock up the person waiting on it.
			const bool wait( const DWORD timeout = 2000 ) const;

protected:	// Get the handle
			HANDLE get_handle( void ) const;

private:	// Constructor when referenting another ID, this is done by messages.
			trigger( const DWORD id );
			
			// Setup the item (called by the cache)
			bool setup( trigger_event *p_event );

			// Set and reset the event
			void set( const bool flag = true );

			// Get the trigger ID
			const DWORD trigger_id( void ) const;			
	
			// The ID of this trigger
			trigger_event *m_p_event;

			// The reference count
			volatile mutable long m_ref;

			// This is who sets me up
			friend trigger_cache;
			friend message;
			friend get_event_handle;
};