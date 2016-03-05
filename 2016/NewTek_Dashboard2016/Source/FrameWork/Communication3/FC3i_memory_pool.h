//#pragma once

template< const int size >
struct memory_pool
{			// Constructor
			memory_pool( void );

			// Destructor
			~memory_pool( void );
	
			// Get the item of this size
			void*	malloc( void );
			void	free( void* p_data );

private:	// Items are stored in a linked list
			SLIST_HEADER*	m_p_header;

#ifdef	_DEBUG
			volatile LONG	m_count;
#endif	_DEBUG
};


// Constructor
template< const int size >
memory_pool< size >::memory_pool( void )
	:	m_p_header( (SLIST_HEADER*)::_aligned_malloc( sizeof( SLIST_HEADER ), MEMORY_ALLOCATION_ALIGNMENT ) )
#ifdef	_DEBUG
		,m_count( 0 )
#endif	_DEBUG
{	// Initialize the SList
	::InitializeSListHead( m_p_header );
}

// Destructor
template< const int size >
memory_pool< size >::~memory_pool( void )
{	// Free all blocks
	while( true )
	{	// Get an entry off the list
		SLIST_ENTRY* p_ret = ::InterlockedPopEntrySList( m_p_header );
		if ( !p_ret ) break;

		// Free the item
		::_aligned_free( p_ret );
	}

	// Free the header
	::_aligned_free( m_p_header );
}

// Get the item of this size
template< const int size >
void*	memory_pool< size >::malloc( void )
{	// Debugging
#ifdef	_DEBUG
	::InterlockedIncrement( &m_count );
#endif	_DEBUG
	
	// Get an entry off the list
	SLIST_ENTRY*	p_ret = ::InterlockedPopEntrySList( m_p_header );

	// If there was an item, use it
	return p_ret ? p_ret : ::_aligned_malloc( std::max<int>( size, (int)sizeof( SLIST_ENTRY ) ), MEMORY_ALLOCATION_ALIGNMENT );
}

template< const int size >
void	memory_pool< size >::free( void* p_mem )
{	// Debugging
#ifdef	_DEBUG
	::InterlockedDecrement( &m_count );
#endif	_DEBUG
	
	// Add the item to the list
	::InterlockedPushEntrySList( m_p_header, (SLIST_ENTRY*)p_mem );
}