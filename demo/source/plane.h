#ifndef PLANE_H
#define PLANE_H

#include "vector3.h"
#include "fixed.h"
#include "ray3.h"

class Plane
{
public:
	Plane() {}
	Plane(const Vector3 &n, const fixed16 d) : n(n), d(d) {}
	
	Vector3 n;
	fixed16 d;
};

inline fixed16 intersect_ray_plane(const Ray3 &ray, const Plane &plane)
{
	fixed16 dot1 = dot(ray.o, plane.n);
	fixed16 dot2 = dot(ray.d, plane.n);
	
	if (dot2 > fixed16(0)) 	return -((dot1 - plane.d) / MAX(-dot2, fixed16::make_raw(-1 << 16)));
	else                    return  ((dot1 + plane.d) / MIN( dot2, fixed16::make_raw(+1 << 16)));

/*	if (dot2 > fixed16(0)) return (dot1 - plane.d) / MAX(dot2, fixed16::make_raw(-1 << 16));
	else                   return (dot1 + plane.d) / MIN(dot2, fixed16::make_raw(+1 << 16));
*/
}


#endif /* PLANE_H */
