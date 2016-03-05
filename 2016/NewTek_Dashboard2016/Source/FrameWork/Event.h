#pragma once

struct event_array;

///	This class creates an event which can optionally trigger when it is waited upon.
///	By default it's state is "Off"
struct event 
{			// Constructor
			event( const bool AutoReset=true );

			// Destructor
			~event( void );

			// Wait for the event. If the time is exceeded, it will return false;
			bool wait( const DWORD Time = INFINITE ) const;

			/// Set (and reset) the object
			void set( const bool Flag = true /* false for reset */ );

			/// Reset the object. Same as calling Set(false)
			void reset( void );

			// Assign
			void operator= ( const bool value );
			operator bool ( void ) const;

			/// This is useful for cases where you need to work directly in win32 (e.g. overlapped io) where they need the handle
			operator HANDLE (void) const;

private:	/// The event handle
			HANDLE m_event_handle;

			// A friend
			friend event_array;
};

struct event_manual : public event
{			event_manual( void ) : event( false ) {}
};


///	This class is mainly created to give a method of waiting for multiple objects.  Simply populate this class's list with all the events you want
///	to wait for, and use this wait.  You can use the WaitAll bool to wait for any or wait for all.
struct event_array 
{
			/// Add an event to the list
			void push_back( const event &new_event );

			/// Clear the list
			void clear( void );

			/// Get the number of events
			size_t size( void ) const;

			/// This will wait for multiple objects.  If the time is exceeded, it will return false;
			/// \param WaitAll if this is true, the function returns when the state of all events are signaled.
			/// If WaitAll is false the function returns when the state of any one of the events is signaled.
			/// \return >0 if an event was signaled; If WaitAll is false this will return the array index of the event which was signaled.
			//  All other cases will return -1.  
			int wait( const bool WaitAll, const DWORD Time = INFINITE );

private:	/// The list of events
			std::vector<HANDLE>	m_events;
};