#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <stdio.h>
#include <string.h>
#include "../fb.h"
#include "../grid.h"
#include "../grideffects.h"
#include "../palette.h"
#include "../part.h"
#include "../timer.h"
#include "../noise.h"

static s32 currents[GBA_HEIGHT] EWRAM_DATA;

extern const u16 flextextiles_pal[];
extern const u8 flextextiles_tex[];
static grid g EWRAM_DATA;

static bool block = true;
static fixed16 endBeat;
static void vblank()
{
	if (block) return;
	
	noise::update();

	fixed16 beat = timer::getBeat() / 4;
	fixed16 flash = fixed16(1) - (beat & 0xFFFF);
	int f = flash.get_val() >> 8;
	f >>= 3;
	f = MIN(f, 31);
	f = MAX(f, 0);
	
//	REG_BLDALPHA = (16<<8) | ((f>>1));
//	REG_BLDALPHA = 0 | (0 << 8);
	int alpha = f >> 2;
	
	REG_BLDALPHA = (alpha) | ((16 - alpha)<<8);
	
	palette::make_fadepal(BG_COLORS, flextextiles_pal, RGB5(f, f, f));
	fixed16 fader = (endBeat - timer::getBeat())*(1.0/4);
	if (fader<fixed16(1.0)) {
		int amt = fader.get_val()>>8;
		if (amt>256) amt=256;
		if (amt<0) amt=0;
		palette::boost(BG_COLORS, BG_COLORS, 256, amt-256);
	}
	BG_COLORS[0] = 0;
}

extern const u8  sfc_logo_tileset_raw[];
extern const u16 sfc_logo_tileset_pal[];
extern const u8  sfc_logo_oam[];

static void init()
{
	VBlankIntrWait();
/*	OVERLAY(sfc_logo); */

//	SetMode(MODE_4 | BG2_ON);
	fb::setMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP);
//	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16) | (16<<8);
	REG_BLDCNT = 0;
	noise::setup();
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16) | (16<<8);

	palette::make_fadepal(BG_COLORS, flextextiles_pal, 0);
}

extern u8 __iwram_overlay_start[];
extern u8 __load_start_iwram1[];
extern u8 __load_stop_iwram1[];
extern u8 __load_start_iwram0[];
extern u8 __load_stop_iwram0[];


namespace parts
{
	void sphere(fixed16 start, fixed16 end)
	{
		endBeat=end;
		part::set_part(init, vblank);
		memcpy(__iwram_overlay_start, __load_start_iwram1, ((int)__load_stop_iwram1) - ((int)__load_start_iwram1));
		
		block = false;
		while (timer::getBeat() < end)
		{
			fixed16 time = timer::getBeat();

			Vector3 rot(
				-cos(time >> 3) >> 3,
				fixed16(3.1415 * 2), // -cos(time >> 2) >> 3,
				-cos(time >> 3)
			);
			rot.normalize();
			
			Vector3 trans(
				sin(time >> 2) >> 1,
				cos(time >> 2) >> 1,
				fixed16(1) - cos(time >> 1) * fixed16(0.1)
			);
			
			Matrix4x4 mrot;
			mrot.rotate(rot);
			Matrix4x4 mtrans;
			mtrans.translate(trans);
			
			Matrix4x4 m = mrot * mtrans;
			
			grideffects::sphere(g, m, (3 << 15));
			draw_grid(fb::bb, g, flextextiles_tex);
			
			part::swap();
		}
		
		block = true;
		
		int svart=0;
		CpuFastSet(&svart, BG_COLORS, FILL|(512/4));
		CpuFastSet(&svart, fb::bb, FILL|(240*160/4));
		CpuFastSet(&svart, fb::fb, FILL|(240*160/4));

		memcpy(__iwram_overlay_start, __load_start_iwram0, ((int)__load_stop_iwram0) - ((int)__load_start_iwram0));
	}
};
