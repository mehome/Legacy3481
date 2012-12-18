#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Constructor
spinhelp::spinhelp( void )
	:	m_spins( spinhelp_count ), 
		m_no_sleeps( 0 )
{
}

// Perform a spin_lock
void spinhelp::operator++ ( int )
{	operator++();
}

void spinhelp::operator++ ( void )
{	// Decrease the spinlock count but avoid ever needing to
	// lock up if we are a time-critical thread.
	if ( ! (--m_spins) ) 
	{	// Defer to other threads for a tiny bit
		::Sleep( ( m_no_sleeps++ ) ? 1 : 0 );

		// Reset the spin count
		m_spins = spinhelp_count;
	}
}