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
#include "../decomp.h"

#include "../fixed.h"
#include "../math.h"
#include "../grid.h"
#include "../grideffects.h"
#include "../fixed.h"

#include "../adpcm.h"
#include "../timer.h"
#include "../beatcounter.h"

static u16 pal[256] EWRAM_DATA;
static u16 pal_bright[128] EWRAM_DATA;

DECLARE_OVERLAY(speaker1_01)
DECLARE_OVERLAY(speaker1_02)

DECLARE_OVERLAY(speaker01)
DECLARE_OVERLAY(speaker02)
DECLARE_OVERLAY(speaker03)
DECLARE_OVERLAY(speaker04)

extern const u16 sfc7_mask[];
static int fadein = 0;

static int fade = 0;
static int fade_delta = 0;

static int pulse_time     = 0;
static int pulse_len      = 8;
static int pulse_fadein   = 4;
static int pulse_fadeout  = 1;

const fixed16 bc_data[] = {
	0, 2, 4, 6, 6.5, 8, 10, 12, 14, 14.5, 16, 18, 20, 22, 24, 26,
	33, 35, 37, 39, 39.5, 41, 43, 45, 47, 47.5, 49, 51, 53, 55, 55.5, 57, 59, 61, 62, 63
};
BeatCounter bc = BEATCOUNTER(bc_data);

const fixed16 bc2_data[] = {
	0, 0.25, 2, 2.25, 4, 4.25, 6, 6.25, 6.5, 7.75, 8, 8.25, 10, 10.25, 12, 14, 14.5, 16, 18, 20, 22, 24, 26,
	33, 33.25, 35, 35.25, 37, 37.25, 39, 39.25, 39.5, 39.75, 41, 41.25, 43, 43.25, 45, 45.25, 47, 47.25, 47.5, 47.75, 49, 49.25, 51, 51.25, 53, 53.25, 55, 55.25, 55.5, 55.75, 57, 57.25, 59, 59.25, 61, 61.25, 62, 62.25, 63, 63.25
};
BeatCounter bc2 = BEATCOUNTER(bc2_data);

static const fixed16 time_bias = -18;
static void vblank()
{
	int beat = int(timer::getBeat());
	u32 temp = pal[255];
	for (int i = 0; i < 128; ++i)
	{
		pal[255 - i] = pal[255 - i - 1];
	}
	pal[128] = temp;
	
	if (pulse_time < pulse_len) fade += pulse_fadein;
	else                        fade -= pulse_fadeout;
	pulse_time++;
	
//	if (pulse_time > 128) pulse_time = 0;

/*
	static int pulse_pos = 0;
	static int timing_table[] = {};
*/
	fixed16 b = timer::getBeat() + time_bias;
	static int last_beat = 0;
	if (bc.getPos(b) != last_beat) pulse_time = 0;
	last_beat = bc.getPos(b);
	
	fade = MAX(fade, 0);
	fade = MIN(fade, 31);

	pal[128] = RGB5(fade, fade, fade);	
	
	// copy pal contents into BG_COLORS, with modulation
	for (int i = 0; i < 128; ++i)
	{
		int col = pal[128 + i];
		int r = col & 31;
		int g = (col >> 5) & 31;
		int b = (col >> 10) & 31;

		r = (r * pal_bright[i]) >> 16;
		g = (g * pal_bright[i]) >> 16;
		b = (b * pal_bright[i]) >> 16;
	
		col = r | (g << 5) | (b << 10);
		BG_COLORS[128 + i] = col;
	}

	// make shiftable palette
	palette::make_fadepal(BG_COLORS, BG_COLORS, 0);
	
	/* update alpha */
	REG_BLDALPHA = ((16) << 8) | (fadein >> 1);
	
	if (beat > 16 && fadein < 32) fadein++;
}

static void init()
{	
	VBlankIntrWait();
	
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16 << 8);
	OVERLAY(speaker1_01);
	fb::setMode(MODE_4 | BG2_ON | OBJ_ENABLE | OBJ_1D_MAP);
}

inline int get_col(int x, int y)
{
	int dist = rcp(rsq(x*x + y*y)) >> 8;
	dist = MIN(dist, 127);
	int col = (128 + (dist & 127));
	int xf = (1 << 16) + fixed_sin(x << 10);
	int yf = (1 << 16) + fixed_sin(y << 10);
	
	return col;
}

static void setup_radial_gradient()
{
	u16 *dst = (u16*)fb::bb;
	for (int y = 0; y < GBA_HEIGHT; ++y)
	{
		for (int x = 0; x < GBA_WIDTH; x += 2)
		{
			int nx = x - GBA_WIDTH / 2;
			int ny = y - GBA_HEIGHT / 2;
			u16 dcol = get_col(nx, ny) | (get_col(nx + 1, ny) << 8);
			*dst++ = dcol;
		}
	}
}

/* u8 radial_tex[256 * 256]; 

static void setup_radial_tex()
{
	BG_COLORS[0] = 0xFFFF;
	
	u8 *dst = (u8*)radial_tex;
	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			int nx = x - 128;
			int ny = y - 128;

			u16 dcol = get_col(nx, ny);
			*dst++ = dcol;
		}
	}
	
	BG_COLORS[0] = 0;
}
*/
/*
static void setup_radial_gradient2(fixed16 time)
{
	grid g;
	Plane plane;
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
	
	Matrix mrot;
	mrot.rotate(rot);
	Matrix mtrans;
	mtrans.translate(trans);
	
	Matrix m = mrot * mtrans;
	
	int d = (2 << 16) + (time * 10).rcp().get_val() * 3;
	plane.d = fixed16::make_raw(d);
	grideffects::plane(g, m, plane, (3 << 15));
		
	dra
	w_grid(fb::bb, g, radial_tex);
}
*/

extern const u8 dist_persp1_raw[];
extern const u16 speaker01_mask[];

namespace parts
{
	void radial(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		BG_COLORS[0] = 0;
		setup_radial_gradient();
		decomp::rleImage4shift(fb::bb, fb::bb+GBA_HEIGHT*GBA_WIDTH, sfc7_mask, 0);
		
		for (int i = 0; i < 128; ++i)
		{
			pal[128 + i] = 0;
			
			int b = (127 - i) << 9;
			b = 128 + (b * 2);
			pal_bright[i] = MIN(MAX(b, 0), ((1 << 16) - 1));
		}
		part::swap();

		while ((timer::getBeat() - start) < 8)
		{
			VBlankIntrWait();
		}
		
		OVERLAY(speaker1_02);
		while ((timer::getBeat() - start) < 32)
		{
			VBlankIntrWait();
		}
		
		CpuFastSet(dist_persp1_raw, fb::bb, (GBA_HEIGHT * GBA_WIDTH) / 4);
		decomp::rleImage4shift(fb::bb, fb::bb+GBA_HEIGHT*GBA_WIDTH, speaker01_mask, 0);
		fadein = 0;
		REG_BLDALPHA = (16 << 8) | 0;
		OVERLAY(speaker01);
		part::swap();
		
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start - 32;
			
			if (beat < 8)
			{
				if (bc2.getPos(timer::getBeat() - start) & 1) OVERLAY(speaker01);
				else                   OVERLAY(speaker02);
			}
			else
			{
				if (bc2.getPos(timer::getBeat() - start) & 1) OVERLAY(speaker03);
				else                   OVERLAY(speaker04);
			}
			
			VBlankIntrWait();
		}
	}
};
