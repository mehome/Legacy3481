#pragma warning(disable : 4244)
#include "osg_Matrix.h"
#pragma warning(default : 4244)

// specialise Matrix_implementaiton to be Matrixd
#define  Matrix_implementation Matrixd

osg::Matrixd::Matrixd( const osg::Matrixf& mat )
{
    set(mat.ptr());
}

osg::Matrixd& osg::Matrixd::operator = (const osg::Matrixf& rhs)
{
    set(rhs.ptr());
    return *this;
}

void osg::Matrixd::set(const osg::Matrixf& rhs)
{
    set(rhs.ptr());
}

// now compile up Matrix via Matrix_implementation
#include "Matrix_implementation.hpp"

#ifdef __UseSSE__
//TODO offer possible SSE2 optimization; for now the c code is hard to beat!
#define SET_ROW(row, v1, v2, v3, v4 )    \
	_mat[(row)][0] = (v1); \
	_mat[(row)][1] = (v2); \
	_mat[(row)][2] = (v3); \
	_mat[(row)][3] = (v4);

#define INNER_PRODUCT(a,b,r,c) \
	((a)._mat[r][0] * (b)._mat[0][c]) \
	+((a)._mat[r][1] * (b)._mat[1][c]) \
	+((a)._mat[r][2] * (b)._mat[2][c]) \
	+((a)._mat[r][3] * (b)._mat[3][c])


void Matrix_implementation::mult( const Matrix_implementation& lhs, const Matrix_implementation& rhs )
{   
	if (&lhs==this)
	{
		postMult(rhs);
		return;
	}
	if (&rhs==this)
	{
		preMult(lhs);
		return;
	}

	// PRECONDITION: We assume neither &lhs nor &rhs == this
	// if it did, use preMult or postMult instead
	_mat[0][0] = INNER_PRODUCT(lhs, rhs, 0, 0);
	_mat[0][1] = INNER_PRODUCT(lhs, rhs, 0, 1);
	_mat[0][2] = INNER_PRODUCT(lhs, rhs, 0, 2);
	_mat[0][3] = INNER_PRODUCT(lhs, rhs, 0, 3);
	_mat[1][0] = INNER_PRODUCT(lhs, rhs, 1, 0);
	_mat[1][1] = INNER_PRODUCT(lhs, rhs, 1, 1);
	_mat[1][2] = INNER_PRODUCT(lhs, rhs, 1, 2);
	_mat[1][3] = INNER_PRODUCT(lhs, rhs, 1, 3);
	_mat[2][0] = INNER_PRODUCT(lhs, rhs, 2, 0);
	_mat[2][1] = INNER_PRODUCT(lhs, rhs, 2, 1);
	_mat[2][2] = INNER_PRODUCT(lhs, rhs, 2, 2);
	_mat[2][3] = INNER_PRODUCT(lhs, rhs, 2, 3);
	_mat[3][0] = INNER_PRODUCT(lhs, rhs, 3, 0);
	_mat[3][1] = INNER_PRODUCT(lhs, rhs, 3, 1);
	_mat[3][2] = INNER_PRODUCT(lhs, rhs, 3, 2);
	_mat[3][3] = INNER_PRODUCT(lhs, rhs, 3, 3);
}

void Matrix_implementation::preMult( const Matrix_implementation& other )
{
	// brute force method requiring a copy
	//Matrix_implementation tmp(other* *this);
	// *this = tmp;

	// more efficient method just use a value_type[4] for temporary storage.
	value_type t[4];
	for(int col=0; col<4; ++col) {
		t[0] = INNER_PRODUCT( other, *this, 0, col );
		t[1] = INNER_PRODUCT( other, *this, 1, col );
		t[2] = INNER_PRODUCT( other, *this, 2, col );
		t[3] = INNER_PRODUCT( other, *this, 3, col );
		_mat[0][col] = t[0];
		_mat[1][col] = t[1];
		_mat[2][col] = t[2];
		_mat[3][col] = t[3];
	}

}

void Matrix_implementation::postMult( const Matrix_implementation& other )
{
	// brute force method requiring a copy
	//Matrix_implementation tmp(*this * other);
	// *this = tmp;

	// more efficient method just use a value_type[4] for temporary storage.
	value_type t[4];
	for(int row=0; row<4; ++row)
	{
		t[0] = INNER_PRODUCT( *this, other, row, 0 );
		t[1] = INNER_PRODUCT( *this, other, row, 1 );
		t[2] = INNER_PRODUCT( *this, other, row, 2 );
		t[3] = INNER_PRODUCT( *this, other, row, 3 );
		SET_ROW(row, t[0], t[1], t[2], t[3] )
	}
}

#undef INNER_PRODUCT

bool Matrix_implementation::invert_4x4( const Matrix_implementation& mat )
{
    if (&mat==this) {
       Matrix_implementation tm(mat);
       return invert_4x4(tm);
    }

    unsigned int indxc[4], indxr[4], ipiv[4];
    unsigned int i,j,k,l,ll;
    unsigned int icol = 0;
    unsigned int irow = 0;
    double temp, pivinv, dum, big;

    // copy in place this may be unnecessary
    *this = mat;

    for (j=0; j<4; j++) ipiv[j]=0;

    for(i=0;i<4;i++)
    {
       big=0.0;
       for (j=0; j<4; j++)
          if (ipiv[j] != 1)
             for (k=0; k<4; k++)
             {
                if (ipiv[k] == 0)
                {
                   if (SGL_ABS(operator()(j,k)) >= big)
                   {
                      big = SGL_ABS(operator()(j,k));
                      irow=j;
                      icol=k;
                   }
                }
                else if (ipiv[k] > 1)
                   return false;
             }
       ++(ipiv[icol]);
       if (irow != icol)
          for (l=0; l<4; l++) SGL_SWAP(operator()(irow,l),
                                       operator()(icol,l),
                                       temp);

       indxr[i]=irow;
       indxc[i]=icol;
       if (operator()(icol,icol) == 0)
          return false;

       pivinv = 1.0/operator()(icol,icol);
       operator()(icol,icol) = 1;
       for (l=0; l<4; l++) operator()(icol,l) *= pivinv;
       for (ll=0; ll<4; ll++)
          if (ll != icol)
          {
             dum=operator()(ll,icol);
             operator()(ll,icol) = 0;
             for (l=0; l<4; l++) operator()(ll,l) -= operator()(icol,l)*dum;
          }
    }
    for (int lx=4; lx>0; --lx)
    {
       if (indxr[lx-1] != indxc[lx-1])
          for (k=0; k<4; k++) SGL_SWAP(operator()(k,indxr[lx-1]),
                                       operator()(k,indxc[lx-1]),temp);
    }

    return true;
}

#undef SET_ROW
#endif //__UseSSE__