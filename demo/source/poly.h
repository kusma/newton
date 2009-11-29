#ifndef _POLY_H_
#define _POLY_H_
#include "exstructs.h"
#include <gba_base.h>

#ifdef __cplusplus
#define restrict
#endif

//#include "RamVertex.h"
struct PolyVertex {
	s32 x, y;
	s32 u, v;
};
//struct LightDir;
//void draw_flat_triangle(u8 *target, PolyVertex v[3], unsigned short color) IWRAM_CODE;
//void draw_bump_triangle(u8 *target, PolyVertex v[3], const u8 *tex, int palOffs, const s16* light) IWRAM_CODE;
//void draw_texture_triangle(u8 *target, PolyVertex v[3], const u8 *tex) IWRAM_CODE;

void polyfiller(u8* target, PolyVertex v[3], const Material& mtl, const s16 light[3], int clip_top, int clip_bottom) IWRAM_CODE;



#endif /* _POLY_H_ */
