#pragma once

/* /// Here is a quick handy function to get a time stamp using the QueryPerformanceCounter
double FRAMEWORK_BASE_API get_current_time(void);*/

///Here is a container which houses time particularly for time stamping operations.  This natively stores the value as a fixed precision of
///10,000,000 to 1 to a 64bit integer (i.e. __int64).  Overloaded operators have been provided for ease of conversion between various types
///The two primary types are __int64 and double.  \note This fixed precision standard is native to Direct Show sometimes in the form of
///LARGE_INTEGER
struct _declspec(dllexport) time_type
{
	public:
		/// \note The default does not do anything except initialize the internal time to 0xcdcdcdcd
		time_type();
		time_type(double NewValue);
		time_type(__int64 NewValue);
		time_type(const time_type &NewValue);
		///Construct a time type by gathering pieces of a large integer (e.g. passed through 32bit lparam callbacks)
		time_type(DWORD LowPart,LONG HighPart);
		/// \todo add a format spec for timecode
		//time_type(wchar_t *format=NULL)
		inline operator double (void) const;
		inline operator __int64 (void) const;
		inline operator __int64* (void);
		inline operator LARGE_INTEGER* (void);
		//TODO timecode
		//operator const wchar_t*(void) const;
		//The copy constructor
		inline time_type& operator= (const time_type & NewValue);
		inline void operator= (double NewValue);
		inline void operator= (__int64 NewValue);

		inline time_type operator- (const time_type &NewValue) const;
		inline time_type operator+ (const time_type &NewValue) const;
		inline void operator+= (const __int64 &NewValue);
		inline void operator-= (const __int64 &NewValue);
		inline bool operator>  (const time_type &Value) const;
		inline bool operator>= (const time_type &Value) const;
		inline bool operator<  (const time_type &Value) const;
		inline bool operator<= (const time_type &Value) const;
		//Working with LARGE_INTEGER break down
		DWORD GetLowPart();
		LONG GetHighPart();

		///This will produce a time stamp of system time (i.e. QueryPerformanceCounter) and offers it in a form that is compliant to this type
		///In a way that minimizes precision loss (int64 division + remainder)
		static time_type get_current_time();
	private:
		///There are 10,000,000 (10 million) units for one second 
		__int64 m_Time;
};
