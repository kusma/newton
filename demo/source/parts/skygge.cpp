#include "exstructs.h"


#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "glow.h"
#include "3dthen.h"
#include "rle.h"
#include "events.h"

// annet stuff
//extern const u16 brushlb_mask[];

// kuber stuff
const int svbumppal2_subpal = 256-16;

extern const u8  shadnmdm_nmap[];
extern const u8  shadnmdm2_nmap[];

extern const u16  cam1_pal[];
extern const u16  cam2_pal[];
extern const u16  cam3_pal[];

extern const u8  cam1_raw[];
extern const u8  cam2_raw[];
extern const u8  cam3_raw[];



namespace scene {
#include "scene_skygge.h"
};
extern const u16 shitface_pal[];
extern const u8 shitface_spr[];
extern const u8 shitface_oam[];



static const u16* part_pal=NULL;

static void vblank() {
	int beat = events::pattern_beat();
	if (beat<64+5) {
		int hei = 64-(beat-5);
		hei <<= 2;
		if (hei<0) hei=0;
		if (hei>255) hei=255;
		palette::boost(BG_COLORS, part_pal, 256, -hei);
	} else if (beat>255-64) {
		int hei = 64+(beat-255);
		hei <<= 2;
		if (hei<0) hei=0;
		if (hei>255) hei=255;
		palette::boost(BG_COLORS, part_pal, 256, -hei);
	} else if (beat<64+10) {
		CpuFastSet(part_pal, BG_COLORS, 512/4);
	}

	static int tulling=0;
	int hei = (fixed_cos(tulling*256/2)+0x10000) >> 10;
	palette::boost(OBJ_COLORS, shitface_pal, 256, -hei-64);
	tulling++;
}

static void event(int event, int data) {
}


static int startpattern;
namespace parts {
	void skygge() {
		startpattern=events::pattern;
		part_pal = cam1_pal;
		events::setHandlers(vblank, event);

		fb::overlay(shitface_oam, shitface_spr, shitface_pal);
	//	CpuFastSet(shad_pal, BG_COLORS, 512/4);

//		palette::boost(BG_COLORS, shad_pal, 256, 64);
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);


		part_pal = cam1_pal;
		while (events::pattern < startpattern+1) {
			int frame = (events::pattern_beat()/2);

			trede.setFrame(frame);
			CpuFastSet(cam1_raw, fb::bb, 240*160/4);
			trede.draw(fb::bb);
			trede.shadow(fb::bb);

			events::frame();
			fb::swap();
		}

		part_pal = cam2_pal;
		while (events::pattern < startpattern+2) {
			int frame = (events::pattern_beat()/2) + 128;

			trede.setFrame(frame);
			CpuFastSet(cam2_raw, fb::bb, 240*160/4);
			trede.draw(fb::bb);
			trede.shadow(fb::bb);

			events::frame();
			fb::swap();
		}

		part_pal = cam3_pal;
		while (events::pattern < startpattern+3 && (events::pattern_beat()<128)) {
			int frame = (events::pattern_beat()/2) + 256;

			trede.setFrame(frame);
			CpuFastSet(cam3_raw, fb::bb, 240*160/4);
			trede.draw(fb::bb);
			trede.shadow(fb::bb);

			events::frame();
			fb::swap();
		}

		fb::waitline(160);
		part_pal = cam1_pal;
		CpuFastSet(part_pal, BG_COLORS, 512/4);
		while (events::pattern < startpattern+3) {
			int frame = (events::pattern_beat()/2);

			trede.setFrame(frame);
			CpuFastSet(cam1_raw, fb::bb, 240*160/4);
			trede.draw(fb::bb);
			trede.shadow(fb::bb);

			events::frame();
			fb::swap();
		}

	}
};
