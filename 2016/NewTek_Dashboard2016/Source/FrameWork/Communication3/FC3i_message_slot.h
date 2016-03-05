#pragma once

struct message_slot : private mapped_file
{			// Constructor
			message_slot( const wchar_t name[] );

			// Destructor
			~message_slot( void );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;

			// Was there an error
			const bool error( void ) const;

			// Recover a message from a particular slot
			template< typename ret_message >
			ret_message* get( const int slot_no )
			{	// Debugging
				assert( (slot_no>=0) && (slot_no<config::slots_items_per_group) );
				if ( ( slot_no < 0 ) || ( slot_no >= config::slots_items_per_group ) ) return NULL;

				// Get the current message lock
				spinhelp spin;	
				LONGLONG cur_message;
				while( ( cur_message = _InterlockedExchange64( &m_p_header->m_slots[ slot_no ], -1 ) ) == -1 ) spin++;

				// No message means do nothing
				if ( !cur_message )
				{	// Nothing on the queue !
					_InterlockedExchange64( &m_p_header->m_slots[ slot_no ], 0LL );

					// Nothing to do.
					return NULL;
				}

				// Get the message
				ret_message* p_ret_msg = new ret_message( (DWORD)( cur_message >> 32 ), (DWORD)( cur_message ) );

				// Now replace the existing queue item
				_InterlockedExchange64( &m_p_header->m_slots[ slot_no ], cur_message );
				
				// We need to simulate a send message since it will be held on the queue
				p_ret_msg->simulate_send();

				// Return the previous message
				return p_ret_msg;
			}

			template< typename ret_message, typename new_message >
			ret_message* replace( const int slot_no, new_message* p_new_item )
			{	// Debugging
				assert( (slot_no>=0) && (slot_no<config::slots_items_per_group) );
				if ( ( slot_no < 0 ) || ( slot_no >= config::slots_items_per_group ) ) return NULL;

				// Get the current message lock
				spinhelp spin;	
				LONGLONG cur_message;
				while( ( cur_message = _InterlockedExchange64( &m_p_header->m_slots[ slot_no ], -1 ) ) == -1 ) spin++;

				// Get the message
				ret_message* p_ret_msg = cur_message ? new ret_message( (DWORD)( cur_message >> 32 ), (DWORD)( cur_message ) ) : NULL;

				// If we are replacing this with a new message type
				if ( p_new_item )
				{	// Now replace the existing queue item
					_InterlockedExchange64( &m_p_header->m_slots[ slot_no ], p_new_item->addr_64() );
				
					// We need to simulate a send message since it will be held on the queue
					p_new_item->simulate_send();
				}
				else
				{	// Nothing on the queue !
					_InterlockedExchange64( &m_p_header->m_slots[ slot_no ], 0LL );
				}

				// Return the previous message
				return p_ret_msg;
			}

			template< typename new_message >
			const bool put( const int slot_no, new_message* p_new_item )
			{	// Debugging
				assert( (slot_no>=0) && (slot_no<config::slots_items_per_group) );
				if ( ( slot_no < 0 ) || ( slot_no >= config::slots_items_per_group ) ) return false;
				
				// Place it
				delete replace< new_message, new_message >( slot_no, p_new_item );

				// Success
				return true;
			}

			// Get the name of this slot
			const wchar_t* name( void ) const;

private:	// The header
			struct header
			{	volatile LONG		m_ref_count;
				volatile LONGLONG	m_slots[ config::slots_items_per_group ];
			};

			static const int header_size = fc3_size_align( sizeof( header ) );

			// The header in the memory map
			header *m_p_header;

			// The name of this item
			std::wstring m_name;

			// Everything is reference counted
			mutable volatile long m_ref;

			// The number of pools in this process
			static DWORD g_debug_no_objects;
};