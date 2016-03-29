#pragma once

struct FRAMEWORKCOMMUNICATION3_API mapped_file
{			// Constructor
			mapped_file( const wchar_t filename[], const DWORD size, const bool force_page_mapped = true, const bool must_exist = false );

			// Destructor
			~mapped_file( void );
			
			// Get the size (0 means error)
			const DWORD size( void ) const;			

			// Was there an error
			const bool error( void ) const;

			// Get a pointer with index
				  BYTE* ptr( const DWORD addr );
			const BYTE* ptr( const DWORD addr ) const;

				  BYTE* ptr( void );
			const BYTE* ptr( void ) const;

			// Get an address from a pointer
				  DWORD addr( const BYTE* ptr );
			const DWORD addr( const BYTE* ptr ) const;

protected:	// When using inheritance, you can avoid an extra new
			mapped_file( void );
			bool setup( const wchar_t filename[], const DWORD size, const bool force_page_mapped, const bool must_exist );

private:	// The memory map handle
			HANDLE	m_mapping_handle;

			// The memory pointers
			BYTE* m_p_memory;
			BYTE* m_p_memory_end;
};