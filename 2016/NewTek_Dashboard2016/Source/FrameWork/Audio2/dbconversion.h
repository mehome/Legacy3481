#pragma once

struct FRAMEWORKAUDIO2_API db
{
	static const double ratio( const double db_value );

	// -1 = -inf
	// +1 = +inf
	//  0 = other values
	static const int inf( const double value );
};

struct FRAMEWORKAUDIO2_API ratio
{
	static const double db( const double ratio_value );

	// -1 = -inf
	// +1 = +inf
	//  0 = other values
	static const int inf( const double value );
};
