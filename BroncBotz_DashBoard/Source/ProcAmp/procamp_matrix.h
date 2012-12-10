#pragma once

namespace FGL
{
	template< typename value_type >
	struct parameter_access
	{			// Constructor
		parameter_access( value_type &var, bool *p_changed = NULL ) : m_x( var ), m_p_changed( p_changed ) {}

		// Assignment
		__forceinline void operator= ( const value_type x ) { if ( ::memcmp( &m_x, &x, sizeof(value_type) ) ) { m_x = x; if ( m_p_changed ) *m_p_changed = true; } }

		// Get the values
		__forceinline operator const value_type ( void ) const { return m_x; }

	private:	// The variable
		value_type	&m_x;
		bool		*m_p_changed;
	};
}

/*	contrast:	matrix([C,0,0,(1-C)/2],
					   [0,1,0,0],
					   [0,0,1,0],
					   [0,0,0,1] );

	brightness:	matrix([1,0,0,B],
					   [0,1,0,0],
					   [0,0,1,0],
					   [0,0,0,1] );

	saturation:	matrix([1,0,0,0],
					   [0,S,0,(1-S)/2],
					   [0,0,S,(1-S)/2],
					   [0,0,0,1] );

	hue: 		matrix([1,0,0,0],
					   [0,cos(H),-sin(H),(sin(H)-cos(H)+1)/2],
					   [0,sin(H), cos(H),(1-sin(H)-cos(H))/2],
					   [0,0,0,1] );

	uoffset:	matrix([1,0,0,0],
					   [0,1,0,UO],
					   [0,0,1,0],
					   [0,0,0,1] );

	voffset:	matrix([1,0,0,0],
					   [0,1,0,0],
					   [0,0,1,VO],
					   [0,0,0,1] );

	ugain:		matrix([1,0,0,0],
					   [0,UG,0,(1-UG)/2],
					   [0,0,1,0],
					   [0,0,0,1] );

	vgain:		matrix([1,0,0,0],
					   [0,1,0,0],
					   [0,0,VG,(1-VG)/2],
					   [0,0,0,1] );

	This is the order the values are applied in :
		contrast
		brightness
		hue
		saturation
		ugain
		vgain
		uoffset
		voffset
*/
struct procamp_matrix
{			// This puts all color correction settings into a single place.
			struct settings
			{	float m_contrast;
				float m_brightness;
				float m_saturation;
				float m_hue;
				float m_u_offset;
				float m_v_offset;
				float m_u_gain;
				float m_v_gain;

				// Set the settings to default
				void reset( void );
			};
			
			// Constructor
			procamp_matrix( void );
			procamp_matrix( const settings &current_settings );
			typedef Matrix::SPMatrix SPMatrix;
			
			// Is the procamp default ?
			bool is_default( void ) const;

			// Get the color conversion matrix in RGB space
			const SPMatrix& rgb( void ) const;
			
			// Get the color conversion matrix in YCbCr color space
			operator const SPMatrix& ( void ) const;
			const SPMatrix& ycbcr( void ) const;
			
			// Get the color correction matrix which also converts from RGB to YCbCr while applying the correction
			const SPMatrix& to_ycbcr( void ) const;
			
			// Get the color correction matrix which also converts from YCbCr to RGB
			const SPMatrix& to_rgb( void ) const;
			
			// Set the values to default
			void reset( void );
			
			// Set and get the values
			FGL::parameter_access<float> contrast( void );
			FGL::parameter_access<float> brightness( void );
			FGL::parameter_access<float> saturation( void );
			FGL::parameter_access<float> hue( void );
			FGL::parameter_access<float> u_offset( void );
			FGL::parameter_access<float> v_offset( void );
			FGL::parameter_access<float> u_gain( void );
			FGL::parameter_access<float> v_gain( void );
			
			// Change all settings at once
			operator const settings& ( void ) const;
			void operator= ( const settings& from );
			
			// Get the values
			const float contrast( void ) const;
			const float brightness( void ) const;
			const float saturation( void ) const;
			const float hue( void ) const;
			const float u_offset( void ) const;
			const float v_offset( void ) const;
			const float u_gain( void ) const;
			const float v_gain( void ) const;
			
private:	// The current matrix
			mutable SPMatrix	m_matr_ycbcr;
			mutable SPMatrix	m_matr_rgb;
			mutable SPMatrix	m_matr_to_rgb;
			mutable SPMatrix	m_matr_to_ycbcr;
			
			// Setup 
			void setup( void ) const;
			
			// Have any values changed
			mutable	bool m_changed;
			
			// The current settings
			settings m_settings;
};

class Procamp_Manager
{
	public:
		Procamp_Manager();
		bool Set_ProcAmp(ProcAmp_enum ProcSetting,double value);
		double Get_ProcAmp(ProcAmp_enum ProcSetting) const;
		//Client code can check for NULL to setup in an optimized fashion
		typedef float color_matrix[ 3 ][ 4 ];
		virtual const color_matrix *Get_Procamp_Matrix() const {return m_MatrixToSend;}

		static void* operator new ( const size_t size )
		{
			return ::_aligned_malloc( size,16 );
		}
		static void  operator delete ( void* ptr )
		{
			::_aligned_free(ptr);
		}

		void operator()(FrameWork::Bitmaps::bitmap_ycbcr_u8 &apply_to_bitmap);
	private:
		double m_FloodControl[e_no_procamp_items]; //ProcAmp() uses this to determine if a change has been made
		//The component I2C's require a 4x3 matrix; the procamp object handles these values
		procamp_matrix m_SDI_Matrix;
		//Keep these aligned!
		Matrix::SPMatrix m_ycbcr_matrix;  //This contains an up-to-date color matrix to be used
		//This is a transposed form used for processing FX
		Matrix::SPMatrix m_MatrixTransposed;
		color_matrix *m_MatrixToSend; //This is a transposed matrix to use for processing FX (this is not needed one hardware is working)

};
