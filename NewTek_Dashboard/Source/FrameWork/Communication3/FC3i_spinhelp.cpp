#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3i;

// Constructor
spinhelp::spinhelp( void )
	:	m_spins( spinhelp_count )
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
		SwitchToThread();

		// Reset the spin count
		m_spins = spinhelp_count;
	}
}