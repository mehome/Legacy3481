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
			const bool send_message( const DWORD block_id, const DWORD addr, const DWORD time_out );

			// Would a send succeed
			const bool would_send_message_succeed( const DWORD time_out );

			// Wait for a message (0 means time-out). If the time-out is 
			// zero we just "ping" the message_queue.
			const bool get_message( DWORD &block_id, DWORD &addr, const DWORD time_out );

			// Get the current instantenous queue depth
			const DWORD queue_depth( void ) const;

			// This will lock the write queue, this is used when flushing a queue.
			// This must be handled with great care.
			const DWORD lock_write( void );
			void unlock_write( const DWORD prev_lock_posn );

			// Error
			const bool error( void ) const;

			// This will wait and determine whether the server is alive
			const bool is_server_alive( const DWORD timeout = 0 );

private:	// Destructor
			~server( void );
				
			// We have a message event and queue
			message_queue m_queue;

			// Everything is reference counted
			mutable volatile long m_ref;

			// We maintain an event that can be used to wait and determine whether the
			// server is actually running. 
			HANDLE	m_hServerAlive;

			// We maintain an allocation for the name of the object
			wchar_t *m_p_name;
};