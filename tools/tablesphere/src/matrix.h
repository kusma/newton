#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "vector3.h"
#include "vector4.h"

class Matrix
{
public:
	INLINE Matrix() { }

	void identity();
	void rotate(const Vector3 &v);
	void translate(const Vector3 &v);
	void scale(const Vector3 &v);
	void project(fixed16 fovy, fixed16 aspect, fixed16 zn, fixed16 zf);
	void look_at(const Vector3 &position, const Vector3 &target, fixed16 roll);

	INLINE Vector3 get_translation() const
	{
		return Vector3(matrix[3][0], matrix[3][1], matrix[3][2]);
	}

	void transpose();
	void inverse();
	void swap_rows(const unsigned row1, const unsigned row2);
	
	INLINE Vector4 operator*(const Vector3 &v) const
	{
		return Vector4(
			v.x * matrix[0][0] + v.y * matrix[1][0] + v.z * matrix[2][0] + matrix[3][0],
			v.x * matrix[0][1] + v.y * matrix[1][1] + v.z * matrix[2][1] + matrix[3][1],
			v.x * matrix[0][2] + v.y * matrix[1][2] + v.z * matrix[2][2] + matrix[3][2],
			v.x * matrix[0][3] + v.y * matrix[1][3] + v.z * matrix[2][3] + matrix[3][3]
		);
	}

	INLINE Vector4 operator*(const Vector4 &v) const
	{
		return Vector4(
			v.x * matrix[0][0] + v.y * matrix[1][0] + v.z * matrix[2][0] + v.w * matrix[3][0],
			v.x * matrix[0][1] + v.y * matrix[1][1] + v.z * matrix[2][1] + v.w * matrix[3][1],
			v.x * matrix[0][2] + v.y * matrix[1][2] + v.z * matrix[2][2] + v.w * matrix[3][2],
			v.x * matrix[0][3] + v.y * matrix[1][3] + v.z * matrix[2][3] + v.w * matrix[3][3]
		);
	}

	INLINE Vector3 rotate_vector(const Vector3 &v) const
	{
		return Vector3(
			v.x * matrix[0][0] + v.y * matrix[1][0] + v.z * matrix[2][0],
			v.x * matrix[0][1] + v.y * matrix[1][1] + v.z * matrix[2][1],
			v.x * matrix[0][2] + v.y * matrix[1][2] + v.z * matrix[2][2]
		);
	}

	INLINE Vector3 transform_vector(const Vector3 &v) const
	{
		return Vector3(
			v.x * matrix[0][0] + v.y * matrix[1][0] + v.z * matrix[2][0] + matrix[3][0],
			v.x * matrix[0][1] + v.y * matrix[1][1] + v.z * matrix[2][1] + matrix[3][1],
			v.x * matrix[0][2] + v.y * matrix[1][2] + v.z * matrix[2][2] + matrix[3][2]
		);
	}

	Matrix  operator*(const Matrix &m) const;
	Matrix &operator *=(const Matrix &m);

	fixed16 matrix[4][4];
};

#endif /* _MATRIX_H_ */
