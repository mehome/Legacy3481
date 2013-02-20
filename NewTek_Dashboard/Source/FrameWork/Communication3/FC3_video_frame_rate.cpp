#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::video;

const message::frame_rate_type	message::frame_rate_type_60   ( 60000, 1000 );
const message::frame_rate_type	message::frame_rate_type_59_94( 60000, 1001 );
const message::frame_rate_type	message::frame_rate_type_30   ( 30000, 1000 );
const message::frame_rate_type	message::frame_rate_type_29_97( 30000, 1001 );
const message::frame_rate_type	message::frame_rate_type_25   ( 30000, 1200 );
const message::frame_rate_type	message::frame_rate_type_24   ( 24000, 1000 );
const message::frame_rate_type	message::frame_rate_type_23_97( 24000, 1001 );

// Constructors
message::frame_rate_type::frame_rate_type( const frame_rate_type &from )
	:	m_n( from.m_n ), m_d( from.m_d )
{
}

message::frame_rate_type::frame_rate_type( const int n, const int d )
	:	m_n( n ), m_d( d )
{
}

message::frame_rate_type::frame_rate_type( void )
	:	m_n( 1 ), m_d( 0 )
{
}

message::frame_rate_type::frame_rate_type( const frame_rate_type &x0, const frame_rate_type &x1 )
	:	m_n( x0.m_n*x1.m_d - x0.m_d*x1.m_n ), 
		m_d( x0.m_d*x1.m_d )
{	
}

// Get the values
const int message::frame_rate_type::n( void ) const
{	return m_n;
}

const int message::frame_rate_type::d( void ) const
{	return m_d;
}

// Set the values
int& message::frame_rate_type::n( void )
{	return m_n;
}

int& message::frame_rate_type::d( void )
{	return m_d;
}

// Comparison operators
const bool message::frame_rate_type::operator== ( const frame_rate_type &from ) const
{	return m_n * from.m_d == m_d * from.m_n;
}

const bool message::frame_rate_type::operator!= ( const frame_rate_type &from ) const
{	return m_n * from.m_d != m_d * from.m_n;
}

const bool message::frame_rate_type::operator> ( const frame_rate_type &from ) const
{	return m_n * from.m_d > m_d * from.m_n;
}

const bool message::frame_rate_type::operator< ( const frame_rate_type &from ) const
{	return m_n * from.m_d < m_d * from.m_n;
}

const bool message::frame_rate_type::operator>= ( const frame_rate_type &from ) const
{	return m_n * from.m_d >= m_d * from.m_n;
}

const bool message::frame_rate_type::operator<= ( const frame_rate_type &from ) const
{	return m_n * from.m_d <= m_d * from.m_n;
}
	
void message::frame_rate_type::operator= ( const frame_rate_type &from )
{	m_n = from.m_n;
	m_d = from.m_d;
}

const double message::frame_rate_type::frames_per_second( void ) const
{	const double ret = (double)m_n / (double)m_d;
	return ( ret >= 0 ) ? ret : -ret;
}

const double message::frame_rate_type::time_in_ms( void ) const
{	const double ret = 1000.0 * (double)m_d / (double)m_n;
	return ( ret >= 0 ) ? ret : -ret;
}

const double message::frame_rate_type::time_in_s( void ) const
{	const double ret = (double)m_d / (double)m_n;
	return ( ret >= 0 ) ? ret : -ret;
}

// Compute the 10ns frame time
const __int64 message::frame_rate_type::frame_time( const __int64 time_base ) const
{	// Get the value
	const __int64 ret = (__int64)( m_d*time_base + m_n/2 ) / (__int64)m_n;
	return ( ret >= 0 ) ? ret : -ret;
}

message::frame_rate_type::operator float  ( void ) const
{	return (float)m_n/(float)m_d;
}

message::frame_rate_type::operator double ( void ) const
{	return (double)m_n/(double)m_d;
}

message::frame_rate_type::operator int    ( void ) const
{	return (int)( 0.5 + (double)m_n/(double)m_d );	
}