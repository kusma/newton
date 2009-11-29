#include "exstructs.h"
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
#include "palette.h"
#include "3dthen.h"
#include "glow.h"
#include "exposure.h"
#include <pimpmobile.h>

extern const u8  clown_oam[];
extern const u8  clown_tileset_raw[];
extern const u16 clown_tileset_pal[];

extern const u8 env1_tex[];
extern const u16 flower_xpal[];
extern const u8 flower_cityenv_tex[];
extern const u8 normalmap_ebump[];

namespace scene {
//#include "scene_blobb.h"
#include "scene_flower.h"
};

static int blanks = 0;
void global_vblank();
static void vblank()
{
	pimp_vblank();
	pimp_frame();

	s32 alpha = ((fixed_fsin(blanks << 2) >> (16 - 3)) + 8) >> 1;
	REG_BLDALPHA = (alpha << 0) | ((16 - alpha) << 8);


	int flash_fade = 0x0;
	exposure::update(0x000 - flash_fade >> 1, 0x8000 - flash_fade, 0, 0, flower_xpal);


	global_vblank();
	blanks++;
}

static void callback(int type, int data)
{
}

#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)

namespace parts
{

	void sphere2k(int len, int starttime)
	{
		pimp_set_callback(callback);

		DisableInterrupt(IE_HBL);		
		REG_DMA0CNT = 0;

		int swaps = 0;
		blanks = 0;

		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);


		int time=starttime;
//		OVERLAY(clown);

		int start_order = pimp_get_order();
		while ((pimp_get_order() - start_order < len) && (pimp_get_order() >= start_order))
		{
			trede.setFrame(time, 0);
			trede.draw(fb::bb);
			time++;
			
			if (swaps==0) {
				VBlankIntrWait();
				for (int i = 0; i < 128; ++i) OAM[i].attr0 = OBJ_DISABLE;
				REG_BLDCNT = 0;
				REG_BLDCNT = BIT(10) | (1 << 6) | BIT(4) | BIT(13);
				SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP | (1 << 5));
				SetInterrupt(IE_VBL, vblank);
				// fix palette
				exposure::update(0x0000, 0x8000, 0, 0, flower_xpal);
			}
			fb::swap();
			swaps++;
		}
		pimp_set_callback(NULL);
	}

};
