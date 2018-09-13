#include "pch.h"
#include "Time_Type.h"

  /*******************************************************************************************************/
 /*												time_type												*/
/*******************************************************************************************************/
/*double FrameWork::Base::get_current_time(void)
{	// Make sure that we have the update frequency
	static double inv_freq = 0.0f;
	if ( !inv_freq )
	{	LARGE_INTEGER freq;
		QueryPerformanceFrequency( &freq );
		inv_freq = 1.0 / (double)freq.QuadPart;
	}

	// Get the current time
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);

	// Return the time in seconds.
	return inv_freq*(double)current.QuadPart;
}*/

time_type::time_type()
{
	m_Time=0xcdcdcdcd;
}
time_type::time_type(double NewValue)
{
	*this=NewValue;
}
time_type::time_type(__int64 NewValue)
{
	*this=NewValue;
}

time_type::time_type(const time_type &NewValue)
{
	m_Time=NewValue.m_Time;
}

time_type::time_type(DWORD LowPart,LONG HighPart)
{
	LARGE_INTEGER NewValue;
	NewValue.LowPart=LowPart;
	NewValue.HighPart=HighPart;
	m_Time=NewValue.QuadPart;
}

inline time_type::operator double (void) const
{
	return m_Time/10000000.0;
}
inline time_type::operator __int64 (void) const
{
	return m_Time;
}
inline time_type::operator __int64* (void)
{
	return &m_Time;
}
inline time_type::operator LARGE_INTEGER* (void)
{
	return ((LARGE_INTEGER *)((__int64 *)&m_Time));
}

inline void time_type::operator= (double NewValue)
{
	m_Time=(__int64)(NewValue*10000000.0);
}
inline void time_type::operator= (__int64 NewValue)
{
	m_Time=NewValue;
}
inline time_type& time_type::operator= (const time_type & NewValue)
{
	m_Time=NewValue.m_Time;
	return *this;
}
inline time_type time_type::operator- (const time_type &NewValue) const
{
	time_type Temp(*this);
	Temp-=NewValue;
	return Temp;
	//return m_Time-NewValue.m_Time;
}

inline time_type time_type::operator+ (const time_type &NewValue) const
{
	time_type Temp(*this);
	Temp+=NewValue;
	return Temp;
	//return m_Time+NewValue.m_Time;
}
inline void time_type::operator+= (const __int64 &NewValue)
{
	m_Time+=NewValue;
}
inline void time_type::operator-= (const __int64 &NewValue)
{
	m_Time-=NewValue;
}
inline bool time_type::operator> (const time_type &Value) const
{
	return (__int64)m_Time>(__int64)Value;
}
inline bool time_type::operator>= (const time_type &Value) const
{
	return (__int64)m_Time>=(__int64)Value;
}

inline bool time_type::operator< (const time_type &Value) const
{
	return (__int64)m_Time<(__int64)Value;
}
inline bool time_type::operator<= (const time_type &Value) const
{
	return (__int64)m_Time<=(__int64)Value;
}

DWORD time_type::GetLowPart()
{
	LARGE_INTEGER NewValue;
	NewValue.QuadPart=m_Time;
	return NewValue.LowPart;
}
LONG time_type::GetHighPart()
{
	LARGE_INTEGER NewValue;
	NewValue.QuadPart=m_Time;
	return NewValue.HighPart;
}

time_type time_type::get_current_time()
{
	LARGE_INTEGER freq,current;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&current);

	//Using const helps the compiler pull the remainder from the same IDIV instruction (for 64 bit)
	const __int64 TimeStamp_Divisor=(__int64)current.QuadPart;
	const __int64 TimeBase_Dividend=(__int64)freq.QuadPart;

		  __int64 Quotient= (TimeStamp_Divisor/TimeBase_Dividend);
	const __int64 Remainder=(TimeStamp_Divisor%TimeBase_Dividend);

	//Now to scale the integer and remainder to 10 microsecond units
	const __int64 TimeTypeUnitBase=10000000;
	Quotient*=TimeTypeUnitBase;
	__int64 RemainderScaled=(Remainder*TimeTypeUnitBase)/TimeBase_Dividend;
	Quotient+=RemainderScaled;
	return Quotient;
}
