#pragma once

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
			BYTE* malloc( const DWORD size, const bool blocking = true );

			// This is a way to signal the block that memory has been released. If you fail to do this
			// The memory does end up being released when all handles are discared, but it is less ewfficient.
			void free( const BYTE* p_mem );

			// Get the ID
			const DWORD block_id( void ) const;

private:	// Destructor
			~memory_block( void );	

			// Lock the pool
			const bool lock( const bool blocking );
			void unlock( void );

			// The block id
			DWORD m_block_id;

			// The memory allocator object
			memory_allocator	*m_p_allocator;

			// This is used for efficient locking of the allocator
			HANDLE m_lock_handle;
			
			// Everything is reference counted
			mutable volatile long m_ref;

			// The number of pools in this process
			static DWORD g_debug_no_objects;
};