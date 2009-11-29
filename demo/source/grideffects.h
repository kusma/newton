#ifndef _GRIDEFFECTS_H_
#define _GRIDEFFECTS_H_

#include "grid.h"
#include "vector3.h"
#include "matrix4x4.h"

#include "plane.h"

namespace grideffects
{

void init();
// void plane(grid g, fixed16 time, fixed16 scrolltimex, fixed16 scrolltimey);
// void plane(grid g, const Matrix &m, int fog);
void plane(grid g, const Matrix4x4 &m, const Plane &plane, int fog);

void sphere(grid g, const Matrix4x4 &m, int fog);
void flat(grid g, s32 xscroll, s32 yscroll);

}

#endif
