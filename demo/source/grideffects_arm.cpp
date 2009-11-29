#include <math.h>
#include <float.h>

#include "base.h"
#include "grideffects.h"
#include "matrix4x4.h"

extern Vector3 direction[GRID_WIDTH * GRID_HEIGHT];

#include "ray3.h"
#include "sphere.h"
#include "plane.h"

/*
void grideffects::plane(grid g, const Matrix4x4 &m, const Plane &plane, int fog)
{

	Vector3 origin = m.get_translation();
//	Vector3 origin = Vector3(0, 0, 0);
		
	Vector3 *dir = direction;
	grid_node *n = &g[0];

	for (u32 y = GRID_HEIGHT; y; --y)
	{
		for (u32 x = GRID_WIDTH; x; --x)
		{
			Vector3 &d = *dir++;
			
			const Ray3 ray(origin, m.rotate_vector(d));
//			const Ray3 ray(origin, d);
			
			fixed16 t = intersect_ray_plane(ray, plane);
			Vector3 intersection = ray.o + ray.d * t;
			
			n->u = intersection.x.get_val() >> 2;
			n->v = intersection.y.get_val() >> 2;

			int shade = (1 << 11) - (t.rcp().get_val() * 30);
			shade = ((7 << 16) + (1 << 15)) - shade;
			n->col = MIN(MAX(shade, 1 << 15), ((7 << 16) + (1 << 15)));
			n++;
		}
	}
}
*/

void grideffects::sphere(grid g, const Matrix4x4 &m, int fog) __attribute__((section(".iwram1"), long_call));

void grideffects::sphere(grid g, const Matrix4x4 &m, int fog)
{

	Vector3 origin = m.get_translation();
//	Vector3 origin = Vector3(0, 0, 0);
	
	const Sphere sphere(Vector3(0, 0, fixed16(2.5)), fixed16(1.25));
	
	Vector3 *dir = direction;
	grid_node *n = &g[0];

	for (u32 y = GRID_HEIGHT; y; --y)
	{
		for (u32 x = GRID_WIDTH; x; --x)
		{
			Vector3 &d = *dir++;
			
			const Ray3 ray(origin, m.rotate_vector(d));
//			const Ray3 ray(origin, d);
			
//			fixed16 t = intersect_ray_plane(ray, plane);
			fixed16 t = intersect_ray_sphere(ray, sphere);
			Vector3 intersection = ray.o + ray.d * t;
			
			Vector3 normal = intersection - sphere.o;
			normal *= sphere.r_rcp;
			
			n->u = intersection.x.get_val();
			n->v = intersection.y.get_val();

			int shade = (1 << 10) - (t.rcp().get_val() * fog);
			n->col = (7 << 16) - MIN(MAX(shade, 255), (7 << 16) - 255);
//			n->col = (7 << 16) - MIN(MAX(shade, ((1 << 16) - 1)), (7 << 16) - 255);
//			n->col = 0;
//			shade = rcp(fixed16(100000) + t * 100).get_val(); //  + (3 << 16);
#if 0
			shade = t.get_val(); //intersection.z.get_val();
			shade = abs(shade);
			shade *= 32;
//			shade = shade - (32 << 16);
			shade = shade - (20 << 16);
#else
/*
			shade = rcp((t >> 4) + fixed16(0.25)).get_val(); //intersection.z.get_val();
			shade = (8 << 16) - shade;			
*/
			normal.z *= normal.z;
			normal.z *= normal.z;
			normal.z *= normal.z;
			normal.z *= normal.z;
			normal.z *= fixed16(1.75);
			normal.z -= fixed16(0.25);

			shade = (fixed16(8) - normal.z * 8).get_val();
#endif
			n->col = MIN(MAX(shade, 1 << 15), ((7 << 16) + (1 << 15)));
			n++;
		}
	}
}

#if 0
void grideffects::flat(grid g, s32 xscroll, s32 yscroll)
{
	grid_node *n = &g[0];

	for (u32 y = GRID_HEIGHT; y; --y)
	{
		for (u32 x = GRID_WIDTH; x; --x)
		{
			n->u = (x << 10) + (fixed_sin((y << 3) + (x << 4) + yscroll) >> 2);
			n->v = (y << 10) + (fixed_cos((x << 4) + (y << 3) + xscroll) >> 2);
			
			n->u = (y << 11) + (fixed_sin((n->v << 1) + (n->u << 2)) >> 4) + yscroll << 1;
			n->v = (x << 11) + (fixed_sin((n->u << 2) - (n->v << 1)) >> 4) + xscroll << 1;

			n->u = (y << 11) + (fixed_sin((n->v << 0) + (n->u << 1)) >> 4) + yscroll << 0;
			n->v = (x << 11) + (fixed_sin((n->u << 1) - (n->v << 0)) >> 4) + xscroll << 0;
			n++;
		}
	}
}
#endif
