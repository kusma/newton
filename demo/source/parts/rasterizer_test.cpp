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

const u8 bogo_tex[4][256] =
{
	{0x01, 0x01, 0x01, 0x01},
	{0x01, 0x02, 0x03, 0x01},
	{0x01, 0x03, 0x02, 0x01},
	{0x01, 0x01, 0x01, 0x01},
};

namespace parts
{
	void test(int len) IWRAM_CODE;
	void test(int len)
	{
		part::set_part(init, vblank);
		
		/* setup material */
		Material m;
		m.filler = FILLER_TEX256;
		m.oneone = true;
		m.uvGen = UVGEN_UVS;
		m.tex[0] = flextextiles_tex;
		m.tex[0] = (u8*)bogo_tex;
		
		while (true)
		{
			int bg_color = 0;
			bg_color |= bg_color << 8;
			bg_color |= bg_color << 16;
			CpuFastSet(&bg_color, fb::bb, ((GBA_WIDTH * GBA_HEIGHT) / 4) | FILL);
			
			PolyVertex polyverts[3];
			
			polyverts[0].x = blanks % 60;
			polyverts[0].y = 0;
			polyverts[0].u = 0   << 8;
			polyverts[0].v = 0   << 8;
			
			polyverts[1].x = 0;
			polyverts[1].y = 150;
			polyverts[1].u = 0 << 8;
			polyverts[1].v = 4 << 8;
			
			polyverts[2].x = 150;
			polyverts[2].y = 0;
			polyverts[2].u = 4 << 8;
			polyverts[2].v = 0 << 8;
			
			polyfiller(fb::bb, polyverts, m, NULL, 0, GBA_HEIGHT);
			
			VBlankIntrWait();
			part::swap();
		}
	}
};
