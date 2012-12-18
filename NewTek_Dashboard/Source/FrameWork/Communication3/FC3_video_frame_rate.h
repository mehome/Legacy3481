#pragma once

			// This defines how time-stamps are defined.
			struct FRAMEWORKCOMMUNICATION3_API frame_rate_type
			{	// Constructors
				frame_rate_type( const frame_rate_type &from );
				frame_rate_type( const int n, const int d );
				frame_rate_type( void );

				// Compute the difference between two frame rates. 
				// This is very useful.
				frame_rate_type( const frame_rate_type &x0, const frame_rate_type &x1 );

				// Get the values
				const int n( void ) const;
				const int d( void ) const;

				// Set the values
				int& n( void );
				int& d( void );

				// Copy from different values
				void operator= ( const frame_rate_type &from );

				// Comparison operators
				const bool operator== ( const frame_rate_type &from ) const;
				const bool operator!= ( const frame_rate_type &from ) const;

				const bool operator> ( const frame_rate_type &from ) const;
				const bool operator< ( const frame_rate_type &from ) const;

				const bool operator>= ( const frame_rate_type &from ) const;
				const bool operator<= ( const frame_rate_type &from ) const;

				// Compute the frame-rate as a double
				const double frames_per_second( void ) const;
				const double time_in_ms( void ) const;
				const double time_in_s( void ) const;

				// Compute the 10ns frame time
				const __int64 frame_time( const __int64 time_base = 10000000LL ) const;
				
private:		// The numerator and denominator
				int	m_n, m_d;
			};