#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_affine.h>
#include <stdio.h>
#include "../palette.h"
#include "../part.h"
#include "../fb.h"
#include "../math.h"
#include "../decomp.h"

#include "../math.h"
#include "../fixed.h"
#include "../vector4.h"
#include "../matrix4x4.h"
#include "../poly.h"

#include "../renderer.h"

extern const u8 flextextiles_tex[];
extern const u16 part_growy_pal[];

static volatile int blanks = 0;
static void vblank()
{
	blanks++;
}

static void init()
{
	VBlankIntrWait();
	
	palette::make_fadepal(BG_COLORS, part_growy_pal, RGB5(31, 31, 31));
	BG_COLORS[0] = RGB5(31, 31, 31);
	
	BG_COLORS[0x00] = RGB5(0, 0, 0);
	BG_COLORS[0x01] = RGB5(31,0, 0);
	BG_COLORS[0x02] = RGB5(0,31, 0);
	BG_COLORS[0x03] = RGB5(0, 0,31);
	SetMode(MODE_4 | BG2_ON);
}

typedef struct triangle
{
	/* vertex attributes */
	struct 
	{
		u16 pos;
		u16 uv;
	} vertices[3];
	/* todo: material, normal, vertex center etc */
};

typedef struct mesh
{
	triangle *triangles;
	int triangle_count;
	
	s16 *positions;
	int positiion_count;
	
	s16 *uvs;
	int uv_count;
};

struct triangle tris[] =
{
	{
		0, 0,
		1, 1,
		3, 3,
	},
	{
		3, 3,
		1, 1,
		2, 2,
	},
};

s16 positions[] = 
{
	-1, -1, 0,
	-1,  1, 0,
	 1,  1, 0,
	 1, -1, 0,
};

s16 uvs[3 * 3] = 
{
	  0<<8,   0<<8,
	  0<<8, 128<<8,
	128<<8, 128<<8,
	128<<8,   0<<8,
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
mesh m = 
{
	tris, ARRAY_SIZE(tris),
	positions, ARRAY_SIZE(positions),
	uvs, ARRAY_SIZE(uvs),
};

Vector3 pos[4] EWRAM_DATA;

Renderer renderer EWRAM_DATA;
Vector4 clip_pos[4] EWRAM_DATA;
unsigned int clip_code[4] EWRAM_DATA;

void draw_mesh(u8 *fb, mesh *m, Material *mat)
{
	for (int i = 0 ; i < 4; ++i)
	{
		Vector4 cpos = renderer.mvp * pos[i];
		unsigned int ccode = 0;
		
		ccode |= 0;
		
		clip_pos[i] = cpos;
		clip_code[i] = ccode;
	}
	
	
	renderer.vertex_cache.trans.vertex_array = clip_pos;
	renderer.vertex_cache.invalidate();
	
	for (int i = 0; i < m->triangle_count; ++i)
	{
		PolyVertex polyverts[3];	
		for (int j = 0; j < 3; ++j)
		{
			int pos_index = m->triangles[i].vertices[j].pos;
			int uv_index  = m->triangles[i].vertices[j].uv;
			
			const Vertex &v = renderer.vertex_cache.getVertex(pos_index);
			
			polyverts[j].x = v.sx;
			polyverts[j].y = v.sy;
			polyverts[j].u = m->uvs[uv_index * 2 + 0];
			polyverts[j].v = m->uvs[uv_index * 2 + 1];
		}
		polyfiller(fb, polyverts, *mat, NULL, 0, GBA_HEIGHT);
	}
}

namespace parts
{
	void test(int len)
	{
		part::set_part(init, vblank);
		
		/* setup material */
		Material mat;
		mat.filler = FILLER_TEX256;
		mat.oneone = true;
		mat.uvGen = UVGEN_UVS;
		mat.tex[0] = flextextiles_tex;
		
		for (int i = 0; i < 4; ++i)
		{
			pos[i].x = positions[i * 3 + 0];
			pos[i].y = positions[i * 3 + 1];
			pos[i].z = positions[i * 3 + 2];
		}
		
		int swaps = 0;
		while (true)
		{
			int bg_color = 0;
			bg_color |= bg_color << 8;
			bg_color |= bg_color << 16;
			CpuFastSet(&bg_color, fb::bb, ((GBA_WIDTH * GBA_HEIGHT) / 4) | FILL);

			fixed16 t = fixed16::make_raw(blanks << 6);
//			fixed16 r = 100;
/*
			m.positions[0 * 3 + 0] = (SCREEN_WIDTH  / 2) + int(sin(t + fixed16(0.0)) * r);
			m.positions[0 * 3 + 1] = (SCREEN_HEIGHT / 2) + int(cos(t + fixed16(0.0)) * r);
			m.positions[1 * 3 + 0] = (SCREEN_WIDTH  / 2) + int(sin(t + fixed16(0.25)) * r);
			m.positions[1 * 3 + 1] = (SCREEN_HEIGHT / 2) + int(cos(t + fixed16(0.25)) * r);
			m.positions[2 * 3 + 0] = (SCREEN_WIDTH  / 2) + int(sin(t + fixed16(0.5)) * r);
			m.positions[2 * 3 + 1] = (SCREEN_HEIGHT / 2) + int(cos(t + fixed16(0.5)) * r);
			m.positions[3 * 3 + 0] = (SCREEN_WIDTH  / 2) + int(sin(t + fixed16(0.75)) * r);
			m.positions[3 * 3 + 1] = (SCREEN_HEIGHT / 2) + int(cos(t + fixed16(0.75)) * r);
*/


//			m.positions[0 * 3] = blanks & 63;
//			m.positions[1 * 3] = blanks & 127;

			Matrix4x4 rot, trans;
			t *= 2;
			rot.rotate(Vector3(t * fixed16(0.26), t * fixed16(0.24), t * fixed16(0.22)));
//			trans.translate(Vector3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0));
			trans.translate(Vector3(0, 0, -2));
			
			Matrix4x4 proj;
			proj.project(fixed16(90), fixed16(SCREEN_WIDTH) / SCREEN_HEIGHT, fixed16(1), fixed16(100));
			
			Matrix4x4 mvp;
			mvp = (rot * trans) * proj;
			
			renderer.mvp = mvp;
			draw_mesh(fb::bb, &m, &mat);
			
			VBlankIntrWait();
			part::swap();
			swaps++;
		}
	}
};
