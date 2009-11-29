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

extern const u8  skybox_tex[];
extern const u8  skybox2_tex[];
extern const u8  normal_nmap[];
extern const u16 pal_xpal[];

extern const u8  five_oam[];
extern const u8  five_tileset_raw[];
extern const u16 five_tileset_pal[];

namespace scene {
#include "scene_greeble_skybox.h"
};

static bool regen = true;
static int blanks = 0;

static s32 targets[GBA_HEIGHT] EWRAM_DATA;
static s32 currents[GBA_HEIGHT] EWRAM_DATA;
static int deltas[GBA_HEIGHT] EWRAM_DATA;

static void vblank()
{
	pimp_vblank();
	
	s32 *dst  = currents;
	s32 *targ = targets;
	s32 offs = fixed_sin(blanks - 100) >>5;
	s32 rands = s32(cheap_rand()) >> 18;
	for (int i = GBA_HEIGHT; i; --i)
	{
		rands += (s32(cheap_rand()) >> 22);
		*dst++ = *targ + ((rands * *targ) >> 14); // *targ;
		targ++;
	}
	
//	REG_BG2X = blanks * 256;
	REG_DMA0CNT = 0;
	DMA0COPY(&currents[0], &REG_BG2X, DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA32 | 1);

	if (regen)
	{
		s32 *dst = targets;
		s32 offs = s32(cheap_rand()) >> 22;
		rands = 0;
		for (int i = GBA_HEIGHT; i; --i)
		{
			offs = fixed_fsin(i + blanks) >> 4;
			offs += fixed_fsin((i - blanks) * 5) >> 5;
			rands += ((s32(cheap_rand()) >> 22) * offs >> 11) ;
			*dst++ = (offs + rands); // *targ;
		}
		regen = false;
	}

//	REG_BG2X = pimp_get_row() * 256;
//	DMA_HBLANK

	blanks++;
	pimp_frame();
}

static void callback(int type, int data)
{
}

namespace parts
{
	void greeble_skybox(int len)
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

//		fb::overlay(five_oam, five_tileset_raw, five_tileset_pal);
		SetMode(MODE_4 | BG2_ON | OBJ_1D_MAP);
		
		int flash = 128;
		while (pimp_get_order() < 300)
		{
			static fixed16 time = 0;
			static int last_order = 0;
			
			int order = pimp_get_order();
			if (order != last_order)
			{
				regen = true;
				last_order = order;
			}
			

			trede.setTime(time);
			trede.draw(fb::bb);

			time  += 1.0;
			exposure::update(0x10000 - (flash << 9), 0xC000, 0x18000, pal_xpal);
			BG_COLORS[0] = 0;
			fb::swap();
			swaps++;
			if (flash > 0) flash -= 8;
		}
		pimp_set_callback(NULL);
	}

};
