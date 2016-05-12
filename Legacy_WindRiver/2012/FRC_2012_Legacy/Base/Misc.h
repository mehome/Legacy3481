#pragma once
#include <math.h>
#include <stdlib.h>
#include <map>

typedef std::map<std::string, std::string, std::greater<std::string> > StringMap;

namespace Framework
{
	namespace Base
	{

		std::string BuildString(const char *format, ... );
		void DebugOutput(const char *format, ... );
		char* GetLastSlash(char* fn, char* before=NULL);
		std::string GetContentDir_FromFile(const char* fn);

		// Parses name=value pairs, stripping whitespace and comments starting with '#'
		// Returns true if file was opened properly
		bool ReadStringMapFromIniFile(std::string filename, StringMap& resMap);
		void StripCommentsAndTrailingWhiteSpace(char* line);
		std::string TrimString( const std::string& StrToTrim );

		//! Returns false iff c == [ 'f', 'F', 'n', 'N', '0', 0 ]
		bool ParseBooleanFromChar(char c);

		//! Trying to keep the Win32 FindFirstFile stuff wrapped
		//! -4 for neither file exists
		//! -3 the first file does not exist but the second does
		//! -2 the second file does not exist but the first does
		//! -1 First file time is earlier than second file time.
		//! 0 The times are the same
		//! 1 the second file time is earlier than the first
		int CompareFileLastWriteTimes(const char* f1, const char* f2);

		// Internal methods
		struct track_memory_impl
		{	static void add   ( void* p_data, const char* p_name );
			static void remove( void* p_data, const char* p_name );
		};
	};
};

// A templated averager, make sure the type being averaged can handle the +, -, and / functions
template<class T, unsigned NUMELEMENTS>
class Averager
{
public:
	Averager() : m_array(NULL), m_currIndex((unsigned)-1)
	{
		if (NUMELEMENTS > 1)
			m_array = new T[NUMELEMENTS];
	}
	virtual ~Averager() {if (m_array) delete[] m_array;}
	T GetAverage(T newItem)
	{
		if (!m_array)	// We are not really using the Averager
			return newItem;

		// If the first time called, set up the array and use this value
		if (m_currIndex == -1)
		{
			m_array[0] = newItem;
			m_currIndex = -2;
			m_sum = newItem;
			return newItem;
		}
		else if (m_currIndex < -1)
		{
			// We have not populated the array for the first time yet, still populating
			m_sum += newItem;
			int arrayIndex = (m_currIndex*-1)-1;
			m_array[arrayIndex] = newItem;

			// Still counting backwards unless we have filled all of the array
			if (arrayIndex == (NUMELEMENTS-1))	// This means we have filled the array
				m_currIndex = 0;				// Start taking from the array next time
			else
				--m_currIndex;

			// Return the average based on what we have counted so far
			return (m_sum / (arrayIndex+1));
		}
		else // 0 or greater, we have filled the array
		{
			m_sum += newItem;
			m_sum -= m_array[m_currIndex];
			m_array[m_currIndex] = newItem;
			++m_currIndex;
			if (m_currIndex == NUMELEMENTS)
				m_currIndex = 0;
			return (m_sum / NUMELEMENTS);
		}
	}

	void Reset(){m_currIndex=-1;}

private:
	T* m_array;
	int m_currIndex;
	T m_sum;
};

//Threshold average only changes to new value when that value has been requested THRESHOLD times
template<class T, unsigned THRESHOLD>
class Threshold_Averager
{
public:
	Threshold_Averager() : m_Count(-1) {}
	T GetValue(T newItem)
	{
		//first time init with newItem
		if (m_Count==(size_t)-1)
		{
			m_CurrentItem=m_LastRequestedItem=newItem;
			m_Count=0;
		}
		//T ret=m_CurrentItem;   hmmm not used
		if (newItem!=m_CurrentItem)
		{
			if (newItem==m_LastRequestedItem)
			{
				m_Count++;
				if (m_Count>THRESHOLD)
					m_CurrentItem=newItem,m_Count=0;
			}
			else
				m_Count=0;
			m_LastRequestedItem=newItem;
		}
		return m_CurrentItem;
	}
private:
	size_t m_Count;
	T m_CurrentItem,m_LastRequestedItem;
};

//Here is a very light weight averager that uses the blend technique to obtain an average.  Designed to easily replace Averager.
template<class T>
class Blend_Averager
{
public:
	/// \param SmoothingValue a range from 0.0 - 1.0 representing percentage of new item to factor in  
	Blend_Averager(double SmoothingValue=0.5) : m_SmoothingValue(-1.0),m_DefaultSmoothingValue(SmoothingValue) {}
	//get and set smoothing value  (optional)
	//exposing this allows for dynamic smoothing
	double &GetSmoothingValue() {return m_SmoothingValue;}
	T GetAverage(T newItem)
	{
		if (m_SmoothingValue!=-1.0)
			m_CurrentValue=((newItem * m_SmoothingValue ) + (m_CurrentValue  * (1.0-m_SmoothingValue)));
		else
		{
			m_SmoothingValue=m_DefaultSmoothingValue;
			m_CurrentValue=newItem;
		}
		return m_CurrentValue;
	}
	void Reset(){m_SmoothingValue=-1;}
private:
	double m_SmoothingValue,m_DefaultSmoothingValue;
	T m_CurrentValue;
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PIF
#define M_PIF 3.141592654f
#endif
#define M_PID 3.14159265358979323846

#define DEG_2_RAD(x)		((x)*M_PI/180.0)
#define RAD_2_DEG(x)		((x)*180.0/M_PI)
#define ARRAY_SIZE(things)	((sizeof(things)/sizeof(*(things))))

#define inches2Meters(x)	((x)*0.0254)
#define Meters2Feet(x)		((x)*3.2808)
#define MIN(a,b)			((a)<(b)?(a):(b))
#define MAX(a,b)			((a)>(b)?(a):(b))

#ifndef NULL
#define NULL 0
#endif

#ifndef null
#define null 0
#endif

//! Easily compare doubles with a delta
inline bool Equals(double d1, double d2)
{
	double tol = (d2>0.0) ? d2*0.000001 : -d2*0.000001;
	double delta = (d1>d2) ? d1-d2 : d2-d1;
	return (delta < tol);
}
inline bool Equals(float d1, float d2)
{
	double tol = (d2>0.0f) ? d2*0.000001f : -d2*0.000001f;
	double delta = (d1>d2) ? d1-d2 : d2-d1;
	return (delta < tol);
}

//Use this to check for zero if your value is going to be used as a denominator in division
inline bool IsZero(double value,double tolerance=1e-5)
{
	return fabs(value)<tolerance;
}

inline double RAND_GEN(double minR = 0.0, double maxR = 1.0)
{
	return minR + (maxR-minR) * ((double)(rand()) / (double)(RAND_MAX));
}

#define wchar2char(wchar2char_pwchar_source) \
	const size_t wchar2char_Length=wcstombs(NULL,wchar2char_pwchar_source,0)+1; \
	char *wchar2char_pchar = (char *)_alloca(wchar2char_Length);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs(wchar2char_pchar,wchar2char_pwchar_source,wchar2char_Length);

#define char2wchar(char2wchar_pchar_source) \
	const size_t char2wchar_Length=((strlen(char2wchar_pchar_source)+1)*sizeof(wchar_t)); \
	wchar_t *char2wchar_pwchar = (wchar_t *)_alloca(char2wchar_Length);; /* ";;" is needed to fix a compiler bug */ \
	mbstowcs(char2wchar_pwchar,char2wchar_pchar_source,char2wchar_Length);

#define _aligned_alloca( size, alignement ) ( (void*)( ((size_t)((alignement)-1)+(size_t)_alloca( (size)+(alignement) ))&(~((alignement)-1)) ) )
