#pragma once

struct FRAMEWORKCOMMUNICATION3_API receive
{			struct client
			{	// You should inherit off your own message and it can create these. You should
				// check type before delivery.
				// If this asserts, then you have probably got a race condition in setting up the vtable of this function. Ask Andrew
				// if this confuses you.
				virtual void deliver( const DWORD block_id, const DWORD addr ) { assert( false ); }
			};			

			// Set the message processing thread priority. This allows you to determine whether
			// the processing of messages is more important than average for this item. It is 
			// tempting to think that all messages are important, however I would look at this as
			// being whether you are truly timing dependant on receiving messages down to the sub
			// milli-second level. Likewise, if you are taking a high priority you must ensure that
			// nothing you do in your message processing takes any degree of CPU time. In practice
			// I think that the only higher than "default" (normal) priority message processing item
			// should be the actual output frames. Feel free, and encouraged to drop your priority
			// if you do not need millisecond level control over timing, for instance in the UI or
			// if you are just responding to very slow user events like starting and stopping streaming.
			// Treat these with care !
			void priority_idle( void );
			void priority_lowest( void );
			void priority_below_normal( void );
			void priority_normal( void );
			void priority_above_normal( void );
			void priority_highest( void );
			void priority_time_critical( void );

			// Get the queue depth. This is an instantaneous measure of queue depth and since multiple
			// threads are reading and writing from the queue you should assume it as only a general
			// guide-line of how many messages there are waiting to be processed.
			const DWORD queue_depth( void ) const;

			// This will flush the server queue
			void flush_queue( void );

			// Get the server name
			const wchar_t* name( void ) const;

protected:	// Start and stop the server. Pull servers specify client as NULL
			const bool start( const wchar_t name[], client *p_client, const bool flush_queue, const DWORD stack_size = 16 * 1024 );
			const bool stop( void );

			// This is only used by pull servers and allows you to poll for frames with a time-out
			const bool pull( const DWORD timeout, DWORD& block_id, DWORD& addr );

			// Constructor and destructor
			 receive( void );
			~receive( void );
			
private:	// The thread proc
			static DWORD WINAPI threadproc( void* lpParameter );
			
			// We maintain a thread that we deliver messages on.
			HANDLE	m_h_thread;
			DWORD	m_thread_id;

			// Should we exit
			volatile bool m_should_exit;

			// This is the server that we are receiving messages on.
			server *m_p_server;

			// The client that we will be receiving messages from
			client *m_p_client;			
};