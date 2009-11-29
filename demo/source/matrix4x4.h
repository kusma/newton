#ifndef _MATRIX4X4_H_
#define _MATRIX4X4_H_

#include <algorithm>
#include "vector3.h"
#include "vector4.h"

class Matrix4x4
{
public:
	INLINE Matrix4x4() { }
	
	void identity();
	void rotate(const Vector3 &v);
	void translate(const Vector3 &v);
	void scale(const Vector3 &v);
	void project(fixed16 fovy, fixed16 aspect, fixed16 zn, fixed16 zf);
	void frustum(fixed16 left, fixed16 right, fixed16 bottom, fixed16 top, fixed16 near, fixed16 far);
	
	void look_at(const Vector3 &position, const Vector3 &target, fixed16 roll);
	
	INLINE Vector3 get_translation() const
	{
		return Vector3(m[3][0], m[3][1], m[3][2]);
	}
	
	void transpose();
	void inverse();
	void swap_rows(const unsigned row1, const unsigned row2);
	
	INLINE fixed16 operator()(int i, int j) const
	{
		ASSERT(i >= 0);
		ASSERT(i <  4);
		ASSERT(j >= 0);
		ASSERT(j <  4);
		return m[j][i];
	}
	
	INLINE fixed16& operator()(int i, int j)
	{
		ASSERT(i >= 0);
		ASSERT(i <  4);
		ASSERT(j >= 0);
		ASSERT(j <  4);
		return m[j][i];
	}
	
	INLINE Vector3 rotate_vector(const Vector3 &v) const
	{
		return Vector3(
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2]
		);
	}
	
	INLINE Vector3 transform_vector(const Vector3 &v) const
	{
		return Vector3(
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2]
		);
	}
	
	Matrix4x4  operator  *(const Matrix4x4 &m) const;
	Matrix4x4 &operator *=(const Matrix4x4 &m);
	
private:
	fixed16 m[4][4];
};

/* transforms */

INLINE Vector4 operator*(const Matrix4x4 &mat, const Vector3 &v)
{
	return Vector4(
		v.x * mat(0,0) + v.y * mat(0,1) + v.z * mat(0,2) + mat(0,3),
		v.x * mat(1,0) + v.y * mat(1,1) + v.z * mat(1,2) + mat(1,3),
		v.x * mat(2,0) + v.y * mat(2,1) + v.z * mat(2,2) + mat(2,3),
		v.x * mat(3,0) + v.y * mat(3,1) + v.z * mat(3,2) + mat(3,3)
	);
}

INLINE Vector4 operator*(const Matrix4x4 &mat, const Vector4 &v)
{
	return Vector4(
		v.x * mat(0,0) + v.y * mat(0,1) + v.z * mat(0,2) + v.w * mat(0,3),
		v.x * mat(1,0) + v.y * mat(1,1) + v.z * mat(1,2) + v.w * mat(1,3),
		v.x * mat(2,0) + v.y * mat(2,1) + v.z * mat(2,2) + v.w * mat(2,3),
		v.x * mat(3,0) + v.y * mat(3,1) + v.z * mat(3,2) + v.w * mat(3,3)
	);
}

/* transpose transforms */

INLINE Vector4 operator*(const Vector3 &v, const Matrix4x4 &mat)
{
	return Vector4(
		v.x * mat(0,0) + v.y * mat(1,0) + v.z * mat(2,0) + mat(3,0),
		v.x * mat(0,1) + v.y * mat(1,1) + v.z * mat(2,1) + mat(3,1),
		v.x * mat(0,2) + v.y * mat(1,2) + v.z * mat(2,2) + mat(3,2),
		v.x * mat(0,3) + v.y * mat(1,3) + v.z * mat(2,3) + mat(3,3)
	);
}

INLINE Vector4 operator*(const Vector4 &v, const Matrix4x4 &mat)
{
	return Vector4(
		v.x * mat(0,0) + v.y * mat(1,0) + v.z * mat(2,0) + v.w * mat(3,0),
		v.x * mat(0,1) + v.y * mat(1,1) + v.z * mat(2,1) + v.w * mat(3,1),
		v.x * mat(0,2) + v.y * mat(1,2) + v.z * mat(2,2) + v.w * mat(3,2),
		v.x * mat(0,3) + v.y * mat(1,3) + v.z * mat(2,3) + v.w * mat(3,3)
	);
}


#endif /* _MATRIX4X4_H_ */
