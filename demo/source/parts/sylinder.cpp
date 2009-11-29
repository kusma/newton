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
extern const u16 brushlb_mask[];

extern u16 title_pal[];
extern u16 title_spr[];
extern u16 title_oam[];

extern u16 title2_pal[];
extern u16 title2_spr[];
extern u16 title2_oam[];

// kuber stuff
extern const u16 supersyl_pal[];
extern const u8  supersyl_bg[];
extern const u8  supersylnm01_ebump[];
extern const u8  supersylnm02_ebump[];
extern const u8  supersylnm03_ebump[];
extern const u8  supersylnm04_ebump[];
extern const u8  supersyl_tex[];
const u8* env02_tex = supersyl_tex;
namespace scene {
#include "scene_supersyl.h"
};

const u16* ovrpal=title_pal;

static int boost=0;
static int boost2=0;
static void vblank() {
	boost += ((int(cheap_rand()>>16) & 0xFF) - 128)>>2;
	if (boost>96) boost=96;
	if (boost<-0) boost=-0;
	boost2 += ((int(cheap_rand()>>16) & 0xFF) - 128)>>2;
	if (boost2>96) boost2=96;
	if (boost2<-96) boost2=-96;
	static int tulling=0;
	tulling++;
	int hei = (fixed_sin(tulling* 512)) >> 10;
	palette::color_boost(fb::temppal, ovrpal, 256, boost);
	palette::boost(OBJ_COLORS, fb::temppal, 256, (-(boost2)>>8) + hei);
}

static void event(int event, int data) {
}

static int startpattern;

namespace parts {
	void sylinder(int len) {
		startpattern=events::pattern;
		events::setHandlers(vblank, event);
		fb::overlay(title_oam, title_spr, title_pal);
		ovrpal=title_pal;

		CpuFastSet(supersyl_pal, BG_COLORS, 512/4);
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		//trede.letterbox=20;

	
		//glow::init();
		//glow::setup_vcount();
		//vcount::start();
//		bool annetoverlay=false;
		int bytt=0;
		while (events::pattern < startpattern+len-1 || events::pattern_beat()<128+64) {
			int frame = (events::pattern_beat()) + (events::pattern-startpattern)*256 - 512 - 128;
			//frame = MUL16(frame, (128<<16)/100);
			trede.setFrame(frame);

			if (bytt!=2 && events::pattern==startpattern+2) {
				fb::overlay(title2_oam, title2_spr, title2_pal);
				ovrpal=title2_pal;
				bytt=2;
			} else if (bytt!=3 && events::pattern==startpattern+3) {
				fb::overlay(title_oam, title_spr, title_pal);
				ovrpal=title_pal;
				bytt==3;
			}

			//trede.next();
			CpuFastSet(supersyl_bg, fb::bb, 240*160/4);
			trede.draw(fb::bb);


			events::frame();
			fb::swap();
		}


	}
};
