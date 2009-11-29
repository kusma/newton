#include "exstructs.h"


#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "glow.h"
#include "3dthen.h"
#include "rle.h"
#include "events.h"
#include "grid.h"
#include "grideffects.h"
#include "glow.h"

extern u16 parallax_pal[];
extern u8  parallax_height_raw[];
extern u8  parallax_raw[];

extern u8 parallax_glow[];

extern u16 offset_pal[];
extern u16 offset_oam[];
extern u16 offset_spr[];

static bool stopp=true;
static int frame=416420;
static void event(int event, int data) {
//	if (event!=KRAP_CB_MARK) return;
	if (data==0) stopp = !stopp;
	if (data==1) frame += 25;
}

grid g EWRAM_DATA;
static int startpattern;

static int fade=0;
static int fade_speed=0;
static void vblank() {
	if (fade) palette::boost(BG_COLORS, parallax_pal, 256, max(-fade, -256));
	if (fade) palette::boost(OBJ_COLORS, offset_pal, 256, max(-fade, -256));
	fade += fade_speed;
}

namespace parts {
	void parallax(int len) {
		startpattern=events::pattern;

		glow::init();
		fb::overlay(offset_oam, offset_spr, offset_pal);

		events::setHandlers(vblank, event);

		CpuFastSet(parallax_pal, BG_COLORS, 256*2/4);


		int scroller=0;
		while (events::pattern < startpattern+len) {
			if (events::pattern==startpattern+len-1 && events::pattern_beat()>128+64) fade_speed=4;
			grideffects::plane(g, fixed16::make_raw(frame << 9), fixed16::make_raw(scroller << 10), (stopp)?fixed16(0):fixed16::make_raw(scroller << 10));
			draw_parallax_grid(fb::bb, g, parallax_raw, parallax_height_raw);
			events::frame();
			fb::swap();
			if (!stopp) frame++;
			scroller++;
		}
		for (unsigned i = 0; i < 128; i++) OAM[i].attr0 = OBJ_DISABLE;

	}
};
