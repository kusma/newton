#include "exstructs.h"

#include <gba_interrupt.h>
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
#include "tunnel.h"

extern "C" {
	extern u16 tunnel_pal[];
	extern u8  tunnel_raw[];


	extern u16 tunnel_ovalai_pal[];
	extern u8 tunnel_ovalai_spr[];
	extern u8 tunnel_ovalai_oam[];

	extern u16 tunnel_ovalai_text_pal[];
	extern u8 tunnel_ovalai_text_spr[];
	extern u8 tunnel_ovalai_text_oam[];
};



static volatile int superspeed=3;
static void event(int event, int data) {
	if (data==0) {
		superspeed ^= 16;
	}
}


static bool parten=true;
static int tulling=0;
static void vblank() {
	if (!parten) {
		if ((tulling&7)==0) {
			int hei = (-fixed_cos(tulling* 256*2)+0x10000) >> 11;
			palette::boost(OBJ_COLORS, tunnel_ovalai_text_pal, 256, -hei);
		}
		tulling++;
	}
}

static int startpattern;

namespace parts {
	void tunnel(int len) {
		startpattern=events::pattern;
		setup_vcount_tunnel();

		events::setHandlers(vblank, event);

		CpuFastSet(tunnel_pal, BG_COLORS, 512/4);

	
		vcount::start();
		int offs=0;

		int movex=0;
		int movey=0;



		fb::overlay(tunnel_ovalai_oam, tunnel_ovalai_spr, tunnel_ovalai_pal);

		while (events::pattern < startpattern+1) {
			int scrollx = fixed_sin(movex<<6)>>8;
			int scrolly = movey;
			int offset = ((scrolly<<8)&0xFF00) | (scrollx&0xFF);
			draw_tunnel(fb::bb, tunnel_raw, offset);
			fb::waitline(160);
			fb::swap();
			events::frame();

			movex++;
			movey += superspeed;
		}

		parten=false;
		fb::overlay(tunnel_ovalai_text_oam, tunnel_ovalai_text_spr, tunnel_ovalai_text_pal);
		while (events::pattern < startpattern+2) {
			int scrollx = fixed_sin(movex<<6)>>8;
			int scrolly = movey;
			int offset = ((scrolly<<8)&0xFF00) | (scrollx&0xFF);
			draw_tunnel(fb::bb, tunnel_raw, offset);
			fb::waitline(160);
			fb::swap();
			events::frame();

			movex++;
			movey += superspeed;
		}
		// vent til skjermen er flippa rett før vcount stoppes
		fb::waitline(10);
		vcount::stop();
	}
};
