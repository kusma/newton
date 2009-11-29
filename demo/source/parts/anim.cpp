#include "exstructs.h"

#include <pimpmobile.h>
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
#include "3dthen.h"
#include "decomp.h"
#include "flikk.h"
#include "flikkfx.h"
#include "noise.h"
#include "palette.h"

// annet stuff
extern const u16 brushlb_mask[];
extern const u16 shitface_pal[];
extern const u8 shitface_spr[];
extern const u8 shitface_oam[];
 
extern u16 parallax_pal[];
extern u8  parallax_height_raw[];
extern u8  parallax_raw[];

extern u16 test_mask[];
extern u16 kvakk_flikk[];
extern u16 sfc_flikk[];
extern u16 djdoc_flikk[];
extern u16 street_flikk[];

extern u8  flikkmask_raw[];
extern u16 flikkmask_pal[];




extern const u16 shit_mask[];

static int blanks=0;

static void vblank() {
	pimp_vblank();
	blanks++;
	//noise::update();
	pimp_frame();
}

static void event(int event, int data) {
}


namespace scene {
#include "scene_dots.h"
};
namespace parts {

	void anim(int len)
	{
		//noise::setup();
		int startorder = pimp_get_order();
		
		int svart=0;
		CpuFastSet(&svart, fb::bb, FILL| (240*160)/4);
		CpuFastSet(&svart, fb::fb, FILL| (240*160)/4);
		CpuFastSet(&svart, BG_COLORS, FILL|(512/4));

		Flikk flikk(sfc_flikk);
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);
		blanks=0;
		int swaps=0;
		int fade=0;
		while (pimp_get_order() < startorder + 3)
		{
			flikkfx::cheap_blur(fb::bb, fb::fb);
	//		int hehetall = 0;
	//		CpuFastSet(&hehetall, fb::bb, FILL| (240*160)/4);
			
			int hehetall = ((fixed_fsin(swaps<<4)+0x18000)>>8) + 0x100;
			
			if (blanks < 500) {
				fade += 3;
			} else {
				fade -= 3;
			}
			
			if (fade>255) fade=255;
			if (fade<0) fade=0;
			hehetall = (hehetall*fade)>>8;
			palette::boost(BG_COLORS, flikkmask_pal, 256, hehetall-0x100);
			
			
			flikk.drawAdd(fb::bb);
			trede.drawDots(fb::bb, 0x2);

			flikkfx::overlay(fb::fb, fb::bb, -swaps, flikkmask_raw, 240);
			
			//palette::colorize(BG_COLORS, flikkmask_pal, 256, RGB5(31, 31, 31), fixed_fsin(swaps<<16));

			trede.setTime(fixed16(0.05) * blanks);
			flikk.setFrame(fixed16(0.17) * blanks);
			fb::waitline(159);
			fb::waitline(160);
//			fb::swap();
			swaps++;
		}
	}

};
