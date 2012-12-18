#pragma once

// Implementation to scale a buffer
template<const typename src_type> 
__forceinline void scale( src_type* __restrict p_buffer, const int width, double scale_factor ) 
{ 
	for (size_t i=0;i<width;i++)
		p_buffer[i]*=scale_factor;   //not optimized
}

template<> void FRAMEWORKAUDIO2_API scale( float* __restrict p_buffer, const int width, double scale_factor );		// Optimized

template< typename sample_type >
struct buffer
{			// Type definitions
			typedef typename sample_type sample_type;

			// Constructor
			buffer( const int no_channels, const int no_samples, const int align = 0 );
			buffer(		  sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes = 0 );
			buffer( const sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes = 0 );
			buffer( void );

			// Copy constructors
			template< typename sample_type_from >
			explicit buffer( const buffer<sample_type_from>& from ) { m_data = from.m_data; }

			// Copy from an identical buffer
			explicit buffer( const buffer& from );

			// Resize the buffer
			bool resize_in_bytes( const int no_channels, const int no_samples, const int align = 16, const int stride_in_bytes = 0 );
			bool resize         ( const int no_channels, const int no_samples, const int align = 16, const int stride_in_bytes = 0 );

			// Free memory
			void clear( void );

			// Reference buffers
			void reference_in_bytes(	   sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes = 0 );
			void reference_in_bytes( const sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes = 0 );

			void reference(		  sample_type *p_samples, const int no_channels, const int no_samples, const int stride = 0 );
			void reference( const sample_type *p_samples, const int no_channels, const int no_samples, const int stride = 0 );
			void reference(       buffer &from );
			void reference( const buffer &from );

			// Get the resolution
			int no_samples( void ) const;
			int no_samples_in_bytes( void ) const;
			int no_channels( void ) const;

			// Get the size
			const size_t size( void ) const;

			// Is this buffer empty
			const bool empty( void ) const;

			// Are we the same size as anotber buffer
			template< typename other_buffer >
			bool is_same_size_as( const other_buffer &with ) const { return ((no_samples()==with.no_samples())&&(no_channels()==with.no_channels())); }

			// Do we follow a particular alignment (must be power of two)
			bool is_aligned( const int alignment ) const;

			// Get the stride
			int stride( void ) const;
			int stride_in_bytes( void ) const;		

			// Copy from
			template< typename pixel_type_from >
			void operator=( const buffer<pixel_type_from>& from )	{ FrameWork::Bitmaps::bitmap<pixel_type_from> from_bmp; from_bmp.reference_in_bytes( from(), from.no_samples(), from.no_channels(), from.stride_in_bytes() ); m_data = from_bmp; }

			// Copy from
			void operator=( const buffer& from );

			// Set
			void operator=( const sample_type& x );

			// Scale the current buffer by a new value coefficient
			inline void operator*= (const double scale_factor);

			// Get access to this as a bitmap
			operator const FrameWork::Bitmaps::bitmap< sample_type >& ( void ) { return m_data; }
			operator       FrameWork::Bitmaps::bitmap< sample_type >& ( void ) { return m_data; }

			// Swap with
			void swap_with( buffer& other );
			void swap_to( buffer& other );

			// Memory access
				  sample_type*	operator() ( void );
			const sample_type*	operator() ( void ) const;

				  sample_type*	operator() ( const int channel_no );
			const sample_type*	operator() ( const int channel_no ) const;

				  sample_type&	operator() ( const int channel_no, const int sample_no );
			const sample_type&	operator() ( const int channel_no, const int sample_no ) const;

private:	// Internal memory
			FrameWork::Bitmaps::bitmap< sample_type >	m_data;
};	