#pragma once

// Constructor
template< typename sample_type >
buffer<sample_type>::buffer( const int no_channels, const int no_samples, const int align )
	: m_data( no_samples, no_channels, align )
{
}

template< typename sample_type >
buffer<sample_type>::buffer(	sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes )
{
	m_data.reference_in_bytes( p_samples, no_samples, no_channels, stride_in_bytes );
}

template< typename sample_type >
buffer<sample_type>::buffer( const sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes )
{
	m_data.reference_in_bytes( p_samples, no_samples, no_channels, stride_in_bytes );
}

template< typename sample_type >
buffer<sample_type>::buffer( void )
{
}

template< typename sample_type >
buffer<sample_type>::buffer( const buffer& from )
{	resize( from.no_channels(), from.no_samples() );
	m_data = from.m_data;
}

// Is this buffer empty
template< typename sample_type >
const bool buffer<sample_type>::empty( void ) const
{
	return ( no_samples() == true );
}

// Resize the buffer
template< typename sample_type >
bool buffer<sample_type>::resize_in_bytes( const int no_channels, const int no_samples, const int align, const int stride_in_byte )
{
	return m_data.resize_in_bytes( no_samples, no_channels, align, stride_in_bytes );
}

template< typename sample_type >
bool buffer<sample_type>::resize( const int no_channels, const int no_samples, const int align, const int stride_in_bytes )
{
	return m_data.resize( no_samples, no_channels, align, stride_in_bytes );
}

// Free memory
template< typename sample_type >
void buffer<sample_type>::clear( void )
{
	m_data.clear();
}

// Reference buffers
template< typename sample_type >
void buffer<sample_type>::reference_in_bytes( sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes )
{
	m_data.reference_in_bytes( p_samples, no_samples, no_channels, stride_in_bytes );
}

template< typename sample_type >
void buffer<sample_type>::reference_in_bytes( const sample_type *p_samples, const int no_channels, const int no_samples, const int stride_in_bytes )
{
	m_data.reference_in_bytes( p_samples, no_samples, no_channels, stride_in_bytes );
}

template< typename sample_type >
void buffer<sample_type>::reference( sample_type *p_samples, const int no_channels, const int no_samples,  const int stride )
{
	m_data.reference_in_bytes( p_samples, no_samples, no_channels, stride );
}

template< typename sample_type >
void buffer<sample_type>::reference( const sample_type *p_samples, const int no_channels, const int no_samples, const int stride )
{
	m_data.reference_in_bytes( p_samples, no_samples, no_channels, stride );
}

template< typename sample_type >
void buffer<sample_type>::reference( buffer &from )
{
	m_data.reference( from.m_data );
}

template< typename sample_type >
void buffer<sample_type>::reference( const buffer &from )
{
	m_data.reference( from.m_data );
}

template< typename sample_type >
sample_type* buffer<sample_type>::operator() ( void )
{	return m_data();
}

template< typename sample_type >
const sample_type* buffer<sample_type>::operator() ( void ) const
{	return m_data();
}

template< typename sample_type >
sample_type* buffer<sample_type>::operator() ( const int channel_no )
{	return m_data( channel_no );
}

template< typename sample_type >
const sample_type* buffer<sample_type>::operator() ( const int channel_no ) const
{	return m_data( channel_no );
}

template< typename sample_type >
sample_type& buffer<sample_type>::operator() ( const int channel_no, const int sample_no )
{	return m_data( sample_no, channel_no );
}

template< typename sample_type >
const sample_type& buffer<sample_type>::operator() ( const int channel_no, const int sample_no ) const
{	return m_data( sample_no, channel_no );
}

// Get the resolution
template< typename sample_type >
int buffer<sample_type>::no_samples( void ) const
{	return m_data.xres();
}

template< typename sample_type >
int buffer<sample_type>::no_samples_in_bytes( void ) const
{	return m_data.xres_in_bytes();
}

template< typename sample_type >
int buffer<sample_type>::no_channels( void ) const
{	return m_data.yres();
}

// Do we follow a particular alignment (must be power of two)
template< typename sample_type >
bool buffer<sample_type>::is_aligned( const int alignment ) const
{	return m_data.is_aligned( alignment );
}

// Get the stride
template< typename sample_type >
int buffer<sample_type>::stride( void ) const
{	return m_data.stride();
}

template< typename sample_type >
int buffer<sample_type>::stride_in_bytes( void ) const
{	return m_data.stride_in_bytes();
}

// Copy from
template< typename sample_type >
void buffer<sample_type>::operator=( const buffer& from )
{	
	m_data = from.m_data;
}

// Set
template< typename sample_type >
void buffer<sample_type>::operator=( const sample_type& x )
{	
	m_data = x;
}

template< typename sample_type >
void buffer<sample_type>::operator*= (const double scale_factor) 
{
	for( int y=0; y<m_data.yres(); y++ )
		scale<sample_type>((*this)( y ),this->no_samples(),scale_factor);
}

// Swap with
template< typename sample_type >
void buffer<sample_type>::swap_with( buffer& other )
{	
	m_data.swap_with( other.m_data );
}

template< typename sample_type >
void buffer<sample_type>::swap_to( buffer& other )
{
	m_data.swap_to( other.m_data );
}

// Get the size
template< typename sample_type >
const size_t buffer<sample_type>::size( void ) const
{	return no_samples()*no_channels()*sizeof(sample_type);
}