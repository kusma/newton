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
#include "palette.h"
#include "3dthen.h"
#include "exposure.h"
#include <pimpmobile.h>

/* turntable textures */
extern const u16 turntable_xpal[];
extern const u8 t1_tex[];
extern const u8 armtex_tex[];
extern const u8 plane01completemap_tex[];
extern const u8 normalmap_ebump[];
extern const u8 cityenv_tex[];
extern const u8 clownmat2_tex[];

/* turntable textures */
extern const u16 rubber_xpal[];
extern const u8 env1_tex[];

namespace scene {
#include "scene_turntable.h"
};

namespace scene2 {
#include "scene_turntable2.h"
};

extern const u8  turnover_oam[];
extern const u8  turnover_tileset_raw[];
extern const u16 turnover_tileset_pal[];


static bool flash = true;
static s32  next_flash_fade = 0x10000;
static s32  flash_fade = 0;

static int blanks = 0;
void global_vblank();

extern const u16 clown5_pal[];
extern const u8  clown5_raw[];

static volatile bool blinkers = false;

static void vblank()
{
	pimp_vblank();

	flash_fade = (flash_fade * 250) >> 8;
	exposure::update(0x000 - flash_fade >> 1, 0x10000 - flash_fade, 0, 0, turntable_xpal);

	global_vblank();
	blanks++;
	pimp_frame();

	if (blinkers)
	{
		blinkers = false;
		fb::hyperblink(clown5_raw, clown5_pal);
		blanks += 6;
	}
}

static bool locked = true;

static int cam = 0;
static bool scratch = false;
static int time_offset = 0;

static void callback(int type, int data)
{
	switch (data)
	{
		case 0:
//			flash = true;
//			next_flash_fade = max(0x10000, flash_fade);
			blinkers = true;
		break;

		case 1:
			if (locked) return;
			flash = true;
			next_flash_fade = max(0xD000, flash_fade);
			cam++;
			blanks = 60 * 1;
		break;

		case 2:
			if (locked) return;
			flash = true;
			next_flash_fade = max(0xC000, flash_fade);
			scratch = !scratch;
			cam++;
			blanks = 60 * 1;
		break;

		case 3:
			flash = true;
			next_flash_fade = max(0xD000, flash_fade);
		break;
	}
}

#define OVERLAY(name) fb::overlay(name##_oam, name##_tileset_raw, name##_tileset_pal)

namespace parts
{

	void cube2(int len)
	{
		DisableInterrupt(IE_HBL);
		DisableInterrupt(IE_VCNT);
		REG_DMA0CNT = 0;
		pimp_set_callback(callback);

		int swaps = 0;
		blanks = 0;

		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		Trede trede2(scene2::verts, scene2::vertcount, scene2::faces, scene2::facecount, scene2::uvs, scene2::materials, scene2::framestates, scene2::framecount);

		SetMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP | (1 << 5));
		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		OVERLAY(turnover);
		REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
		REG_BLDALPHA = (16) | (16<<8);

		int start_order = pimp_get_order();
		while ((pimp_get_order() - start_order < len) && (pimp_get_order() >= start_order))
		{
			int order = pimp_get_order();
			
			if (order != start_order) locked = false;

			u32 black = 0;
			CpuFastSet(&black, fb::bb, FILL|COPY32|((120 * 240)/4));

			if (!scratch)
			{
				trede.setFrame(((cam % 3) * 128) + (blanks >> 2), 0);
				trede.draw(fb::bb);
			}
			else
			{
				trede2.setFrame(((cam % 3) * 128) + (blanks >> 2), 0);
				trede2.draw(fb::bb);
			}
 
			if (flash)
			{
				flash_fade = next_flash_fade;
				flash = false;
			}

			fb::swap();
			swaps++;
		}
		pimp_set_callback(NULL);
	}
};
