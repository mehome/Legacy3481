#pragma once

struct trigger_event : public event
{			// Constructor
			trigger_event( const DWORD trigger_id );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;

			// Get the trigger ID
			const DWORD trigger_id( void ) const;

protected:	// Get the handle
			HANDLE get_handle( void ) const;

private:	// Destructor
			~trigger_event( void );			

			// The number of pools in this process
			static DWORD g_debug_no_objects;

			// The reference count
			volatile mutable long m_ref;

			// Store the current ID
			DWORD m_trigger_id;			

			// A friend
			friend get_event_handle;
			friend trigger;
};