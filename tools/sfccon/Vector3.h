#pragma once
#include <math.h>


struct Vector3 {
	Vector3(){}
	Vector3(double x, double y, double z) :x(x), y(y), z(z) {}
	double x, y, z;
	double magnitude_pow() const { return x*x+y*y+z*z; }
	double magnitude() const { return sqrt(magnitude_pow()); }
	Vector3 normalize() const {
		double mag_inv = 1.0/magnitude();
		return Vector3(x*mag_inv, y*mag_inv, z*mag_inv);
	}
	Vector3 operator*(double op) const { return Vector3(x*op, y*op, z*op); }
	Vector3 operator-(const Vector3& op) const { return Vector3(x-op.x, y-op.y, z-op.z); }
	Vector3 operator+(const Vector3& op) const { return Vector3(x+op.x, y+op.y, z+op.z); }
	void operator *=(double op) {
		*this = *this * op;
	}
	double dot(const Vector3& op) const {
		return x*op.x+y*op.y+z*op.z;
	}
};
