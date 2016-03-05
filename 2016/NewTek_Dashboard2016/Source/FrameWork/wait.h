#pragma once

template< const int max_do_not_wait_items > struct multiwait;

struct wait
{			// Constructor
			__forceinline wait( void ) : m_p_wait_item( NULL )	{}
			__forceinline wait( wait &from ) : m_p_wait_item( from.m_p_wait_item ) { from.m_p_wait_item = NULL; }
			__forceinline wait( wait_item *p_wait_item ) : m_p_wait_item( p_wait_item ) {}

			// Destructor
			__forceinline ~wait( void ) { if ( m_p_wait_item ) operator()(); }			

			// Wait with a time-out
			bool operator() ( const DWORD time_out = INFINITE );

			// Assignment
			__forceinline void operator= ( wait &from ) { if ( m_p_wait_item ) (*this)(); m_p_wait_item = from.m_p_wait_item; from.m_p_wait_item = NULL; }
			__forceinline void operator= ( wait_item *p_wait_item ) { if ( m_p_wait_item ) (*this)(); m_p_wait_item = p_wait_item; }

private:	// The wait item to use
			wait_item *m_p_wait_item;

			template< const int max_do_not_wait_items > 
			friend struct multiwait;
};