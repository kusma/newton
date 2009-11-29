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

/* cube2 textures */
extern const u8 tex1_tex[];
extern const u16 clownf_xpal[];
extern const u16 clownf_bg_flikk[];
extern const u8  hands_oam[];
extern const u8  hands_tileset_raw[];
extern const u16 hands_tileset_pal[];
#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)


namespace scene {
#include "scene_cube2.h"
};

static int blanks = 0;
void global_vblank();
static void vblank()
{
	pimp_vblank();
	pimp_frame();


	for (int i = 0; i < 10; ++i)
	{
//		OAM[i].attr1 = 0;
	}

	global_vblank();
	blanks++;
}

static void callback(int type, int data)
{
}

#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)

namespace parts
{

	void cube3(int len)
	{
		pimp_set_callback(callback);

		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);
//		EnableInterrupt(IE_HBL);


		int swaps = 0;
		blanks = 0;

		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);

		VBlankIntrWait();
		for (int i = 0; i < 128; ++i)
		{
			OAM[i].attr0 = OBJ_DISABLE;
		}

		REG_BLDCNT = 0;

		SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP | (1 << 5));
		exposure::update(0x000, 0x10000, 0, 0, clownf_xpal);
		BG_COLORS[0] = 0;

		Flikk flikk(clownf_bg_flikk);
		OVERLAY(hands);

		int start_order = pimp_get_order();
		while ((pimp_get_order() - start_order < len) && (pimp_get_order() >= start_order))
		{
			u32 black = 0;
//			CpuFastSet(&black, fb::bb, FILL|COPY32|((120 * 240)/4));

//			trede.setFrame(blanks >> 2, blanks );
			trede.setTime(fixed16(blanks) >> 4);
			trede.draw(fb::bb);

			flikk.drawMov(fb::bb, 0);
 
			fb::swap();
			swaps++;
		}
		pimp_set_callback(NULL);
	}
};
