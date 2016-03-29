#pragma once


#include <math.h>
#include <float.h>

namespace osg {

// define the standard trig values
#ifdef PI
#undef PI
#undef PI_2
#undef PI_4
#endif
const double PI   = 3.14159265358979323846;
const double PI_2 = 1.57079632679489661923;
const double PI_4 = 0.78539816339744830962;
const double LN_2 = 0.69314718055994530942;
const double INVLN_2 = 1.0 / LN_2;


/** return the minimum of two values, equivalent to std::min.
  * std::min not used because of STL implementation under IRIX not
  * containing std::min.
*/
template<typename T>
inline T absolute(T v) { return v<(T)0?-v:v; }

/** return true if float lhs and rhs are equivalent,
  * meaning that the difference between them is less than an epsilon value
  * which defaults to 1e-6.
*/
inline bool equivalent(float lhs,float rhs,float epsilon=1e-6)
  { float delta = rhs-lhs; return delta<0.0f?delta>=-epsilon:delta<=epsilon; }

/** return true if double lhs and rhs are equivalent,
  * meaning that the difference between them is less than an epsilon value
  * which defaults to 1e-6.
*/
inline bool equivalent(double lhs,double rhs,double epsilon=1e-6)
  { double delta = rhs-lhs; return delta<0.0?delta>=-epsilon:delta<=epsilon; }

/** return the minimum of two values, equivalent to std::min.
  * std::min not used because of STL implementation under IRIX not containing
  * std::min.
*/
template<typename T>
inline T minimum(T lhs,T rhs) { return lhs<rhs?lhs:rhs; }

/** return the maximum of two values, equivalent to std::max.
  * std::max not used because of STL implementation under IRIX not containing
  * std::max.
*/
template<typename T>
inline T maximum(T lhs,T rhs) { return lhs>rhs?lhs:rhs; }

template<typename T>
inline T clampTo(T v,T minimum,T maximum)
  { return v<minimum?minimum:v>maximum?maximum:v; }

template<typename T>
inline T clampAbove(T v,T minimum) { return v<minimum?minimum:v; }

template<typename T>
inline T clampBelow(T v,T maximum) { return v>maximum?maximum:v; }

template<typename T>
inline T clampBetween(T v,T minimum, T maximum)
  { return clampBelow(clampAbove(v,minimum),maximum); }

template<typename T>
inline T sign(T v) { return v<(T)0?(T)-1:(T)1; }

template<typename T>
inline T signOrZero(T v) { return v<(T)0 ? (T)-1 : ( v>(T)0 ? (T)1 : 0 ); }

template<typename T>
inline T square(T v) { return v*v; }

template<typename T>
inline T signedSquare(T v) { return v<(T)0?-v*v:v*v;; }

inline float inDegrees(float angle) { return angle*(float)PI/180.0f; }
inline double inDegrees(double angle) { return angle*PI/180.0; }

template<typename T>
inline T inRadians(T angle) { return angle; }

inline float DegreesToRadians(float angle) { return angle*(float)PI/180.0f; }
inline double DegreesToRadians(double angle) { return angle*PI/180.0; }

inline float RadiansToDegrees(float angle) { return angle*180.0f/(float)PI; }
inline double RadiansToDegrees(double angle) { return angle*180.0/PI; }

inline float round(float v) { return v>=0.0f?floorf(v+0.5f):ceilf(v-0.5f); }
inline double round(double v) { return v>=0.0?floor(v+0.5):ceil(v-0.5); }

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MWERKS__)
    inline bool isNaN(float v) { return _isnan(v)!=0; }
    inline bool isNaN(double v) { return _isnan(v)!=0; }
#else
    #if defined(__APPLE__)
        inline bool isNaN(float v) { return std::isnan(v); }
        inline bool isNaN(double v) { return std::isnan(v); }
    #else
        // Need to use to std::isnan to avoid undef problem from <cmath>
        inline bool isNaN(float v) { return isnan(v); }
        inline bool isNaN(double v) { return isnan(v); }
    #endif
#endif


/** compute the volume of a tetrahedron. */
template<typename T>
inline float computeVolume(const T& a,const T& b,const T& c,const T& d)
{
    return fabsf(((b-c)^(a-b))*(d-b));
}

/** compute the volume of a prism. */
template<typename T>
inline float computeVolume(const T& f1,const T& f2,const T& f3,
                           const T& b1,const T& b2,const T& b3)
{
    return computeVolume(f1,f2,f3,b1)+
           computeVolume(b1,b2,b3,f2)+
           computeVolume(b1,b3,f2,f3);
}



class Vec2f
{
    public:

        /** Type of Vec class.*/
        typedef float value_type;

        /** Number of vector components. */
        enum { num_components = 2 };
        
        /** Vec member varaible. */
        value_type _v[2];
        

        Vec2f() {_v[0]=0.0; _v[1]=0.0;}
        Vec2f(value_type x,value_type y) { _v[0]=x; _v[1]=y; }


        inline bool operator == (const Vec2f& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1]; }

        inline bool operator != (const Vec2f& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1]; }

        inline bool operator <  (const Vec2f& v) const
        {
            if (_v[0]<v._v[0]) return true;
            else if (_v[0]>v._v[0]) return false;
            else return (_v[1]<v._v[1]);
        }

        inline value_type * ptr() { return _v; }
        inline const value_type * ptr() const { return _v; }

        inline void set( value_type x, value_type y ) { _v[0]=x; _v[1]=y; }

        inline value_type & operator [] (int i) { return _v[i]; }
        inline value_type operator [] (int i) const { return _v[i]; }

        inline value_type & x() { return _v[0]; }
        inline value_type & y() { return _v[1]; }

        inline value_type x() const { return _v[0]; }
        inline value_type y() const { return _v[1]; }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]); }

        /** Dot product. */
        inline value_type operator * (const Vec2f& rhs) const
        {
            return _v[0]*rhs._v[0]+_v[1]*rhs._v[1];
        }

        /** Multiply by scalar. */
        inline const Vec2f operator * (value_type rhs) const
        {
            return Vec2f(_v[0]*rhs, _v[1]*rhs);
        }

        /** Unary multiply by scalar. */
        inline Vec2f& operator *= (value_type rhs)
        {
            _v[0]*=rhs;
            _v[1]*=rhs;
            return *this;
        }

        /** Divide by scalar. */
        inline const Vec2f operator / (value_type rhs) const
        {
            return Vec2f(_v[0]/rhs, _v[1]/rhs);
        }

        /** Unary divide by scalar. */
        inline Vec2f& operator /= (value_type rhs)
        {
            _v[0]/=rhs;
            _v[1]/=rhs;
            return *this;
        }

        /** Binary vector add. */
        inline const Vec2f operator + (const Vec2f& rhs) const
        {
            return Vec2f(_v[0]+rhs._v[0], _v[1]+rhs._v[1]);
        }

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        inline Vec2f& operator += (const Vec2f& rhs)
        {
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            return *this;
        }

        /** Binary vector subtract. */
        inline const Vec2f operator - (const Vec2f& rhs) const
        {
            return Vec2f(_v[0]-rhs._v[0], _v[1]-rhs._v[1]);
        }

        /** Unary vector subtract. */
        inline Vec2f& operator -= (const Vec2f& rhs)
        {
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            return *this;
        }

        /** Negation operator. Returns the negative of the Vec2f. */
        inline const Vec2f operator - () const
        {
            return Vec2f (-_v[0], -_v[1]);
        }

        /** Length of the vector = sqrt( vec . vec ) */
        inline value_type length() const
        {
            return sqrtf( _v[0]*_v[0] + _v[1]*_v[1] );
        }

        /** Length squared of the vector = vec . vec */
        inline value_type length2( void ) const
        {
            return _v[0]*_v[0] + _v[1]*_v[1];
        }

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        inline value_type normalize()
        {
            value_type norm = Vec2f::length();
            if (norm>0.0)
            {
                value_type inv = 1.0f/norm;
                _v[0] *= inv;
                _v[1] *= inv;
            }
            return( norm );
        }
        
};    // end of class Vec2f

class Vec3f
{
    public:

        /** Type of Vec class.*/
        typedef float value_type;

        /** Number of vector components. */
        enum { num_components = 3 };
        
        value_type _v[3];

        Vec3f() { _v[0]=0.0f; _v[1]=0.0f; _v[2]=0.0f;}
        Vec3f(value_type x,value_type y,value_type z) { _v[0]=x; _v[1]=y; _v[2]=z; }
        Vec3f(const Vec2f& v2,value_type zz)
        {
            _v[0] = v2[0];
            _v[1] = v2[1];
            _v[2] = zz;
        }


        inline bool operator == (const Vec3f& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1] && _v[2]==v._v[2]; }
        
        inline bool operator != (const Vec3f& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1] || _v[2]!=v._v[2]; }

        inline bool operator <  (const Vec3f& v) const
        {
            if (_v[0]<v._v[0]) return true;
            else if (_v[0]>v._v[0]) return false;
            else if (_v[1]<v._v[1]) return true;
            else if (_v[1]>v._v[1]) return false;
            else return (_v[2]<v._v[2]);
        }

        inline value_type* ptr() { return _v; }
        inline const value_type* ptr() const { return _v; }

        inline void set( value_type x, value_type y, value_type z)
        {
            _v[0]=x; _v[1]=y; _v[2]=z;
        }

        inline void set( const Vec3f& rhs)
        {
            _v[0]=rhs._v[0]; _v[1]=rhs._v[1]; _v[2]=rhs._v[2];
        }

        inline value_type& operator [] (int i) { return _v[i]; }
        inline value_type operator [] (int i) const { return _v[i]; }

        inline value_type& x() { return _v[0]; }
        inline value_type& y() { return _v[1]; }
        inline value_type& z() { return _v[2]; }

        inline value_type x() const { return _v[0]; }
        inline value_type y() const { return _v[1]; }
        inline value_type z() const { return _v[2]; }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]) || osg::isNaN(_v[2]); }

        /** Dot product. */
        inline value_type operator * (const Vec3f& rhs) const
        {
            return _v[0]*rhs._v[0]+_v[1]*rhs._v[1]+_v[2]*rhs._v[2];
        }

        /** Cross product. */
        inline const Vec3f operator ^ (const Vec3f& rhs) const
        {
            return Vec3f(_v[1]*rhs._v[2]-_v[2]*rhs._v[1],
                         _v[2]*rhs._v[0]-_v[0]*rhs._v[2] ,
                         _v[0]*rhs._v[1]-_v[1]*rhs._v[0]);
        }

        /** Multiply by scalar. */
        inline const Vec3f operator * (value_type rhs) const
        {
            return Vec3f(_v[0]*rhs, _v[1]*rhs, _v[2]*rhs);
        }

        /** Unary multiply by scalar. */
        inline Vec3f& operator *= (value_type rhs)
        {
            _v[0]*=rhs;
            _v[1]*=rhs;
            _v[2]*=rhs;
            return *this;
        }

        /** Divide by scalar. */
        inline const Vec3f operator / (value_type rhs) const
        {
            return Vec3f(_v[0]/rhs, _v[1]/rhs, _v[2]/rhs);
        }

        /** Unary divide by scalar. */
        inline Vec3f& operator /= (value_type rhs)
        {
            _v[0]/=rhs;
            _v[1]/=rhs;
            _v[2]/=rhs;
            return *this;
        }

        /** Binary vector add. */
        inline const Vec3f operator + (const Vec3f& rhs) const
        {
            return Vec3f(_v[0]+rhs._v[0], _v[1]+rhs._v[1], _v[2]+rhs._v[2]);
        }

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        inline Vec3f& operator += (const Vec3f& rhs)
        {
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            _v[2] += rhs._v[2];
            return *this;
        }

        /** Binary vector subtract. */
        inline const Vec3f operator - (const Vec3f& rhs) const
        {
            return Vec3f(_v[0]-rhs._v[0], _v[1]-rhs._v[1], _v[2]-rhs._v[2]);
        }

        /** Unary vector subtract. */
        inline Vec3f& operator -= (const Vec3f& rhs)
        {
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            _v[2]-=rhs._v[2];
            return *this;
        }

        /** Negation operator. Returns the negative of the Vec3f. */
        inline const Vec3f operator - () const
        {
            return Vec3f (-_v[0], -_v[1], -_v[2]);
        }

        /** Length of the vector = sqrt( vec . vec ) */
        inline value_type length() const
        {
            return sqrtf( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] );
        }

        /** Length squared of the vector = vec . vec */
        inline value_type length2() const
        {
            return _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2];
        }

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        inline value_type normalize()
        {
            value_type norm = Vec3f::length();
            if (norm>0.0)
            {
                value_type inv = 1.0f/norm;
                _v[0] *= inv;
                _v[1] *= inv;
                _v[2] *= inv;
            }                
            return( norm );
        }

};    // end of class Vec3f

class Vec2d
{
    public:

        /** Type of Vec class.*/
        typedef double value_type;

        /** Number of vector components. */
        enum { num_components = 2 };
        
        value_type _v[2];

        Vec2d() {_v[0]=0.0; _v[1]=0.0;}

        Vec2d(value_type x,value_type y) { _v[0]=x; _v[1]=y; }

        inline Vec2d(const Vec2f& vec) { _v[0]=vec._v[0]; _v[1]=vec._v[1]; }
        
        inline operator Vec2f() const { return Vec2f(static_cast<float>(_v[0]),static_cast<float>(_v[1]));}


        inline bool operator == (const Vec2d& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1]; }

        inline bool operator != (const Vec2d& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1]; }

        inline bool operator <  (const Vec2d& v) const
        {
            if (_v[0]<v._v[0]) return true;
            else if (_v[0]>v._v[0]) return false;
            else return (_v[1]<v._v[1]);
        }

        inline value_type* ptr() { return _v; }
        inline const value_type* ptr() const { return _v; }

        inline void set( value_type x, value_type y ) { _v[0]=x; _v[1]=y; }

        inline value_type& operator [] (int i) { return _v[i]; }
        inline value_type operator [] (int i) const { return _v[i]; }

        inline value_type& x() { return _v[0]; }
        inline value_type& y() { return _v[1]; }

        inline value_type x() const { return _v[0]; }
        inline value_type y() const { return _v[1]; }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]); }

        /** Dot product. */
        inline value_type operator * (const Vec2d& rhs) const
        {
            return _v[0]*rhs._v[0]+_v[1]*rhs._v[1];
        }

        /** Multiply by scalar. */
        inline const Vec2d operator * (value_type rhs) const
        {
            return Vec2d(_v[0]*rhs, _v[1]*rhs);
        }

        /** Unary multiply by scalar. */
        inline Vec2d& operator *= (value_type rhs)
        {
            _v[0]*=rhs;
            _v[1]*=rhs;
            return *this;
        }

        /** Divide by scalar. */
        inline const Vec2d operator / (value_type rhs) const
        {
            return Vec2d(_v[0]/rhs, _v[1]/rhs);
        }

        /** Unary divide by scalar. */
        inline Vec2d& operator /= (value_type rhs)
        {
            _v[0]/=rhs;
            _v[1]/=rhs;
            return *this;
        }

        /** Binary vector add. */
        inline const Vec2d operator + (const Vec2d& rhs) const
        {
            return Vec2d(_v[0]+rhs._v[0], _v[1]+rhs._v[1]);
        }

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        inline Vec2d& operator += (const Vec2d& rhs)
        {
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            return *this;
        }

        /** Binary vector subtract. */
        inline const Vec2d operator - (const Vec2d& rhs) const
        {
            return Vec2d(_v[0]-rhs._v[0], _v[1]-rhs._v[1]);
        }

        /** Unary vector subtract. */
        inline Vec2d& operator -= (const Vec2d& rhs)
        {
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            return *this;
        }

        /** Negation operator. Returns the negative of the Vec2d. */
        inline const Vec2d operator - () const
        {
            return Vec2d (-_v[0], -_v[1]);
        }

        /** Length of the vector = sqrt( vec . vec ) */
        inline value_type length() const
        {
            return sqrt( _v[0]*_v[0] + _v[1]*_v[1] );
        }

        /** Length squared of the vector = vec . vec */
        inline value_type length2( void ) const
        {
            return _v[0]*_v[0] + _v[1]*_v[1];
        }

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        inline value_type normalize()
        {
            value_type norm = Vec2d::length();
            if (norm>0.0)
            {
                value_type inv = 1.0/norm;
                _v[0] *= inv;
                _v[1] *= inv;
            }
            return( norm );
        }

};    // end of class Vec2d


class Vec3d
{
    public:

        /** Type of Vec class.*/
        typedef double value_type;

        /** Number of vector components. */
        enum { num_components = 3 };
        
        value_type _v[3];

        Vec3d() { _v[0]=0.0; _v[1]=0.0; _v[2]=0.0;}

        inline Vec3d(const Vec3f& vec) { _v[0]=vec._v[0]; _v[1]=vec._v[1]; _v[2]=vec._v[2];}
        
        inline operator Vec3f() const { return Vec3f(static_cast<float>(_v[0]),static_cast<float>(_v[1]),static_cast<float>(_v[2]));}

        Vec3d(value_type x,value_type y,value_type z) { _v[0]=x; _v[1]=y; _v[2]=z; }
        Vec3d(const Vec2d& v2,value_type zz)
        {
            _v[0] = v2[0];
            _v[1] = v2[1];
            _v[2] = zz;
        }

        inline bool operator == (const Vec3d& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1] && _v[2]==v._v[2]; }
        
        inline bool operator != (const Vec3d& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1] || _v[2]!=v._v[2]; }

        inline bool operator <  (const Vec3d& v) const
        {
            if (_v[0]<v._v[0]) return true;
            else if (_v[0]>v._v[0]) return false;
            else if (_v[1]<v._v[1]) return true;
            else if (_v[1]>v._v[1]) return false;
            else return (_v[2]<v._v[2]);
        }

        inline value_type* ptr() { return _v; }
        inline const value_type* ptr() const { return _v; }

        inline void set( value_type x, value_type y, value_type z)
        {
            _v[0]=x; _v[1]=y; _v[2]=z;
        }

        inline void set( const Vec3d& rhs)
        {
            _v[0]=rhs._v[0]; _v[1]=rhs._v[1]; _v[2]=rhs._v[2];
        }

        inline value_type& operator [] (int i) { return _v[i]; }
        inline value_type operator [] (int i) const { return _v[i]; }

        inline value_type& x() { return _v[0]; }
        inline value_type& y() { return _v[1]; }
        inline value_type& z() { return _v[2]; }

        inline value_type x() const { return _v[0]; }
        inline value_type y() const { return _v[1]; }
        inline value_type z() const { return _v[2]; }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]) || osg::isNaN(_v[2]); }

        /** Dot product. */
        inline value_type operator * (const Vec3d& rhs) const
        {
            return _v[0]*rhs._v[0]+_v[1]*rhs._v[1]+_v[2]*rhs._v[2];
        }

        /** Cross product. */
        inline const Vec3d operator ^ (const Vec3d& rhs) const
        {
            return Vec3d(_v[1]*rhs._v[2]-_v[2]*rhs._v[1],
                         _v[2]*rhs._v[0]-_v[0]*rhs._v[2] ,
                         _v[0]*rhs._v[1]-_v[1]*rhs._v[0]);
        }

        /** Multiply by scalar. */
        inline const Vec3d operator * (value_type rhs) const
        {
            return Vec3d(_v[0]*rhs, _v[1]*rhs, _v[2]*rhs);
        }

        /** Unary multiply by scalar. */
        inline Vec3d& operator *= (value_type rhs)
        {
            _v[0]*=rhs;
            _v[1]*=rhs;
            _v[2]*=rhs;
            return *this;
        }

        /** Divide by scalar. */
        inline const Vec3d operator / (value_type rhs) const
        {
            return Vec3d(_v[0]/rhs, _v[1]/rhs, _v[2]/rhs);
        }

        /** Unary divide by scalar. */
        inline Vec3d& operator /= (value_type rhs)
        {
            _v[0]/=rhs;
            _v[1]/=rhs;
            _v[2]/=rhs;
            return *this;
        }

        /** Binary vector add. */
        inline const Vec3d operator + (const Vec3d& rhs) const
        {
            return Vec3d(_v[0]+rhs._v[0], _v[1]+rhs._v[1], _v[2]+rhs._v[2]);
        }

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        inline Vec3d& operator += (const Vec3d& rhs)
        {
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            _v[2] += rhs._v[2];
            return *this;
        }

        /** Binary vector subtract. */
        inline const Vec3d operator - (const Vec3d& rhs) const
        {
            return Vec3d(_v[0]-rhs._v[0], _v[1]-rhs._v[1], _v[2]-rhs._v[2]);
        }

        /** Unary vector subtract. */
        inline Vec3d& operator -= (const Vec3d& rhs)
        {
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            _v[2]-=rhs._v[2];
            return *this;
        }

        /** Negation operator. Returns the negative of the Vec3d. */
        inline const Vec3d operator - () const
        {
            return Vec3d (-_v[0], -_v[1], -_v[2]);
        }

        /** Length of the vector = sqrt( vec . vec ) */
        inline value_type length() const
        {
            return sqrt( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] );
        }

        /** Length squared of the vector = vec . vec */
        inline value_type length2() const
        {
            return _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2];
        }

        /** Normalize the vector so that it has length unity.
          * Returns the previous length of the vector.
        */
        inline value_type normalize()
        {
            value_type norm = Vec3d::length();
            if (norm>0.0)
            {
                value_type inv = 1.0/norm;
                _v[0] *= inv;
                _v[1] *= inv;
                _v[2] *= inv;
            }                
            return( norm );
        }

};    // end of class Vec3d


class Vec4f
{
public:

	/** Type of Vec class.*/
	typedef float value_type;

	/** Number of vector components. */
	enum { num_components = 4 };

	/** Vec member variable. */
	value_type _v[4];

	// Methods are defined here so that they are implicitly inlined

	Vec4f() { _v[0]=0.0f; _v[1]=0.0f; _v[2]=0.0f; _v[3]=0.0f;}

	Vec4f(value_type x, value_type y, value_type z, value_type w)
	{
		_v[0]=x;
		_v[1]=y;
		_v[2]=z;
		_v[3]=w;
	}

	Vec4f(const Vec3f& v3,value_type w)
	{
		_v[0]=v3[0];
		_v[1]=v3[1];
		_v[2]=v3[2];
		_v[3]=w;
	}

	inline bool operator == (const Vec4f& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1] && _v[2]==v._v[2] && _v[3]==v._v[3]; }

	inline bool operator != (const Vec4f& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1] || _v[2]!=v._v[2] || _v[3]!=v._v[3]; }

	inline bool operator <  (const Vec4f& v) const
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

	/** Dot product. */
	inline value_type operator * (const Vec4f& rhs) const
	{
		return _v[0]*rhs._v[0]+
			_v[1]*rhs._v[1]+
			_v[2]*rhs._v[2]+
			_v[3]*rhs._v[3] ;
	}

	/** Multiply by scalar. */
	inline Vec4f operator * (value_type rhs) const
	{
		return Vec4f(_v[0]*rhs, _v[1]*rhs, _v[2]*rhs, _v[3]*rhs);
	}

	/** Unary multiply by scalar. */
	inline Vec4f& operator *= (value_type rhs)
	{
		_v[0]*=rhs;
		_v[1]*=rhs;
		_v[2]*=rhs;
		_v[3]*=rhs;
		return *this;
	}

	/** Divide by scalar. */
	inline Vec4f operator / (value_type rhs) const
	{
		return Vec4f(_v[0]/rhs, _v[1]/rhs, _v[2]/rhs, _v[3]/rhs);
	}

	/** Unary divide by scalar. */
	inline Vec4f& operator /= (value_type rhs)
	{
		_v[0]/=rhs;
		_v[1]/=rhs;
		_v[2]/=rhs;
		_v[3]/=rhs;
		return *this;
	}

	/** Binary vector add. */
	inline Vec4f operator + (const Vec4f& rhs) const
	{
		return Vec4f(_v[0]+rhs._v[0], _v[1]+rhs._v[1],
			_v[2]+rhs._v[2], _v[3]+rhs._v[3]);
	}

	/** Unary vector add. Slightly more efficient because no temporary
	* intermediate object.
	*/
	inline Vec4f& operator += (const Vec4f& rhs)
	{
		_v[0] += rhs._v[0];
		_v[1] += rhs._v[1];
		_v[2] += rhs._v[2];
		_v[3] += rhs._v[3];
		return *this;
	}

	/** Binary vector subtract. */
	inline Vec4f operator - (const Vec4f& rhs) const
	{
		return Vec4f(_v[0]-rhs._v[0], _v[1]-rhs._v[1],
			_v[2]-rhs._v[2], _v[3]-rhs._v[3] );
	}

	/** Unary vector subtract. */
	inline Vec4f& operator -= (const Vec4f& rhs)
	{
		_v[0]-=rhs._v[0];
		_v[1]-=rhs._v[1];
		_v[2]-=rhs._v[2];
		_v[3]-=rhs._v[3];
		return *this;
	}

	/** Negation operator. Returns the negative of the Vec4f. */
	inline const Vec4f operator - () const
	{
		return Vec4f (-_v[0], -_v[1], -_v[2], -_v[3]);
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
		value_type norm = Vec4f::length();
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

};    // end of class Vec4f

class Vec4d
{
    public:

        /** Type of Vec class.*/
        typedef double value_type;

        /** Number of vector components. */
        enum { num_components = 4 };
        
        value_type _v[4];

        Vec4d() { _v[0]=0.0; _v[1]=0.0; _v[2]=0.0; _v[3]=0.0; }

        Vec4d(value_type x, value_type y, value_type z, value_type w)
        {
            _v[0]=x;
            _v[1]=y;
            _v[2]=z;
            _v[3]=w;
        }

        Vec4d(const Vec3d& v3,value_type w)
        {
            _v[0]=v3[0];
            _v[1]=v3[1];
            _v[2]=v3[2];
            _v[3]=w;
        }
            
        inline Vec4d(const Vec4f& vec) { _v[0]=vec._v[0]; _v[1]=vec._v[1]; _v[2]=vec._v[2]; _v[3]=vec._v[3];}
        
        inline operator Vec4f() const { return Vec4f(static_cast<float>(_v[0]),static_cast<float>(_v[1]),static_cast<float>(_v[2]),static_cast<float>(_v[3]));}


        inline bool operator == (const Vec4d& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1] && _v[2]==v._v[2] && _v[3]==v._v[3]; }

        inline bool operator != (const Vec4d& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1] || _v[2]!=v._v[2] || _v[3]!=v._v[3]; }

        inline bool operator <  (const Vec4d& v) const
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
            return (unsigned int)clampTo((_v[0]*255.0),0.0,255.0)<<24 |
                   (unsigned int)clampTo((_v[1]*255.0),0.0,255.0)<<16 |
                   (unsigned int)clampTo((_v[2]*255.0),0.0,255.0)<<8  |
                   (unsigned int)clampTo((_v[3]*255.0),0.0,255.0);
        }

        inline unsigned int asRGBA() const
        {
            return (unsigned int)clampTo((_v[3]*255.0),0.0,255.0)<<24 |
                   (unsigned int)clampTo((_v[2]*255.0),0.0,255.0)<<16 |
                   (unsigned int)clampTo((_v[1]*255.0),0.0,255.0)<<8  |
                   (unsigned int)clampTo((_v[0]*255.0),0.0,255.0);
        }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]) || osg::isNaN(_v[2]) || osg::isNaN(_v[3]); }

        /** Dot product. */
        inline value_type operator * (const Vec4d& rhs) const
        {
            return _v[0]*rhs._v[0]+
                   _v[1]*rhs._v[1]+
                   _v[2]*rhs._v[2]+
                   _v[3]*rhs._v[3] ;
        }

        /** Multiply by scalar. */
        inline Vec4d operator * (value_type rhs) const
        {
            return Vec4d(_v[0]*rhs, _v[1]*rhs, _v[2]*rhs, _v[3]*rhs);
        }

        /** Unary multiply by scalar. */
        inline Vec4d& operator *= (value_type rhs)
        {
            _v[0]*=rhs;
            _v[1]*=rhs;
            _v[2]*=rhs;
            _v[3]*=rhs;
            return *this;
        }

        /** Divide by scalar. */
        inline Vec4d operator / (value_type rhs) const
        {
            return Vec4d(_v[0]/rhs, _v[1]/rhs, _v[2]/rhs, _v[3]/rhs);
        }

        /** Unary divide by scalar. */
        inline Vec4d& operator /= (value_type rhs)
        {
            _v[0]/=rhs;
            _v[1]/=rhs;
            _v[2]/=rhs;
            _v[3]/=rhs;
            return *this;
        }

        /** Binary vector add. */
        inline Vec4d operator + (const Vec4d& rhs) const
        {
            return Vec4d(_v[0]+rhs._v[0], _v[1]+rhs._v[1],
                         _v[2]+rhs._v[2], _v[3]+rhs._v[3]);
        }

        /** Unary vector add. Slightly more efficient because no temporary
          * intermediate object.
        */
        inline Vec4d& operator += (const Vec4d& rhs)
        {
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            _v[2] += rhs._v[2];
            _v[3] += rhs._v[3];
            return *this;
        }

        /** Binary vector subtract. */
        inline Vec4d operator - (const Vec4d& rhs) const
        {
            return Vec4d(_v[0]-rhs._v[0], _v[1]-rhs._v[1],
                         _v[2]-rhs._v[2], _v[3]-rhs._v[3] );
        }

        /** Unary vector subtract. */
        inline Vec4d& operator -= (const Vec4d& rhs)
        {
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            _v[2]-=rhs._v[2];
            _v[3]-=rhs._v[3];
            return *this;
        }

        /** Negation operator. Returns the negative of the Vec4d. */
        inline const Vec4d operator - () const
        {
            return Vec4d (-_v[0], -_v[1], -_v[2], -_v[3]);
        }

        /** Length of the vector = sqrt( vec . vec ) */
        inline value_type length() const
        {
            return sqrt( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] + _v[3]*_v[3]);
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
            value_type norm = Vec4d::length();
            if (norm>0.0f)
            {
                value_type inv = 1.0/norm;
                _v[0] *= inv;
                _v[1] *= inv;
                _v[2] *= inv;
                _v[3] *= inv;
            }
            return( norm );
        }

};    // end of class Vec4d

class Matrixf;
class Matrixd;

/** A quaternion class. It can be used to represent an orientation in 3D space.*/
class Quat
{

    public:

        typedef double value_type;

        value_type  _v[4];    // a four-vector

        inline Quat() { _v[0]=0.0; _v[1]=0.0; _v[2]=0.0; _v[3]=1.0; }

        inline Quat( value_type x, value_type y, value_type z, value_type w )
        {
            _v[0]=x;
            _v[1]=y;
            _v[2]=z;
            _v[3]=w;
        }

        inline Quat( const Vec4f& v )
        {
            _v[0]=v.x();
            _v[1]=v.y();
            _v[2]=v.z();
            _v[3]=v.w();
        }

        inline Quat( const Vec4d& v )
        {
            _v[0]=v.x();
            _v[1]=v.y();
            _v[2]=v.z();
            _v[3]=v.w();
        }

        inline Quat( value_type angle, const Vec3f& axis)
        {
            makeRotate(angle,axis);
        }
        inline Quat( value_type angle, const Vec3d& axis)
        {
            makeRotate(angle,axis);
        }

        inline Quat( value_type angle1, const Vec3f& axis1, 
                     value_type angle2, const Vec3f& axis2,
                     value_type angle3, const Vec3f& axis3)
        {
            makeRotate(angle1,axis1,angle2,axis2,angle3,axis3);
        }

        inline Quat( value_type angle1, const Vec3d& axis1, 
                     value_type angle2, const Vec3d& axis2,
                     value_type angle3, const Vec3d& axis3)
        {
            makeRotate(angle1,axis1,angle2,axis2,angle3,axis3);
        }

        inline Quat& operator = (const Quat& v) { _v[0]=v._v[0];  _v[1]=v._v[1]; _v[2]=v._v[2]; _v[3]=v._v[3]; return *this; }

        inline bool operator == (const Quat& v) const { return _v[0]==v._v[0] && _v[1]==v._v[1] && _v[2]==v._v[2] && _v[3]==v._v[3]; }

        inline bool operator != (const Quat& v) const { return _v[0]!=v._v[0] || _v[1]!=v._v[1] || _v[2]!=v._v[2] || _v[3]!=v._v[3]; }

        inline bool operator <  (const Quat& v) const
        {
            if (_v[0]<v._v[0]) return true;
            else if (_v[0]>v._v[0]) return false;
            else if (_v[1]<v._v[1]) return true;
            else if (_v[1]>v._v[1]) return false;
            else if (_v[2]<v._v[2]) return true;
            else if (_v[2]>v._v[2]) return false;
            else return (_v[3]<v._v[3]);
        }

        /* ----------------------------------
           Methods to access data members
        ---------------------------------- */

        inline Vec4d asVec4() const
        {
            return Vec4d(_v[0], _v[1], _v[2], _v[3]);
        }

        inline Vec3d asVec3() const
        {
            return Vec3d(_v[0], _v[1], _v[2]);
        }

        inline void set(value_type x, value_type y, value_type z, value_type w)
        {
            _v[0]=x;
            _v[1]=y;
            _v[2]=z;
            _v[3]=w;
        }
        
        inline void set(const osg::Vec4f& v)
        {
            _v[0]=v.x();
            _v[1]=v.y();
            _v[2]=v.z();
            _v[3]=v.w();
        }

        inline void set(const osg::Vec4d& v)
        {
            _v[0]=v.x();
            _v[1]=v.y();
            _v[2]=v.z();
            _v[3]=v.w();
        }
        
        void set(const Matrixf& matrix);
        
        void set(const Matrixd& matrix);
        
        void get(Matrixf& matrix) const;

        void get(Matrixd& matrix) const;
        

        inline value_type & operator [] (int i) { return _v[i]; }
        inline value_type   operator [] (int i) const { return _v[i]; }

        inline value_type & x() { return _v[0]; }
        inline value_type & y() { return _v[1]; }
        inline value_type & z() { return _v[2]; }
        inline value_type & w() { return _v[3]; }

        inline value_type x() const { return _v[0]; }
        inline value_type y() const { return _v[1]; }
        inline value_type z() const { return _v[2]; }
        inline value_type w() const { return _v[3]; }

        /** return true if the Quat represents a zero rotation, and therefore can be ignored in computations.*/
        bool zeroRotation() const { return _v[0]==0.0 && _v[1]==0.0 && _v[2]==0.0 && _v[3]==1.0; } 


         /* ------------------------------------------------------------- 
                   BASIC ARITHMETIC METHODS            
        Implemented in terms of Vec4s.  Some Vec4 operators, e.g.
        operator* are not appropriate for quaternions (as
        mathematical objects) so they are implemented differently.
        Also define methods for conjugate and the multiplicative inverse.            
        ------------------------------------------------------------- */
        /// Multiply by scalar 
        inline const Quat operator * (value_type rhs) const
        {
            return Quat(_v[0]*rhs, _v[1]*rhs, _v[2]*rhs, _v[3]*rhs);
        }

        /// Unary multiply by scalar 
        inline Quat& operator *= (value_type rhs)
        {
            _v[0]*=rhs;
            _v[1]*=rhs;
            _v[2]*=rhs;
            _v[3]*=rhs;
            return *this;        // enable nesting
        }

        /// Binary multiply 
        inline const Quat operator*(const Quat& rhs) const
        {
            return Quat( rhs._v[3]*_v[0] + rhs._v[0]*_v[3] + rhs._v[1]*_v[2] - rhs._v[2]*_v[1],
                 rhs._v[3]*_v[1] - rhs._v[0]*_v[2] + rhs._v[1]*_v[3] + rhs._v[2]*_v[0],
                 rhs._v[3]*_v[2] + rhs._v[0]*_v[1] - rhs._v[1]*_v[0] + rhs._v[2]*_v[3],
                 rhs._v[3]*_v[3] - rhs._v[0]*_v[0] - rhs._v[1]*_v[1] - rhs._v[2]*_v[2] );
        }

        /// Unary multiply 
        inline Quat& operator*=(const Quat& rhs)
        {
            value_type x = rhs._v[3]*_v[0] + rhs._v[0]*_v[3] + rhs._v[1]*_v[2] - rhs._v[2]*_v[1];
            value_type y = rhs._v[3]*_v[1] - rhs._v[0]*_v[2] + rhs._v[1]*_v[3] + rhs._v[2]*_v[0];
            value_type z = rhs._v[3]*_v[2] + rhs._v[0]*_v[1] - rhs._v[1]*_v[0] + rhs._v[2]*_v[3];
            _v[3]   = rhs._v[3]*_v[3] - rhs._v[0]*_v[0] - rhs._v[1]*_v[1] - rhs._v[2]*_v[2];

            _v[2] = z;
            _v[1] = y;
            _v[0] = x;

            return (*this);            // enable nesting
        }

        /// Divide by scalar 
        inline Quat operator / (value_type rhs) const
        {
            value_type div = 1.0/rhs;
            return Quat(_v[0]*div, _v[1]*div, _v[2]*div, _v[3]*div);
        }

        /// Unary divide by scalar 
        inline Quat& operator /= (value_type rhs)
        {
            value_type div = 1.0/rhs;
            _v[0]*=div;
            _v[1]*=div;
            _v[2]*=div;
            _v[3]*=div;
            return *this;
        }

        /// Binary divide 
        inline const Quat operator/(const Quat& denom) const
        {
            return ( (*this) * denom.inverse() );
        }

        /// Unary divide 
        inline Quat& operator/=(const Quat& denom)
        {
            (*this) = (*this) * denom.inverse();
            return (*this);            // enable nesting
        }

        /// Binary addition 
        inline const Quat operator + (const Quat& rhs) const
        {
            return Quat(_v[0]+rhs._v[0], _v[1]+rhs._v[1],
                _v[2]+rhs._v[2], _v[3]+rhs._v[3]);
        }

        /// Unary addition
        inline Quat& operator += (const Quat& rhs)
        {
            _v[0] += rhs._v[0];
            _v[1] += rhs._v[1];
            _v[2] += rhs._v[2];
            _v[3] += rhs._v[3];
            return *this;            // enable nesting
        }

        /// Binary subtraction 
        inline const Quat operator - (const Quat& rhs) const
        {
            return Quat(_v[0]-rhs._v[0], _v[1]-rhs._v[1],
                _v[2]-rhs._v[2], _v[3]-rhs._v[3] );
        }

        /// Unary subtraction 
        inline Quat& operator -= (const Quat& rhs)
        {
            _v[0]-=rhs._v[0];
            _v[1]-=rhs._v[1];
            _v[2]-=rhs._v[2];
            _v[3]-=rhs._v[3];
            return *this;            // enable nesting
        }

        /** Negation operator - returns the negative of the quaternion.
        Basically just calls operator - () on the Vec4 */
        inline const Quat operator - () const
        {
            return Quat (-_v[0], -_v[1], -_v[2], -_v[3]);
        }

        /// Length of the quaternion = sqrt( vec . vec )
        value_type length() const
        {
            return sqrt( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] + _v[3]*_v[3]);
        }

        /// Length of the quaternion = vec . vec
        value_type length2() const
        {
            return _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] + _v[3]*_v[3];
        }

        /// Conjugate 
        inline Quat conj () const
        { 
             return Quat( -_v[0], -_v[1], -_v[2], _v[3] );
        }

        /// Multiplicative inverse method: q^(-1) = q^*/(q.q^*)
        inline const Quat inverse () const
        {
             return conj() / length2();
         }

      /* -------------------------------------------------------- 
               METHODS RELATED TO ROTATIONS
        Set a quaternion which will perform a rotation of an
        angle around the axis given by the vector (x,y,z).
        Should be written to also accept an angle and a Vec3?

        Define Spherical Linear interpolation method also

        Not inlined - see the Quat.cpp file for implementation
        -------------------------------------------------------- */
        void makeRotate( value_type  angle, 
                          value_type  x, value_type  y, value_type  z );
        void makeRotate ( value_type  angle, const Vec3f& vec );
        void makeRotate ( value_type  angle, const Vec3d& vec );

        void makeRotate ( value_type  angle1, const Vec3f& axis1, 
                          value_type  angle2, const Vec3f& axis2,
                          value_type  angle3, const Vec3f& axis3);
        void makeRotate ( value_type  angle1, const Vec3d& axis1, 
                          value_type  angle2, const Vec3d& axis2,
                          value_type  angle3, const Vec3d& axis3);

        /** Make a rotation Quat which will rotate vec1 to vec2.
            Generally take a dot product to get the angle between these
            and then use a cross product to get the rotation axis
            Watch out for the two special cases when the vectors
            are co-incident or opposite in direction.*/
        void makeRotate( const Vec3f& vec1, const Vec3f& vec2 );
        /** Make a rotation Quat which will rotate vec1 to vec2.
            Generally take a dot product to get the angle between these
            and then use a cross product to get the rotation axis
            Watch out for the two special cases of when the vectors
            are co-incident or opposite in direction.*/
        void makeRotate( const Vec3d& vec1, const Vec3d& vec2 );
    
        void makeRotate_original( const Vec3d& vec1, const Vec3d& vec2 );

        /** Return the angle and vector components represented by the quaternion.*/
        void getRotate ( value_type & angle, value_type & x, value_type & y, value_type & z ) const;

        /** Return the angle and vector represented by the quaternion.*/
        void getRotate ( value_type & angle, Vec3f& vec ) const;

        /** Return the angle and vector represented by the quaternion.*/
        void getRotate ( value_type & angle, Vec3d& vec ) const;

        /** Spherical Linear Interpolation.
        As t goes from 0 to 1, the Quat object goes from "from" to "to". */
        void slerp   ( value_type  t, const Quat& from, const Quat& to);
               
        /** Rotate a vector by this quaternion.*/
        Vec3f operator* (const Vec3f& v) const
        {
            // nVidia SDK implementation
            Vec3f uv, uuv; 
            Vec3f qvec(_v[0], _v[1], _v[2]);
            uv = qvec ^ v;
            uuv = qvec ^ uv; 
            uv *= ( 2.0f * _v[3] ); 
            uuv *= 2.0f; 
            return v + uv + uuv;
        }
               
        /** Rotate a vector by this quaternion.*/
        Vec3d operator* (const Vec3d& v) const
        {
            // nVidia SDK implementation
            Vec3d uv, uuv; 
            Vec3d qvec(_v[0], _v[1], _v[2]);
            uv = qvec ^ v;
            uuv = qvec ^ uv; 
            uv *= ( 2.0f * _v[3] ); 
            uuv *= 2.0f; 
            return v + uv + uuv;
        }
        
    protected:
    
};    // end of class prototype

class Matrixf
{
    public:
    
        typedef float value_type;

        inline Matrixf() { makeIdentity(); }
        inline Matrixf( const Matrixf& mat) { set(mat.ptr()); }
        Matrixf( const Matrixd& mat );
        inline explicit Matrixf( float const * const ptr ) { set(ptr); }
        inline explicit Matrixf( double const * const ptr ) { set(ptr); }
        inline explicit Matrixf( const Quat& quat ) { makeRotate(quat); }

        Matrixf( value_type a00, value_type a01, value_type a02, value_type a03,
                 value_type a10, value_type a11, value_type a12, value_type a13,
                 value_type a20, value_type a21, value_type a22, value_type a23,
                 value_type a30, value_type a31, value_type a32, value_type a33);

        ~Matrixf() {}

        int compare(const Matrixf& m) const;

        bool operator < (const Matrixf& m) const { return compare(m)<0; }
        bool operator == (const Matrixf& m) const { return compare(m)==0; }
        bool operator != (const Matrixf& m) const { return compare(m)!=0; }

        inline value_type& operator()(int row, int col) { return _mat[row][col]; }
        inline value_type operator()(int row, int col) const { return _mat[row][col]; }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_mat[0][0]) || osg::isNaN(_mat[0][1]) || osg::isNaN(_mat[0][2]) || osg::isNaN(_mat[0][3]) ||
                                                 osg::isNaN(_mat[1][0]) || osg::isNaN(_mat[1][1]) || osg::isNaN(_mat[1][2]) || osg::isNaN(_mat[1][3]) ||
                                                 osg::isNaN(_mat[2][0]) || osg::isNaN(_mat[2][1]) || osg::isNaN(_mat[2][2]) || osg::isNaN(_mat[2][3]) ||
                                                 osg::isNaN(_mat[3][0]) || osg::isNaN(_mat[3][1]) || osg::isNaN(_mat[3][2]) || osg::isNaN(_mat[3][3]); }

        inline Matrixf& operator = (const Matrixf& rhs)
        {
            if( &rhs == this ) return *this;
            set(rhs.ptr());
            return *this;
        }
        
        Matrixf& operator = (const Matrixd& other);

        inline void set(const Matrixf& rhs) { set(rhs.ptr()); }

        void set(const Matrixd& rhs);

        inline void set(float const * const ptr)
        {
            value_type* local_ptr = (value_type*)_mat;
            for(int i=0;i<16;++i) local_ptr[i]=(value_type)ptr[i];
        }
        
        inline void set(double const * const ptr)
        {
            value_type* local_ptr = (value_type*)_mat;
            for(int i=0;i<16;++i) local_ptr[i]=(value_type)ptr[i];
        }

        void set(value_type a00, value_type a01, value_type a02,value_type a03,
                 value_type a10, value_type a11, value_type a12,value_type a13,
                 value_type a20, value_type a21, value_type a22,value_type a23,
                 value_type a30, value_type a31, value_type a32,value_type a33);
                  
        value_type * ptr() { return (value_type*)_mat; }
        const value_type * ptr() const { return (const value_type *)_mat; }

        bool isIdentity() const
        {
            return _mat[0][0]==1.0f && _mat[0][1]==0.0f && _mat[0][2]==0.0f &&  _mat[0][3]==0.0f &&
                   _mat[1][0]==0.0f && _mat[1][1]==1.0f && _mat[1][2]==0.0f &&  _mat[1][3]==0.0f &&
                   _mat[2][0]==0.0f && _mat[2][1]==0.0f && _mat[2][2]==1.0f &&  _mat[2][3]==0.0f &&
                   _mat[3][0]==0.0f && _mat[3][1]==0.0f && _mat[3][2]==0.0f &&  _mat[3][3]==1.0f;
        }

        void makeIdentity();
        
        void makeScale( const Vec3f& );
        void makeScale( const Vec3d& );
        void makeScale( value_type, value_type, value_type );
        
        void makeTranslate( const Vec3f& );
        void makeTranslate( const Vec3d& );
        void makeTranslate( value_type, value_type, value_type );
        
        void makeRotate( const Vec3f& from, const Vec3f& to );
        void makeRotate( const Vec3d& from, const Vec3d& to );
        void makeRotate( value_type angle, const Vec3f& axis );
        void makeRotate( value_type angle, const Vec3d& axis );
        void makeRotate( value_type angle, value_type x, value_type y, value_type z );
        void makeRotate( const Quat& );
        void makeRotate( value_type angle1, const Vec3f& axis1, 
                         value_type angle2, const Vec3f& axis2,
                         value_type angle3, const Vec3f& axis3);
        void makeRotate( value_type angle1, const Vec3d& axis1, 
                         value_type angle2, const Vec3d& axis2,
                         value_type angle3, const Vec3d& axis3);

        
        /** decompose the matrix into translation, rotation, scale and scale orientation.*/        
        void decompose( osg::Vec3f& translation,
                        osg::Quat& rotation, 
                        osg::Vec3f& scale, 
                        osg::Quat& so ) const;

        /** decompose the matrix into translation, rotation, scale and scale orientation.*/        
        void decompose( osg::Vec3d& translation,
                        osg::Quat& rotation, 
                        osg::Vec3d& scale, 
                        osg::Quat& so ) const;


        /** Set to an orthographic projection.
         * See glOrtho for further details.
        */
        void makeOrtho(double left,   double right,
                       double bottom, double top,
                       double zNear,  double zFar);

        /** Get the orthographic settings of the orthographic projection matrix.
          * Note, if matrix is not an orthographic matrix then invalid values 
          * will be returned.
        */
        bool getOrtho(double& left,   double& right,
                      double& bottom, double& top,
                      double& zNear,  double& zFar) const;

        /** Set to a 2D orthographic projection.
          * See glOrtho2D for further details.
        */
        inline void makeOrtho2D(double left,   double right,
                                double bottom, double top)
        {
            makeOrtho(left,right,bottom,top,-1.0,1.0);
        }


        /** Set to a perspective projection.
          * See glFrustum for further details.
        */
        void makeFrustum(double left,   double right,
                         double bottom, double top,
                         double zNear,  double zFar);

        /** Get the frustum settings of a perspective projection matrix.
          * Note, if matrix is not a perspective matrix then invalid values
          * will be returned.
        */
        bool getFrustum(double& left,   double& right,
                        double& bottom, double& top,
                        double& zNear,  double& zFar) const;

        /** Set to a symmetrical perspective projection.
          * See gluPerspective for further details.
          * Aspect ratio is defined as width/height.
        */
        void makePerspective(double fovy,  double aspectRatio,
                             double zNear, double zFar);

        /** Get the frustum settings of a symmetric perspective projection
          * matrix.
          * Return false if matrix is not a perspective matrix,
          * where parameter values are undefined. 
          * Note, if matrix is not a symmetric perspective matrix then the
          * shear will be lost.
          * Asymmetric matrices occur when stereo, power walls, caves and
          * reality center display are used.
          * In these configuration one should use the AsFrustum method instead.
        */
        bool getPerspective(double& fovy,  double& aspectRatio,
                            double& zNear, double& zFar) const;

        /** Set the position and orientation to be a view matrix,
          * using the same convention as gluLookAt.
        */
        void makeLookAt(const Vec3d& eye,const Vec3d& center,const Vec3d& up);

        /** Get to the position and orientation of a modelview matrix,
          * using the same convention as gluLookAt.
        */
        void getLookAt(Vec3f& eye,Vec3f& center,Vec3f& up,
                       value_type lookDistance=1.0f) const;

        /** Get to the position and orientation of a modelview matrix,
          * using the same convention as gluLookAt.
        */
        void getLookAt(Vec3d& eye,Vec3d& center,Vec3d& up,
                       value_type lookDistance=1.0f) const;

        /** invert the matrix rhs, automatically select invert_4x3 or invert_4x4. */
        inline bool invert( const Matrixf& rhs)
        {
            bool is_4x3 = (rhs._mat[0][3]==0.0f && rhs._mat[1][3]==0.0f &&  rhs._mat[2][3]==0.0f && rhs._mat[3][3]==1.0f);
            return is_4x3 ? invert_4x3(rhs) :  invert_4x4(rhs);
        }

        /** 4x3 matrix invert, not right hand column is assumed to be 0,0,0,1. */
        bool invert_4x3( const Matrixf& rhs);

        /** full 4x4 matrix invert. */
        bool invert_4x4( const Matrixf& rhs);

        /** ortho-normalize the 3x3 rotation & scale matrix */ 
        void orthoNormalize(const Matrixf& rhs); 

        //basic utility functions to create new matrices
        inline static Matrixf identity( void );
        inline static Matrixf scale( const Vec3f& sv);
        inline static Matrixf scale( const Vec3d& sv);
        inline static Matrixf scale( value_type sx, value_type sy, value_type sz);
        inline static Matrixf translate( const Vec3f& dv);
        inline static Matrixf translate( const Vec3d& dv);
        inline static Matrixf translate( value_type x, value_type y, value_type z);
        inline static Matrixf rotate( const Vec3f& from, const Vec3f& to);
        inline static Matrixf rotate( const Vec3d& from, const Vec3d& to);
        inline static Matrixf rotate( value_type angle, value_type x, value_type y, value_type z);
        inline static Matrixf rotate( value_type angle, const Vec3f& axis);
        inline static Matrixf rotate( value_type angle, const Vec3d& axis);
        inline static Matrixf rotate( value_type angle1, const Vec3f& axis1, 
                                      value_type angle2, const Vec3f& axis2,
                                      value_type angle3, const Vec3f& axis3);
        inline static Matrixf rotate( value_type angle1, const Vec3d& axis1, 
                                      value_type angle2, const Vec3d& axis2,
                                      value_type angle3, const Vec3d& axis3);
        inline static Matrixf rotate( const Quat& quat);
        inline static Matrixf inverse( const Matrixf& matrix);
        inline static Matrixf orthoNormal(const Matrixf& matrix); 
        
        /** Create an orthographic projection matrix.
          * See glOrtho for further details.
        */
        inline static Matrixf ortho(double left,   double right,
                                    double bottom, double top,
                                    double zNear,  double zFar);

        /** Create a 2D orthographic projection.
          * See glOrtho for further details.
        */
        inline static Matrixf ortho2D(double left,   double right,
                                      double bottom, double top);

        /** Create a perspective projection.
          * See glFrustum for further details.
        */
        inline static Matrixf frustum(double left,   double right,
                                      double bottom, double top,
                                      double zNear,  double zFar);

        /** Create a symmetrical perspective projection.
          * See gluPerspective for further details.
          * Aspect ratio is defined as width/height.
        */
        inline static Matrixf perspective(double fovy,  double aspectRatio,
                                          double zNear, double zFar);

        /** Create the position and orientation as per a camera,
          * using the same convention as gluLookAt.
        */
        inline static Matrixf lookAt(const Vec3f& eye,
                                     const Vec3f& center,
                                     const Vec3f& up);

        /** Create the position and orientation as per a camera,
          * using the same convention as gluLookAt.
        */
        inline static Matrixf lookAt(const Vec3d& eye,
                                     const Vec3d& center,
                                     const Vec3d& up);

        inline Vec3f preMult( const Vec3f& v ) const;
        inline Vec3d preMult( const Vec3d& v ) const;
        inline Vec3f postMult( const Vec3f& v ) const;
        inline Vec3d postMult( const Vec3d& v ) const;
        inline Vec3f operator* ( const Vec3f& v ) const;
        inline Vec3d operator* ( const Vec3d& v ) const;
        inline Vec4f preMult( const Vec4f& v ) const;
        inline Vec4d preMult( const Vec4d& v ) const;
        inline Vec4f postMult( const Vec4f& v ) const;
        inline Vec4d postMult( const Vec4d& v ) const;
        inline Vec4f operator* ( const Vec4f& v ) const;
        inline Vec4d operator* ( const Vec4d& v ) const;

#ifdef USE_DEPRECATED_API
        inline void set(const Quat& q) { makeRotate(q); }
        inline void get(Quat& q) const { q = getRotate(); }
#endif

        void setRotate(const Quat& q);
        /** Get the matrix rotation as a Quat. Note that this function
          * assumes a non-scaled matrix and will return incorrect results
          * for scaled matrixces. Consider decompose() instead.
          */
        Quat getRotate() const;


        void setTrans( value_type tx, value_type ty, value_type tz );
        void setTrans( const Vec3f& v );
        void setTrans( const Vec3d& v );
        
        inline Vec3d getTrans() const { return Vec3d(_mat[3][0],_mat[3][1],_mat[3][2]); } 
        
        inline Vec3d getScale() const {
          Vec3d x_vec(_mat[0][0],_mat[1][0],_mat[2][0]); 
          Vec3d y_vec(_mat[0][1],_mat[1][1],_mat[2][1]); 
          Vec3d z_vec(_mat[0][2],_mat[1][2],_mat[2][2]); 
          return Vec3d(x_vec.length(), y_vec.length(), z_vec.length()); 
        }
        
        /** apply a 3x3 transform of v*M[0..2,0..2]. */
        inline static Vec3f transform3x3(const Vec3f& v,const Matrixf& m);

        /** apply a 3x3 transform of v*M[0..2,0..2]. */
        inline static Vec3d transform3x3(const Vec3d& v,const Matrixf& m);

        /** apply a 3x3 transform of M[0..2,0..2]*v. */
        inline static Vec3f transform3x3(const Matrixf& m,const Vec3f& v);

        /** apply a 3x3 transform of M[0..2,0..2]*v. */
        inline static Vec3d transform3x3(const Matrixf& m,const Vec3d& v);

        // basic Matrixf multiplication, our workhorse methods.
        void mult( const Matrixf&, const Matrixf& );
        void preMult( const Matrixf& );
        void postMult( const Matrixf& );

        inline void operator *= ( const Matrixf& other ) 
        {    if( this == &other ) {
                Matrixf temp(other);
                postMult( temp );
            }
            else postMult( other ); 
        }

		//James was here... added these for benchmark tests...
		inline void operator *= (const double value)
		{
			for (size_t row=0;row<4;row++)
			{
				_mat[row][0] *= value;
				_mat[row][1] *= value;
				_mat[row][2] *= value;
				_mat[row][3] *= value;
			}
		}

		inline void operator += (const Matrixf& other)
		{
			for (size_t row=0;row<4;row++)
			{
				_mat[row][0] += other(row,0);
				_mat[row][1] += other(row,1);
				_mat[row][2] += other(row,2);
				_mat[row][3] += other(row,3);
			}
		}

        inline Matrixf operator * ( const Matrixf &m ) const
        {
            osg::Matrixf r;
            r.mult(*this,m);
            return  r;
        }

    protected:
        value_type _mat[4][4];

};

inline Matrixf Matrixf::inverse( const Matrixf& matrix)
{
	Matrixf m;
	m.invert(matrix);
	return m;
}

inline Matrixf Matrixf::translate(value_type tx, value_type ty, value_type tz)
{
	Matrixf m;
	m.makeTranslate(tx,ty,tz);
	return m;
}

inline Matrixf Matrixf::translate(const Vec3f& v )
{
	return translate(v.x(), v.y(), v.z() );
}

inline Matrixf Matrixf::translate(const Vec3d& v )
{
	return translate(v.x(), v.y(), v.z() );
}

inline Vec3f Matrixf::postMult( const Vec3f& v ) const
{
	value_type d = 1.0f/(_mat[3][0]*v.x()+_mat[3][1]*v.y()+_mat[3][2]*v.z()+_mat[3][3]) ;
	return Vec3f( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3])*d,
		(_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3])*d,
		(_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3])*d) ;
}
inline Vec3d Matrixf::postMult( const Vec3d& v ) const
{
	value_type d = 1.0f/(_mat[3][0]*v.x()+_mat[3][1]*v.y()+_mat[3][2]*v.z()+_mat[3][3]) ;
	return Vec3d( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3])*d,
		(_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3])*d,
		(_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3])*d) ;
}

inline Vec3f Matrixf::preMult( const Vec3f& v ) const
{
	value_type d = 1.0f/(_mat[0][3]*v.x()+_mat[1][3]*v.y()+_mat[2][3]*v.z()+_mat[3][3]) ;
	return Vec3f( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0])*d,
		(_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1])*d,
		(_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2])*d);
}
inline Vec3d Matrixf::preMult( const Vec3d& v ) const
{
	value_type d = 1.0f/(_mat[0][3]*v.x()+_mat[1][3]*v.y()+_mat[2][3]*v.z()+_mat[3][3]) ;
	return Vec3d( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0])*d,
		(_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1])*d,
		(_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2])*d);
}

inline Vec4f Matrixf::postMult( const Vec4f& v ) const
{
	return Vec4f( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3]*v.w()),
		(_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3]*v.w()),
		(_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3]*v.w()),
		(_mat[3][0]*v.x() + _mat[3][1]*v.y() + _mat[3][2]*v.z() + _mat[3][3]*v.w())) ;
}
inline Vec4d Matrixf::postMult( const Vec4d& v ) const
{
	return Vec4d( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3]*v.w()),
		(_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3]*v.w()),
		(_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3]*v.w()),
		(_mat[3][0]*v.x() + _mat[3][1]*v.y() + _mat[3][2]*v.z() + _mat[3][3]*v.w())) ;
}

inline Vec4f Matrixf::preMult( const Vec4f& v ) const
{
	return Vec4f( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0]*v.w()),
		(_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1]*v.w()),
		(_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2]*v.w()),
		(_mat[0][3]*v.x() + _mat[1][3]*v.y() + _mat[2][3]*v.z() + _mat[3][3]*v.w()));
}
inline Vec4d Matrixf::preMult( const Vec4d& v ) const
{
	return Vec4d( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0]*v.w()),
		(_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1]*v.w()),
		(_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2]*v.w()),
		(_mat[0][3]*v.x() + _mat[1][3]*v.y() + _mat[2][3]*v.z() + _mat[3][3]*v.w()));
}
inline Vec3f Matrixf::transform3x3(const Vec3f& v,const Matrixf& m)
{
	return Vec3f( (m._mat[0][0]*v.x() + m._mat[1][0]*v.y() + m._mat[2][0]*v.z()),
		(m._mat[0][1]*v.x() + m._mat[1][1]*v.y() + m._mat[2][1]*v.z()),
		(m._mat[0][2]*v.x() + m._mat[1][2]*v.y() + m._mat[2][2]*v.z()));
}
inline Vec3d Matrixf::transform3x3(const Vec3d& v,const Matrixf& m)
{
	return Vec3d( (m._mat[0][0]*v.x() + m._mat[1][0]*v.y() + m._mat[2][0]*v.z()),
		(m._mat[0][1]*v.x() + m._mat[1][1]*v.y() + m._mat[2][1]*v.z()),
		(m._mat[0][2]*v.x() + m._mat[1][2]*v.y() + m._mat[2][2]*v.z()));
}

inline Vec3f Matrixf::transform3x3(const Matrixf& m,const Vec3f& v)
{
	return Vec3f( (m._mat[0][0]*v.x() + m._mat[0][1]*v.y() + m._mat[0][2]*v.z()),
		(m._mat[1][0]*v.x() + m._mat[1][1]*v.y() + m._mat[1][2]*v.z()),
		(m._mat[2][0]*v.x() + m._mat[2][1]*v.y() + m._mat[2][2]*v.z()) ) ;
}
inline Vec3d Matrixf::transform3x3(const Matrixf& m,const Vec3d& v)
{
	return Vec3d( (m._mat[0][0]*v.x() + m._mat[0][1]*v.y() + m._mat[0][2]*v.z()),
		(m._mat[1][0]*v.x() + m._mat[1][1]*v.y() + m._mat[1][2]*v.z()),
		(m._mat[2][0]*v.x() + m._mat[2][1]*v.y() + m._mat[2][2]*v.z()) ) ;
}


class Matrixd
{
    public:
    
        typedef double value_type;

        inline Matrixd() { makeIdentity(); }
        inline Matrixd( const Matrixd& mat) { set(mat.ptr()); }
        Matrixd( const Matrixf& mat );
        inline explicit Matrixd( float const * const ptr ) { set(ptr); }
        inline explicit Matrixd( double const * const ptr ) { set(ptr); }
        inline explicit Matrixd( const Quat& quat ) { makeRotate(quat); }

        Matrixd(value_type a00, value_type a01, value_type a02, value_type a03,
                value_type a10, value_type a11, value_type a12, value_type a13,
                value_type a20, value_type a21, value_type a22, value_type a23,
                value_type a30, value_type a31, value_type a32, value_type a33);

        ~Matrixd() {}

        int compare(const Matrixd& m) const;

        bool operator < (const Matrixd& m) const { return compare(m)<0; }
        bool operator == (const Matrixd& m) const { return compare(m)==0; }
        bool operator != (const Matrixd& m) const { return compare(m)!=0; }

        inline value_type& operator()(int row, int col) { return _mat[row][col]; }
        inline value_type operator()(int row, int col) const { return _mat[row][col]; }

        inline bool valid() const { return !isNaN(); }
        inline bool isNaN() const { return osg::isNaN(_mat[0][0]) || osg::isNaN(_mat[0][1]) || osg::isNaN(_mat[0][2]) || osg::isNaN(_mat[0][3]) ||
                                                 osg::isNaN(_mat[1][0]) || osg::isNaN(_mat[1][1]) || osg::isNaN(_mat[1][2]) || osg::isNaN(_mat[1][3]) ||
                                                 osg::isNaN(_mat[2][0]) || osg::isNaN(_mat[2][1]) || osg::isNaN(_mat[2][2]) || osg::isNaN(_mat[2][3]) ||
                                                 osg::isNaN(_mat[3][0]) || osg::isNaN(_mat[3][1]) || osg::isNaN(_mat[3][2]) || osg::isNaN(_mat[3][3]); }

        inline Matrixd& operator = (const Matrixd& rhs)
        {
            if( &rhs == this ) return *this;
            set(rhs.ptr());
            return *this;
        }
        
        Matrixd& operator = (const Matrixf& other);

        inline void set(const Matrixd& rhs) { set(rhs.ptr()); }

        void set(const Matrixf& rhs);

        inline void set(float const * const ptr)
        {
            value_type* local_ptr = (value_type*)_mat;
            for(int i=0;i<16;++i) local_ptr[i]=(value_type)ptr[i];
        }
        
        inline void set(double const * const ptr)
        {
            value_type* local_ptr = (value_type*)_mat;
            for(int i=0;i<16;++i) local_ptr[i]=(value_type)ptr[i];
        }

        void set(value_type a00, value_type a01, value_type a02,value_type a03,
                 value_type a10, value_type a11, value_type a12,value_type a13,
                 value_type a20, value_type a21, value_type a22,value_type a23,
                 value_type a30, value_type a31, value_type a32,value_type a33);
                  
        value_type * ptr() { return (value_type*)_mat; }
        const value_type * ptr() const { return (const value_type *)_mat; }

        bool isIdentity() const
        {
            return _mat[0][0]==1.0 && _mat[0][1]==0.0 && _mat[0][2]==0.0 &&  _mat[0][3]==0.0 &&
                   _mat[1][0]==0.0 && _mat[1][1]==1.0 && _mat[1][2]==0.0 &&  _mat[1][3]==0.0 &&
                   _mat[2][0]==0.0 && _mat[2][1]==0.0 && _mat[2][2]==1.0 &&  _mat[2][3]==0.0 &&
                   _mat[3][0]==0.0 && _mat[3][1]==0.0 && _mat[3][2]==0.0 &&  _mat[3][3]==1.0;
        }

        void makeIdentity();
        
        void makeScale( const Vec3f& );
        void makeScale( const Vec3d& );
        void makeScale( value_type, value_type, value_type );
        
        void makeTranslate( const Vec3f& );
        void makeTranslate( const Vec3d& );
        void makeTranslate( value_type, value_type, value_type );
        
        void makeRotate( const Vec3f& from, const Vec3f& to );
        void makeRotate( const Vec3d& from, const Vec3d& to );
        void makeRotate( value_type angle, const Vec3f& axis );
        void makeRotate( value_type angle, const Vec3d& axis );
        void makeRotate( value_type angle, value_type x, value_type y, value_type z );
        void makeRotate( const Quat& );
        void makeRotate( value_type angle1, const Vec3f& axis1, 
                         value_type angle2, const Vec3f& axis2,
                         value_type angle3, const Vec3f& axis3);
        void makeRotate( value_type angle1, const Vec3d& axis1, 
                         value_type angle2, const Vec3d& axis2,
                         value_type angle3, const Vec3d& axis3);


        /** decompose the matrix into translation, rotation, scale and scale orientation.*/        
        void decompose( osg::Vec3f& translation,
                        osg::Quat& rotation, 
                        osg::Vec3f& scale, 
                        osg::Quat& so ) const;

        /** decompose the matrix into translation, rotation, scale and scale orientation.*/        
        void decompose( osg::Vec3d& translation,
                        osg::Quat& rotation, 
                        osg::Vec3d& scale, 
                        osg::Quat& so ) const;


        /** Set to an orthographic projection.
         * See glOrtho for further details.
        */
        void makeOrtho(double left,   double right,
                       double bottom, double top,
                       double zNear,  double zFar);

        /** Get the orthographic settings of the orthographic projection matrix.
          * Note, if matrix is not an orthographic matrix then invalid values 
          * will be returned.
        */
        bool getOrtho(double& left,   double& right,
                      double& bottom, double& top,
                      double& zNear,  double& zFar) const;

        /** Set to a 2D orthographic projection.
          * See glOrtho2D for further details.
        */
        inline void makeOrtho2D(double left,   double right,
                                double bottom, double top)
        {
            makeOrtho(left,right,bottom,top,-1.0,1.0);
        }


        /** Set to a perspective projection.
          * See glFrustum for further details.
        */
        void makeFrustum(double left,   double right,
                         double bottom, double top,
                         double zNear,  double zFar);

        /** Get the frustum settings of a perspective projection matrix.
          * Note, if matrix is not a perspective matrix then invalid values
          * will be returned.
        */
        bool getFrustum(double& left,   double& right,
                        double& bottom, double& top,
                        double& zNear,  double& zFar) const;

        /** Set to a symmetrical perspective projection.
          * See gluPerspective for further details.
          * Aspect ratio is defined as width/height.
        */
        void makePerspective(double fovy,  double aspectRatio,
                             double zNear, double zFar);

        /** Get the frustum settings of a symmetric perspective projection
          * matrix.
          * Return false if matrix is not a perspective matrix,
          * where parameter values are undefined. 
          * Note, if matrix is not a symmetric perspective matrix then the
          * shear will be lost.
          * Asymmetric matrices occur when stereo, power walls, caves and
          * reality center display are used.
          * In these configuration one should use the AsFrustum method instead.
        */
        bool getPerspective(double& fovy,  double& aspectRatio,
                            double& zNear, double& zFar) const;

        /** Set the position and orientation to be a view matrix,
          * using the same convention as gluLookAt.
        */
        void makeLookAt(const Vec3d& eye,const Vec3d& center,const Vec3d& up);

        /** Get to the position and orientation of a modelview matrix,
          * using the same convention as gluLookAt.
        */
        void getLookAt(Vec3f& eye,Vec3f& center,Vec3f& up,
                       value_type lookDistance=1.0f) const;

        /** Get to the position and orientation of a modelview matrix,
          * using the same convention as gluLookAt.
        */
        void getLookAt(Vec3d& eye,Vec3d& center,Vec3d& up,
                       value_type lookDistance=1.0f) const;

        /** invert the matrix rhs, automatically select invert_4x3 or invert_4x4. */
        inline bool invert( const Matrixd& rhs)
        {
            bool is_4x3 = (rhs._mat[0][3]==0.0 && rhs._mat[1][3]==0.0 &&  rhs._mat[2][3]==0.0 && rhs._mat[3][3]==1.0);
            return is_4x3 ? invert_4x3(rhs) :  invert_4x4(rhs);
        }

        /** 4x3 matrix invert, not right hand column is assumed to be 0,0,0,1. */
        bool invert_4x3( const Matrixd& rhs);

        /** full 4x4 matrix invert. */
        bool invert_4x4( const Matrixd& rhs);

        /** ortho-normalize the 3x3 rotation & scale matrix */ 
        void orthoNormalize(const Matrixd& rhs); 

        // basic utility functions to create new matrices
        inline static Matrixd identity( void );
        inline static Matrixd scale( const Vec3f& sv);
        inline static Matrixd scale( const Vec3d& sv);
        inline static Matrixd scale( value_type sx, value_type sy, value_type sz);
        inline static Matrixd translate( const Vec3f& dv);
        inline static Matrixd translate( const Vec3d& dv);
        inline static Matrixd translate( value_type x, value_type y, value_type z);
        inline static Matrixd rotate( const Vec3f& from, const Vec3f& to);
        inline static Matrixd rotate( const Vec3d& from, const Vec3d& to);
        inline static Matrixd rotate( value_type angle, value_type x, value_type y, value_type z);
        inline static Matrixd rotate( value_type angle, const Vec3f& axis);
        inline static Matrixd rotate( value_type angle, const Vec3d& axis);
        inline static Matrixd rotate( value_type angle1, const Vec3f& axis1, 
                                      value_type angle2, const Vec3f& axis2,
                                      value_type angle3, const Vec3f& axis3);
        inline static Matrixd rotate( value_type angle1, const Vec3d& axis1, 
                                      value_type angle2, const Vec3d& axis2,
                                      value_type angle3, const Vec3d& axis3);
        inline static Matrixd rotate( const Quat& quat);
        inline static Matrixd inverse( const Matrixd& matrix);
        inline static Matrixd orthoNormal(const Matrixd& matrix); 
        /** Create an orthographic projection matrix.
          * See glOrtho for further details.
        */
        inline static Matrixd ortho(double left,   double right,
                                    double bottom, double top,
                                    double zNear,  double zFar);

        /** Create a 2D orthographic projection.
          * See glOrtho for further details.
        */
        inline static Matrixd ortho2D(double left,   double right,
                                      double bottom, double top);

        /** Create a perspective projection.
          * See glFrustum for further details.
        */
        inline static Matrixd frustum(double left,   double right,
                                      double bottom, double top,
                                      double zNear,  double zFar);

        /** Create a symmetrical perspective projection.
          * See gluPerspective for further details.
          * Aspect ratio is defined as width/height.
        */
        inline static Matrixd perspective(double fovy,  double aspectRatio,
                                          double zNear, double zFar);

        /** Create the position and orientation as per a camera,
          * using the same convention as gluLookAt.
        */
        inline static Matrixd lookAt(const Vec3f& eye,
                                     const Vec3f& center,
                                     const Vec3f& up);

        /** Create the position and orientation as per a camera,
          * using the same convention as gluLookAt.
        */
        inline static Matrixd lookAt(const Vec3d& eye,
                                     const Vec3d& center,
                                     const Vec3d& up);

        inline Vec3f preMult( const Vec3f& v ) const;
        inline Vec3d preMult( const Vec3d& v ) const;
        inline Vec3f postMult( const Vec3f& v ) const;
        inline Vec3d postMult( const Vec3d& v ) const;
        inline Vec3f operator* ( const Vec3f& v ) const;
        inline Vec3d operator* ( const Vec3d& v ) const;
        inline Vec4f preMult( const Vec4f& v ) const;
        inline Vec4d preMult( const Vec4d& v ) const;
        inline Vec4f postMult( const Vec4f& v ) const;
        inline Vec4d postMult( const Vec4d& v ) const;
        inline Vec4f operator* ( const Vec4f& v ) const;
        inline Vec4d operator* ( const Vec4d& v ) const;

#ifdef USE_DEPRECATED_API
        inline void set(const Quat& q) { makeRotate(q); }
        inline void get(Quat& q) const { q = getRotate(); }
#endif

        void setRotate(const Quat& q);
        /** Get the matrix rotation as a Quat. Note that this function
          * assumes a non-scaled matrix and will return incorrect results
          * for scaled matrixces. Consider decompose() instead.
          */
        Quat getRotate() const;

        void setTrans( value_type tx, value_type ty, value_type tz );
        void setTrans( const Vec3f& v );
        void setTrans( const Vec3d& v );
        
        inline Vec3d getTrans() const { return Vec3d(_mat[3][0],_mat[3][1],_mat[3][2]); } 
        
        inline Vec3d getScale() const {
          Vec3d x_vec(_mat[0][0],_mat[1][0],_mat[2][0]); 
          Vec3d y_vec(_mat[0][1],_mat[1][1],_mat[2][1]); 
          Vec3d z_vec(_mat[0][2],_mat[1][2],_mat[2][2]); 
          return Vec3d(x_vec.length(), y_vec.length(), z_vec.length()); 
        }
        
        /** apply a 3x3 transform of v*M[0..2,0..2]. */
        inline static Vec3f transform3x3(const Vec3f& v,const Matrixd& m);

        /** apply a 3x3 transform of v*M[0..2,0..2]. */
        inline static Vec3d transform3x3(const Vec3d& v,const Matrixd& m);

        /** apply a 3x3 transform of M[0..2,0..2]*v. */
        inline static Vec3f transform3x3(const Matrixd& m,const Vec3f& v);

        /** apply a 3x3 transform of M[0..2,0..2]*v. */
        inline static Vec3d transform3x3(const Matrixd& m,const Vec3d& v);

        // basic Matrixd multiplication, our workhorse methods.
        void mult( const Matrixd&, const Matrixd& );
        void preMult( const Matrixd& );
        void postMult( const Matrixd& );

        inline void operator *= ( const Matrixd& other ) 
        {    if( this == &other ) {
                Matrixd temp(other);
                postMult( temp );
            }
            else postMult( other ); 
        }

		//James was here... added these for benchmark tests...
		inline void operator *= (const double value)
		{
			for (size_t row=0;row<4;row++)
			{
				_mat[row][0] *= value;
				_mat[row][1] *= value;
				_mat[row][2] *= value;
				_mat[row][3] *= value;
			}
		}

		inline void operator += (const Matrixd& other)
		{
			for (size_t row=0;row<4;row++)
			{
				_mat[row][0] += other(row,0);
				_mat[row][1] += other(row,1);
				_mat[row][2] += other(row,2);
				_mat[row][3] += other(row,3);
			}
		}


        inline Matrixd operator * ( const Matrixd &m ) const
        {
            osg::Matrixd r;
            r.mult(*this,m);
            return  r;
        }

    protected:
        value_type _mat[4][4];

};

inline Matrixd Matrixd::inverse( const Matrixd& matrix)
{
	Matrixd m;
	m.invert(matrix);
	return m;
}

inline Matrixd Matrixd::translate(value_type tx, value_type ty, value_type tz)
{
	Matrixd m;
	m.makeTranslate(tx,ty,tz);
	return m;
}

inline Matrixd Matrixd::translate(const Vec3f& v )
{
	return translate(v.x(), v.y(), v.z() );
}

inline Matrixd Matrixd::translate(const Vec3d& v )
{
	return translate(v.x(), v.y(), v.z() );
}

inline Vec3f Matrixd::postMult( const Vec3f& v ) const
{
    value_type d = 1.0f/(_mat[3][0]*v.x()+_mat[3][1]*v.y()+_mat[3][2]*v.z()+_mat[3][3]) ;
    return Vec3f( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3])*d,
        (_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3])*d,
        (_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3])*d) ;
}

inline Vec3d Matrixd::postMult( const Vec3d& v ) const
{
    value_type d = 1.0f/(_mat[3][0]*v.x()+_mat[3][1]*v.y()+_mat[3][2]*v.z()+_mat[3][3]) ;
    return Vec3d( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3])*d,
        (_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3])*d,
        (_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3])*d) ;
}

inline Vec3f Matrixd::preMult( const Vec3f& v ) const
{
    value_type d = 1.0f/(_mat[0][3]*v.x()+_mat[1][3]*v.y()+_mat[2][3]*v.z()+_mat[3][3]) ;
    return Vec3f( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0])*d,
        (_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1])*d,
        (_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2])*d);
}

inline Vec3d Matrixd::preMult( const Vec3d& v ) const
{
    value_type d = 1.0f/(_mat[0][3]*v.x()+_mat[1][3]*v.y()+_mat[2][3]*v.z()+_mat[3][3]) ;
    return Vec3d( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0])*d,
        (_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1])*d,
        (_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2])*d);
}

inline Vec4f Matrixd::postMult( const Vec4f& v ) const
{
    return Vec4f( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3]*v.w()),
        (_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3]*v.w()),
        (_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3]*v.w()),
        (_mat[3][0]*v.x() + _mat[3][1]*v.y() + _mat[3][2]*v.z() + _mat[3][3]*v.w())) ;
}
inline Vec4d Matrixd::postMult( const Vec4d& v ) const
{
    return Vec4d( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3]*v.w()),
        (_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3]*v.w()),
        (_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3]*v.w()),
        (_mat[3][0]*v.x() + _mat[3][1]*v.y() + _mat[3][2]*v.z() + _mat[3][3]*v.w())) ;
}

inline Vec4f Matrixd::preMult( const Vec4f& v ) const
{
    return Vec4f( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0]*v.w()),
        (_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1]*v.w()),
        (_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2]*v.w()),
        (_mat[0][3]*v.x() + _mat[1][3]*v.y() + _mat[2][3]*v.z() + _mat[3][3]*v.w()));
}

inline Vec4d Matrixd::preMult( const Vec4d& v ) const
{
    return Vec4d( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0]*v.w()),
        (_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1]*v.w()),
        (_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2]*v.w()),
        (_mat[0][3]*v.x() + _mat[1][3]*v.y() + _mat[2][3]*v.z() + _mat[3][3]*v.w()));
}

inline Vec3f Matrixd::transform3x3(const Vec3f& v,const Matrixd& m)
{
    return Vec3f( (m._mat[0][0]*v.x() + m._mat[1][0]*v.y() + m._mat[2][0]*v.z()),
                 (m._mat[0][1]*v.x() + m._mat[1][1]*v.y() + m._mat[2][1]*v.z()),
                 (m._mat[0][2]*v.x() + m._mat[1][2]*v.y() + m._mat[2][2]*v.z()));
}
inline Vec3d Matrixd::transform3x3(const Vec3d& v,const Matrixd& m)
{
    return Vec3d( (m._mat[0][0]*v.x() + m._mat[1][0]*v.y() + m._mat[2][0]*v.z()),
                 (m._mat[0][1]*v.x() + m._mat[1][1]*v.y() + m._mat[2][1]*v.z()),
                 (m._mat[0][2]*v.x() + m._mat[1][2]*v.y() + m._mat[2][2]*v.z()));
}

inline Vec3f Matrixd::transform3x3(const Matrixd& m,const Vec3f& v)
{
    return Vec3f( (m._mat[0][0]*v.x() + m._mat[0][1]*v.y() + m._mat[0][2]*v.z()),
                 (m._mat[1][0]*v.x() + m._mat[1][1]*v.y() + m._mat[1][2]*v.z()),
                 (m._mat[2][0]*v.x() + m._mat[2][1]*v.y() + m._mat[2][2]*v.z()) ) ;
}
inline Vec3d Matrixd::transform3x3(const Matrixd& m,const Vec3d& v)
{
    return Vec3d( (m._mat[0][0]*v.x() + m._mat[0][1]*v.y() + m._mat[0][2]*v.z()),
                 (m._mat[1][0]*v.x() + m._mat[1][1]*v.y() + m._mat[1][2]*v.z()),
                 (m._mat[2][0]*v.x() + m._mat[2][1]*v.y() + m._mat[2][2]*v.z()) ) ;
}



} //namespace osg