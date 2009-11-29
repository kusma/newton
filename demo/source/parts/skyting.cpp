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
extern const u16 brushlb_mask[];

// kuber stuff
static int svbumppal3_subpal = 256-16;
extern const u16 skyboxting_pal[];
extern const u8  blob100nm_nmap[];
extern const u8  skydel1_tex[];
extern const u8  skydel2_tex[];


namespace scene {
#include "scene_skyboxting.h"
};


static int blackfade=0;
static int blackfade_speed=0;


static u16 part_glowpal[256];



namespace intro {

	static void stripe(int y, int param) {
		if (param==0) {
			REG_BG2PA = 0x108;
		} else {
			REG_BG2PA = -0x108;
		}
		REG_BG2PB = 0x000;
		REG_BG2PC = 0x000;
		REG_BG2PD = 0x200;

		if (param==0) {
			REG_BG2X = -0x00400;
			//REG_BG2Y = -0x00A00;			
		} else {
			REG_BG2X =  0x0F400;
			//REG_BG2Y = -0x00600;
		}
		REG_BG2Y = 0;

		if (param==0) {
			blackfade += blackfade_speed;
			if (blackfade>255) blackfade=255;
			if (blackfade<0) blackfade=0;
			if (blackfade) {
				palette::boost(BG_COLORS, skyboxting_pal, 256, -blackfade);
			}
		}

	}

};


//int frame;
static void vblank() {
	blackfade += blackfade_speed;
	if (blackfade>255) blackfade=255;
	if (blackfade<0) blackfade=0;
	if (blackfade) {
		palette::boost(BG_COLORS, skyboxting_pal, 256, -blackfade);
		palette::boost(OBJ_COLORS, part_glowpal, 256, -blackfade);
	}
}

static void event(int event, int data) {
	if (data==0) blackfade_speed=4;
}


namespace parts {
	static int startpattern=0;
	void skyboxting(bool intro, int startframe, int len) {
		startpattern=events::pattern;
		
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);


		{
			int svart=0;
			CpuFastSet(&svart, fb::bb, FILL | (240*160/4));
			CpuFastSet(&svart, fb::fb, FILL | (240*160/4));
		}

		if (intro) {
			// ## INTRO
			// #####################
			events::setHandlers(NULL, event);
			blackfade=255;
			blackfade_speed=-2;
//			trede.setFrame(512);
			trede.setFrame(startframe);
			vcount::clear();
			vcount::insert(190, intro::stripe, 0);
			vcount::insert(79, intro::stripe, -1);
			trede.letterbox=20;
			vcount::start();
			while (events::pattern < startpattern+1) {
				if ((events::pattern_beat())>128+40) blackfade_speed=2;
				trede.next();
				trede.draw(fb::bb);
				//rle::shiftMask(fb::bb, fb::bb+HEIGHT*WIDTH, brushlb_mask);
				events::frame();
				fb::swap();
			}
			vcount::stop();
			fb::waitline(160);
			REG_BG2PA = 0x100;
			REG_BG2PB = 0x000;
			REG_BG2PC = 0x000;
			REG_BG2PD = 0x100;
			REG_BG2X = 0x00000;
			REG_BG2Y = 0x00000;
		} else {

			palette::boost(part_glowpal, glow_pal, 256, -25);
			palette::bw(part_glowpal, part_glowpal, 256);

			// ## MAIN
			// #################

			trede.letterbox=0;
			int svart=0;
			CpuSet(&svart, glow::glowbuff, DMA16|FILL|((31*21)/2));
			glow::init(); 
			glow::update_pal(skyboxting_pal, 64); // goood 1
		//	glow::update_pal(skyboxting_pal, 96);
		//	palette::boost(OBJ_COLORS, glow_pal, 256, -80); // GOOOD 1
			//palette::colorize(OBJ_COLORS, OBJ_COLORS, 256, RGB5(31, 0, 0), 7);
			CpuFastSet(part_glowpal, OBJ_COLORS, 512/4);
			CpuFastSet(skyboxting_pal, BG_COLORS, 512/4);


			events::setHandlers(vblank, event);

			blackfade=255;
			blackfade_speed = -2;
			glow::setup_vcount();
			vcount::start();
			int frame=startframe;
			while (events::pattern < startpattern+len) {
				bool fadeout = (events::pattern_beat()) > 128+40;
				static int oldpattern=-1;
				int newpattern = events::pattern;
				if (oldpattern!=newpattern) {
					if (oldpattern!=-1) frame = (frame & 0xFFFFFF00) + 256;
					oldpattern=newpattern;
				}
//				int frame = (events::pattern_beat()) + (newpattern*256) + startframe;
//				frame >>= 2;
				//if ((frame&255)>256-32) blackfade_speed=2;
				//else blackfade_speed=-2;
				trede.setFrame(frame+50);
				frame++;

				trede.draw(fb::bb);
				glow::glowblur(glow::glowbuff, fb::bb, 0x4000, 0x7000, 0xD000, 0x10000); // GOOOD 1
		//		glow::glowblur(glowbuff, fb::bb, 0x4000, 0x7000, 0xF000, 0x10000);
				glow::update_glow(glow::glowbuff);


				events::frame();
				fb::swap();

				if (fadeout) blackfade_speed=2;
				else blackfade_speed=-2;
			}	
			vcount::stop();
		}
		for (unsigned i = 0; i < 128; i++) OAM[i].attr0 = OBJ_DISABLE;
	}
};
