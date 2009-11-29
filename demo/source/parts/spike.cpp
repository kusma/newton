#include "events.h"

#include "rle.h"
#include "3dthen.h"
#include "fb.h"
#include "palette.h"
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_video.h>
#include <gba_sprites.h>

static int svbumppal1234_subpal=256-16;
extern const u16 spike_pal[];
extern const u8  spikesbv_tex[];
extern const u8  spikesbh_tex[];
extern const u8  spike_nmap[];
extern const u16 spike_mask[];
namespace scene_obj {
#include "scene_spike_obj.h"
};
namespace scene_room {
#include "scene_spike_room.h"
};






extern u16 spikeover_pal[];
extern u8 spikeover_oam[];
extern u8 spikeover_spr[];

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
	int hei = (fixed_sin(tulling* 512)+0x10000) >> 10;
//	palette::boost(BG_COLORS, spike_pal, 256-32, hei);
//	palette::color_boost(fb::temppal, junk_pal, 256, boost);
//	palette::boost(OBJ_COLORS, junk_pal, 256, hei);
}

int fuzz=0;
static void hblank()
{
	static int seed = 0x12345678;
	if ((seed >> 20) & 1)
	{
		seed *= 83745;
		int f = (fuzz * fuzz) >> 6;
		f = (f * f) >> 8;
		/*		REG_BG2X = (((seed >> 23) * f) >> 8) - (f >> 2); */
		REG_BG2X = ((((seed>>16) & 0xFF) * f) >> 8) - (f >> 1);
	}
	seed += 234523;
	seed *= 83745;
	seed ^= 0x34263462;
}


static void event(int event, int data) {
	if (data==0) fuzz ^= 16;
}



namespace parts {
	static int startpattern;
	void spike(int len) {
		startpattern=events::pattern;
		events::setHandlers(vblank, event);

//		fb::overlay(spikeover_oam, spikeover_spr, spikeover_pal);

		palette::boost(BG_COLORS, spike_pal, 256, 0);
		Trede room(scene_room::verts, scene_room::vertcount, scene_room::faces, scene_room::facecount, scene_room::uvs, scene_room::materials, scene_room::framestates, scene_room::framecount);
		Trede obj(scene_obj::verts, scene_obj::vertcount, scene_obj::faces, scene_obj::facecount, scene_obj::uvs, scene_obj::materials, scene_obj::framestates, scene_obj::framecount);
		room.letterbox=23;

//		palette::boost(BG_COLORS, spike_pal, 256-32, 200);

//		SetInterrupt(Int_Hblank, hblank);
//		EnableInterrupt(Int_Hblank);


		//trede.setFrame(256*3);
		while (events::pattern < startpattern+len+1) {
			int frame = (events::pattern_beat()) + (events::pattern-startpattern)*256;
			if (events::pattern==startpattern)  frame = frame - 128 - 64;
			room.setFrame(frame);
			obj.setFrame(frame);
			room.draw(fb::bb);
			rle::shiftMask(fb::bb, fb::bb+GBA_HEIGHT*GBA_WIDTH, spike_mask);
			obj.draw(fb::bb);

			events::frame();
			fb::swap();
		}

//		DisableInterrupt(Int_Hblank);
	}
};
