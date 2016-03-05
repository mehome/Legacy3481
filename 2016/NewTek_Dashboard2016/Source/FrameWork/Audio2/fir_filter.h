#pragma once

struct FRAMEWORKAUDIO2_API filter
{			// Constructor
			filter( const low_pass&		from , const float gain = 1.0f );
			filter( const high_pass&	from, const float gain = 1.0f );
			filter( const band_pass&	from, const float gain = 1.0f );
			filter( const custom_pass&	from, const float gain = 1.0f );

			// Destructor
			~filter( void );

			// Get the size of the current filter
			const int size( void ) const;

			// This will apply a FIR filter to a raw block of data. This will fill
			// in no_samples - size() in the destination, which is the maximum that
			// can be filled in with no history buffers.
			const int operator() ( const float* p_src, float *p_dst, const int no_samples ) const;

private:	// A constraint descriptor
			typedef std::pair< double, double >		constraint_type;
			typedef std::vector< constraint_type >	constraint_list_type;

			// This uses matrices to solve the equations
			void compute_coefficients( const constraint_list_type& H );

			// The array of values used to apply the operation
			float	*m_p_coefficients;
			int		m_filter_size;
			int		m_real_filter_size;
};

// Applying a filter requires that some history from previous buffers is maintained in order
// to apply the operation across boundaries. 
struct FRAMEWORKAUDIO2_API history
{			// Constructor
			history( filter& filter_to_use );

			// Destructor
			~history( void );

			// Apply a FIR filter to a buffer
			void operator() ( const buffer_f32& src, buffer_f32& dst );

private:	// Change the number of channels
			void change_no_channels( const int no_channels );

			// The current number of channels
			int m_no_channels;
	
			// For each channel we store some history
			std::vector< float* >	m_history;

			// The filter information
			filter& m_filter_to_use;

			// The filter size
			const int m_filter_size;
};