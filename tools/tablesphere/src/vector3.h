#ifndef VECTOR3_H
#define VECTOR3_H

#include "math.h"
#include "fixed.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// #define USE_FLOAT
#define FIXED_TO_FLOAT(x) (float(x.get_val()) * (1.f / (1 << 16)))
#define FLOAT_TO_FIXED(f) (fixed16::make_raw(int(f * (1 << 16))))

class Vector3 {
public:

	INLINE Vector3() {}
	INLINE Vector3(const Vector3 &v) : x(v.x), y(v.y), z(v.z) {}

	INLINE Vector3(const fixed16 x, const fixed16 y, const fixed16 z) : x(x), y(y), z(z) {}

	INLINE Vector3 operator+(const Vector3 &v) const
	{
		return Vector3(x + v.x,  y + v.y, z + v.z);
	}

	INLINE Vector3 &operator+=(const Vector3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	INLINE Vector3 operator-(const Vector3 &v) const
	{
		return Vector3(x - v.x,  y - v.y, z - v.z);
	}

	INLINE Vector3 &operator-=(const Vector3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	
	INLINE Vector3 operator*(const Vector3 &v) const
	{
#ifdef USE_FLOAT
		return Vector3(
			FLOAT_TO_FIXED(FIXED_TO_FLOAT(x) * FIXED_TO_FLOAT(v.x)),
			FLOAT_TO_FIXED(FIXED_TO_FLOAT(y) * FIXED_TO_FLOAT(v.y)),
			FLOAT_TO_FIXED(FIXED_TO_FLOAT(z) * FIXED_TO_FLOAT(v.z))
		);
#else
		return Vector3(x * v.x, y * v.y, z * v.z);
#endif
	}


	INLINE Vector3 operator*(const fixed16 f) const
	{
#ifdef USE_FLOAT
		return Vector3(
			FLOAT_TO_FIXED(FIXED_TO_FLOAT(x) * FIXED_TO_FLOAT(f)),
			FLOAT_TO_FIXED(FIXED_TO_FLOAT(y) * FIXED_TO_FLOAT(f)),
			FLOAT_TO_FIXED(FIXED_TO_FLOAT(z) * FIXED_TO_FLOAT(f))
		);
#else
		return Vector3(x * f, y * f, z * f);
#endif
	}

	INLINE Vector3 &operator*=(const fixed16 f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	INLINE Vector3 &operator-()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}
	
	INLINE Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}


	INLINE fixed16 mag() const
	{
		return x * x + y * y + z * z;
	}

	INLINE fixed16 magnitude() const {
		return mag().rsq().rcp();
	}

	INLINE Vector3 cross(const Vector3 &v) const
	{
//		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
		return Vector3(z * v.y - y * v.z, x * v.z - z * v.x, y * v.x - x * v.y);
	}

	INLINE fixed16 dot(const Vector3 &v) const
	{
#ifdef USE_FLOAT
		float f = FIXED_TO_FLOAT(x) * FIXED_TO_FLOAT(v.x) + FIXED_TO_FLOAT(y) * FIXED_TO_FLOAT(v.y) + FIXED_TO_FLOAT(z) * FIXED_TO_FLOAT(v.z);
		return FLOAT_TO_FIXED(f);
#else
		return x * v.x + y * v.y + z * v.z;
#endif
	}

	INLINE u64 mag_long()
	{
		u64 mag = (long long)x.get_val() * x.get_val();
		mag += (long long)y.get_val() * y.get_val();
		mag += (long long)z.get_val() * z.get_val();
		return mag;
	}

	INLINE Vector3 normalize()
	{
#if 1
		/* GAAAH! */
		float fx = FIXED_TO_FLOAT(x);
		float fy = FIXED_TO_FLOAT(y);
		float fz = FIXED_TO_FLOAT(z);

		{
			float mag = fx * fx + fy * fy + fz * fz;
			float scale = 1.f / float(sqrt(mag));
			fx *= scale;
			fy *= scale;
			fz *= scale;
//			printf("%d - %f %f %f\n", i, fx, fy, fz);
		}

		x = FLOAT_TO_FIXED(fx);
		y = FLOAT_TO_FIXED(fy);
		z = FLOAT_TO_FIXED(fz);
		return *this;

#else
//		while ((abs(x.get_val()) | abs(y.get_val()) | abs(z.get_val())) > (1 << 16))
		while (
			(abs(x) > fixed16(0.5)) ||
			(abs(y) > fixed16(0.5)) ||
			(abs(z) > fixed16(0.5))
			)
		{
			x >>= 8;
			y >>= 8;
			z >>= 8;
		}

		*this *= rsq(mag());
		return *this;
#endif
	}
/*
	INLINE Vector3 normalize() const
	{
		return Vector3(*this).normalize();
	}
*/

	fixed16 x, y, z;
};

INLINE fixed16 dot(const Vector3 &v1, const Vector3 &v2)
{
	return v1.dot(v2);
//	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

#endif /* VECTOR3_H */
