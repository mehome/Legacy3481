#pragma once

struct message_queue : private mapped_file
{			// Constructor
			message_queue( const wchar_t name[] );

			// Destructor
			~message_queue( void );

			// This will add a message to the message_queue
			bool push( const DWORD block_id, const DWORD addr );

			// This will lock the write queue, this is used when flushing a queue
			const DWORD lock_write( void );
			void unlock_write( const DWORD lock_write_return );

			// Wait for a message (0 means time-out). If the time-out is 
			// zero we just "ping" the message_queue.
			bool pop( DWORD &block_id, DWORD &addr );

			// Get the message_queue name
			const wchar_t *name( void ) const;

			// Get the current instantenous queue depth
			const DWORD queue_depth( void ) const;

			// Error
			const bool error( void ) const;

			// Get and set the heart-beat.
			const __int64 heart_beat( void ) const;
			void update_heart_beat( void );

			// This is only called when the object is closed to 
			// reset the heart-beat to zero.
			void reset_heart_beat( void );

private:	// The header
			struct header
			{	volatile DWORD		m_read_posn;
				volatile DWORD		m_write_posn;
				volatile DWORD		m_queue_depth;
				volatile LONGLONG	m_running;
			};

			static const int header_size = fc3_size_align( sizeof( header ) );
			static const int message_queue_size = fc3_size_align( sizeof( LONGLONG ) * FrameWork::Communication3::config::message_queue_length );

			// The number of pools in this process
			static DWORD g_debug_no_objects;

			// The header in the memory map
			header *m_p_header;

			// This is the list of DWORD pairs.
			volatile LONGLONG *m_p_msg_message_queue;
};