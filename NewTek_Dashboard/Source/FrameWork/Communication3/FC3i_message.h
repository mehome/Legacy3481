#pragma once

struct FRAMEWORKCOMMUNICATION3_API message
{			// This will send a message. If the destination is not responding fast enough to requests, this will return false. 
			// In practice the destination needs to be over 500 messages behind the server in order for a message delivery
			// to fail; in which case it is fair to assume that something is going to need to be dropped.
			// There is no longer an async flag, all messages are sent asyncronously without the issues that existed in the previous 
			// API. 
				bool send( const wchar_t destination[] ) const;

			// Outstanding send support
				// Set the outstanding sents trigger
				void set_outstanding_sends_trigger( trigger& from );

				// Is there an outstanding sends trigger ?
				const bool has_outstanding_sends_trigger( void ) const;

				// This allows you to remove a sends trigger from a frame. This was not previously available.
				void reset_outstanding_sends_trigger( void );

				// Get the number of outstanding sends for this frame
				const long no_outstanding_sends( void ) const;

			// Displayed support
				// Set the display trigger
				// The display granularity determines how often we trigget the actual displayed callback is called.
				// For instance if you are sending a interleaved frame to the switcher and want the switcher to 
				// tell you every time each field is displayed, set it to 1 (every time it is dislayed it gets 
				// triggered). If instead you only want to know every time the entire frame has been shown you
				// can set it to 2. Setting it to 0xffffffff has the impact of making it only be triggered a single
				// time.
				// For technical clarity, the decision on when to trigger is 
				// if ( ( no_times_displayed_displayed % display_granularity ) == 1 ) 
				void set_displayed_trigger( trigger& from, const DWORD display_granularity = (DWORD)-1 ) const;

				// This is a new method that allows you to "remove" a trigger from a frame. This was not previously
				// available.
				void reset_displayed_trigger( void );

				// Mark this frame displayed
				void displayed( void ) const;

				// Is there a displayed trigger
				const bool has_displayed_trigger( void ) const;

				// Reset the displayed count. This is a new method that allows someone who wants to resend the same
				// frame many times (e.g. Remote VGA that updates a rectangle of each frame) to still be able to 
				// reset a displayed could so that it can track specific displayed numbers even 
				void reset_displayed_count( void );				

				// Get the displayed count
				const long displayed_count( void ) const;	

			// Remote access. This will tell you whether the frame was originall sent from a remote source or not
				const bool was_sent_from_remote_source( void ) const;

protected:	// Data access
				// Get the data size of this message
				const DWORD size( void ) const;

				// Is there an error
				const bool error( void ) const;
				
				// Get the data pointed to by this message
				const BYTE* operator() ( const DWORD i = 0 ) const;
					  BYTE* operator() ( const DWORD i = 0 );

				const BYTE* ptr( const DWORD i = 0 ) const;
					  BYTE* ptr( const DWORD i = 0 );

				// Get access to the raw pointer
				const BYTE*	ptr_raw( void ) const;
					  BYTE*	ptr_raw( void );

			// Message types
				typedef DWORD message_type;
				static const message_type message_type_error  = 0;		// Do not use this !
				static const message_type message_type_raw    = 1;		// The message type is not yet known.
				static const message_type message_type_xml    = 2;		// A XML string
				static const message_type message_type_video  = 3;		// A video frame paclet
				static const message_type message_type_audio  = 4;		// A video frame

				static const message_type message_type_debug  = 5;		// A debug message

				static const message_type message_type_custom = 10;		// Here on up is open range !			

				// Get and set the message type
					  message_type& type( void );
				const message_type  type( void ) const;
					  
			// Constructor
			message( const DWORD size );

			// Constructor on the receive side
			message( const DWORD block_id, const DWORD addr );
	
			// Destructor
			~message( void );

			// If you need to create messages of different classes to determine
			// what a message is, only the first would handle the outstanding sends
			// properly. This offers a work-around for this.
			void simulate_send( void );

			// Set this message as being from the network
			void set_from_network( const DWORD size /* Just in case people did not populate this right */ );
	
private:	// Setup of the item
			bool setup( memory_block* p_block, BYTE *p_ptr, const DWORD size );
			bool setup( memory_block* p_block, BYTE *p_ptr );			

			// Get allocation size
			static const DWORD allocation_size( const DWORD size );

			// Clear all data
			void clear( void );			

			// The block header
			struct header
			{	// This is a cookie that is non zero and is used to be sure we have found a valid message
				message_type m_type;				
				
				// The size of this block, excluding the header
				DWORD	m_size;				

				// The approximate, short crashes(!), references to this message.
				DWORD	m_ref;

				// The outstanding sends
				DWORD	m_no_outstanding_sends;
				DWORD	m_trigger_outstanding_sends;				

				// The displayed frame support.
				DWORD	m_no_displayed;
				DWORD	m_trigger_displayed;
				DWORD	m_trigger_granularity;

				// This tells you whether the message was sent from a remote source or not.
				DWORD	m_from_network;
			};

protected:	static const int header_size = fc3_size_align( sizeof( header ) );
private:
			// We have a pointer back to the block that allocated me, which
			// we have an outstanding reference count upon
			memory_block* m_p_block;

			// Is this the original message allocation ?
			bool	m_msg_source;

			// This is the data pointer
			BYTE*	m_p_data;
			header*	m_p_header;

			// A friend
			friend memory_cache;
};