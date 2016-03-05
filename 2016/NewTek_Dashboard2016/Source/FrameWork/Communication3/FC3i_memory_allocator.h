#pragma once

struct  memory_allocator
{		// Constructor
		memory_allocator( void* p_mem, const DWORD size );

		// Allocator
		void*   malloc( const DWORD size );
		void	free( void* p_ptr );

		// Get the locking element
		volatile LONG* lock( void );

		// This will initialize the object if it is not already. 
		void	init( void );

#ifdef	_DEBUG
		// This is used by a test-bed for checking memory allocations. It 
		// checks that there is a single free block of all original memory.
		void	check_all_free( void );
#endif	_DEBUG

private:// This represents the structure of the memory inside of the memory pool
		struct  data_segment
		{	// The number of block sizes to have as a hash for free blocks
			static const int		no_free_pools		= 22;
			static const int		maximum_wasteage	= 32;	// = 128 bytes
			static const int		header_size			= 1/*m_data*/+1/*m_data_size*/+no_free_pools/*m_free_pool*/;

			// Most likely used for locking the allocator
			DWORD   m_data[ 1 ];

			// The total size of the pool we are working out (the last element)
			DWORD   m_data_size;

			// The set of linked list in the free pools
			DWORD   m_free_pool[ no_free_pools ];

			// Initialize the list
			void init( const DWORD size );

			// Get which pool a block size will fall into
			DWORD free_pool_idx( const DWORD sz4 );

			// Add an item to the free list
			void add_free( const DWORD curr_hdr, const DWORD sz4 );

			// Remove an item from the free list
			void remove_free( const DWORD curr_hdr );

			// Perform a memory allocation
			void* malloc( const DWORD size );

			// Free a block
			void free( void* p_ptr );

			// Debugging check
#ifdef	_DEBUG
			void debug_check( const DWORD curr_hdr );
			void check_all_free( void );
#else	_DEBUG
			__forceinline void debug_check( const DWORD curr_hdr ) {}
#endif	_DEBUG

		}	*m_p_data;

		// The size of the obkect
		DWORD	m_size;
};