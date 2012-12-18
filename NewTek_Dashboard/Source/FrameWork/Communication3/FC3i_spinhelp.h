#pragma once

struct spinhelp
{			// Constructor
			spinhelp( void );			

			// Perform a spin_lock
			void operator++ ( int );
			void operator++ ( void );

private:	// The number of spins at which point to defer to another thread
			static const int spinhelp_count = 16;

			// The number of iterations through the spin lock
			int		m_spins, m_no_sleeps;
};