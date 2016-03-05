#pragma warning(disable : 4244)
#include "osg_Matrix.h"

// specialise Matrix_implementaiton to be Matrixf
#define  Matrix_implementation Matrixf

osg::Matrixf::Matrixf( const osg::Matrixd& mat )
{
    set(mat.ptr());
}

osg::Matrixf& osg::Matrixf::operator = (const osg::Matrixd& rhs)
{
    set(rhs.ptr());
    return *this;
}

void osg::Matrixf::set(const osg::Matrixd& rhs)
{
    set(rhs.ptr());
}

// now compile up Matrix via Matrix_implementation
#include "Matrix_implementation.hpp"
#pragma warning(default : 4244)

#ifdef __UseSSE__

#include <xmmintrin.h> /* Streaming SIMD Extensions Intrinsics include file */

#include <assert.h>
#include "fvec.h" 

#define is_aligned( a ) ( ! ( 15 & (size_t)a ) )

void Matrix_implementation::mult( const Matrix_implementation& lhs, const Matrix_implementation& rhs )
{   
	if (&lhs==this)
	{
		postMult(rhs);
		return;
	}

	//TODO see if aligned is worth effort
	Aligned_Vec4f temp1,temp2;  //general purpose registers for temporary storage
	Aligned_Vec4f OtherRow1=_mm_loadu_ps(rhs.ptr()+ 0);
	Aligned_Vec4f OtherRow2=_mm_loadu_ps(rhs.ptr()+ 4);
	Aligned_Vec4f OtherRow3=_mm_loadu_ps(rhs.ptr()+ 8);
	Aligned_Vec4f OtherRow4=_mm_loadu_ps(rhs.ptr()+12);

	temp1 =  Aligned_Vec4f(lhs(0,0)) * OtherRow1;
	temp2 =  Aligned_Vec4f(lhs(1,0)) * OtherRow1;
	temp1 += Aligned_Vec4f(lhs(0,1)) * OtherRow2;
	temp2 += Aligned_Vec4f(lhs(1,1)) * OtherRow2;
	temp1 += Aligned_Vec4f(lhs(0,2)) * OtherRow3;
	temp2 += Aligned_Vec4f(lhs(1,2)) * OtherRow3;
	temp1 += Aligned_Vec4f(lhs(0,3)) * OtherRow4;
	temp2 += Aligned_Vec4f(lhs(1,3)) * OtherRow4;
	_mm_storeu_ps(&_mat[0][0],temp1);
	_mm_storeu_ps(&_mat[1][0],temp2);

	temp1 =  Aligned_Vec4f(lhs(2,0)) * OtherRow1;
	temp2 =  Aligned_Vec4f(lhs(3,0)) * OtherRow1;
	temp1 += Aligned_Vec4f(lhs(2,1)) * OtherRow2;
	temp2 += Aligned_Vec4f(lhs(3,1)) * OtherRow2;
	temp1 += Aligned_Vec4f(lhs(2,2)) * OtherRow3;
	temp2 += Aligned_Vec4f(lhs(3,2)) * OtherRow3;
	temp1 += Aligned_Vec4f(lhs(2,3)) * OtherRow4;
	temp2 += Aligned_Vec4f(lhs(3,3)) * OtherRow4;
	_mm_storeu_ps(&_mat[2][0],temp1);
	_mm_storeu_ps(&_mat[3][0],temp2);
}

void Matrix_implementation::preMult( const Matrix_implementation& other )
{
	//This may be optimized similar to postMult, but we'll need to check
	mult(other,*this);
}

void Matrix_implementation::postMult( const Matrix_implementation& other )
{
	//TODO see if aligned is worth effort
	Aligned_Vec4f temp1,temp2;  //general purpose registers for temporary storage
	Aligned_Vec4f OtherRow1=_mm_loadu_ps(other.ptr()+ 0);
	Aligned_Vec4f OtherRow2=_mm_loadu_ps(other.ptr()+ 4);
	Aligned_Vec4f OtherRow3=_mm_loadu_ps(other.ptr()+ 8);
	Aligned_Vec4f OtherRow4=_mm_loadu_ps(other.ptr()+12);
	Aligned_Vec4f _R0=_mm_loadu_ps(&_mat[0][0]);
	Aligned_Vec4f _R1=_mm_loadu_ps(&_mat[1][0]);
	Aligned_Vec4f _R2=_mm_loadu_ps(&_mat[2][0]);
	Aligned_Vec4f _R3=_mm_loadu_ps(&_mat[3][0]);


	temp1 =  (Aligned_Vec4f)_mm_shuffle_ps(_R0,_R0,_MM_SHUFFLE(0,0,0,0)) * OtherRow1;
	temp2 =  (Aligned_Vec4f)_mm_shuffle_ps(_R1,_R1,_MM_SHUFFLE(0,0,0,0)) * OtherRow1;
	temp1 += (Aligned_Vec4f)_mm_shuffle_ps(_R0,_R0,_MM_SHUFFLE(1,1,1,1)) * OtherRow2;
	temp2 += (Aligned_Vec4f)_mm_shuffle_ps(_R1,_R1,_MM_SHUFFLE(1,1,1,1)) * OtherRow2;
	temp1 += (Aligned_Vec4f)_mm_shuffle_ps(_R0,_R0,_MM_SHUFFLE(2,2,2,2)) * OtherRow3;
	temp2 += (Aligned_Vec4f)_mm_shuffle_ps(_R1,_R1,_MM_SHUFFLE(2,2,2,2)) * OtherRow3;
	temp1 += (Aligned_Vec4f)_mm_shuffle_ps(_R0,_R0,_MM_SHUFFLE(3,3,3,3)) * OtherRow4;
	temp2 += (Aligned_Vec4f)_mm_shuffle_ps(_R1,_R1,_MM_SHUFFLE(3,3,3,3)) * OtherRow4;
	_mm_storeu_ps(&_mat[0][0],temp1);
	_mm_storeu_ps(&_mat[1][0],temp2);

	temp1 =  (Aligned_Vec4f)_mm_shuffle_ps(_R2,_R2,_MM_SHUFFLE(0,0,0,0)) * OtherRow1;
	temp2 =  (Aligned_Vec4f)_mm_shuffle_ps(_R3,_R3,_MM_SHUFFLE(0,0,0,0)) * OtherRow1;
	temp1 += (Aligned_Vec4f)_mm_shuffle_ps(_R2,_R2,_MM_SHUFFLE(1,1,1,1)) * OtherRow2;
	temp2 += (Aligned_Vec4f)_mm_shuffle_ps(_R3,_R3,_MM_SHUFFLE(1,1,1,1)) * OtherRow2;
	temp1 += (Aligned_Vec4f)_mm_shuffle_ps(_R2,_R2,_MM_SHUFFLE(2,2,2,2)) * OtherRow3;
	temp2 += (Aligned_Vec4f)_mm_shuffle_ps(_R3,_R3,_MM_SHUFFLE(2,2,2,2)) * OtherRow3;
	temp1 += (Aligned_Vec4f)_mm_shuffle_ps(_R2,_R2,_MM_SHUFFLE(3,3,3,3)) * OtherRow4;
	temp2 += (Aligned_Vec4f)_mm_shuffle_ps(_R3,_R3,_MM_SHUFFLE(3,3,3,3)) * OtherRow4;
	_mm_storeu_ps(&_mat[2][0],temp1);
	_mm_storeu_ps(&_mat[3][0],temp2);
}

//The following code has been translated from the matlib2 source here;
//http://www.cs.technion.ac.il/~zdevir/main1.html
//The following changes have been made:
// 1. Expanded the mm_shuffle immediate to a more readable form
// 2. Using unaligned pointers instead of the SPMatrix class

const _MM_ALIGN16 __int32 __MASKSIGNs_[4] = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const _MM_ALIGN16 __int32 _Sign_PNPN[4] = { 0x00000000, 0x80000000, 0x00000000, 0x80000000 };
const _MM_ALIGN16 __int32 _Sign_NPNP[4] = { 0x80000000, 0x00000000, 0x80000000, 0x00000000 };
const _MM_ALIGN16 __int32 _Sign_PNNP[4] = { 0x00000000, 0x80000000, 0x80000000, 0x00000000 };
const _MM_ALIGN16 __int32 __0FFF_[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
const _MM_ALIGN16 float __ZERONE_[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
const _MM_ALIGN16 float __1000_[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

#define _MASKSIGNs_ (*(Aligned_Vec4f*)&__MASKSIGNs_)      // - - - -
#define Sign_PNPN   (*(Aligned_Vec4f*)&_Sign_PNPN)        // + - + -
#define Sign_NPNP   (*(Aligned_Vec4f*)&_Sign_NPNP)        // - + - +
#define Sign_PNNP   (*(Aligned_Vec4f*)&_Sign_PNNP)        // + - - +
#define _0FFF_      (*(Aligned_Vec4f*)&__0FFF_)           // 0 * * *
#define _ZERONE_    (*(Aligned_Vec4f*)&__ZERONE_)         // 1 0 0 1
#define _1000_      (*(Aligned_Vec4f*)&__1000_)           // 1 0 0 0

bool Matrix_implementation::invert_4x4( const Matrix_implementation& mat )
{
	// The inverse is calculated using a "Divide and Conquer" technique. The original matrix is divided into four 2x2 sub-matrices. Since each 
	// register holds four matrix elements, the smaller matrices are represented as registers. Hence we get a better locality of the 
	// calculations.
	Aligned_Vec4f MatRow1=_mm_loadu_ps(mat.ptr()+ 0);
	Aligned_Vec4f MatRow2=_mm_loadu_ps(mat.ptr()+ 4);
	Aligned_Vec4f MatRow3=_mm_loadu_ps(mat.ptr()+ 8);
	Aligned_Vec4f MatRow4=_mm_loadu_ps(mat.ptr()+12);

	Aligned_Vec4f A = _mm_movelh_ps(MatRow1, MatRow2),    // the four sub-matrices 
			B = _mm_movehl_ps(MatRow2, MatRow1),
			C = _mm_movelh_ps(MatRow3, MatRow4),
			D = _mm_movehl_ps(MatRow4, MatRow3);
	Aligned_Vec4f iA, iB, iC, iD,					// partial inverse of the sub-matrices
			DC, AB;
	F32vec1 dA, dB, dC, dD;                 // determinant of the sub-matrices
	F32vec1 det, d, d1, d2;
	Aligned_Vec4f rd;

	//  AB = A# * B
	AB = _mm_mul_ps(_mm_shuffle_ps(A,A,_MM_SHUFFLE(0,0,3,3)), B);
	AB -= (Aligned_Vec4f)_mm_mul_ps(_mm_shuffle_ps(A,A,_MM_SHUFFLE(2,2,1,1)), _mm_shuffle_ps(B,B,_MM_SHUFFLE(1,0,3,2)));
	//  DC = D# * C
	DC = _mm_mul_ps(_mm_shuffle_ps(D,D,_MM_SHUFFLE(0,0,3,3)), C);
	DC -= (Aligned_Vec4f)_mm_mul_ps(_mm_shuffle_ps(D,D,_MM_SHUFFLE(2,2,1,1)), _mm_shuffle_ps(C,C,_MM_SHUFFLE(1,0,3,2)));

	//  dA = |A|
	dA = _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(1,1,3,3)),A);
	dA = _mm_sub_ss(dA, _mm_movehl_ps(dA,dA));
	//  dB = |B|
	dB = _mm_mul_ps(_mm_shuffle_ps(B, B, _MM_SHUFFLE(1,1,3,3)),B);
	dB = _mm_sub_ss(dB, _mm_movehl_ps(dB,dB));

	//  dC = |C|
	dC = _mm_mul_ps(_mm_shuffle_ps(C, C, _MM_SHUFFLE(1,1,3,3)),C);
	dC = _mm_sub_ss(dC, _mm_movehl_ps(dC,dC));
	//  dD = |D|
	dD = _mm_mul_ps(_mm_shuffle_ps(D, D, _MM_SHUFFLE(1,1,3,3)),D);
	dD = _mm_sub_ss(dD, _mm_movehl_ps(dD,dD));

	//  d = trace(AB*DC) = trace(A#*B*D#*C)
	d = _mm_mul_ps(_mm_shuffle_ps(DC,DC,_MM_SHUFFLE(3,1,2,0)),AB);

	//  iD = C*A#*B
	iD = _mm_mul_ps(_mm_shuffle_ps(C,C,_MM_SHUFFLE(2,2,0,0)), _mm_movelh_ps(AB,AB));
	iD += (Aligned_Vec4f)_mm_mul_ps(_mm_shuffle_ps(C,C,_MM_SHUFFLE(3,3,1,1)), _mm_movehl_ps(AB,AB));
	//  iA = B*D#*C
	iA = _mm_mul_ps(_mm_shuffle_ps(B,B,_MM_SHUFFLE(2,2,0,0)), _mm_movelh_ps(DC,DC));
	iA += (Aligned_Vec4f)_mm_mul_ps(_mm_shuffle_ps(B,B,_MM_SHUFFLE(3,3,1,1)), _mm_movehl_ps(DC,DC));

	//  d = trace(AB*DC) = trace(A#*B*D#*C) [continue]
	d = _mm_add_ps(d, _mm_movehl_ps(d, d));
	d = _mm_add_ss(d, _mm_shuffle_ps(d, d, _MM_SHUFFLE(0,0,0,1)));
	d1 = dA*dD;
	d2 = dB*dC;

	//  iD = D*|A| - C*A#*B
	iD = D*_mm_shuffle_ps(dA,dA,_MM_SHUFFLE(0,0,0,0)) - iD;

	//  iA = A*|D| - B*D#*C;
	iA = A*_mm_shuffle_ps(dD,dD,_MM_SHUFFLE(0,0,0,0)) - iA;

	//  det = |A|*|D| + |B|*|C| - trace(A#*B*D#*C)
	det = d1+d2-d;
	rd = (__m128)(F32vec1(1.0f)/det);

	//TODO see if inverse of a singular matrix should zero it
	//rd = _mm_and_ps(_mm_cmpneq_ss(det,_mm_setzero_ps()), rd);

	//  iB = D * (A#B)# = D*B#*A
	iB = _mm_mul_ps(D, _mm_shuffle_ps(AB,AB,_MM_SHUFFLE(0,3,0,3)));
	iB -= (Aligned_Vec4f)_mm_mul_ps(_mm_shuffle_ps(D,D,_MM_SHUFFLE(2,3,0,1)), _mm_shuffle_ps(AB,AB,_MM_SHUFFLE(1,2,1,2)));
	//  iC = A * (D#C)# = A*C#*D
	iC = _mm_mul_ps(A, _mm_shuffle_ps(DC,DC,_MM_SHUFFLE(0,3,0,3)));
	iC -= (Aligned_Vec4f)_mm_mul_ps(_mm_shuffle_ps(A,A,_MM_SHUFFLE(2,3,0,1)), _mm_shuffle_ps(DC,DC,_MM_SHUFFLE(1,2,1,2)));

	rd = _mm_shuffle_ps(rd,rd,_MM_SHUFFLE(0,0,0,0));
	rd ^= Sign_PNNP;

	//  iB = C*|B| - D*B#*A
	iB = C*_mm_shuffle_ps(dB,dB,_MM_SHUFFLE(0,0,0,0)) - iB;

	//  iC = B*|C| - A*C#*D;
	iC = B*_mm_shuffle_ps(dC,dC,_MM_SHUFFLE(0,0,0,0)) - iC;

	//  iX = iX / det
	iA *= rd;
	iB *= rd;
	iC *= rd;
	iD *= rd;

	_mm_storeu_ps(&_mat[0][0],_mm_shuffle_ps(iA,iB,_MM_SHUFFLE(1,3,1,3)));
	_mm_storeu_ps(&_mat[1][0],_mm_shuffle_ps(iA,iB,_MM_SHUFFLE(0,2,0,2)));
	_mm_storeu_ps(&_mat[2][0],_mm_shuffle_ps(iC,iD,_MM_SHUFFLE(1,3,1,3)));
	_mm_storeu_ps(&_mat[3][0],_mm_shuffle_ps(iC,iD,_MM_SHUFFLE(0,2,0,2)));

	//Ideally we should return the determinant
	//return *(float*)&det;
	//For now just return true; fortunately no client is using the return value
	return true;
}

#endif //__UseSSE__