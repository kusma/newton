#include "exstructs.h"

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
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "3dthen.h"
#include "decomp.h"
#include "flikk.h"
#include "flikkfx.h"
#include "glow.h"
#include "exposure.h"
#include <pimpmobile.h>

extern const u8 pipescompletemap_tex[];
extern const u8 skjermcompletemap_tex[];
extern const u8 veggcompletemap_tex[];
extern const u8 ballnormalsmap_nmap[];
extern const u16 proj_xpal[];

extern const u16 overproj_flikk[];
extern const u16 overproj2_flikk[];

//extern const u16 overproj_spike_flikk[];

/*
namespace scene {
#include "scene_projector.h"
};
namespace scene_spike {
const int svbumppal1234_subpal=256-16;
#include "scene_projector_spike.h"
};
*/

namespace scene {
const int svbumppal1234b_subpal=256-16;
#include "scene_projector.h"
};

namespace scene2 {
const int svbumppal1234b_subpal=256-16;
#include "scene_projector2.h"
};

/*
void proj_switch(Trede& trede, bool on) {
	if (on) {
		scene::materials[0].tex[0] = pipescompletemap_tex;
		skjermcompletemap_tex = skjermcompletemap_tex;
		veggcompletemap_tex = veggcompletemap_tex;
	} else {
		pipescompletemap_tex = pipescompletemap_off_tex;
		skjermcompletemap_tex = skjermcompletemap_off_tex;
		veggcompletemap_tex = veggcompletemap_off_tex;
	}
}
*/

static int blanks = 0;

static volatile int flash = 0;



static void vblank() {
	pimp_vblank();
	blanks++;
	//glow::vblank();
	pimp_frame();
	if (flash > 0) flash -= 8;
}

static void callback(int type, int data)
{
	flash = 256;
}


namespace parts
{
	void projector(int len)
	{
		int startorder = pimp_get_order();
		fb::reset();

		pimp_set_callback(callback);
		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);


		static u16 temp[256] EWRAM_DATA;
		exposure::update(0x10000, 0xC000, 0x18000, proj_xpal, temp);

		exposure::update(0x10000, 0xC000, 0x18000, proj_xpal);

//		Flikk flikk1(overproj_flikk);
//		Trede trede1(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);

//		Flikk flikk2(overproj_spike_flikk);
//		Trede trede2(scene_spike::verts, scene_spike::vertcount, scene_spike::faces, scene_spike::facecount, scene_spike::uvs, scene_spike::materials, scene_spike::framestates, scene_spike::framecount);


		int swaps = 0;
		blanks = 0;

		SetMode(MODE_4 | BG2_ON);

		if (len == 0)
		{
			Flikk flikk(overproj_flikk);
			Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
			fixed16 time = 0;
			while (pimp_get_order() < startorder + 2)
			{
				trede.setTime(time);
				flikk.setFrame(time);
				trede.draw(fb::bb);
				flikk.drawShift(fb::bb);
				time  += 1;
				int f = flash >> 4;
				palette::add(BG_COLORS, temp, 256, RGB5(f,f,f));
				fb::swap();
				swaps++;
			}
		}

		if (len == 1)
		{
			Flikk flikk2(overproj2_flikk);
			Trede trede2(scene2::verts, scene2::vertcount, scene2::faces, scene2::facecount, scene2::uvs, scene2::materials, scene2::framestates, scene2::framecount);
			fixed16 time = 0;
			while (pimp_get_order() < startorder + 2)
			{
				trede2.setTime(time);
				flikk2.setFrame(time);
				trede2.draw(fb::bb);
				flikk2.drawShift(fb::bb);
				time  += 1;
				int f = flash >> 4;
				palette::add(BG_COLORS, temp, 256, RGB5(f,f,f));
				fb::swap();
				swaps++;
			}
			
		}

	/*	// frampek
		if (len == 0)
		{
			fixed16 time = 0;
			while (pimp_get_order() < startorder + 2)
			{
				trede1.setTime(time);
				flikk1.setFrame(time);
				trede1.draw(fb::bb);
				flikk1.drawShift(fb::bb);
				time  += 1.0;
				fb::swap();
				swaps++;
			}
		}
		
		// spikeball
		if (len == 1)
		{
			fixed16 time = 0;			
			while (pimp_get_order() < startorder + 2)
			{
				trede2.setTime(time);
				flikk2.setFrame(time);
				trede2.draw(fb::bb);
				flikk2.drawShift(fb::bb);
				time  += 1.0;
				fb::swap();
				swaps++;
			}
		}*/
		DisableInterrupt(IE_VCNT);
	}

};
