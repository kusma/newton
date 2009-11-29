#ifndef VECTOR4_H
#define VECTOR4_H

#include "math.h"
#include "fixed.h"
#include "vector3.h"

class Vector4 {
public:

	INLINE Vector4() {}
	INLINE Vector4(const fixed16 x, const fixed16 y, const fixed16 z, const fixed16 w) : x(x), y(y), z(z), w(w) {}
	INLINE Vector4(const Vector3 v, const fixed16 w) : x(v.x), y(v.y), z(v.z), w(w) {}

	INLINE Vector4 operator+(const Vector4 &v) const
	{
		return Vector4(x + v.x,  y + v.y, z + v.z, w + v.w);
	}

	INLINE Vector4 &operator+=(const Vector4 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	INLINE Vector4 operator-(const Vector4 &v) const
	{
		return Vector4(x - v.x,  y - v.y, z - v.z, w - v.w);
	}

	INLINE Vector4 &operator-=(const Vector4 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	INLINE Vector4 operator*(const fixed16 f) const
	{
		return Vector4(x * f, y * f, z * f, w * f);
	}

	INLINE Vector4 &operator*=(const fixed16 f)
	{
		x *= f;
		y *= f;
		z *= f;
		w *= f;
		return *this;
	}

	INLINE Vector4 &operator-() {
		x = -x;
		y = -y;
		z = -z;
		w = -w;
		return *this;
	}

	fixed16 x, y, z, w;
};

#endif /* VECTOR4_H */
