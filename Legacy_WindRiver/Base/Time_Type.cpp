#include "Timer.h"
#include "Base_Includes.h"
#include "Time_Type.h"

//using namespace FrameWork;

  /*******************************************************************************************************/
 /*												time_type												*/
/*******************************************************************************************************/

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

time_type::operator double ( void ) const
{
	return m_Time/10000000.0;
}
time_type::operator __int64 ( void ) const
{
	return m_Time;
}
time_type::operator __int64* ( void )
{
	return &m_Time;
}

void time_type::operator= (double NewValue)
{
	m_Time=(__int64)(NewValue*10000000.0);
}
void time_type::operator= (__int64 NewValue)
{
	m_Time=NewValue;
}
time_type& time_type::operator= (const time_type & NewValue)
{
	m_Time=NewValue.m_Time;
	return *this;
}
time_type time_type::operator- (const time_type &NewValue) const
{
	time_type Temp(*this);
	Temp-=NewValue;
	return Temp;
	//return m_Time-NewValue.m_Time;
}

time_type time_type::operator+ (const time_type &NewValue) const
{
	time_type Temp(*this);
	Temp+=NewValue;
	return Temp;
	//return m_Time+NewValue.m_Time;
}
void time_type::operator+= (const __int64 &NewValue)
{
	m_Time+=NewValue;
}
void time_type::operator+= (double NewValue)
{
	m_Time+=(__int64)(NewValue*10000000.0);
}
void time_type::operator+= (const time_type &NewValue)
{
	m_Time+=(__int64)NewValue;
}
void time_type::operator-= (const __int64 &NewValue)
{
	m_Time-=NewValue;
}
void time_type::operator-= (double NewValue)
{
	m_Time-=(__int64)(NewValue*10000000.0);
}
void time_type::operator-= (const time_type &NewValue)
{
	m_Time-=(__int64)NewValue;
}
bool time_type::operator> (const time_type &Value) const
{
	return (__int64)m_Time>(__int64)Value;
}
bool time_type::operator>= (const time_type &Value) const
{
	return (__int64)m_Time>=(__int64)Value;
}

bool time_type::operator< (const time_type &Value) const
{
	return (__int64)m_Time<(__int64)Value;
}
bool time_type::operator<= (const time_type &Value) const
{
	return (__int64)m_Time<=(__int64)Value;
}

time_type time_type::get_current_time()
{
	const double CurrentTime=GetTime();
	return CurrentTime;
}
