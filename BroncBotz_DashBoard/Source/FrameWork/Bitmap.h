//Just using basic stuff here... don't need the deprecated warnings
#define  _CRT_SECURE_NO_WARNINGS

#include <fcntl.h>			//Needed only for _O_RDWR definition
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <xutility>
#include <vector>

using namespace std;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

//Got some win32 stuff here... not going to include windows.h though
//I've gotta pack it since the bfSize must precede immediately after the bfType

#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif // WIN32

typedef struct tagBITMAPFILEHEADER { 
	WORD    bfType; 
	DWORD   bfSize; 
	WORD    bfReserved1; 
	WORD    bfReserved2; 
	DWORD   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD  biSize; 
	LONG   biWidth; 
	LONG   biHeight; 
	WORD   biPlanes; 
	WORD   biBitCount; 
	DWORD  biCompression; 
	DWORD  biSizeImage; 
	LONG   biXPelsPerMeter; 
	LONG   biYPelsPerMeter; 
	DWORD  biClrUsed; 
	DWORD  biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER; 

#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(pop)
#else
#pragma pack()
#endif // WIN32


#define MakeID(a,b) ((b)<<8|(a))

struct pixel_bgr_u8		{ typedef unsigned char value_type;		value_type m_b, m_g, m_r; };

const size_t c_X_Resolution=720;
const size_t c_Y_Resolution=540;

//Not as fancy as Andrew's but gets the job done
class Bitmap
{
	public:
		Bitmap(size_t XRes, size_t YRes) : m_XRes(XRes),m_YRes(YRes),m_pData(NULL)
		{
			m_stride_in_bytes = m_XRes * 3; //nothing fancy for this

			size_t BitmapSize=YRes*m_stride_in_bytes;
			m_pData=new pixel_bgr_u8[XRes*YRes];
			memset(m_pData,0,BitmapSize);  //fill with black
		}
		~Bitmap()
		{
			if (m_pData)
			{
				delete [] m_pData;
				m_pData=NULL;
			}
		}

		// Get the resolution
		int xres( void ) const
		{	return m_XRes;
		}

		int yres( void ) const
		{	return m_YRes;
		}

		// Get pixel locations
		pixel_bgr_u8 &operator() ( size_t x, size_t y )
		{
			assert( ( x>=0 ) && ( x<m_XRes ) && ( y>=0 ) && ( y<m_YRes ) );
			assert( m_pData );

			return ( (pixel_bgr_u8*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x ];
		}
		pixel_bgr_u8 &operator() ( size_t x, size_t y ) const
		{
			assert( ( x>=0 ) && ( x<m_XRes ) && ( y>=0 ) && ( y<m_YRes ) );
			assert( m_pData );

			return ( (pixel_bgr_u8*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x ];

		}
		// Get the image pointers
		pixel_bgr_u8 *operator()()
		{
			return m_pData;
		}

		const pixel_bgr_u8 *operator()() const
		{
			return m_pData;
		}

		const size_t size( void ) const
		{	return m_XRes*m_YRes*sizeof(pixel_bgr_u8);
		}
	private:
		size_t m_XRes,m_YRes, m_stride_in_bytes;
		pixel_bgr_u8 *m_pData;
};
