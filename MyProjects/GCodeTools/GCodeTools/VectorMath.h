#pragma once

class Vec3d
{
public:

	/** Type of Vec class.*/
	typedef double value_type;

	/** Number of vector components. */
	enum { num_components = 3 };

	value_type _v[3];

	Vec3d() { _v[0] = 0.0; _v[1] = 0.0; _v[2] = 0.0; }

	//inline Vec3d(const Vec3f& vec) { _v[0] = vec._v[0]; _v[1] = vec._v[1]; _v[2] = vec._v[2]; }

	//inline operator Vec3f() const { return Vec3f(static_cast<float>(_v[0]), static_cast<float>(_v[1]), static_cast<float>(_v[2])); }

	Vec3d(value_type x, value_type y, value_type z) { _v[0] = x; _v[1] = y; _v[2] = z; }
	//Vec3d(const Vec2d& v2, value_type zz)
	//{
	//	_v[0] = v2[0];
	//	_v[1] = v2[1];
	//	_v[2] = zz;
	//}

	inline bool operator == (const Vec3d& v) const { return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2]; }

	inline bool operator != (const Vec3d& v) const { return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2]; }

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

	inline void set(value_type x, value_type y, value_type z)
	{
		_v[0] = x; _v[1] = y; _v[2] = z;
	}

	inline void set(const Vec3d& rhs)
	{
		_v[0] = rhs._v[0]; _v[1] = rhs._v[1]; _v[2] = rhs._v[2];
	}

	inline value_type& operator [] (int i) { return _v[i]; }
	inline value_type operator [] (int i) const { return _v[i]; }

	inline value_type& x() { return _v[0]; }
	inline value_type& y() { return _v[1]; }
	inline value_type& z() { return _v[2]; }

	inline value_type x() const { return _v[0]; }
	inline value_type y() const { return _v[1]; }
	inline value_type z() const { return _v[2]; }

	//inline bool valid() const { return !isNaN(); }
	//inline bool isNaN() const { return osg::isNaN(_v[0]) || osg::isNaN(_v[1]) || osg::isNaN(_v[2]); }

	/** Dot product. */
	inline value_type operator * (const Vec3d& rhs) const
	{
		return _v[0] * rhs._v[0] + _v[1] * rhs._v[1] + _v[2] * rhs._v[2];
	}

	/** Cross product. */
	inline const Vec3d operator ^ (const Vec3d& rhs) const
	{
		return Vec3d(_v[1] * rhs._v[2] - _v[2] * rhs._v[1],
			_v[2] * rhs._v[0] - _v[0] * rhs._v[2],
			_v[0] * rhs._v[1] - _v[1] * rhs._v[0]);
	}

	/** Multiply by scalar. */
	inline const Vec3d operator * (value_type rhs) const
	{
		return Vec3d(_v[0] * rhs, _v[1] * rhs, _v[2] * rhs);
	}

	/** Unary multiply by scalar. */
	inline Vec3d& operator *= (value_type rhs)
	{
		_v[0] *= rhs;
		_v[1] *= rhs;
		_v[2] *= rhs;
		return *this;
	}

	/** Divide by scalar. */
	inline const Vec3d operator / (value_type rhs) const
	{
		return Vec3d(_v[0] / rhs, _v[1] / rhs, _v[2] / rhs);
	}

	/** Unary divide by scalar. */
	inline Vec3d& operator /= (value_type rhs)
	{
		_v[0] /= rhs;
		_v[1] /= rhs;
		_v[2] /= rhs;
		return *this;
	}

	/** Binary vector add. */
	inline const Vec3d operator + (const Vec3d& rhs) const
	{
		return Vec3d(_v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2]);
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
		return Vec3d(_v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2]);
	}

	/** Unary vector subtract. */
	inline Vec3d& operator -= (const Vec3d& rhs)
	{
		_v[0] -= rhs._v[0];
		_v[1] -= rhs._v[1];
		_v[2] -= rhs._v[2];
		return *this;
	}

	/** Negation operator. Returns the negative of the Vec3d. */
	inline const Vec3d operator - () const
	{
		return Vec3d(-_v[0], -_v[1], -_v[2]);
	}

	/** Length of the vector = sqrt( vec . vec ) */
	inline value_type length() const
	{
		return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]);
	}

	/** Length squared of the vector = vec . vec */
	inline value_type length2() const
	{
		return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
	}

	/** Normalize the vector so that it has length unity.
	* Returns the previous length of the vector.
	*/
	inline value_type normalize()
	{
		value_type norm = Vec3d::length();
		if (norm>0.0)
		{
			value_type inv = 1.0 / norm;
			_v[0] *= inv;
			_v[1] *= inv;
			_v[2] *= inv;
		}
		return(norm);
	}

};    // end of class Vec3d



