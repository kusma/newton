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
extern const u16 glow_pal[];

// kuber stuff
static int svbumppal3_subpal = 256-16;
extern const u16 skyboxting_pal[];
extern const u8  blob100nm_nmap[];
extern const u8  skydel1_tex[];
extern const u8  skydel2_tex[];


namespace scene {
#include "scene_skyboxting.h"
};

int zoomin=512;
static void stripe(int y, int param) {
	int omgx = (zoomin>>3);
	int omgy = (zoomin>>1);

	REG_BG2PA = 0x100 + omgx;
	REG_BG2PB = 0x000;
	REG_BG2PC = 0x000;
	REG_BG2PD = 0x200 + omgy;

	if (y>10 && y<150) {
		omgy>>=1;
		omgx = -omgx;
	}
	REG_BG2X = 0x00000 - (omgx<<7);

	REG_BG2Y = 0x00000 - (omgy<<6);
}



static void vblank() {
	int synch = events::pattern_beat();
	zoomin = (255-synch)<<1;
	if (synch>128+64) {
		int fade = (synch-(128+64))*4;
		u16 addcol = RGB8(fade, fade, fade);
		palette::add(BG_COLORS, skyboxting_pal, 256, addcol);
	}
}





static void event(int event, int data) {
}


namespace parts {
	static int startpattern=0;
	void skyboxting_fucked(int len) {
		startpattern=events::pattern;
		events::setHandlers(vblank, event);


		// sett opp noe vcount-greier
		vcount::clear();
		vcount::insert(190, stripe, 1);
		vcount::insert(79, stripe, -1);


		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);


		trede.setFrame(256);
		CpuFastSet(skyboxting_pal, BG_COLORS, 512/4);

		vcount::start();
		while (events::pattern < startpattern+len) {
			trede.next();
			trede.draw(fb::bb);
			events::frame();
			fb::swap();
		}
		vcount::stop();


		REG_BG2PA = 0x100;
		REG_BG2PB = 0x000;
		REG_BG2PC = 0x000;
		REG_BG2PD = 0x100;

		REG_BG2X = 0x00000;
		REG_BG2Y = 0x00000;
	}
};
