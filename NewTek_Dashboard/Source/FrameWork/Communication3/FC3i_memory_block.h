#pragma once

#ifdef	FC3_VERSION_3_5

#include "FC3i_memory_block_35.h"

#else	FC3_VERSION_3_5

struct memory_block : public mapped_file
{			// Constructor
			memory_block( const DWORD block_id, const bool must_exist = false );

			// Reference counting
			const long addref( void ) const;
			const long release( void ) const;

			// This will return the current reference count. Obviously there are thread safety
			// issues relating to it's use.
			const long refcount( void ) const;

			// This will allocate memory from the block, NULL if the allocation is not possible
			// NULL means error. This is an offset into the memory block, this can then be used to
			// get an actual pointer from the mapped_file class.
			BYTE* malloc( const DWORD size );

			// This is a way to signal the block that memory has been released. If you fail to do this
			// The memory does end up being released when all handles are discared, but it is less ewfficient.
			void free( const BYTE* p_mem );

			// Get the ID
			const DWORD block_id( void ) const;

			// This will time-stamp the current block
			void time_stamp( void ) const;

			// Are we old enough that this item could reasonably be removed
			bool time_stamp_is_old( void ) const;
			
private:	// Destructor
			~memory_block( void );			

private:	// This will try to recover an item from the linked list of free blocks
			BYTE* recycle( const DWORD block_sz );
	
			// The block id
			DWORD m_block_id;
			
			// Everything is reference counted
			mutable volatile long	m_ref;

			// Each block allocated has a small header attached to it.
			struct alloc_header
			{	// This represents the actual block number that this is part of
				DWORD	m_size_idx;

				// When free, this represents the next item in the linked list.
				DWORD	m_next_free;
			};

			static const int alloc_header_size = fc3_size_align( sizeof( alloc_header ) );			

			// This is the block header
			struct header
			{	// This represents a time-stamp of the last time that a message was sent
				// from this buffer. This helps ensure that messages do not get dropped by
				// keeping the buffer around for an ammount of extra time that can be quite 
				// small. This is in addition to the extra pools in the cache
				mutable
				DWORD m_time_stamp;
				
				// We have a list of stacks of unused buffers which we use to re-cycle buffers as 
				// they are returned if possible.
				DWORD m_free_mem[ 64 ];
				
				// The current memory allocation position
				DWORD m_stack_position;				

			}	*m_p_header;

public:		static const int header_size = fc3_size_align( sizeof( header ) );

			// The set of block sizes
private:	static const DWORD memory_block::g_block_sizes[ 64 ];

			// The number of pools in this process
			static DWORD g_debug_no_objects;

			// This uses the current block list to generate a size and index for the size number
			static const std::pair< DWORD, DWORD > alloc_size( const DWORD raw_size );

			// Get linked list items
			static const DWORD LO_DWORD( const LONGLONG x ) { return ( (DWORD*)&x )[ 0 ]; }
			static const DWORD HI_DWORD( const LONGLONG x ) { return ( (DWORD*)&x )[ 1 ]; }
			static const LONGLONG MAKE_LONGLONG( const DWORD lo, const DWORD hi ) { DWORD ret[ 2 ] = { lo, hi }; return *(LONGLONG*)ret; }
};

#endif	FC3_VERSION_3_5