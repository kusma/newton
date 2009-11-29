#include "matrix.h"
#include <algorithm>

void Matrix::rotate(const Vector3 &v)
{
	fixed16 cx = cos(v.x);
	fixed16 cy = cos(v.y);
	fixed16 cz = cos(v.z);
	fixed16 sx = sin(v.x);
	fixed16 sy = sin(v.y);
	fixed16 sz = sin(v.z);

	matrix[0][0] = cy*cz;			matrix[0][1] = cy*sz;			matrix[0][2] = -sy;		matrix[0][3] = 0;
	matrix[1][0] = sx*sy*cz-cx*sz;	matrix[1][1] = sx*sy*sz+cx*cz;	matrix[1][2] = sx*cy;	matrix[1][3] = 0;
	matrix[2][0] = cx*sy*cz+sx*sz;	matrix[2][1] = cx*sy*sz-sx*cz;	matrix[2][2] = cx*cy;	matrix[2][3] = 0;
	matrix[3][0] = 0;				matrix[3][1] = 0;				matrix[3][2] = 0;		matrix[3][3] = 1;
}

void Matrix::translate(const Vector3 &v)
{
	matrix[0][0] = 1;	matrix[0][1] = 0;	matrix[0][2] = 0;	matrix[0][3] = 0;
	matrix[1][0] = 0;	matrix[1][1] = 1;	matrix[1][2] = 0;	matrix[1][3] = 0;
	matrix[2][0] = 0;	matrix[2][1] = 0;	matrix[2][2] = 1;	matrix[2][3] = 0;
	matrix[3][0] = v.x;	matrix[3][1] = v.y;	matrix[3][2] = v.z;	matrix[3][3] = 1;
}

void Matrix::scale(const Vector3 &v)
{
	matrix[0][0] = v.x;	matrix[0][1] = 0;	matrix[0][2] = 0;	matrix[0][3] = 0;
	matrix[1][0] = 0;	matrix[1][1] = v.y;	matrix[1][2] = 0;	matrix[1][3] = 0;
	matrix[2][0] = 0;	matrix[2][1] = 0;	matrix[2][2] = v.z;	matrix[2][3] = 0;
	matrix[3][0] = 0;	matrix[3][1] = 0;	matrix[3][2] = 0;	matrix[3][3] = 1;

}

#if 0
#include <math.h>
#include <float.h>
#ifndef M_PI
#define M_PI 3.141592
#endif

void Matrix::project(const fixed16 fovy, const fixed16 aspect_, const fixed16 zn, const fixed16 zf)
{
	fixed16 aspect = aspect_;
	fixed16 f = rcp(fixed16(tan( float(fovy) / 360.f * M_PI )));
	if (aspect == fixed16(0)) aspect=1;

	matrix[0][0] = f/aspect; matrix[0][1] = 0; matrix[0][2] = 0;                           matrix[0][3] = 0;
	matrix[1][0] = 0;        matrix[1][1] = f; matrix[1][2] = 0;                           matrix[1][3] = 0;
	matrix[2][0] = 0;        matrix[2][1] = 0; matrix[2][2] = (zf + zn) / (zn - zf);       matrix[2][3] = -1;
	matrix[3][0] = 0;        matrix[3][1] = 0; matrix[3][2] = (2 * zf * zn) / (zn - zf); matrix[3][3] = 0;
}
#endif

void Matrix::transpose()
{
	for (int i=0; i<4; i++) {
		std::swap(matrix[i][0], matrix[0][i]);
		std::swap(matrix[i][1], matrix[1][i]);
		std::swap(matrix[i][2], matrix[2][i]);
		std::swap(matrix[i][3], matrix[3][i]);
	}
}

#if 1

Matrix &Matrix::operator*=(const Matrix &m)
{
	for (unsigned i = 0; i < 4; ++i)
	{
		fixed16 d0 = matrix[i][0], d1 = matrix[i][1], d2 = matrix[i][2], d3 = matrix[i][3];
		matrix[i][0] = d0 * m.matrix[0][0] + d1 * m.matrix[1][0] + d2 * m.matrix[2][0] + d3 * m.matrix[3][0];
		matrix[i][1] = d0 * m.matrix[0][1] + d1 * m.matrix[1][1] + d2 * m.matrix[2][1] + d3 * m.matrix[3][1];
		matrix[i][2] = d0 * m.matrix[0][2] + d1 * m.matrix[1][2] + d2 * m.matrix[2][2] + d3 * m.matrix[3][2];
		matrix[i][3] = d0 * m.matrix[0][3] + d1 * m.matrix[1][3] + d2 * m.matrix[2][3] + d3 * m.matrix[3][3];
	}
	return *this;
}

Matrix Matrix::operator *(const Matrix &m) const
{
	Matrix temp;
	for (unsigned i = 0; i < 4; ++i)
	{
		fixed16 d0 = matrix[i][0], d1 = matrix[i][1], d2 = matrix[i][2], d3 = matrix[i][3];
		temp.matrix[i][0] = d0 * m.matrix[0][0] + d1 * m.matrix[1][0] + d2 * m.matrix[2][0] + d3 * m.matrix[3][0];
		temp.matrix[i][1] = d0 * m.matrix[0][1] + d1 * m.matrix[1][1] + d2 * m.matrix[2][1] + d3 * m.matrix[3][1];
		temp.matrix[i][2] = d0 * m.matrix[0][2] + d1 * m.matrix[1][2] + d2 * m.matrix[2][2] + d3 * m.matrix[3][2];
		temp.matrix[i][3] = d0 * m.matrix[0][3] + d1 * m.matrix[1][3] + d2 * m.matrix[2][3] + d3 * m.matrix[3][3];
	}
	return temp;
}

#else
static inline void matrix4x4_multiply(Matrix &dst, const Matrix &m0, const Matrix &m1)
{
	/*
		a     * b     = c
		b ^ T * a ^ T = c ^ T
	*/

	unsigned i;
	for (i = 0; i < 4; ++i)
	{
		fixed16 d0 = m0.matrix[0][i];
		fixed16 d1 = m0.matrix[1][i];
		fixed16 d2 = m0.matrix[2][i];
		fixed16 d3 = m0.matrix[3][i];
		dst.matrix[0][i] = d0 * m1.matrix[0][0] + d1 * m1.matrix[0][1] + d2 * m1.matrix[0][2] + d3 * m1.matrix[0][3];
		dst.matrix[1][i] = d0 * m1.matrix[1][0] + d1 * m1.matrix[1][1] + d2 * m1.matrix[1][2] + d3 * m1.matrix[1][3];
		dst.matrix[2][i] = d0 * m1.matrix[2][0] + d1 * m1.matrix[2][1] + d2 * m1.matrix[2][2] + d3 * m1.matrix[2][3];
		dst.matrix[3][i] = d0 * m1.matrix[3][0] + d1 * m1.matrix[3][1] + d2 * m1.matrix[3][2] + d3 * m1.matrix[3][3];
	}
}

Matrix Matrix::operator *(const Matrix &m) const
{
	Matrix result;
	matrix4x4_multiply(result, *this, m);
	return result;
}

Matrix &Matrix::operator *=(const Matrix &m)
{
	matrix4x4_multiply(*this, *this, m);
	return *this;
}
#endif
