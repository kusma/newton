static int seeed;
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
#include "../timer.h"

extern u16 sprite_pal[256];
extern u8  sprite_raw[];

OBJATTR sprites[128] ALIGN(32);

extern u8 spheregrid_overlay1_raw[];
extern u8 spheregrid_overlay2_raw[];
extern u8 spheregrid_overlay3_raw[];

void make_fade_pal(u16 *dst, const u16 *src, u16 col)
{
	int dst_r = (col >>  0) & 31;
	int dst_g = (col >>  5) & 31;
	int dst_b = (col >> 10) & 31;
	
	for (int i = 0; i < 256; ++i)
	{
		int col = src[i & 15];
		int r = (col >>  0) & ((1 << 5) - 1);
		int g = (col >>  5) & ((1 << 5) - 1);
		int b = (col >> 10) & ((1 << 5) - 1);
		
		int alpha = i / 16;
		
		r = r + (((dst_r - r) * alpha) / 16);
		g = g + (((dst_g - g) * alpha) / 16);
		b = b + (((dst_b - b) * alpha) / 16);
		
		dst[i] = RGB5(r, g, b);
	}
}

extern u32 offsets[64];
extern u8  sphere00[64];
extern u8 *spheremaps[];

extern u8  sphere_tex[];
extern u16 sphere_pal[];

#define ANGLE_COUNT (128 * 2)


static void init()
{
	VBlankIntrWait();
	
//	REG_BLDCNT = 0; // BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDCNT = BIT(10) | BIT(6)| BIT(7) | BIT(4) | BIT(13);
	REG_BLDALPHA = (2 << 8) | (16);
	
//	OVERLAY(sfc7_1);
	u16 fog_col = RGB5(31, 31, 31);
//	u16 fog_col = RGB5(0, 0, 0);
	make_fade_pal(OBJ_COLORS, sphere_pal, fog_col);
	BG_COLORS[0] = fog_col;
	BG_COLORS[1] = RGB5(0, 0, 0);
//	BG_COLORS[0] = 0;
	
	int zero = 0;
	CpuFastSet(&zero, BITMAP_OBJ_BASE_ADR, FILL | ((8 * 8 + 4 * 4 + 2 * 2 + 1) * 2 * 10));
	
/*	
	for (int i = 0; i < 128; ++i)
	{
		OAM[i].attr0 = OBJ_Y(0) | ATTR0_COLOR_16;
		OAM[i].attr1 = OBJ_X(0) | OBJ_SIZE(Sprite_64x64);
		OAM[i].attr2 = OBJ_CHAR(512) | ATTR2_PALETTE(0);
	}
*/
	
	SetMode(MODE_4 | BG2_ON | OBJ_ENABLE | OBJ_1D_MAP);
}

extern int yflip;

void update_sphere(u8 *dst_, int slong_offs, int xrot);
void downsample_tiles(u8 *src_, u8 *dst_, u32 tile_pitch);

void update_sprite_32x32(u8 *src, u8 *dst)
{
	const u32 tile_pitch = (64 * 8) / 2;
	for (int y = 0; y < 4; ++y)
	{
		downsample_tiles(src, dst, tile_pitch);
		src += ((8 * 8) / 2) * 2;
		dst += ((8 * 8) / 2);
		
		downsample_tiles(src, dst, tile_pitch);
		src += ((8 * 8) / 2) * 2;
		dst += ((8 * 8) / 2);
		
		downsample_tiles(src, dst, tile_pitch);
		src += ((8 * 8) / 2) * 2;
		dst += ((8 * 8) / 2);

		downsample_tiles(src, dst, tile_pitch);
		src += ((8 * 8) / 2) * 2;
		dst += ((8 * 8) / 2);
		
		/* jump to next tile-line */
		src += tile_pitch;
	}
}

void update_sprite_16x16(u8 *src, u8 *dst)
{
	const u32 tile_pitch = (32 * 8) / 2;
	for (int y = 0; y < 2; ++y)
	{
		downsample_tiles(src, dst, tile_pitch);
		src += ((8 * 8) / 2) * 2;
		dst += ((8 * 8) / 2);
		
		downsample_tiles(src, dst, tile_pitch);
		src += ((8 * 8) / 2) * 2;
		dst += ((8 * 8) / 2);
		
		/* jump to next tile-line */
		src += tile_pitch;
	}
}


void update_sprite_8x8(u8 *src, u8 *dst)
{
	const u32 tile_pitch = (16 * 8) / 2;
	downsample_tiles(src, dst, tile_pitch);
}

#include "../math.h"
#include "../fixed.h"
#include "../vector4.h"
#include "../matrix4x4.h"

/* 8x8x8 grid -> 512 nodes, divided by 6 -> 85 nodes w/ 45 degree fov */
#define GRID_SIZE 7

Matrix4x4 proj;

void sphere_my_ass(int frame)
{
	fixed16 time = fixed16::make_raw( seeed + (frame << 4));

	Vector3 rot(
		-cos(time >> 2) << 3,
		fixed16(3.1415 * 2) + -sin(time >> 2) << 2,
		(fixed16(5) >> 3) - (cos(time >> 1) << 1)
	);
	
	Vector3 trans(
		sin(time >> 2) * 50,
		cos(time >> 3) * 50,
		cos(time >> 2) * 40
	);
	
//			rot.z = 0;
//			rot  = Vector3(0, 0, time * 10);
//			trans = Vector3(0, 0, 0);

	trans.x &= 0xFFFF;
	trans.y &= 0xFFFF;
	trans.z &= 0xFFFF;
	trans -= Vector3(fixed16(0.5), fixed16(0.5), fixed16(0.5));
	
//			((volatile u16*)BG_COLORS)[0] = RGB5(0, 0, 31);
	Matrix4x4 mrot_x, mrot_y, mrot_z;
	mrot_x.rotate(Vector3(rot.x, 0, 0));
	mrot_y.rotate(Vector3(0, rot.y, 0));
	mrot_z.rotate(Vector3(0, 0, rot.z));
	
	Matrix4x4 mrot, mtrans;
	mrot.rotate(rot);
//			mrot = mrot_x * mrot_y * mrot_z;
	mrot = mrot_y * mrot_x * mrot_z;
	mtrans.translate(trans);
	Matrix4x4 mat = mtrans * mrot;
	
	Matrix4x4 mvp = mat * proj;
	
	int yrot = rot.y.get_val() >> 8; // ArcTan2(mvp(0,2).get_val() >> 8, mvp(2,2).get_val() >> 8) >> 8;
	int xrot = -rot.x.get_val() >> 8; // ArcTan2(mvp(1,2).get_val() >> 8, mvp(2,2).get_val() >> 8) >> 8;
	int zrot = -rot.z.get_val() >> 8;

/*
	int yrot = ArcTan2(mvp(0,2).get_val() >> 8, mvp(2,2).get_val() >> 8) >> 8;
	int xrot = ArcTan2(mvp(1,2).get_val() >> 8, mvp(2,2).get_val() >> 8) >> 8;
	int zrot = ArcTan2(mvp(0,2).get_val() >> 8, mvp(1,2).get_val() >> 8) >> 8;
*/

//			((volatile u16*)BG_COLORS)[0] = RGB5(31, 0, 0);
#define TILES ((8 * 8) + (4 * 4) + (2 * 2) + 1)
	u8 *tiles[2] = 
	{
		(u8*)BITMAP_OBJ_BASE_ADR,
		(u8*)BITMAP_OBJ_BASE_ADR + (TILES) * ((8 * 8) / 2)
	};
	
	u32 tiles_start[2] = 
	{
		512,
		512 + TILES
	};
	int tile_start = tiles_start[frame & 1];

//			update_sphere(tiles[0], yrot, xrot);
	update_sphere(tiles[frame & 1], yrot, xrot);

//			((volatile u16*)BG_COLORS)[0] = RGB5(31, 31, 31);
	
	u8 *src = (u8*)BITMAP_OBJ_BASE_ADR + ((tile_start - 512) * ((8 * 8) / 2));
	u8 *dst = (u8*)BITMAP_OBJ_BASE_ADR + ((tile_start - 512) + (8 * 8)) * ((8 * 8) / 2);
	
	/* update sprite "mipmaps" */
	update_sprite_32x32(src, dst);
	src += (64 * 64) / 2;
	dst += (32 * 32) / 2;
	update_sprite_16x16(src, dst);
	src += (32 * 32) / 2;
	dst += (16 * 16) / 2;
	update_sprite_8x8(src, dst);
	
	
	{
		fixed16 theta = fixed16::make_raw(zrot << 8);
		fixed16 ct = cos(theta);
		fixed16 st = sin(theta);
		
		/*
m[0][0] =  cz; m[0][1] = sz;
m[1][0] = -sz; m[1][1] = cz;
		*/
		for (int i = 0; i < 32; ++i)
		{
			fixed16 s = fixed16(1) + (fixed16(i) / 32);
			((OBJAFFINE*)sprites)[i].pa =  ( ct * s).get_val() >> 8;
			((OBJAFFINE*)sprites)[i].pb =  ( st * s).get_val() >> 8;
			((OBJAFFINE*)sprites)[i].pc =  (-st * s).get_val() >> 8;
			((OBJAFFINE*)sprites)[i].pd =  ( ct * s).get_val() >> 8;
			
			if (yflip)
			{
				((OBJAFFINE*)sprites)[i].pc = -((OBJAFFINE*)sprites)[i].pc;
				((OBJAFFINE*)sprites)[i].pd = -((OBJAFFINE*)sprites)[i].pd;
			}
		}
	}


//			((volatile u16*)BG_COLORS)[0] = RGB5(0, 0, 0);			
//			((volatile u16*)BG_COLORS)[0] = RGB5(31, 0, 31);
	
	Vector4 xaxis(mvp(0,0), mvp(1,0), mvp(2,0), mvp(3,0));
	Vector4 yaxis(mvp(0,1), mvp(1,1), mvp(2,1), mvp(3,1));
	Vector4 zaxis(mvp(0,2), mvp(1,2), mvp(2,2), mvp(3,2));
	Vector4  zpos(mvp(0,3), mvp(1,3), mvp(2,3), mvp(3,3));
	
	/* make sure z is positive */
	if (xaxis.z < fixed16(0)) xaxis = -xaxis;
	if (yaxis.z < fixed16(0)) yaxis = -yaxis;
	if (zaxis.z < fixed16(0)) zaxis = -zaxis;
	
	/* sort axes by z */
#if 0
	if (yaxis.z < xaxis.z) std::swap(yaxis, xaxis);
	if (zaxis.z < yaxis.z) std::swap(zaxis, yaxis);
	if (xaxis.z > zaxis.z) std::swap(xaxis, zaxis);
	if (yaxis.z < xaxis.z) std::swap(yaxis, xaxis); /* TEMP fix! (TODO: fix this shit) */
#else
	if (xaxis.z > yaxis.z) std::swap(xaxis, yaxis);
	if (xaxis.z > zaxis.z) std::swap(xaxis, zaxis);
	if (yaxis.z > zaxis.z) std::swap(yaxis, zaxis);
#endif			
	/* center the grid */
	zpos -= xaxis * (fixed16(GRID_SIZE - 1) >> 1);
	zpos -= yaxis * (fixed16(GRID_SIZE - 1) >> 1);
	zpos -= zaxis * (fixed16(GRID_SIZE - 1) >> 1);

	tile_start = tiles_start[frame & 1];

//			((volatile u16*)BG_COLORS)[0] = RGB5(0, 0, 31);
	int sprite = 0;
	for (int z = 0; z < GRID_SIZE; ++z)
	{
		Vector4 ypos = zpos;
		for (int y = 0; y < GRID_SIZE; ++y)
		{
			Vector4 xpos = ypos;
			for (int x = 0; x < GRID_SIZE; ++x)
			{
/*					Vector3 opos(x, y, z);
				Vector4 p = mvp * opos; */

				Vector4 p = xpos;
				bool cull = false;
				
				if (-p.w > p.z) cull = true;
				if ( p.w < p.z) cull = true;

				if (-p.w > (p.x + fixed16(0.2))) cull = true;
				if ( p.w < (p.x - fixed16(0.2))) cull = true;
				if (-p.w > (p.y + fixed16(0.33))) cull = true;
				if ( p.w < (p.y - fixed16(0.33))) cull = true;

				if (!cull)
				{
					/* bounce :) */
//							if (p.w < fixed16(1.0)) p.w = fixed16(1.0) + abs(p.w - fixed16(1.0));
					
					fixed16 w_rcp = rcp(p.w);
					int sx = (p.x * w_rcp) * fixed16( SCREEN_WIDTH  / 2) + fixed16(SCREEN_WIDTH  / 2);
					int sy = (p.y * w_rcp) * fixed16(-SCREEN_HEIGHT / 2) + fixed16(SCREEN_HEIGHT / 2);

					int size = (p.w - fixed16(0.75)) * 32;
//							int size = 32 * w_rcp;
					
					int sprite_size = 0;
					if (size > 31)
					{
						size -= 32;
						sprite_size++;
					}
					if (size > 31)
					{
						size -= 32;
						sprite_size++;
					}
					if (size > 31)
					{
						size -= 32;
						sprite_size++;
					}
					if (size > 31) size = 31;
						
					if (size < 0)  size = 0;
					
/*							if ((sx >= -s / 2 && sx < SCREEN_WIDTH + s / 2) &&
						(sy >= -s / 2 && sy < SCREEN_HEIGHT + s / 2)) */
					{
						p.z = fixed16(-0.35) + p.z * fixed16(0.65);
//								p.z = fixed16(1.0) - p.z;
//								int pal = (p.z * p.z) * 15;
						int pal = p.z * 15;
						if (pal < 0)  pal = 0;
						if (pal > 15) pal = 15;
						
#if 1
						switch (sprite_size)
						{
							case 0:
								sprites[sprite].attr0 = OBJ_Y(sy - 32) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
								sprites[sprite].attr1 = OBJ_X(sx - 32) | OBJ_SIZE(Sprite_64x64) | ATTR1_ROTDATA(size);
								sprites[sprite].attr2 = OBJ_CHAR(tile_start) | ATTR2_PALETTE(pal) | OBJ_TRANSLUCENT;
							break;
							case 1:
								sprites[sprite].attr0 = OBJ_Y(sy - 16) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
								sprites[sprite].attr1 = OBJ_X(sx - 16) | OBJ_SIZE(Sprite_32x32) | ATTR1_ROTDATA(size);
								sprites[sprite].attr2 = OBJ_CHAR(tile_start + ((64 * 64) / (8 * 8))) | ATTR2_PALETTE(pal) | OBJ_TRANSLUCENT;
							break;
							case 2:
								sprites[sprite].attr0 = OBJ_Y(sy - 8) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
								sprites[sprite].attr1 = OBJ_X(sx - 8) | OBJ_SIZE(Sprite_16x16) | ATTR1_ROTDATA(size);
								sprites[sprite].attr2 = OBJ_CHAR(tile_start + ((64 * 64) / (8 * 8)) + ((32 * 32) / (8 * 8))) | ATTR2_PALETTE(pal) | OBJ_TRANSLUCENT;
							break;
							case 3:
								sprites[sprite].attr0 = OBJ_Y(sy - 4) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
								sprites[sprite].attr1 = OBJ_X(sx - 4) | OBJ_SIZE(Sprite_8x8) | ATTR1_ROTDATA(size);
								sprites[sprite].attr2 = OBJ_CHAR(tile_start + ((64 * 64) / (8 * 8)) + ((32 * 32) / (8 * 8)) + ((16 * 16) / (8 * 8))) | ATTR2_PALETTE(pal) | OBJ_TRANSLUCENT;
							break;
						}
#elif 0
						sprites[sprite].attr0 = OBJ_Y(sy - 16) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
						sprites[sprite].attr1 = OBJ_X(sx - 16) | OBJ_SIZE(Sprite_32x32) | ATTR1_ROTDATA(size);
						sprites[sprite].attr2 = OBJ_CHAR(512 + ((64 * 64) / (8 * 8))) | ATTR2_PALETTE(pal);
#elif 0
						sprites[sprite].attr0 = OBJ_Y(sy - 8) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
						sprites[sprite].attr1 = OBJ_X(sx - 8) | OBJ_SIZE(Sprite_16x16) | ATTR1_ROTDATA(size);
						sprites[sprite].attr2 = OBJ_CHAR(512 + ((64 * 64) / (8 * 8)) + ((32 * 32) / (8 * 8))) | ATTR2_PALETTE(pal);
#else
						sprites[sprite].attr0 = OBJ_Y(sy - 4) | ATTR0_COLOR_16 | ATTR0_ROTSCALE; // | BIT(10);
						sprites[sprite].attr1 = OBJ_X(sx - 4) | OBJ_SIZE(Sprite_8x8) | ATTR1_ROTDATA(size);
						sprites[sprite].attr2 = OBJ_CHAR(512 + ((64 * 64) / (8 * 8)) + ((32 * 32) / (8 * 8)) + ((16 * 16) / (8 * 8))) | ATTR2_PALETTE(pal);
#endif

						sprite++;
						
						if (sprite >= 128) return; /* should not be needed! */
					}
				}
				
				xpos += xaxis;
			}
			ypos += yaxis;
		}
		zpos += zaxis;
	}
//			BG_COLORS[0] = RGB5(0, 31, 0);
	
	while (sprite < 128)
	{
		sprites[sprite++].attr0 = ATTR0_DISABLED;
	}
}

static volatile int blanks = 0;
static void vblank()
{
	CpuSet(sprites,  OAM, (128 * sizeof(OAM[0])) / 2);
#if 0
#define	REG_BG2PA	*((vu16 *)(REG_BASE + 0x20))
#define	REG_BG2PB	*((vu16 *)(REG_BASE + 0x22))
#define	REG_BG2PC	*((vu16 *)(REG_BASE + 0x24))
#define	REG_BG2PD	*((vu16 *)(REG_BASE + 0x26))
#define	REG_BG2X	*((vu32 *)(REG_BASE + 0x28))
#define	REG_BG2Y	*((vu32 *)(REG_BASE + 0x2c))
#endif

	int scale_x = -(rand() & 0xFF);
	int scale_y = -(rand() & 0xFF);
	if (rand() & 1) scale_x = -scale_x;
	if (rand() & 1) scale_y = -scale_y;
	
	int translate_x = (rand() & 0xFF) - 128;
	int translate_y = (rand() & 0xFF) - 128;
	translate_x >>= 0;
	translate_y >>= 0;
/*
	translate_x = 0;
	translate_y = 0;
*/
	REG_BG2PA = 0x100 + scale_x;
	REG_BG2X  = (-(scale_x * GBA_WIDTH) >> 1) + (translate_x << 8);
	REG_BG2PD = 0x100 + scale_y;
	REG_BG2Y  = (-(scale_y * GBA_HEIGHT) >> 1) + (translate_y << 8);

/*	REG_BG2X  = ((240 / 2) << 8);
//	REG_BG2X  = (((240 / 2) << 8) * scale_x) >> 8;
*/
//	REG_BG2PA = rand();
//	REG_BG2X  = rand();
	
	blanks++;
	sphere_my_ass(blanks);
}

extern u8 __iwram_overlay_start[];
extern u8 __load_start_iwram1[];
extern u8 __load_stop_iwram1[];
extern u8 __load_start_iwram0[];
extern u8 __load_stop_iwram0[];

namespace parts
{
	void spheregrid(fixed16 start, fixed16 end, int seed)
	{
//		memcpy(__iwram_overlay_start, __load_start_iwram1, ((int)__load_stop_iwram1) - ((int)__load_start_iwram1));
		
		seeed = seed;
		part::set_part(init, vblank);
		
		int swaps = 0;
		blanks = 0;
		
		proj.project(105, fixed16(GBA_WIDTH) / fixed16(GBA_HEIGHT), fixed16(0.25), fixed16(GRID_SIZE) / 2);
#if 1
		{
			ObjAffineSource obj_src;
			obj_src.theta = 20 << 8;
			ObjAffineDest obj_dst;
	
			for (int i = 0; i < 32; ++i)
			{
//				fixed16 s = fixed16(1) + (fixed16(i) >> 4);
				fixed16 s = fixed16(1) + (fixed16(i) / 32);
#if 0
				obj_src.sX = s.get_val() >> 8;
				obj_src.sY = s.get_val() >> 8;
				ObjAffineSet(&obj_src, &obj_dst, 1, 2);
				
				/* setup rot/scale matrix */
				((OBJAFFINE*)sprites)[i].pa = obj_dst.pa;
				((OBJAFFINE*)sprites)[i].pb = obj_dst.pb;
				((OBJAFFINE*)sprites)[i].pc = obj_dst.pc;
				((OBJAFFINE*)sprites)[i].pd = obj_dst.pd;
#else
				((OBJAFFINE*)sprites)[i].pa = s.get_val() >> 8;
				((OBJAFFINE*)sprites)[i].pb = 0;
				((OBJAFFINE*)sprites)[i].pc = 0;
				((OBJAFFINE*)sprites)[i].pd = s.get_val() >> 8;
#endif
			}
		}
#endif
#if 0
		ObjAffineSource obj_src[32];
		for (int i = 0; i < 32; ++i)
		{
			fixed16 s = fixed16(1) + (fixed16(i) >> 4);
			obj_src[i].theta = 20 << 8;
			obj_src[i].sX = s.get_val() >> 8;
			obj_src[i].sY = s.get_val() >> 8;
			
			((OBJAFFINE*)sprites)[i].pa = s.get_val() >> 8;
			((OBJAFFINE*)sprites)[i].pb = s.get_val() >> 8;
			((OBJAFFINE*)sprites)[i].pc = s.get_val() >> 8;
			((OBJAFFINE*)sprites)[i].pd = 0;
		}
		void *temp = &(((OBJAFFINE*)sprites)[0].pa);
//		void *temp = (void*)sprites;
//		temp = (void*)((u32)temp + 16);
		ObjAffineSet((ObjAffineSource*)sprites, temp, 16, 8);
#endif

		while (timer::getBeat() < end)
		{

//			((volatile u16*)BG_COLORS)[0] = RGB5(31, 0, 0);
			
//			BG_COLORS[0] = RGB5(0, 0, 0);
			const u8 *overlays[] = 
			{
				spheregrid_overlay1_raw,
				spheregrid_overlay2_raw,
				spheregrid_overlay3_raw,
				spheregrid_overlay2_raw,
				spheregrid_overlay3_raw,
				spheregrid_overlay1_raw,
				spheregrid_overlay3_raw,
				spheregrid_overlay2_raw,
			};
			
			const u8 *overlay = overlays[(swaps >> 3) % (sizeof(overlays) / sizeof(overlays[0]))];
			
			CpuFastSet(overlay, fb::bb, (SCREEN_WIDTH * SCREEN_HEIGHT) / 4);
/*			
			u32 mongo = 0x01010101;
			CpuFastSet(&mongo, fb::bb, FILL | ((SCREEN_WIDTH * SCREEN_HEIGHT) / 4));
			*/
			
			VBlankIntrWait();
			part::swap();
			swaps++;
		}

		// the matrix
		REG_BG2PA = 1<<8;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = 1<<8;
		REG_BG2Y = 0;

//		memcpy(__iwram_overlay_start, __load_start_iwram0, ((int)__load_stop_iwram0) - ((int)__load_start_iwram0));
	}
};
