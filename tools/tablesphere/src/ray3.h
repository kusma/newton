#ifndef RAY3_H
#define RAY3_H

#include "vector3.h"

class Ray3
{
public:
	Ray3(const Vector3 &o, const Vector3 &d) : o(o), d(d) {}

	Vector3 o;
	Vector3 d;
};

#endif /* RAY3_H */
