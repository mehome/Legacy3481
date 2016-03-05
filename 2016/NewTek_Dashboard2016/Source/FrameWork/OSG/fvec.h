

#ifndef _FVEC_H_INCLUDED
#define _FVEC_H_INCLUDED




namespace osg {

/** General purpose float quad. Uses include representation
  * of color coordinates.
  * No support yet added for float * Vec4f - is it necessary?
  * Need to define a non-member non-friend operator*  etc.
  *    Vec4f * float is okay
*/
class Aligned_Vec4f
{
    public:

        /** Type of Vec class.*/
        typedef float value_type;

        /** Number of vector components. */
        enum { num_components = 4 };
        
        /** Vec member variable. */
		union
		{
			value_type _v[4];
			__m128 vec;
		};

        // Methods are defined here so that they are implicitly inlined
#ifdef __UseSSE__
		// initialize 4 SP FP with __m128 data type 
		Aligned_Vec4f(__m128 m)					{ vec = m;}

		/* Explicitly initialize each of 4 SP FPs with same float */
		explicit Aligned_Vec4f(float f)	{ vec = _mm_set_ps1(f); }

		/* Explicitly initialize each of 4 SP FPs with same double */
		explicit Aligned_Vec4f(double d)	{ vec = _mm_set_ps1((float) d); }

		/* Assignment operations */

		Aligned_Vec4f& operator =(float f) { vec = _mm_set_ps1(f); return *this; }

		Aligned_Vec4f& operator =(double d) { vec = _mm_set_ps1((float) d); return *this; }

		/* Conversion functions */
		operator  __m128() const	{ return vec; }		/* Convert to __m128 */

#endif
        Aligned_Vec4f() 
		{ 
#ifdef __UseSSE__
			vec= _mm_setzero_ps();
#else
			_v[0]=0.0f; _v[1]=0.0f; _v[2]=0.0f; _v[3]=0.0f;
#endif
		}
        
        Aligned_Vec4f(value_type x, value_type y, value_type z, value_type w)
        {
#ifdef __UseSSE__
			vec= _mm_set_ps(x,y,z,w);
#else
            _v[0]=x;
            _v[1]=y;
            _v[2]=z;
            _v[3]=w;
#endif
        }

        Aligned_Vec4f(const Vec3f& v3,value_type w)
        {
#ifdef __UseSSE__
			vec= _mm_set_ps(v3[0],v3[1],v3[2],w);
#else
            _v[0]=v3[0];
            _v[1]=v3[1];
            _v[2]=v3[2];
            _v[3]=w;
#endif
        }
            
        inline bool operator == (const Aligned_Vec4f& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1] && _v[2]==v._v[2] && _v[3]==v._v[3]; }

        inline bool operator != (const Aligned_Vec4f& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1] || _v[2]!=v._v[2] || _v[3]!=v._v[3]; }

        inline bool operator <  (const Aligned_Vec4f& v) const
        {
            if (_v[0]<v._v[0]) return true;
            else if (_v[0]>v._v[0]) return false;
            else if (_v[1]<v._v[1]) return true;
            else if (_v[1]>v._v[1]) return false;
            else if (_v[2]<v._v[2]) return true;
            else if (_v[2]>v._v[2]) return false;
            else return (_v[3]<v._v[3]);
        }

        inline value_type* ptr() { return _v; }
        inline const value_type* ptr() const { return _v; }

        inline void set( value_type x, value_type y, value_type z, value_type w)
        {
            _v[0]=x; _v[1]=y; _v[2]=z; _v[3]=w;
        }

        inline value_type& operator [] (unsigned int i) { return _v[i]; }
        inline value_type  operator [] (unsigned int i) const { return _v[i]; }

        inline value_type& x() { return _v[0]; }
        inline value_type& y() { return _v[1]; }
        inline value_type& z() { return _v[2]; }
        inline value_type& w() { return _v[3]; }

        inline value_type x() const { return _v[0]; }
        inline value_type y() const { return _v[1]; }
        inline value_type z() const { return _v[2]; }
        inline value_type w() const { return _v[3]; }

        inline value_type& r() { return _v[0]; }
        inline value_type& g() { return _v[1]; }
        inline value_type& b() { return _v[2]; }
        inline value_type& a() { return _v[3]; }

        inline value_type r() const { return _v[0]; }
        inline value_type g() const { return _v[1]; }
        inline value_type b() const { return _v[2]; }
        inline value_type a() const { return _v[3]; }

        inline unsigned int asABGR() const
        {
            return (unsigned int)clampTo((_v[0]*255.0f),0.0f,255.0f)<<24 |
                   (unsigned int)clampTo((_v[1]*255.0f),0.0f,255.0f)<<16 |
                   (unsigned int)clampTo((_v[2]*255.0f),0.0f,255.0f)<<8  |
                   (unsigned int)clampTo((_v[3]*255.0f),0.0f,255.0f);
        }

        inline unsigned int asRGBA() const
        {
            return (unsigned int)clampTo((_v[3]*255.0f),0.0f,255.0f)<<24 |
                   (unsigned int)clampTo((_v[2]*255.0f),0.0f,255.0f)<<16 |
                   (unsigned int)clampTo((_v[1]*255.0f),0.0f,255.0f)<<8  |
                   (unsigned int)clampTo((_v[0]*255.0f),0.0f,255.0f);
        }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]) || osg::isNaN(_v[2]) || osg::isNaN(_v[3]); }

#ifdef __UseSSE__

		/* Logical Operators */
		friend Aligned_Vec4f operator &(const Aligned_Vec4f &a, const Aligned_Vec4f &b) { return _mm_and_ps(a.vec,b.vec); }
		friend Aligned_Vec4f operator |(const Aligned_Vec4f &a, const Aligned_Vec4f &b) { return _mm_or_ps(a.vec,b.vec); }
		friend Aligned_Vec4f operator ^(const Aligned_Vec4f &a, const Aligned_Vec4f &b) { return _mm_xor_ps(a.vec,b.vec); }

		/* Arithmetic Operators */
		friend Aligned_Vec4f operator *(const Aligned_Vec4f &a, const Aligned_Vec4f &b) { return _mm_mul_ps(a.vec,b.vec); }
		Aligned_Vec4f& operator *=(Aligned_Vec4f &a) { return *this = _mm_mul_ps(vec,a); }

#endif

		//TODO we'll need a different way to get dot product to avoid ambiguity
#if  0
        /** Dot product. */
        inline value_type operator * (const Aligned_Vec4f& rhs) const
        {
            return _v[0]*rhs._v[0]+
                   _v[1]*rhs._v[1]+
                   _v[2]*rhs._v[2]+
                   _v[3]*rhs._v[3] ;
        }
#endif
        /** Multiply by scalar. */
        inline Aligned_Vec4f operator * (value_type rhs) const
        {
#ifdef __UseSSE__
			return _mm_mul_ps(vec,_mm_set1_ps(rhs));
#else
            return Aligned_Vec4f(_v[0]*rhs, _v[1]*rhs, _v[2]*rhs, _v[3]*rhs);
#endif
        }

        /** Unary multiply by scalar. */
        inline Aligned_Vec4f& operator *= (value_type rhs)
        {
#ifdef __UseSSE__
			return *this = _mm_mul_ps(vec,_mm_set1_ps(rhs));
#else
            _v[0]*=rhs;
            _v[1]*=rhs;
            _v[2]*=rhs;
            _v[3]*=rhs;
            return *this;
#endif
        }

        /** Divide by scalar. */
        inline Aligned_Vec4f operator / (value_type rhs) const
        {
#ifdef __UseSSE__
			return _mm_div_ps(vec,_mm_set1_ps(rhs));
#else
            return Aligned_Vec4f(_v[0]/rhs, _v[1]/rhs, _v[2]/rhs, _v[3]/rhs);
#endif
        }

        /** Unary divide by scalar. */
        inline Aligned_Vec4f& operator /= (value_type rhs)
        {
#ifdef __UseSSE__
			return *this = _mm_div_ps(vec,_mm_set1_ps(rhs));
#else
            _v[0]/=rhs;
            _v[1]/=rhs;
            _v[2]/=rhs;
            _v[3]/=rhs;
            return *this;
#endif
        }

        /** Binary vector add. */
        inline Aligned_Vec4f operator + (const Aligned_Vec4f& rhs) const
        {
#ifdef __UseSSE__
			return _mm_add_ps(vec,rhs.vec);
#else
            return Aligned_Vec4f(_v[0]+rhs._v[0], _v[1]+rhs._v[1],
                         _v[2]+rhs._v[2], _v[3]+rhs._v[3]);
#endif
        }

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        inline Aligned_Vec4f& operator += (const Aligned_Vec4f& rhs)
        {
#ifdef __UseSSE__
			return *this =_mm_add_ps(vec,rhs.vec);
#else
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            _v[2] += rhs._v[2];
            _v[3] += rhs._v[3];
            return *this;
#endif
        }

        /** Binary vector subtract. */
        inline Aligned_Vec4f operator - (const Aligned_Vec4f& rhs) const
        {
#ifdef __UseSSE__
			return _mm_sub_ps(vec,rhs.vec);
#else
            return Aligned_Vec4f(_v[0]-rhs._v[0], _v[1]-rhs._v[1],
                         _v[2]-rhs._v[2], _v[3]-rhs._v[3] );
#endif
        }

        /** Unary vector subtract. */
        inline Aligned_Vec4f& operator -= (const Aligned_Vec4f& rhs)
        {
#ifdef __UseSSE__
			return *this = _mm_sub_ps(vec,rhs.vec);
#else
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            _v[2]-=rhs._v[2];
            _v[3]-=rhs._v[3];
            return *this;
#endif
        }

#ifdef __UseSSE__
		Aligned_Vec4f& operator &=(Aligned_Vec4f &a) { return *this = _mm_and_ps(vec,a.vec); }
		Aligned_Vec4f& operator |=(Aligned_Vec4f &a) { return *this = _mm_or_ps(vec,a.vec); }
		Aligned_Vec4f& operator ^=(Aligned_Vec4f &a) { return *this = _mm_xor_ps(vec,a.vec); }
#endif
        /** Negation operator. Returns the negative of the Aligned_Vec4f. */
        inline const Aligned_Vec4f operator - () const
        {
            return Aligned_Vec4f (-_v[0], -_v[1], -_v[2], -_v[3]);
        }

        /** Length of the vector = sqrt( vec . vec ) */
        inline value_type length() const
        {
            return sqrtf( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] + _v[3]*_v[3]);
        }

        /** Length squared of the vector = vec . vec */
        inline value_type length2() const
        {
            return _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] + _v[3]*_v[3];
        }

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        inline value_type normalize()
        {
            value_type norm = Aligned_Vec4f::length();
            if (norm>0.0f)
            {
                value_type inv = 1.0f/norm;
                _v[0] *= inv;
                _v[1] *= inv;
                _v[2] *= inv;
                _v[3] *= inv;
            }
            return( norm );
        }
#ifdef __UseSSE__
		/* Horizontal Add */
		friend float add_horizontal(Aligned_Vec4f &a)
		{
			Aligned_Vec4f ftemp = _mm_add_ss(a.vec,_mm_add_ss(_mm_shuffle_ps(a.vec, a.vec, 1),_mm_add_ss(_mm_shuffle_ps(a.vec, a.vec, 2),_mm_shuffle_ps(a.vec, a.vec, 3))));
			return ftemp[0];
		}

		/* Square Root */
		friend Aligned_Vec4f sqrt(const Aligned_Vec4f &a)		{ return _mm_sqrt_ps(a.vec); }
		/* Reciprocal */
		friend Aligned_Vec4f rcp(const Aligned_Vec4f &a)		{ return _mm_rcp_ps(a.vec); }
		/* Reciprocal Square Root */
		friend Aligned_Vec4f rsqrt(const Aligned_Vec4f &a)		{ return _mm_rsqrt_ps(a.vec); }

#endif
};    // end of class Aligned_Vec4f



/** Compute the dot product of a (Vec3,1.0) and a Vec4f. */
inline Aligned_Vec4f::value_type operator * (const Vec3f& lhs,const Aligned_Vec4f& rhs)
{
    return lhs[0]*rhs[0]+lhs[1]*rhs[1]+lhs[2]*rhs[2]+rhs[3];
}

/** Compute the dot product of a Vec4f and a (Vec3,1.0). */
inline Aligned_Vec4f::value_type operator * (const Aligned_Vec4f& lhs,const Vec3f& rhs)
{
    return lhs[0]*rhs[0]+lhs[1]*rhs[1]+lhs[2]*rhs[2]+lhs[3];
}

}    // end of namespace osg




//TODO:  From this point down this is just a copy from the fvec from VS 2009... please do not merge this into the OSG build.  Somebody who uses another platform should try to create
//a generic form of this to be used, so that the code can remain readable.



#ifdef  _MSC_VER
#pragma pack(push,_CRT_PACKING)
#endif  /* _MSC_VER */

#pragma pack(push,16) /* Must ensure class & union 16-B aligned */

#define EXPLICIT explicit


class F32vec1
{
protected:
	__m128 vec;
public:

	/* Constructors: 1 float */
	F32vec1() {}

	F32vec1(int i)		{ vec = _mm_cvt_si2ss(vec,i);};

	/* Initialize each of 4 SP FPs with same float */
	EXPLICIT F32vec1(float f)	{ vec = _mm_set_ss(f); }

	/* Initialize each of 4 SP FPs with same float */
	EXPLICIT F32vec1(double d)	{ vec = _mm_set_ss((float) d); }

	/* initialize with __m128 data type */
	F32vec1(__m128 m)	{ vec = m; }

	/* Conversion functions */
	operator  __m128() const	{ return vec; }		/* Convert to float */

	/* Logical Operators */
	friend F32vec1 operator &(const F32vec1 &a, const F32vec1 &b) { return _mm_and_ps(a,b); }
	friend F32vec1 operator |(const F32vec1 &a, const F32vec1 &b) { return _mm_or_ps(a,b); }
	friend F32vec1 operator ^(const F32vec1 &a, const F32vec1 &b) { return _mm_xor_ps(a,b); }

	/* Arithmetic Operators */
	friend F32vec1 operator +(const F32vec1 &a, const F32vec1 &b) { return _mm_add_ss(a,b); }
	friend F32vec1 operator -(const F32vec1 &a, const F32vec1 &b) { return _mm_sub_ss(a,b); }
	friend F32vec1 operator *(const F32vec1 &a, const F32vec1 &b) { return _mm_mul_ss(a,b); }
	friend F32vec1 operator /(const F32vec1 &a, const F32vec1 &b) { return _mm_div_ss(a,b); }

	F32vec1& operator +=(F32vec1 &a) { return *this = _mm_add_ss(vec,a); }
	F32vec1& operator -=(F32vec1 &a) { return *this = _mm_sub_ss(vec,a); }
	F32vec1& operator *=(F32vec1 &a) { return *this = _mm_mul_ss(vec,a); }
	F32vec1& operator /=(F32vec1 &a) { return *this = _mm_div_ss(vec,a); }
	F32vec1& operator &=(F32vec1 &a) { return *this = _mm_and_ps(vec,a); }
	F32vec1& operator |=(F32vec1 &a) { return *this = _mm_or_ps(vec,a); }
	F32vec1& operator ^=(F32vec1 &a) { return *this = _mm_xor_ps(vec,a); }


	/* Square Root */
	friend F32vec1 sqrt(const F32vec1 &a)		{ return _mm_sqrt_ss(a); }
	/* Reciprocal */
	friend F32vec1 rcp(const F32vec1 &a)		{ return _mm_rcp_ss(a); }
	/* Reciprocal Square Root */
	friend F32vec1 rsqrt(const F32vec1 &a)		{ return _mm_rsqrt_ss(a); }

	/* NewtonRaphson Reciprocal
	[2 * rcpss(x) - (x * rcpss(x) * rcpss(x))] */
	friend F32vec1 rcp_nr(const F32vec1 &a)
	{
		F32vec1 Ra0 = _mm_rcp_ss(a);
		return _mm_sub_ss(_mm_add_ss(Ra0, Ra0), _mm_mul_ss(_mm_mul_ss(Ra0, a), Ra0));
	}

	/*	NewtonRaphson Reciprocal Square Root
	0.5 * rsqrtss * (3 - x * rsqrtss(x) * rsqrtss(x)) */
#pragma warning(push)
#pragma warning(disable : 4640)
	friend F32vec1 rsqrt_nr(const F32vec1 &a)
	{
		static const F32vec1 fvecf0pt5(0.5f);
		static const F32vec1 fvecf3pt0(3.0f);
		F32vec1 Ra0 = _mm_rsqrt_ss(a);
		return (fvecf0pt5 * Ra0) * (fvecf3pt0 - (a * Ra0) * Ra0);
	}
#pragma warning(pop)

	/* Compares: Mask is returned  */
	/* Macros expand to all compare intrinsics.  Example:
	friend F32vec1 cmpeq(const F32vec1 &a, const F32vec1 &b)
	{ return _mm_cmpeq_ss(a,b);} */
#define Fvec32s1_COMP(op) \
	friend F32vec1 cmp##op (const F32vec1 &a, const F32vec1 &b) { return _mm_cmp##op##_ss(a,b); }
	Fvec32s1_COMP(eq)					/* expanded to cmpeq(a,b) */
		Fvec32s1_COMP(lt)					/* expanded to cmplt(a,b) */
		Fvec32s1_COMP(le)					/* expanded to cmple(a,b) */
		Fvec32s1_COMP(gt)					/* expanded to cmpgt(a,b) */
		Fvec32s1_COMP(ge)					/* expanded to cmpge(a,b) */
		Fvec32s1_COMP(neq)					/* expanded to cmpneq(a,b) */
		Fvec32s1_COMP(nlt)					/* expanded to cmpnlt(a,b) */
		Fvec32s1_COMP(nle)					/* expanded to cmpnle(a,b) */
		Fvec32s1_COMP(ngt)					/* expanded to cmpngt(a,b) */
		Fvec32s1_COMP(nge)					/* expanded to cmpnge(a,b) */
#undef Fvec32s1_COMP

		/* Min and Max */
		friend F32vec1 simd_min(const F32vec1 &a, const F32vec1 &b) { return _mm_min_ss(a,b); }
	friend F32vec1 simd_max(const F32vec1 &a, const F32vec1 &b) { return _mm_max_ss(a,b); }

	/* Debug Features */
#if defined(_ENABLE_VEC_DEBUG)
	/* Output */
	friend std::ostream & operator<<(std::ostream & os, const F32vec1 &a)
	{
		/* To use: cout << "Elements of F32vec1 fvec are: " << fvec; */
		float *fp = (float*)&a;
		os << "float:" << *fp;
		return os;
	}
#endif

};

#pragma pack(pop,16) /* Must ensure class & union 16-B aligned */

#endif /* _FVEC_H_INCLUDED */
#ifdef  _MSC_VER
#pragma pack(pop,_CRT_PACKING)
#endif  /* _MSC_VER */

