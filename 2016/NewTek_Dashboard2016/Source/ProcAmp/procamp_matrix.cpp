#include "StdAfx.h"
#include "ProcAmp.h"

using namespace Processing::FX::procamp;
using namespace Processing::FX::procamp::Matrix;

static const SPMatrix	g_rgb_to_ycbcr( 0.257f ,-0.148f, 0.439f, 0.000f,
									    0.504f ,-0.291f,-0.368f, 0.000f,
									    0.098f , 0.439f,-0.071f, 0.000f,
									    0.0627f, 0.502f, 0.502f, 1.000f );

// This is the exact inverse of the matrix above and not what is most commonly used in color conversion
// This ensures that a round trip to ycbcr is exact. This would only introduce a very minor error when 
// actually performing conversions.
static const SPMatrix	g_ycbcr_to_rgb(  1.1641443538998849f, 1.164144353899884f, 1.1641443538998840f, 0.0f,
									    -0.0017888977136207f,-0.391442764342240f, 2.0178255096008960f, 0.0f,
									     1.5957862054353390f,-0.813482068507790f,-0.0012458394791287f, 0.0f,
									    -0.8731784994658300f, 0.531880415101190f,-1.0853148453906500f, 1.0f );

static const SPMatrix	g_ycbcr_default(  1.0f, 0.0f, 0.0f, 0.0f,
										  0.0f, 1.0f, 0.0f, 0.0f,
									      0.0f, 0.0f, 1.0f, 0.0f,
										  0.0f, 0.0f, 0.0f, 1.0f );


  /***********************************************************************************************************************/
 /*														procamp_matrix													*/
/***********************************************************************************************************************/


const SPMatrix& procamp_matrix::rgb( void ) const
{	// Setup
	setup();

	// Return the matrix
	return m_matr_rgb;
}

const SPMatrix& procamp_matrix::ycbcr( void ) const
{	// Setup
	setup();

	// Return the matrix
	return m_matr_ycbcr;
}

// Get the color correction matrix which also converts from RGB to YCbCr while applying the correction
const SPMatrix& procamp_matrix::to_ycbcr( void ) const
{	// Setup
	setup();

	// Return the matrix
	return m_matr_to_ycbcr;
}

// Get the color correction matrix which also converts from YCbCr to RGB
const SPMatrix& procamp_matrix::to_rgb( void ) const
{	// Setup
	setup();

	// Return the matrix
	return m_matr_to_rgb;
}

// Get the color conversion matrix
procamp_matrix::operator const SPMatrix& ( void ) const
{	// Return the matrix
	return ycbcr();
}

// Setup 
void procamp_matrix::setup( void ) const
{	// Update the matrix
	if ( m_changed ) 
	{	// Convert the hue to radians
		const float deg_to_rad = 3.1415926f / 180.0f;
		const float hue_rad = deg_to_rad * m_settings.m_hue;

		// Setup the matrix
		m_matr_ycbcr[ 0 ][ 0 ] = m_settings.m_contrast;
		m_matr_ycbcr[ 1 ][ 0 ] = 0.0f;
		m_matr_ycbcr[ 2 ][ 0 ] = 0.0f;
		m_matr_ycbcr[ 3 ][ 0 ] = -(m_settings.m_contrast-2.0f*m_settings.m_brightness-1.0f)/2.0f;

		m_matr_ycbcr[ 0 ][ 1 ] = 0.0f;
		m_matr_ycbcr[ 1 ][ 1 ] =  cos(hue_rad)*m_settings.m_saturation*m_settings.m_u_gain;
		m_matr_ycbcr[ 2 ][ 1 ] = -sin(hue_rad)*m_settings.m_saturation*m_settings.m_u_gain;
		m_matr_ycbcr[ 3 ][ 1 ] = (2.0f*m_settings.m_u_offset+(sin(hue_rad)-cos(hue_rad))*m_settings.m_saturation*m_settings.m_u_gain+1.0f)/2.0f;

		m_matr_ycbcr[ 0 ][ 2 ] = 0.0f;
		m_matr_ycbcr[ 1 ][ 2 ] = sin(hue_rad)*m_settings.m_saturation*m_settings.m_v_gain;
		m_matr_ycbcr[ 2 ][ 2 ] = cos(hue_rad)*m_settings.m_saturation*m_settings.m_v_gain;
		m_matr_ycbcr[ 3 ][ 2 ] = (2.0f*m_settings.m_v_offset+(-sin(hue_rad)-cos(hue_rad))*m_settings.m_saturation*m_settings.m_v_gain+1.0f)/2.0f;

		m_matr_ycbcr[ 0 ][ 3 ] = 0.0f;
		m_matr_ycbcr[ 1 ][ 3 ] = 0.0f;
		m_matr_ycbcr[ 2 ][ 3 ] = 0.0f;
		m_matr_ycbcr[ 3 ][ 3 ] = 1.0f;

		m_matr_rgb      = g_ycbcr_to_rgb * m_matr_ycbcr * g_rgb_to_ycbcr;
		m_matr_to_rgb   = g_ycbcr_to_rgb * m_matr_ycbcr;
		m_matr_to_ycbcr = m_matr_ycbcr * g_rgb_to_ycbcr;

		// Not changes
		m_changed = false;
	}
}

// Constructor
procamp_matrix::procamp_matrix( void )
{	reset();
}

procamp_matrix::procamp_matrix( const settings &current_settings )
{	m_settings = current_settings;
	m_changed = true;
}

void procamp_matrix::reset( void )
{	m_settings.reset();
	m_changed = true;
}

// Set the settings to default
void procamp_matrix::settings::reset( void )
{	m_contrast = 1.0f;
	m_brightness = 0.0f;
	m_saturation = 1.0f;
	m_hue = 0.0f;
	m_u_offset = 0.0f;
	m_v_offset = 0.0f;
	m_u_gain = 1.0f;
	m_v_gain = 1.0f;
}

bool  procamp_matrix::is_default( void ) const
{
	SPMatrix matrix(m_matr_ycbcr - g_ycbcr_default);
	float MaxValue=matrix.MaxValue();
	bool ret=MaxValue==0.0;
	//FrameWork::Debug::debug_output(p_debug_category,L"is_default=%d",ret);
	return ret;
}

// Set and get the values
FGL::parameter_access<float>	procamp_matrix::contrast( void )
{	return FGL::parameter_access<float>( m_settings.m_contrast, &m_changed );
}

const float procamp_matrix::contrast( void ) const
{	return m_settings.m_contrast;
}

FGL::parameter_access<float>	procamp_matrix::brightness( void )
{	return FGL::parameter_access<float>( m_settings.m_brightness, &m_changed );
}

const float procamp_matrix::brightness( void ) const
{	return m_settings.m_brightness;
}

FGL::parameter_access<float>	procamp_matrix::saturation( void )
{	return FGL::parameter_access<float>( m_settings.m_saturation, &m_changed );
}

const float procamp_matrix::saturation( void ) const
{	return m_settings.m_saturation;
}

FGL::parameter_access<float>	procamp_matrix::hue( void )
{	return FGL::parameter_access<float>( m_settings.m_hue, &m_changed );
}

const float procamp_matrix::hue( void ) const
{	return m_settings.m_hue;
}

FGL::parameter_access<float>	procamp_matrix::u_offset( void )
{	return FGL::parameter_access<float>( m_settings.m_u_offset, &m_changed );
}

const float procamp_matrix::u_offset( void ) const
{	return m_settings.m_u_offset;
}

FGL::parameter_access<float>	procamp_matrix::v_offset( void )
{	return FGL::parameter_access<float>( m_settings.m_v_offset, &m_changed );
}

const float procamp_matrix::v_offset( void ) const
{	return m_settings.m_v_offset;
}

FGL::parameter_access<float>	procamp_matrix::u_gain( void )
{	return FGL::parameter_access<float>( m_settings.m_u_gain, &m_changed );
}

const float procamp_matrix::u_gain( void ) const
{	return m_settings.m_u_gain;
}

FGL::parameter_access<float>	procamp_matrix::v_gain( void )
{	return FGL::parameter_access<float>( m_settings.m_v_gain, &m_changed );
}

const float procamp_matrix::v_gain( void ) const
{	return m_settings.m_v_gain;
}

// Change all settings at once
procamp_matrix::operator const procamp_matrix::settings& ( void ) const
{	return m_settings;
}

void procamp_matrix::operator= ( const settings& from )
{	m_settings = from;
	m_changed = true;
}


  /***********************************************************************************************************************/
 /*														Procamp_Manager													*/
/***********************************************************************************************************************/


//These should never change
const double c_procamp_NonCalibrated_brightness	= 0.0;
const double c_procamp_NonCalibrated_contrast	= 1.0;
const double c_procamp_NonCalibrated_hue		= 0.0;
const double c_procamp_NonCalibrated_saturation	= 1.0;
const double c_procamp_NonCalibrated_u_offset	= 0.0;
const double c_procamp_NonCalibrated_v_offset	= 0.0;
const double c_procamp_NonCalibrated_u_gain		= 1.0;
const double c_procamp_NonCalibrated_v_gain		= 1.0;

double ProcAmp_Defaults[e_no_procamp_items]=
{
	c_procamp_NonCalibrated_brightness,
	c_procamp_NonCalibrated_contrast,
	c_procamp_NonCalibrated_hue,
	c_procamp_NonCalibrated_saturation,
	c_procamp_NonCalibrated_u_offset,
	c_procamp_NonCalibrated_v_offset,
	c_procamp_NonCalibrated_u_gain,
	c_procamp_NonCalibrated_v_gain,
	0.0, //pedestal
};

Procamp_Manager::Procamp_Manager()
{
	//Note: for the flood control... component will set only one value to -55 to ensure the component gets called at least only once
	//For standard def each setting will get set as each one has a corresponding i2c
	for (size_t i=0;i<e_no_procamp_items;i++)
	{
		if (i!=0)
		{
			//We could init to the defaults, but it would be better to force it to set everything properly the first time
			double value=ProcAmp_Defaults[i];
			m_FloodControl[i]=value;
		}
		else
			m_FloodControl[i]=-55;  //pick some number that is out of range for all
	}
	m_MatrixToSend=NULL; //hold off until we get a legit change
}

bool Procamp_Manager::Set_ProcAmp(ProcAmp_enum ProcSetting,double value)
{
	bool ret=true;
	if (m_FloodControl[ProcSetting]!=value)
	{
		m_FloodControl[ProcSetting]=value;
		bool ret=false;

		switch (ProcSetting)
		{
		case e_procamp_brightness:
			m_SDI_Matrix.brightness()=(float)value;
			break;
		case e_procamp_contrast:
			m_SDI_Matrix.contrast()=(float)value;
			break;
		case e_procamp_hue:
			//This gets the hue in the right direction for software matrix.
			m_SDI_Matrix.hue()=(float)(value*-1.0);
			break;
		case e_procamp_saturation:
			m_SDI_Matrix.saturation()=(float)value;
			break;
		case e_procamp_u_offset:
			m_SDI_Matrix.u_offset()=(float)value;
			break;
		case e_procamp_v_offset:
			m_SDI_Matrix.v_offset()=(float)value;
			break;
		case e_procamp_u_gain:
			m_SDI_Matrix.u_gain()=(float)value;
			break;
		case e_procamp_v_gain:
			m_SDI_Matrix.v_gain()=(float)value;
			break;
		}
		m_ycbcr_matrix=m_SDI_Matrix.ycbcr();
		{//Send this as matrix to send
			m_MatrixTransposed=m_ycbcr_matrix;
			m_MatrixTransposed.Transpose();
			m_MatrixToSend=m_SDI_Matrix.is_default()?NULL:(color_matrix *)&m_MatrixTransposed;
		}

	}
	return ret;
}

double Procamp_Manager::Get_ProcAmp(ProcAmp_enum ProcSetting) const
{
	double ret=0.0;
	switch (ProcSetting)
	{
	case e_procamp_brightness:
		ret=m_SDI_Matrix.brightness();
		break;
	case e_procamp_contrast:
		ret=m_SDI_Matrix.contrast();
		break;
	case e_procamp_hue:
		//This gets the hue in the right direction for software matrix.
		ret=m_SDI_Matrix.hue()*-1.0f;
		break;
	case e_procamp_saturation:
		ret=m_SDI_Matrix.saturation();
		break;
	case e_procamp_u_offset:
		ret=m_SDI_Matrix.u_offset();
		break;
	case e_procamp_v_offset:
		ret=m_SDI_Matrix.v_offset();
		break;
	case e_procamp_u_gain:
		ret=m_SDI_Matrix.u_gain();
		break;
	case e_procamp_v_gain:
		ret=m_SDI_Matrix.v_gain();
		break;
	}
	return ret;
}

void Procamp_Manager::operator()(FrameWork::Bitmaps::bitmap_ycbcr_u8 &apply_to_bitmap)
{
	//This is a temporary solution to apply software procamp to any chip that returns successful to use a color matrix
	const color_matrix *matrix=Get_Procamp_Matrix();
	if (matrix)
		ycbcr(apply_to_bitmap,apply_to_bitmap,*matrix);
}
