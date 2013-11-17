#pragma once
class KalmanFilter
{
public:
	KalmanFilter();
	/// \return the filtered value
	double operator()(double input);
	void Reset();
private:
	//initial values for the Kalman filter
	double m_x_est_last;
	double m_last;
	//the noise in the system
	const double m_Q;
	const double m_R;
	bool m_FirstRun; //This avoids a stall when first starting
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
	T operator()(T newItem)
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

class Predict_simple
{
public:
	Predict_simple()
	{
		Reset();
	}
	/// \return the filtered value
	double operator()(double input,double dTime_s,double PredictTime)
	{
		//compute velocity
		const double velocity=(input-m_last_input) / dTime_s;
		const double Predict_s= input + (velocity * PredictTime);
		m_last_input=input;
		return Predict_s;
	}
	void Reset()
	{
		m_last_input=0.0;
	}
private:
	double m_last_input;
};

struct mem
{
	static void*	alloc( const size_t size, const size_t align )
	{
		return _aligned_malloc( size, /*align*/ 128 );	// We not have a nice alignment, there is no real down-side.
	}
	static void		free( const void* p_ptr )
	{
		_aligned_free( (void*)p_ptr );
	}
};


class Outstream_Interface
{
	public:
		virtual void process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer,bool isInterlaced,double VideoClock,float AspectRatio)=0;
		//virtual void process_audio(const FrameWork::Audio2::buffer_f32 *pBuffer,int SampleRate,double AudioClock)=0;
};

void BlackField( PBYTE pField, const int FrameSize );
void DrawField( PBYTE pField, const int FrameWidth, const int FieldHeight, const int FieldNumber );

void DebugOutput(const char *format, ... );
std::string BuildString(const char *format, ... );

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
inline bool IsZero(double value,double tolerance=1e-5);

#define ASSERT(cond) assert(cond);
#define ASSERT_MSG(cond, msg) if (!(cond)){printf((msg)); assert(cond);}
