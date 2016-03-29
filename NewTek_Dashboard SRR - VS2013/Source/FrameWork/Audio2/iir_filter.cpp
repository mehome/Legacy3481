#include "stdafx.h"
#include "FrameWork.Audio2.h"

// Namespace stuff
using namespace FAUD::iir;

// Setup a filter and return if anything changed.
void butterworth::eq::setup( FrameWork::Audio2::dsp::ButterEq< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | width().changed() | gain().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), width(), gain(), order() );
	}
}

// Setup a filter and return if anything changed.
void butterworth::low_pass::setup( FrameWork::Audio2::dsp::ButterLowPass< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), order() );
	}
}

// Setup a filter and return if anything changed.
void butterworth::high_pass::setup( FrameWork::Audio2::dsp::ButterHighPass< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), order() );
	}	
}

// Setup a filter and return if anything changed.
void butterworth::band_pass::setup( FrameWork::Audio2::dsp::ButterBandPass< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | width().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), width(), order() );
	}
}

// Setup a filter and return if anything changed.
void butterworth::band_stop::setup( FrameWork::Audio2::dsp::ButterBandStop< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | width().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), width(), order() );
	}
}


//---------------------------------------------------------------------------------------------------------------------------------------------
// Setup a filter and return if anything changed.
void bessel::low_pass::setup( FrameWork::Audio2::dsp::BesselLowPass< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), order() );
	}
}

// Setup a filter and return if anything changed.
void bessel::high_pass::setup( FrameWork::Audio2::dsp::BesselHighPass< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), order() );
	}	
}

// Setup a filter and return if anything changed.
void bessel::band_pass::setup( FrameWork::Audio2::dsp::BesselBandPass< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | width().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), width(), order() );
	}
}

// Setup a filter and return if anything changed.
void bessel::band_stop::setup( FrameWork::Audio2::dsp::BesselBandStop< max_filter_order, max_no_channels >& dst )
{	// Check for actual parameter changes
	if ( frequency().changed() | width().changed() | order().changed() )
	{	// Setup
		dst.SetupAs( frequency(), width(), order() );
	}
}

