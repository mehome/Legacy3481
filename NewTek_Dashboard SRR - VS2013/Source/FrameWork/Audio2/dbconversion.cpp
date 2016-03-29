#include "stdafx.h"
#include "FrameWork.Audio2.h"

const double FrameWork::Audio2::utils::ratio::db( const double ratio_value )
{	return 20.0 * log10( ratio_value );
}

const double FrameWork::Audio2::utils::db::ratio( const double db_value )
{	return pow( 10.0, db_value*0.05 );
}

const int FrameWork::Audio2::utils::ratio::inf( const double value )
{	switch( _fpclass( value ) )
	{	case _FPCLASS_NINF:		return -1;
		case _FPCLASS_PINF:		return +1;
		case _FPCLASS_SNAN:		return +1;
		case _FPCLASS_QNAN:		return +1;
		default:				return 0;
	}

}

const int FrameWork::Audio2::utils::db::inf( const double value )
{	return FrameWork::Audio2::utils::ratio::inf( value );
}