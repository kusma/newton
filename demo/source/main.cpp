#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_console.h>
#include "tunnel.h"
#include "grideffects.h"
#include "fb.h"
#include <pimp_gba.h>

#include "support.h"
#include "palette.h"

// #define LAMER_VERSION

#include "timer.h"
#include "noise.h"

#ifndef REG_WAITCNT
#define REG_WAITCNT (*(vu16*)(REG_BASE + 0x0204))
#endif

extern const u8 microunits_mod_bin[];
extern const u8 sample_bank_bin[];

extern const u16 tex1_pal[];
extern const u8 tex1_tex[];
extern const u16 tex2_pal[];
extern const u8 tex2_tex[];

DECLARE_OVERLAY(mrt_001)
DECLARE_OVERLAY(mrt_002)
DECLARE_OVERLAY(mrt_003)
DECLARE_OVERLAY(mrt_004)
DECLARE_OVERLAY(mrt_005)
DECLARE_OVERLAY(mrt_006)
DECLARE_OVERLAY(mrt_007)
DECLARE_OVERLAY(mrt_008)
DECLARE_OVERLAY(mrt_009)
DECLARE_OVERLAY(mrt_010)

static grid g EWRAM_DATA;

extern const u16 intro1_pal[];
extern const u8 intro1_tex[];
extern const u16 intro2_pal[];
extern const u8 intro2_tex[];
extern const u16 intro3_pal[];
extern const u8 intro3_tex[];
extern const u16 intro4_pal[];
extern const u8 intro4_tex[];
extern const u16 intro5_pal[];
extern const u8 intro5_tex[];
extern const u16 intro6_pal[];
extern const u8 intro6_tex[];
extern const u16 intro7_pal[];
extern const u8 intro7_tex[];
extern const u16 intro8_pal[];
extern const u8 intro8_tex[];
extern const u16 intro9_pal[];
extern const u8 intro9_tex[];
extern const u16 credits_pal[];
extern const u8 credits_tex[];

static const u8 *intro_texs[] = {
	intro2_tex, intro3_tex, intro4_tex, intro5_tex,
	intro6_tex, intro7_tex, intro8_tex, intro9_tex
};
static const u16 *intro_pals[] = {
	intro2_pal, intro3_pal, intro4_pal, intro5_pal,
	intro6_pal, intro7_pal, intro8_pal, intro9_pal
};

static int blanks = 0;
static int flash = 0;
static int scroll = 0;
static void vblank(void)
{
	pimp_gba_vblank();
	blanks++;
	if (flash > 0) 
		flash--;
	if (scroll > 0)
		scroll--;

	REG_BG2Y = -scroll << 8;
	REG_BG2X = 0;
	int order = pimp_gba_get_order();
	if (order >= 7 && order < 9) {
		REG_BG2Y = ((rand() & 15) - 8) * ((rand() & 15) - 8) * ((rand() & 15) - 8);
		REG_BG2X = ((rand() & 15) - 8) * ((rand() & 15) - 8) * ((rand() & 15) - 8);
	}


	if (fb::curr_oam) {
		for (int i = 0; i < 128; ++i) {
			OBJATTR tmp = fb::curr_oam[i];
			int y = tmp.attr0 & 0xFF;
			tmp.attr0 &= ~0xFF;
			tmp.attr0 |= OBJ_Y(y + scroll);
			OAM[i] = tmp;
		}
	}
	pimp_gba_frame();
}

static void sync_callback(int type, int param)
{
	if (type == PIMP_CALLBACK_NOTE) {
		if (param == 1)
			flash = 16;
		if (param == 2)
			scroll = 16;
	}
}

int main()
{
	grideffects::init();

	/* setup default states */
	REG_WAITCNT = 0x46d6;

	/* clear bits to be conditionally set later on */
	REG_WAITCNT &= ~(((1 << 3) - 1) << 1);
	REG_WAITCNT &= ~(1 << 14);

	/* enable ROM prefetch */
	REG_WAITCNT = 1 << 14;

	/* setup wait states */	
#ifndef LAMER_VERSION
	REG_WAITCNT |= (1 << 4) | (2 << 2);
#else
	REG_WAITCNT |= (1 << 4) | (1 << 2);
#endif

	irqInit();
	fb::reset();
	pimp_gba_init((const pimp_module *)microunits_mod_bin, sample_bank_bin);
	pimp_gba_set_pos(0, 0);
	pimp_gba_set_callback(sync_callback);

	irqSet(IRQ_VBLANK, vblank);
	irqEnable(IRQ_VBLANK);

	fb::reset();
	CpuFastSet(intro1_tex, fb::bb, (GBA_HEIGHT * GBA_WIDTH) / 4);
	fb::setMode(MODE_4 | BG2_ON);
	VBlankIntrWait();
	fb::swap();
	int fade = 0;
	while (pimp_gba_get_order() == 0) {
		int boost = fade << 1;
		boost += (sin(fixed16(blanks) / 50).get_val() * fade) >> 17;
		if (boost < 0)
			boost = 0;
//		palette::boost(BG_COLORS, intro1_pal, 256, (fade + (sin(fixed16(blanks) / 50) * 100) * 3);
		palette::boost(BG_COLORS, intro1_pal, 256, boost);
		VBlankIntrWait();
		if (fade < 128)
			fade++;
	}

	while (pimp_gba_get_order() < 3) {
		int img = (pimp_gba_get_order() - 1) * 4 + pimp_gba_get_row() / 16;
		CpuFastSet(intro_texs[img], fb::bb, (GBA_HEIGHT * GBA_WIDTH) / 4);
		VBlankIntrWait();
		int col = RGB5(flash, flash, flash);
		palette::add(BG_COLORS, intro_pals[img], 256, col);
		fb::swap();
	}

redo:
	fb::setMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP);
	REG_BLDCNT = 0;

	u16 temp1[256], temp2[256];
	palette::make_fadepal(temp1, tex1_pal, 0);
	palette::make_fadepal(temp2, tex2_pal, 0);

	while (pimp_gba_get_order() < 7 || pimp_gba_get_order() > 8 && pimp_gba_get_order() < 17) {
		int order = pimp_gba_get_order();
		if (order > 14)
			order = 14;
		if (order < 7)
			order -= 3;
		if (order > 8)
			order += 5;


		if (pimp_gba_get_order() <= 14) {
			switch (order % 10) {
			case 0:
				OVERLAY(mrt_001);
				break;
			case 1:
				OVERLAY(mrt_002);
				break;
			case 2:
				OVERLAY(mrt_003);
				break;
			case 3:
				OVERLAY(mrt_004);
				break;
			case 4:
				OVERLAY(mrt_005);
				break;
			case 5:
				OVERLAY(mrt_006);
				break;
			case 6:
				OVERLAY(mrt_007);
				break;
			case 7:
				OVERLAY(mrt_008);
				break;
			case 8:
				OVERLAY(mrt_009);
				break;
			case 9:
				OVERLAY(mrt_010);
				break;
			}
		}
		fixed16 time = fixed16(blanks) >> 5;
		Vector3 rot(
			-cos(time >> 3) >> 1,
			fixed16(3.1415 * 2), // -cos(time >> 2) >> 3,
			-cos(time >> 3)
		);
		rot.normalize();

		Vector3 trans(
			sin(time >> 2) >> 2,
			cos(time >> 2) >> 2,
			fixed16(1) - cos(time >> 1) * fixed16(0.05)
		);

		Matrix4x4 mrot;
		mrot.rotate(rot);
		Matrix4x4 mtrans;
		mtrans.translate(trans);

		Matrix4x4 m = mrot * mtrans;

		Plane p(Vector3(0, 0, 1), 3);
		switch ((order / 2) % 3) {
		case 0:
			grideffects::sphere(g, m, (3 << 15));
			break;
		case 1:
			grideffects::plane(g, m, p, (3 << 15));
			break;
		case 2:
			grideffects::flat(g, 0, sin(time * fixed16(0.00095)).get_val());
			break;
		}

		int col = RGB5(flash, flash, flash);


		if (pimp_gba_get_order() > 14) {
			draw_grid(fb::bb, g, tex1_tex, fb::curr_cov);
			static int fade = 256;
			palette::boost(BG_COLORS, temp1, 256, fade);
			palette::boost(OBJ_COLORS, mrt_010_tileset_pal, 16, fade);
			if (fade > 0)
				fade -= 2;
		} else {
			if ((order / 2) & 1) {
				draw_grid(fb::bb, g, tex1_tex, fb::curr_cov);
				palette::add(BG_COLORS, temp1, 256, col);
			} else {
				draw_grid(fb::bb, g, tex2_tex, fb::curr_cov);
				palette::add(BG_COLORS, temp2, 256, col);
			}
		}
		fb::swap();
	}

	if (pimp_gba_get_order() >= 15)
		goto kaare_sa_jeg_skulle_gjoere_det;

	CpuFastSet(credits_tex, fb::bb, (GBA_HEIGHT * GBA_WIDTH) / 4);
	fb::setMode(MODE_4 | BG2_ON);
	VBlankIntrWait();
	fb::swap();
	while (pimp_gba_get_order() < 9) {
		int boost = 256;
		boost += (sin(fixed16(blanks) / 50).get_val() * 100) >> 17;
		boost *= (rand() & 31) * (rand() & 31) * (rand() & 31);
		boost *= 4;
		boost >>= 17;
		if (boost < 0)
			boost = 0;
		palette::boost(BG_COLORS, credits_pal, 256, boost);
		VBlankIntrWait();
	}

	goto redo;

kaare_sa_jeg_skulle_gjoere_det:

	BG_COLORS[0] = (1 << 5) - 1;
	while (1)
		; /* nothing */

	pimp_gba_close();

	irqDisable(IRQ_VBLANK);
	fb::reset();
	while(1) VBlankIntrWait();
}

