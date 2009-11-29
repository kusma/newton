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




extern const u16 glow_pal[];
extern const u16 glowpart_pal[];

extern const u8  glowglow_tex[];
extern const u8  glownoglow_tex[];
extern const u8  glowblob_tex[];
extern const u8  glowcomplete_tex[];

namespace scene_room {
#include "scene_glow_room.h"
};

namespace scene_obj {
#include "scene_glow_obj.h"
};


static Material room_mtls[] = {
	{ FILLER_TEX256, UVGEN_UVS, -1, { glowglow_tex, NULL } }, 
};


static int fuzz=0;
static void hblank()
{
	if (!fuzz) return;
	int fuzzzzzz=min(256,fuzz);
	static int seed = 0x12345678;
	if ((seed >> 20) & 1)
	{
		seed *= 83745;
		int f = (fuzzzzzz * fuzzzzzz) >> 6;
		f = (f * f) >> 8;
		/*		REG_BG2X = (((seed >> 23) * f) >> 8) - (f >> 2); */
		REG_BG2X = ((((seed>>16) & 0xFF) * f) >> 8) - (f >> 1);
	}
	seed += 234523;
	seed *= 83745;
	seed ^= 0x34263462;
}
 

static int startpattern;
static int zoom=0;
static int zoom_fart=0;
static void vblank() {
	int synch = events::pattern_beat();
	static bool synlig=false;
	bool blinking = (events::pattern&1) && (synch>(256-64));
	bool moddepalett=false;
	if (synch>224) {
		int tull = (synch-224)*8;
		palette::boost(BG_COLORS, glowpart_pal, 256, -tull);
		if (!blinking) palette::boost(OBJ_COLORS, glow_pal, 256, -tull);
		synlig=false;
		moddepalett=true;
	}
	if (blinking) {
		int tull = (cheap_rand()>>16)&255;
		palette::boost(OBJ_COLORS, glow_pal, 256, -tull);
		synlig=false;
		moddepalett=true;
	}
	if (!synlig && !moddepalett) {
		synlig=true;
		int svart=0;
		CpuFastSet(&svart, fb::fb, FILL|GBA_WIDTH*GBA_HEIGHT/4);
		CpuFastSet(glowpart_pal, BG_COLORS, 512/4);
		if (!blinking) CpuFastSet(glow_pal, OBJ_COLORS, 512/4);
	}
	if (fuzz>0) {
		fuzz -= 32;
		if (fuzz<0) fuzz=0;
	}
	zoom += zoom_fart;
	if (zoom) {
		if (zoom>255) zoom=255;
		palette::boost(OBJ_COLORS, glow_pal, 256, -zoom);
		palette::boost(BG_COLORS, glowpart_pal, 256, -zoom);
	}
}

static void event(int event, int data) {
	if (data==0) fuzz = 1024;
	if (data==1) zoom_fart=3;
}

namespace parts {
		
	void glow(int len, int startglow) {
		startpattern=events::pattern;
		events::setHandlers(vblank, event);

		SetInterrupt(IE_HBL, hblank);
		EnableInterrupt(IE_HBL);

		CpuFastSet(glowpart_pal, BG_COLORS, 512/4);
		glow::update_pal(glowpart_pal, 128);
		Trede room(scene_room::verts, scene_room::vertcount, scene_room::faces, scene_room::facecount, scene_room::uvs, room_mtls, scene_room::framestates, scene_room::framecount);
		Trede obj(scene_obj::verts, scene_obj::vertcount, scene_obj::faces, scene_obj::facecount, scene_obj::uvs, scene_obj::materials, scene_obj::framestates, scene_obj::framecount);

		room.letterbox=20;

		glow::init();
		glow::setup_vcount();
		vcount::start();

		room.setFrame(128*3);
		obj.setFrame(128*3);
		while (events::pattern<startpattern+len) {
			int frame = events::pattern_beat()+(events::pattern-startpattern)*256 + 128*3;
			int synch = events::pattern_beat();
			room.setFrame(frame);
			obj.setFrame(frame);

			int hvit=0xEAEAEAEA;
			CpuFastSet(&hvit,  fb::bb+room.letterbox*240, FILL | ((240/4)*(160-2*room.letterbox)));
	

	#if 0
			rle::shiftMask(fb::bb, fb::bb+HEIGHT*WIDTH, brushlb_mask);
	#else
			
			int svart=0xFFFFFFFF;
			CpuFastSet(&svart, fb::bb, FILL | (room.letterbox*240/4));			
			CpuFastSet(&svart, fb::bb+HEIGHT*WIDTH - room.letterbox*240, FILL | (room.letterbox*240/4));
	#endif

			bool draw_obj=false;
			if (events::pattern-startpattern>=startglow || synch>128) {
				if ((events::pattern==startpattern) || (events::pattern&1) || ((synch&(1<<6))&&(synch&(1<<7)))) draw_obj=true;
				
			}

			if (draw_obj) {
				room_mtls[0].tex[0]=glowglow_tex;
			} else {
				room_mtls[0].tex[0]=glownoglow_tex;
			}
			room.draw(fb::bb);

			if (draw_obj) {
				obj.draw(fb::bb);
			}

			glow::glowblur(glow::glowbuff, fb::bb, 0x2000, 0x7000, 0xD000, 0x12000);
			glow::update_glow(glow::glowbuff);


			events::frame();
			fb::swap();
		}
		for (unsigned i = 0; i < 128; i++) OAM[i].attr0 = OBJ_DISABLE;
		vcount::stop();

		DisableInterrupt(IE_HBL);
		REG_BG2X=0;
	}
};
