#pragma once

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
		virtual void process_frame(const FrameWork::Bitmaps::bitmap_ycbcr_u8 *pBuffer)=0;
};
