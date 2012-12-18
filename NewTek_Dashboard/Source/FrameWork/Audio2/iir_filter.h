#pragma once

#include "switcher_accessors.h"
#include "iir_filter_impl.h"

// The maximum filter order
static const int max_filter_order	= 50;
static const int max_no_channels	= 16;

// Butterworth filters
namespace butterworth
{
	// General EQ
	struct FRAMEWORKAUDIO2_API eq : public base_filter< FrameWork::Audio2::dsp::ButterEq< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

			// The width of the filter
			PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

			// The gain of the filter (in dB)
			PSWIT_property_default_with_changed( double, gain, 0.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::ButterEq< max_filter_order, max_no_channels >& dst );
	};

	// Low pass
	struct FRAMEWORKAUDIO2_API low_pass : public base_filter< FrameWork::Audio2::dsp::ButterLowPass< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::ButterLowPass< max_filter_order, max_no_channels >& dst );
	};

	// High Pass
	struct FRAMEWORKAUDIO2_API high_pass : public base_filter< FrameWork::Audio2::dsp::ButterHighPass< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::ButterHighPass< max_filter_order, max_no_channels >& dst );
	};

	// Band Pass
	struct FRAMEWORKAUDIO2_API band_pass : public base_filter< FrameWork::Audio2::dsp::ButterBandPass< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

			// The width of the filter
			PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::ButterBandPass< max_filter_order, max_no_channels >& dst );
	};

	// Band Stop
	struct FRAMEWORKAUDIO2_API band_stop : public base_filter< FrameWork::Audio2::dsp::ButterBandStop< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

			// The width of the filter
			PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::ButterBandStop< max_filter_order, max_no_channels >& dst );
	};
};

// Butterworth filters
namespace bessel
{	// Low pass
	struct FRAMEWORKAUDIO2_API low_pass : public base_filter< FrameWork::Audio2::dsp::BesselLowPass< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::BesselLowPass< max_filter_order, max_no_channels >& dst );
	};

	// High Pass
	struct FRAMEWORKAUDIO2_API high_pass : public base_filter< FrameWork::Audio2::dsp::BesselHighPass< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::BesselHighPass< max_filter_order, max_no_channels >& dst );
	};

	// Band Pass
	struct FRAMEWORKAUDIO2_API band_pass : public base_filter< FrameWork::Audio2::dsp::BesselBandPass< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

			// The width of the filter
			PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::BesselBandPass< max_filter_order, max_no_channels >& dst );
	};

	// Band Stop
	struct FRAMEWORKAUDIO2_API band_stop : public base_filter< FrameWork::Audio2::dsp::BesselBandStop< max_filter_order, max_no_channels > >
	{		// The order of the filter
			PSWIT_property_default_with_changed( int, order, 2 );

			// The frequency to filter at (relative to the sample frequency)
			PSWIT_property_default_with_changed( double, frequency, 1000.0 / 96000.0 );

			// The width of the filter
			PSWIT_property_default_with_changed( double, width, 500.0 / 96000.0 );

private:	// Setup a filter and return if anything changed.
			void setup( FrameWork::Audio2::dsp::BesselBandStop< max_filter_order, max_no_channels >& dst );
	};
};

