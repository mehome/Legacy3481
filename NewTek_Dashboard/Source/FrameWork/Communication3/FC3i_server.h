#pragma once

struct server
{			// Constructor
			server( const wchar_t name[] );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;

			// Get the name
			const wchar_t *name( void ) const;

			// This will add a message to the message_queue
			bool send_message( const DWORD block_id, const DWORD addr );

			// Wait for a message (0 means time-out). If the time-out is 
			// zero we just "ping" the message_queue.
			bool get_message( const DWORD time_out, DWORD &block_id, DWORD &addr );

			// This will lock the write queue, this is used when flushing a queue
			const DWORD lock_write( void );
			void unlock_write( const DWORD lock_write_return );

			// This will abort a get message call by triggering the event. This is important
			// when trying to destoy a server
			bool abort_get_message( void );

			// Get the current instantenous queue depth
			const DWORD queue_depth( void ) const;			

			// Get and set the heart-beat.
			const __int64 heart_beat( void ) const;
			void update_heart_beat( void );

			// Reset the heard beat
			void reset_heart_beat( void );

			// Error
			const bool error( void ) const;

private:	// Destructor
			~server( void );
				
			// We have a message event and queue
			message_queue m_queue;
			message_event m_event;

			// Everything is reference counted
			mutable volatile long m_ref;

			// We maintain an allocation for the name of the object
			wchar_t *m_p_name;
};