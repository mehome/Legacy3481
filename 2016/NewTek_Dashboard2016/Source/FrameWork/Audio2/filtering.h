#pragma once

#include "..\..\Processing\Switcher\switcher_accessors.h"
#include "filtering_impl.h"

// The maximum filter order
static const int max_filter_order = 50;

// Butterworth filters
namespace butterworth
{
	struct FRAMEWORKAUDIO2_API eq : public base_filter< FrameWork::Audio2::dsp::ButterEq< max_filter_order, 1 > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

			// The width of the filter
			PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

			// The gain of the filter (in dB)
			PSWIT_property_default_with_changed( double, gain, 0.0 );

	};
};



/*
struct filtering
{			// Constructor
			filtering( void );

			// Destructor
			~filtering( void );		

			// A Butterworth Eq filter
			struct butterworth_eq_filter_settings
			{	// The order of the filter
				PSWIT_property_default_with_changed( int, order, 2 );

				// The frequency to filter at (relative to the sample frequency)
				PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

				// The width of the filter
				PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

				// The gain of the filter (in dB)
				PSWIT_property_default_with_changed( double, gain, 0.0 );

private:		// Has any part of the filter changed ? (and reset the changed flags)
				const bool changed( void ) const { return order().changed(1) | frequency().changed(1) | width().changed(1) | gain().changed(1); }
			};

private:	
};*/