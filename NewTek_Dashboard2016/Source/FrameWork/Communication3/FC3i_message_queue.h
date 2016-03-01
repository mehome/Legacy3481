#pragma once

struct message_queue : private mapped_file
{			// Constructor
			message_queue( const wchar_t name[] );

			// Destructor
			~message_queue( void );

			// This will flush the queue and is considered a very dangerous function !
			// Since it can lock other processes.
			void flush_queue( const DWORD max_queue_depth );

			// This will add a message to the message_queue
			const bool push( const DWORD block_id, const DWORD addr, const DWORD time_out );

			// Would a send succeed
			const bool would_push_succeed( const DWORD time_out );

			// Wait for a message (0 means time-out). If the time-out is 
			// zero we just "ping" the message_queue.
			const bool pop( DWORD &block_id, DWORD &addr, const DWORD time_out );

			// Get the message_queue name
			const wchar_t *name( void ) const;

			// Get the current instantenous queue depth
			const DWORD queue_depth( void ) const;

			// This will lock the write queue, this is used when flushing a queue
			const DWORD lock_write( void );
			void unlock_write( const DWORD prev_lock_posn );

			// Error
			const bool error( void ) const;

private:	// The header
			struct header
			{	volatile LONG		m_read_posn;
				volatile LONG		m_write_posn;
				volatile LONG		m_queue_depth;
				volatile LONG		m_write_lock;
			};

			static const int header_size = fc3_size_align( sizeof( header ) );
			static const int message_queue_size = fc3_size_align( sizeof( LONGLONG ) * FC3::config::message_queue_length );

			// We maintain two semaphores, one to signal that the queue is empty,
			// and one to maintain that the queue is full.
			HANDLE	m_h_queue_empty;
			HANDLE	m_h_queue_full;

			// The number of pools in this process
			static DWORD g_debug_no_objects;

			// The header in the memory map
			header *m_p_header;

			// This is the list of DWORD pairs.
			volatile LONGLONG *m_p_msg_message_queue;
};