#ifndef SPHERE_H
#define SPHERE_H

#include "vector3.h"
#include "fixed.h"
#include "ray3.h"

class Sphere
{
public:
	Sphere(const Vector3 &o, const fixed16 r) : o(o), rr(r * r) {}
	
	Vector3 o;
	fixed16 rr;
};

inline fixed16 intersect_ray_sphere(const Ray3 &ray, const Sphere &sphere)
{
	Vector3 dst = ray.o - sphere.o;
//	Vector3 dst = sphere.o - ray.o;
	fixed16 a = dot(dst, dst);
	fixed16 b = dot(dst, ray.d);
	fixed16 c = a - sphere.rr;
	fixed16 d = b * b - c;

	if (d < fixed16(0)) return fixed16(0);
//	if (d < fixed16(0)) return -b - sqrt(-d);
	return -b - sqrt(d);
}

#endif /* SPHERE_H */
