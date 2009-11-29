#include "matrix4x4.h"

void Matrix4x4::identity()
{
	m[0][0] = fixed16(1); m[0][1] = fixed16(0); m[0][2] = fixed16(0); m[0][3] = fixed16(0);
	m[1][0] = fixed16(0); m[1][1] = fixed16(1); m[1][2] = fixed16(0); m[1][3] = fixed16(0);
	m[2][0] = fixed16(0); m[2][1] = fixed16(0); m[2][2] = fixed16(1); m[2][3] = fixed16(0);
	m[3][0] = fixed16(0); m[3][1] = fixed16(0); m[3][2] = fixed16(0); m[3][3] = fixed16(1);
}

void Matrix4x4::rotate(const Vector3 &v)
{
	fixed16 cx = cos(v.x);
	fixed16 cy = cos(v.y);
	fixed16 cz = cos(v.z);
	fixed16 sx = sin(v.x);
	fixed16 sy = sin(v.y);
	fixed16 sz = sin(v.z);

	m[0][0] = cy*cz;          m[0][1] = cy*sz;          m[0][2] = -sy;        m[0][3] = fixed16(0);
	m[1][0] = sx*sy*cz-cx*sz; m[1][1] = sx*sy*sz+cx*cz; m[1][2] = sx*cy;      m[1][3] = fixed16(0);
	m[2][0] = cx*sy*cz+sx*sz; m[2][1] = cx*sy*sz-sx*cz; m[2][2] = cx*cy;      m[2][3] = fixed16(0);
	m[3][0] = fixed16(0);     m[3][1] = fixed16(0);     m[3][2] = fixed16(0); m[3][3] = fixed16(1);
}

void Matrix4x4::translate(const Vector3 &v)
{
	m[0][0] = fixed16(1);   m[0][1] = fixed16(0);   m[0][2] = fixed16(0);  m[0][3] = fixed16(0);
	m[1][0] = fixed16(0);   m[1][1] = fixed16(1);   m[1][2] = fixed16(0);  m[1][3] = fixed16(0);
	m[2][0] = fixed16(0);   m[2][1] = fixed16(0);   m[2][2] = fixed16(1);  m[2][3] = fixed16(0);
	m[3][0] = fixed16(v.x); m[3][1] = fixed16(v.y); m[3][2] = v.z;         m[3][3] = fixed16(1);
}

void Matrix4x4::scale(const Vector3 &v)
{
	m[0][0] = v.x;        m[0][1] = fixed16(0); m[0][2] = fixed16(0); m[0][3] = fixed16(0);
	m[1][0] = fixed16(0); m[1][1] = v.y;        m[1][2] = fixed16(0); m[1][3] = fixed16(0);
	m[2][0] = fixed16(0); m[2][1] = fixed16(0); m[2][2] = v.z;        m[2][3] = fixed16(0);
	m[3][0] = fixed16(0); m[3][1] = fixed16(0); m[3][2] = fixed16(0); m[3][3] = fixed16(1);
}

#if 0
void Matrix4x4::frustum(const fixed16 left, const fixed16 right, const fixed16 bottom, const fixed16 top, const fixed16 near, const fixed16 far)
{
	float tmp[4][4];
	__mali_float_matrix4x4_make_frustum(tmp, left, right, bottom, top, near, far);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			m[i][j] = tmp[i][j];
		}
	}
}
#else
void Matrix4x4::frustum(const fixed16 left, const fixed16 right, const fixed16 bottom, const fixed16 top, const fixed16 znear, const fixed16 zfar)
{
	fixed16 wr = rcp(right - left); /* reciprocal width */
	fixed16 hr = rcp(top - bottom); /* reciprocal height */
	fixed16 lr = rcp(zfar - znear);   /* reciprocal length */
	
	m[0][0] = 2 * znear * wr;      m[0][1] = fixed16(0);          m[0][2] = fixed16(0);               m[0][3] =  fixed16(0);
	m[1][0] = fixed16(0);          m[1][1] = 2 * znear * hr;      m[1][2] = fixed16(0);               m[1][3] =  fixed16(0);
	m[2][0] = (right + left) * wr; m[2][1] = (top + bottom) * hr; m[2][2] = -(zfar + znear) * lr;     m[2][3] = -fixed16(1);
	m[3][0] = fixed16(0);          m[3][1] = fixed16(0);          m[3][2] = -(2 * zfar * znear) * lr; m[3][3] =  fixed16(0);
}
#endif


#if 1
void Matrix4x4::project(const fixed16 fovy, const fixed16 aspect_, const fixed16 zn, const fixed16 zf)
{
	fixed16 aspect = aspect_;
	
	/* cotan(x) = 1.0 / tan(x) */
	/* atan(x) = i/2 * log((i + z) / (i - z)) */
	
	/* tan(x) = sin(x) / cos(x) */
	fixed16 f = tan(fovy * fixed16(0.5 / 360.0));
	if (aspect == fixed16(0)) aspect = fixed16(1);

	m[0][0] = f / aspect; m[0][1] = fixed16(0); m[0][2] = fixed16(0);                m[0][3] =  fixed16(0);
	m[1][0] = fixed16(0); m[1][1] = f;          m[1][2] = fixed16(0);                m[1][3] =  fixed16(0);
	m[2][0] = fixed16(0); m[2][1] = fixed16(0); m[2][2] = (zf + zn) / (zn - zf);     m[2][3] = -fixed16(1);
	m[3][0] = fixed16(0); m[3][1] = fixed16(0); m[3][2] = (2 * zf * zn) / (zn - zf); m[3][3] =  fixed16(0);
}
#else
void Matrix4x4::project(const fixed16 fovy, const fixed16 aspect, const fixed16 znear, const fixed16 zfar)
{
	fixed16 xmin, xmax, ymin, ymax;
	
	/* tan(x) = sin(x) / cos(x) */
	ymax = znear * tan(fovy * fixed16(0.5 / 360.0));
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	
	frustum(xmin, xmax, ymin, ymax, znear, zfar);
}
#endif

void Matrix4x4::transpose()
{
	for (int i=0; i<4; i++) {
		std::swap(m[i][0], m[0][i]);
		std::swap(m[i][1], m[1][i]);
		std::swap(m[i][2], m[2][i]);
		std::swap(m[i][3], m[3][i]);
	}
}

static inline void matrix4x4_multiply(Matrix4x4 &dst, const Matrix4x4 &m0, const Matrix4x4 &m1)
{
	/*
		a     * b     = c
		b ^ T * a ^ T = c ^ T
	*/
	for (int i = 0; i < 4; ++i)
	{
		fixed16 d0 = m1(i,0);
		fixed16 d1 = m1(i,1);
		fixed16 d2 = m1(i,2);
		fixed16 d3 = m1(i,3);
		dst(i,0) = d0 * m0(0,0) + d1 * m0(1,0) + d2 * m0(2,0) + d3 * m0(3,0);
		dst(i,1) = d0 * m0(0,1) + d1 * m0(1,1) + d2 * m0(2,1) + d3 * m0(3,1);
		dst(i,2) = d0 * m0(0,2) + d1 * m0(1,2) + d2 * m0(2,2) + d3 * m0(3,2);
		dst(i,3) = d0 * m0(0,3) + d1 * m0(1,3) + d2 * m0(2,3) + d3 * m0(3,3);
	}
}

Matrix4x4 Matrix4x4::operator *(const Matrix4x4 &m) const
{
	Matrix4x4 result;
	matrix4x4_multiply(result, *this, m);
	return result;
}

Matrix4x4 &Matrix4x4::operator *=(const Matrix4x4 &m)
{
	matrix4x4_multiply(*this, *this, m);
	return *this;
}
