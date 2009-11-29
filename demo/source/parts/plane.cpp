#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_affine.h>
#include <stdio.h>
#include "../fb.h"
#include "../grid.h"
#include "../grideffects.h"
#include "../palette.h"
#include "../part.h"
#include "../decomp.h"

static s32 currents[GBA_HEIGHT] EWRAM_DATA;

extern const u16 part_growy_pal[];
extern const u8 flextextiles_tex[];
static grid g EWRAM_DATA;

extern const u8  sfc_logo_tileset_raw[];
extern const u16 sfc_logo_tileset_pal[];
extern const u16 sfc_logo_oam[];

static volatile int blanks = 0;
static volatile int flash  = 256;
static void vblank()
{

	blanks++;
//	palette::boost(BG_COLORS, part_growy_pal, 256, (flash * flash) >> 8);
//	palette::boost(BG_COLORS, part_growy_pal, 256, 128 + (fixed_fsin(blanks * 25) >> 10));

/*
	int f = (flash * flash) >> 8;
	f >>= 3;
	f = MIN(f, 31);
	f = MAX(f, 0);
	palette::make_fadepal(BG_COLORS, part_growy_pal, RGB5(f, f, f));

	flash -= 4;
	if (0 >= flash) flash = 256;
*/
#if 0
	fixed16 s = fixed16::make_raw(blanks << 8);
	s = fixed16::make_raw(s.get_val() & 0xFFFF00);
	
	int attr0_add = OBJ_ROT_SCALE_ON;
	if (s >= fixed16(1))
	{
		s = fixed16(1);
		attr0_add = 0;
	}
	
//	int scale = rcp(s).get_val() >> 8;
	int scale = (1 << 16) / (s.get_val() >> 8);
	int rscale = (1 << 16) / scale;

/*	rscale = (rscale * 250) >> 8; */
	for (int i = 0; i < 128; ++i)
	{
		OAM[i].attr0 |= attr0_add;
		int y = sfc_logo_oam[i * 4 + 0] & 0xFF;
		int x = sfc_logo_oam[i * 4 + 1] & 0x1FF;
		y = (y * rscale) >> 8;
		x = (x * rscale) >> 8;
		
		OAM[i].attr0 &= ~0xFF;
		OAM[i].attr0 |= OBJ_Y(y);

		OAM[i].attr1 &= ~0x1FF;
		OAM[i].attr1 |= OBJ_X(x);
	}

	((OBJAFFINE*)OAM)[0].pa = scale;
	((OBJAFFINE*)OAM)[0].pb = 0;
	((OBJAFFINE*)OAM)[0].pc = 0;
	((OBJAFFINE*)OAM)[0].pd = scale;
#endif

/*	
	ObjAffineSource source;
	source.sX = scale;
	source.sY = scale;
	source.theta = 0; // blanks;
	ObjAffineDest dest;
	ObjAffineSet(&source, &((OBJAFFINE*)OAM)[0].pa, 1, 0);
*/
/*	ObjAffineSet(&source, &dest, 1, 0);
	
	((OBJAFFINE*)OAM)[0].pa = dest.pa;
	((OBJAFFINE*)OAM)[0].pb = dest.pb;
	((OBJAFFINE*)OAM)[0].pc = dest.pc;
	((OBJAFFINE*)OAM)[0].pd = dest.pd;
*/
#if 0
	REG_DMA0CNT = 0;
	DMA0COPY(&currents[0], &REG_BG2X, DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA32 | 1);

	s32 *dst = currents;
	for (int i = GBA_HEIGHT; i; --i)
	{
		int rands = 0;
		int offs = fixed_fsin(i + blanks) >> 5;
		offs += fixed_fsin((i - blanks) * 5) >> 6;
		rands = ((s32(cheap_rand()) >> 22) * offs >> 9) ;
		*dst++ = rands; // *targ;
	}
#endif
}

static Plane plane;

static void init()
{
	VBlankIntrWait();
	OVERLAY(sfc_logo);

	fb::setMode(MODE_4 | BG2_ON);
//	SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP);
/*	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16) | (16<<8); */
	REG_BLDCNT = 0;

	CpuFastSet(part_growy_pal, BG_COLORS, 512 / 4);
	BG_COLORS[0] = 0;

	palette::make_fadepal(BG_COLORS, part_growy_pal, 0);
	plane = Plane(Vector3(0, 0, 1), fixed16::make_raw(3 << 16));	
}

extern const u16 klown_mask[];

namespace parts
{
	void plane(int len)
	{
		part::set_part(init, vblank);

		int swaps = 0;
		blanks = 0;
		while (blanks < 60 * 4 * 4)
		{
			fixed16 time = fixed16::make_raw(swaps << 8);
				
			Vector3 rot(
				-cos(time >> 2) >> 3,
				-sin(fixed16(1) + (time >> 2)) >> 3,
				-cos(fixed16(2) + (time >> 2)) >> 3
			);
			rot.normalize();
			
			Vector3 trans(
				sin(time >> 2) * 10,
				cos(fixed16(2) + time >> 2) * 10,
				cos(fixed16(1) + time) * fixed16(0.75)
			);
			
			Matrix4x4 mrot;
			mrot.rotate(rot);
			Matrix4x4 mtrans;
			mtrans.translate(trans);
			
			Matrix4x4 m = mrot * mtrans;
			
			
			int d = (2 << 16) + (time * 10).rcp().get_val() * 3;
//			d = (25 << 16) - ((time * 100).get_val() % (20 << 16));
			::plane.d = fixed16::make_raw(d);
			grideffects::plane(g, m, ::plane, ((3 << 15) - (flash * flash) >> 4) << 8);
				
			draw_grid(fb::bb, g, flextextiles_tex);
			decomp::rleImage4shift(fb::bb, fb::bb+GBA_HEIGHT*GBA_WIDTH, klown_mask, 0);
				
			part::swap();
			swaps++;
		}
	}
};
